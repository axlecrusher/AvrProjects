#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <string.h>
#include <avr/sfr_defs.h>
#include "usb.h"

#include "avrUsbUtils.h"

#include "SPIPrinting.h"

#define BUFFERLEN 32
//uint8_t sendbuffer[BUFFERLEN];

volatile int16_t mono = 0xffff;
volatile int16_t left = 0xffff;
volatile int16_t right = 0xffff;

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
uint8_t samples = 0;

static inline void SendChannelData();

ISR(INT4_vect)
{
	PORTD ^= _BV(PD6);

	SendChannelData();
}

static inline void SendChannelData()
{
	//send left MSB
	SPDR = ((int8_t*)&left)[1];
	while(!(SPSR & _BV(SPIF))); //wait for complete

	//send left LSB
	SPDR = ((int8_t*)&left)[0];
	while(!(SPSR & _BV(SPIF))); //wait for complete

	//send right MSB
	SPDR = ((uint8_t*)&right)[1];
	while(!(SPSR & _BV(SPIF))); //wait for complete

	//send right LSB
	SPDR = ((uint8_t*)&right)[0];
	while(!(SPSR & _BV(SPIF))); //wait for complete

	//simple test tones can be made here
	samples+=10;
	if (samples>92)
	{
		left ^= 0x8000;
		right+=2;
		samples = 0;
	}
}

void setup_lr_interrupt()
{
	//setup interrupt on rising edge.
	//Rising edge indicates right channel processing by attiny
	//Since we send the left channel over SPI first
	//there is no danger of writing over the buffer being sent to the DAC
	//SPI should always be one channel ahead of the DAC

	EICRB = _BV(ISC41) | _BV(ISC40); //rising edge
	EIMSK = _BV(INT4);
}


void SPI_MasterInit(void)
{
	/* Set MOSI and SCK output, all others input */
	DDRB = (1<<PB2)|(1<<PB1) | _BV(PB0);
	/* Enable SPI, Master, set clock rate fck/4 */
	SPCR = (1<<SPE)|(1<<MSTR);

	//drive SS high
//	PORTB |= _BV(PB0);
}

int main( void )
{
	cli();

//	memset(sendbuffer,0,BUFFERLEN);

	setup_clock();

DDRC = 0x0;

SPI_MasterInit();

//	_delay_ms(10); //wait for tiny 44 to be ready for data
	setup_lr_interrupt();
//	setup_timers();

	//Don't touch any PORTB below 4, those are for printf
//	SetupPrintf();

//	USB_ZeroPrescaler();
//	USB_Init();

	DDRD |= _BV(PD6);
//	PORTD |= _BV(PD6);

	sei();

	while(1);
/*
	uint16_t i = 0;
	while(1)
	{
		if (i >= 710)
		{
			left ^= 0x8000;
			right+=1000;
			i = 0;
		}
		++i;
	}
*/
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
