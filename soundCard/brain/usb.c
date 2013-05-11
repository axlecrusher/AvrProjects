
#include "usb.h"
#include "SPIPrinting.h"
#include "usbconfig.h"
#include <stdio.h>

//unsigned char USB_Initialized;
volatile char USBInitState;

// zero when we are not configured, non-zero when enumerated
static volatile uint8_t usb_configuration=0;

void USB_ZeroPrescaler()
{
        CLKPR=0x80;
        CLKPR=0x00;
}

void USB_Init()
{
	USBInitState = -1;
	USBCON = (1<<USBE)|(1<<FRZCLK); //USB "Freeze" (And enable)
	REGCR = 0; //enable regulator.
	PLLCSR = (1<<PLLE)|(1<<PLLP0); //PLL Config
        while (!(PLLCSR & (1<<PLOCK)));	// wait for PLL lock
	USBCON = 1<<USBE; //Unfreeze USB
        UDCON = 0;// enable attach resistor (go on bus)
//        UDIEN = (1<<EORSTE)|(1<<SOFE);
	UDIEN = (1<<EORSTE); //we only really care about resets, not start of frames
}


// USB Device Interrupt - handle all device-level events
// the transmit buffer flushing is triggered by the start of frame
//

//int triggerep = 0;

ISR(USB_GEN_vect)
{
	//happens once every 1ms, must be fast
	uint8_t intbits;
	intbits = UDINT;
	UDINT = 0;
//	PORTD ^= _BV(PD6); //LED on
	if (intbits & (1<<EORSTI)) {
		//reset
		UENUM = 0;
		UECONX = 1;
		UECFG0X = EP_TYPE_CONTROL;
		UECFG1X = EP_SIZE(ENDPOINT0_SIZE) | EP_SINGLE_BUFFER;
		UEIENX = (1<<RXSTPE);
		usb_configuration = 0;
		USBInitState = 1;
	}

/*
	if ((intbits & (1<<SOFI)) && (USBInitState == 2) && triggerep)
	{
		//no idea what this is for, i guess testing something
		//This can push data via interrupt.
		triggerep--;
		UENUM = 3;
		if (UEINTX & (1<<RWAL)) {
			UEDATX = 0xc0;
			UEDATX = 0xd1;
			UEDATX = 0xd2;
			UEDATX = 0xd3;
			UEDATX = 0xd4;
			UEDATX = 0xd5;
			UEDATX = 0xd6;
			UEDATX = 0xd7;
			UEDATX = 0xd8;
			UEINTX = 0x3A;
		}
	}
*/
}


// USB Endpoint Interrupt - endpoint 0 is handled here.  The
// other endpoints are manipulated by the user-callable
// functions, and the start-of-frame interrupt.
//
ISR(USB_COM_vect)
{
	//this interrupt only happens once so who cares how long it takes
	uint8_t intbits;
	const uint8_t *list;
	const uint8_t *cfg;
	uint8_t i, n, len, en;
	uint8_t bmRequestType;
	uint8_t bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
	uint16_t desc_val;
	const uint8_t *desc_addr;
	uint8_t	desc_length;

	UENUM = 0;
	intbits = UEINTX;

//	SPIPutChar( '*' );	

	if (intbits & (1<<RXSTPI)) {
		bmRequestType = UEDATX;
		bRequest = UEDATX;
		wValue = UEDATX;
		wValue |= (UEDATX << 8);
		wIndex = UEDATX;
		wIndex |= (UEDATX << 8);
		wLength = UEDATX;
		wLength |= (UEDATX << 8);

		UEINTX = ~((1<<RXSTPI) | (1<<RXOUTI) | (1<<TXINI));
		if (bRequest == GET_DESCRIPTOR)
		{
			list = (const uint8_t *)descriptor_list;
			for (i=0; ; i++)
			{
				if (i >= NUM_DESC_LIST)
				{
					UECONX = (1<<STALLRQ)|(1<<EPEN);  //stall
					return;
				}
				desc_val = pgm_read_word(list);
				if (desc_val != wValue)
				{
					list += sizeof(struct descriptor_list_struct);
					continue;
				}
				list += 2;
				desc_val = pgm_read_word(list);
				if (desc_val != wIndex)
				{
					list += sizeof(struct descriptor_list_struct)-2;
					continue;
				}
				list += 2;
				desc_addr = (const uint8_t *)pgm_read_word(list);
				list += 2;
				desc_length = pgm_read_byte(list);
				break;
			}
			len = (wLength < 256) ? wLength : 255;
			if (len > desc_length) len = desc_length;
			do
			{
				// wait for host ready for IN packet
				do
				{
					i = UEINTX;
				} while (!(i & ((1<<TXINI)|(1<<RXOUTI))));
				if (i & (1<<RXOUTI)) return;	// abort
				// send IN packet
				n = len < ENDPOINT0_SIZE ? len : ENDPOINT0_SIZE;
				for (i = n; i; i--)
				{
					UEDATX = pgm_read_byte(desc_addr++);
				}
				len -= n;
				usb_send_in();
			} while (len || n == ENDPOINT0_SIZE);

			return;
		}
		else if (bRequest == SET_ADDRESS)
		{
			usb_send_in();
			usb_wait_in_ready();
			UDADDR = wValue | (1<<ADDEN);
			return;
		}
		else if (bRequest == SET_CONFIGURATION && bmRequestType == 0)
		{
			usb_configuration = wValue;
			usb_send_in();
			cfg = endpoint_config_table;
			for (i=1; i<5; i++)
			{
				UENUM = i;
				en = pgm_read_byte(cfg++);
				UECONX = en;
				if (en)
				{
					UECFG0X = pgm_read_byte(cfg++);
					UECFG1X = pgm_read_byte(cfg++);
				}
			}
			UERST = 0x1E;
			UERST = 0;
			USBInitState = 2;
			return;
		}
		else if (bRequest == GET_CONFIGURATION && bmRequestType == 0x80)
		{
			usb_wait_in_ready();
			UEDATX = usb_configuration;
			usb_send_in();
			return;
		}
		else if (bRequest == GET_STATUS)
		{
			usb_wait_in_ready();
			i = 0;
			#ifdef SUPPORT_ENDPOINT_HALT
			if (bmRequestType == 0x82)
			{
				UENUM = wIndex;
				if (UECONX & (1<<STALLRQ)) i = 1;
				UENUM = 0;
			}
			#endif
			UEDATX = i;
			UEDATX = 0;
			usb_send_in();
			return;
		}
/*
		else if( bRequest == 0xA3 ) //My request/Device->Host
		{
			// wait for host ready for IN packet
			do {
				i = UEINTX;
			} while (!(i & ((1<<TXINI)|(1<<RXOUTI))));
			if (i & (1<<RXOUTI)) return;	// abort

			n = 11;
			for (i = n; i; i--)
			{
				UEDATX = i;
			}
			usb_send_in();

			//printf( "0xA3 / data:\n" );
			SPIPutChar( 'X' );
			SPIPutChar( '\n' );
		}
		else if( bRequest == 0xA4 ) //Control-In (Us to CPU)
		{
			usb_wait_in_ready();
			UEDATX = 4;UEDATX = 5;UEDATX = 6;UEDATX = 7;
			UEDATX = 4;UEDATX = 5;UEDATX = 6;UEDATX = 7;
			usb_send_in();
			SPIPutChar( 'Y' );
			SPIPutChar( '\n' );
			return;
		}
		else if( bRequest == 0xA6 ) //Jumbo  Us to CPU Frame
		{
			SPIPutChar( 'Z' );
			SPIPutChar( '\n' );
			len = 31;
			do {
				// wait for host ready for IN packet
				do {
					i = UEINTX;
				} while (!(i & ((1<<TXINI)|(1<<RXOUTI))));
				if (i & (1<<RXOUTI)) return;	// abort
				// send IN packet
				n = len < ENDPOINT0_SIZE ? len : ENDPOINT0_SIZE;
				for (i = n; i; i--)
				{
					UEDATX = i;
				}
				len -= n;
				usb_send_in();
			} while (len || n == ENDPOINT0_SIZE);
			//			triggerep = 1;
			return;
		}
		else if( bRequest == 0xA5 ) //Control-Out (CPU To Us)  (Only works on control-type messages, else we crash.)
		{
			unsigned i;
			usb_wait_receive_out();
			SPIPutChar( 'Q' );
			for( i = 0; i < wLength; i++ )
			{
				unsigned char c = UEDATX;
				sendhex2( c ); 
			}
			SPIPutChar( '\n' );

			usb_ack_out();
			usb_send_in();

			return;
		}
*/
		else
		{
			//printf( "UNKREQ: %02X\n", bRequest );
			SPIPutChar( '?' );
			SPIPutChar( 'R' );
			sendhex2( bmRequestType );
			sendhex2( bRequest );
			sendhex4( wValue );
			sendhex4( wIndex );
			sendhex4( wLength );
			SPIPutChar( '\n' );
			sendhex2( bRequest );
		}
	}
	UECONX = (1<<STALLRQ) | (1<<EPEN);	// stall
}


/* 
 * Copyright 2011-2012 Charles Lohr
 *   -- based off of --
 * USB Keyboard Example for Teensy USB Development Board
 * http://www.pjrc.com/teensy/usb_keyboard.html
 * Copyright (c) 2009 PJRC.COM, LLC
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
