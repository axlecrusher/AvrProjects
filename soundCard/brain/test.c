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

void SendLeftChannel();
void SendRightChannel();

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

/*
uint8_t clockSkew = 0;
ISR(TIMER1_COMPA_vect) 
{
	PORTD ^= _BV(PD6);

	// Send the data here. We can interleave other instructions while
	//	we wait for sending to complete

	//send left MSB
	SPDR = ((int8_t*)&left)[1];
	OCR1AL = 77; //low byte of 333
	while(!(SPSR & _BV(SPIF))); //wait for complete

	//send left LSB
	SPDR = ((int8_t*)&left)[0];
	++clockSkew;
	while(!(SPSR & _BV(SPIF))); //wait for complete

	//send right MSB
	SPDR = ((uint8_t*)&right)[1];
	if (clockSkew>=3) OCR1AL = 78; //low byte of 334, make up for 333.333
	while(!(SPSR & _BV(SPIF))); //wait for complete

	//send right LSB
	SPDR = ((uint8_t*)&right)[0];
	if (clockSkew>=3) clockSkew = 0;
	while(!(SPSR & _BV(SPIF))); //wait for complete


	//simple test tones can be made here
	++samples;
	if (samples>=100)
	{
		left ^= 0x8000;
		right+=1000;
		samples = 0;
	}

}
*/
ISR(PCINT1_vect)
{
	PORTD ^= _BV(PD6);

	if ((PINC & _BV(PC6)) == 0)
		SendChannelData();

/*
	if (PINC & _BV(PC6))
	{
		SendLeftChannel();
	}
	else
	{
		SendRightChannel();
	}
*/
}

void SendChannelData()
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
	++samples;
	if (samples>=17)
	{
		left ^= 0x8000;
		right+=1000;
		samples = 0;
	}
}

void SendLeftChannel()
{
	//send left MSB
	SPDR = ((int8_t*)&left)[1];
	while(!(SPSR & _BV(SPIF))); //wait for complete

	//send left LSB
	SPDR = ((int8_t*)&left)[0];
	while(!(SPSR & _BV(SPIF))); //wait for complete
}

void SendRightChannel()
{
	//send right MSB
	SPDR = ((uint8_t*)&right)[1];
	while(!(SPSR & _BV(SPIF))); //wait for complete

	//send right LSB
	SPDR = ((uint8_t*)&right)[0];
	while(!(SPSR & _BV(SPIF))); //wait for complete

	//simple test tones can be made here
	++samples;
	if (samples>=17)
	{
		left ^= 0x8000;
		right+=1000;
		samples = 0;
	}
}

void setup_lr_interrupt()
{
	PCIFR |= _BV(PCIF1);
	PCICR = _BV(PCIE1);
	PCMSK1 = _BV(PCINT8);
	PCIFR |= _BV(PCIF1);
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
