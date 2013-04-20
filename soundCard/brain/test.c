#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <string.h>
#include <avr/sfr_defs.h>
#include "usb.h"

#include "avrUsbUtils.h"

#include "SPIPrinting.h"

#define DATAGRAM_SIZE 64

#define BUFFERLEN 128
volatile uint16_t buffer[BUFFERLEN+1];
volatile uint8_t write = 1;
volatile uint8_t read = 0;
volatile uint8_t spaceLeft = 128;

//Check to see if writing is possible. If it is, return a pointer to the next
//position to write to after this current write, 0 if can't write
inline uint8_t* canWrite(uint8_t* ptr, uint8_t x)
{
	movePtr(&ptr,x);
	if (ptr<readPtr) return ptr;
	return 0x0000;
}

void SetupBuffer()
{
	memset((void*)buffer,0,BUFFERLEN);
	spaceLeft = 128;
}


inline void ReadUSBByte()
{
	if (usbIndex>=0)
	{
		if ( USB_HAS_SPACE(UEINTX) )
		{
			writePtr[usbIndex] = UEDATX;
			usbIndex++;
		}
	}
}

static void setup_clock()
{
	CLKPR = 0x80;	/*Setup CLKPCE to be receptive*/
	CLKPR = 0x00; //no divisor
}

/*
static void setup_timers()
{
	TCCR1A = 0x00;
	TCCR1B = _BV(WGM12) | _BV(CS10); //CTC, no divisor
	TIMSK1 = 0x00 | _BV(OCIE1A); //TIMER1_COMPA_vect
	OCR1A = 333; //333.33333333333 we will have to make up for this
}
*/

static inline void SendChannelData();
static inline void SendChannelDataFromUSB();
static void ReadUSB_SendChannelData();

ISR(INT4_vect)
{
//	PORTD ^= _BV(PD6);

	UENUM = 4; //USB endpoint
	if ( USB_READY(UEINTX) && (spaceLeft >= 8) )
	{
		PORTD &= ~_BV(PD6); //LED off
		ReadUSB_SendChannelData();
	}
	else if (spaceLeft >= 2)
	{
		PORTD &= ~_BV(PD6); //LED off
		SendChannelData();
	}
	else
	{
//		underflow
		PORTD |= _BV(PD6); //LED on
	}
}

static void ReadUSB_SendChannelData()
{
	
}

static inline void SendChannelData()
{
	uint8_t* b = (uint8_t*)buffer[read];

	//send left MSB
	SPDR = b[1];
	while(!(SPSR & _BV(SPIF))); //wait for complete

	//send left LSB
	SPDR = b[0];
	while(!(SPSR & _BV(SPIF))); //wait for complete

	//send right MSB
	SPDR = b[3];
	while(!(SPSR & _BV(SPIF))); //wait for complete

	//send right LSB
	SPDR = b[2];
	while(!(SPSR & _BV(SPIF))); //wait for complete

	spaceLeft+=2;
	read += 2;
}

volatile uint8_t bytesRead = 0;

void testAssembly()
{
	uint8_t x = 0;
	uint8_t i = 0;
	do
	{
		UENUM++;
		i++;
	}
	while (i<4);
}

static inline void SendChannelDataFromUSB()
{
	//always read or write in multiples of BUFFERLEN
	uint8_t* wb = (uint8_t*)writePtr;
	uint8_t* rb = (uint8_t*)readPtr;
	uint8_t i = 0;

	PORTD &= ~_BV(PD6);

	UENUM = 4; //interrupts can change this

	if ( USB_READY(UEINTX) && spaceLeft >= 8 )
	{
		UEINTX &= ~_BV(RXOUTI); //ack

		//L_MSB, L_LSB, R_MSB, R_LSB
		for (i=0;i<4;++i)
		{
			SPDR = *rb; //17 clock cycles for this to send
			rb++;
			*(wb++) = UEDATX; //4
			*(wb++) = UEDATX; //4
			*(wb++) = UEDATX; //4
			*(wb++) = UEDATX; //4
			while(!(SPSR & _BV(SPIF))); //wait for complete
		}

		for (i=0;i<48;++i)
		{
			*wb = UEDATX; wb++;
		}

		UEINTX &= ~_BV(FIFOCON);

		setPtr(&wb); //circle the buffer around if needed
		writePtr = wb;
		samplesBuffered += 15; //buffered 16 samples but played 1
	}
	else if (samplesBuffered >= 1)
	{
		for (i=0;i<4;++i)
		{
			SPDR = *rb; //17 clock cycles for this to send
			rb++;
			while(!(SPSR & _BV(SPIF))); //wait for complete
		}

		samplesBuffered--;
	}
	else
	{
		PORTD |= _BV(PD6);
	}

	setPtr(&rb); //circle the buffer around if needed
	readPtr = rb;
}

void setup_lr_interrupt()
{
	//setup interrupt on rising edge.
	//Rising edge indicates right channel processing by attiny
	//Since we send the left channel over SPI first
	//there is no danger of writing over the buffer being sent to the DAC
	//SPI should always be one channel ahead of the DAC

	EICRB = _BV(ISC41) | _BV(ISC40); //rising edge
	EIFR |= _BV(INTF4); //clear interrupt flag
	EIMSK = _BV(INT4);
}


void SPI_MasterInit(void)
{
	/* Set MOSI and SCK output, all others input */
	DDRB = (1<<PB2)|(1<<PB1) | _BV(PB0);
	/* Enable SPI, Master, set clock rate fck/2 */
	SPCR = (1<<SPE)|(1<<MSTR);
	SPSR = _BV(SPI2X);

	//drive SS high
//	PORTB |= _BV(PB0);
}

int main( void )
{
	cli();

	SetupBuffer();

	setup_clock();

	//Don't touch any PORTB below 4, those are for printf
//	SetupPrintf();

	USB_ZeroPrescaler();
	USB_Init();

	DDRC = 0x0;

	SPI_MasterInit();

	_delay_ms(10); //wait for tiny 44 to be ready for data
//	setup_timers();

	setup_lr_interrupt();

	DDRD |= _BV(PD6);

	sei();

	UENUM = 4; //interrupts can change this

	while(1)
	{
/*
		p = canWrite(writePtr,DATAGRAM_SIZE);
		if ( p > 0 )
		{

			if (!USB_READY(UEINTX)) continue;
			
//need to be able to process USB while processing SPI
cli();
			UEINTX &= ~_BV(RXOUTI); //what?

			for( i = 0; i < DATAGRAM_SIZE; i++ )
				writePtr[i] = UEDATX;

			UEINTX &= ~_BV(FIFOCON);
sei();

			writePtr = p;
		}
*/
	}

	return 0;
}

/*

Copyright (c) 2013 Joshua Allen

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
