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
volatile uint16_t buffer[BUFFERLEN];
volatile uint8_t write;
volatile uint8_t read;
volatile uint8_t spaceLeft;

volatile uint8_t bytesRead = 0;

void SetupBuffer()
{
	int i;
	for (i=0; i<BUFFERLEN; ++i) buffer[i] = 0;
	spaceLeft = BUFFERLEN;
	read = write = 0;
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

inline uint8_t inc(uint8_t a, uint8_t b, uint8_t max)
{
	a+=b;
	if (a>=max) a -= max;
	return a;
}

static void ReadUSB_SendChannelData()
{
	uint8_t* wb = (uint8_t*)buffer[write];
	uint8_t* rb = (uint8_t*)buffer[read];

	UEINTX &= ~_BV(RXOUTI); //ack

	//send left MSB
	SPDR = rb[0]; //17 clock cycles for this to send
	wb[0] = UEDATX;
	wb[1] = UEDATX;
	while(!(SPSR & _BV(SPIF))); //wait for complete
	
	//send left LSB
	SPDR = rb[1]; //17 clock cycles for this to send
	wb[2] = UEDATX;
	wb[3] = UEDATX;
	while(!(SPSR & _BV(SPIF))); //wait for complete

	//send right MSB
	SPDR = rb[2]; //17 clock cycles for this to send
	wb[4] = UEDATX;
	wb[5] = UEDATX;
	while(!(SPSR & _BV(SPIF))); //wait for complete

	//send right LSB
	SPDR = rb[3]; //17 clock cycles for this to send
	wb[6] = UEDATX;
	wb[7] = UEDATX;
	while(!(SPSR & _BV(SPIF))); //wait for complete

	//read samples, wrote 4 samples so we have 2 less spaces
	spaceLeft -= 2;

	read = inc(read, 2, BUFFERLEN);
	write = inc(write, 4, BUFFERLEN);

	bytesRead += 8;
	if (bytesRead >= DATAGRAM_SIZE)
	{
		UEINTX &= ~_BV(FIFOCON); //reset USB packet
		bytesRead = 0;
	}
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
	read = inc(read, 2, BUFFERLEN);
}

void testAssembly()
{
	uint8_t i = 0;
	do
	{
		UENUM++;
		i++;
	}
	while (i<4);
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
