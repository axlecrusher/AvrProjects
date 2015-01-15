#ifndef AVRUSBUTILS_H
#define AVRUSBUTILS_H

#include <avr/io.h>
//#include <avr/pgmspace.h>
//#include <avr/interrupt.h>
#include <stdint.h>

//pass UEINTX into these macros, or a register containing UEINTX value
#define USB_READY(X)		(X & _BV(FIFOCON))
#define USB_CAN_WRITE(X)	(X & _BV(TXINI))
#define USB_HAS_SPACE(X)	(X & _BV(RWAL))

#define USB_SEND 		UEINTX = 0x7A
#define USB_KILL 		UEINTX = 0xDF

uint8_t UsbWrite(uint8_t endpoint, uint8_t* data, uint8_t length);
uint8_t UsbWrite_Blocking(uint8_t endpoint, uint8_t* data, uint8_t length);

uint8_t UsbRead_Blocking(uint8_t endpoint, uint8_t* data, uint8_t maxlength);

void usb_write_str(const char* cs);
void usb_write(const char* d, uint8_t l);



static inline void usb_wait_in_ready(void)
{
	while (!(UEINTX & (1<<TXINI))) ;
}
static inline void usb_send_in(void)
{
	UEINTX = ~(1<<TXINI);
}
static inline void usb_wait_receive_out(void)
{
	while (!(UEINTX & (1<<RXOUTI))) ;
}
static inline void usb_ack_out(void)
{
	UEINTX = ~(1<<RXOUTI);
}

#endif

/*

Copyright (c) 2012, Joshua Allen
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
