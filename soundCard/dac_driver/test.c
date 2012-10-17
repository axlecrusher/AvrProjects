#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <string.h>
#include <avr/sfr_defs.h>
#include "avr_print.h"

#define SDATA PA1
#define SCLK PA2
#define LRCLK PA3
#define MCLK PB2 //Fuses burned to system clock output

#define WATCHCLOCKS

uint8_t offset = 0;
volatile uint8_t WriteNewSample = 0x00;

void WriteSample(int8_t* l, int8_t* r);

void delay_ms(uint32_t time) {
  uint32_t i;
  for (i = 0; i < time; i++) {
    _delay_ms(1);
  }
}

#define NOOP asm volatile("nop" ::)

static void setup_clock( void )
{
	/*Examine Page 33*/

	CLKPR = 0x80;	/*Setup CLKPCE to be receptive*/
	CLKPR = 0x00;	/*No scalar, set clock divisor here*/
//	CKSEL = 0x02;
}

void setup_pins()
{
	DDRA = (1<<SDATA) | (1<<SCLK) | (1<<LRCLK);
//	PORTA = (1<<SCLK); //up

//	DDRB =  (1<<MCLK);
//	PORTB = 0x0;

//	PINA = 0x0;
//	PINB = 0x0;
}

void TooSlow()
{
	TIMSK0 = 0;
	while(1)
	{
		PORTA ^= _BV(LRCLK); //switch channel
		delay_ms(500);
	}
}

void FastBlink()
{
	TIMSK0 = 0;
	while(1)
	{
		PORTA ^= _BV(LRCLK); //switch channel
		delay_ms(100);
	}
}

uint8_t ticksSinceInterrupt;
volatile uint8_t stillWritingBits = 0;

ISR( TIM0_COMPA_vect )
{
	//sclock need to be left in the up state before the callback

#ifdef WATCHCLOCKS
	if (stillWritingBits>0) TooSlow();
#endif

//	PORTA ^= _BV(LRCLK); //switch channel
//	PORTA &= ~(_BV(SCLK)); //bring sclock down
	WriteNewSample = 0x01;
}

static void timer_init( void )
{
	//LRCK has to do a copmlete cycle ever 512 clock ticks
	//we have just 256 clock ticks to work with.
	//it takes time to fire the interrupt so allow 20 clocks for that
	//8bit timer
	TCCR0A = 0x02; /* do not clear */
	TCCR0B = 0x03; //every 64 tick
	OCR0A = 7; //every 448 clock ticks
	TIMSK0 = 2; //timer match A
}

#define WRITESAMPLEBIT(DATA, BIT) { PORTA &= ~(_BV(SCLK) | _BV(SDATA)); /*lower sclk and clear data bit*/ \
		if ( ((DATA) & (BIT)) > 0) { PORTA |= _BV(SDATA); } /* 1 data */ \
		PORTA |= _BV(SCLK); } /* sclk up */

void WriteSample(int8_t* l, int8_t* r)
{
#ifdef WATCHCLOCKS
	stillWritingBits = 1;
#endif

	//left channel first
	uint8_t hbyte = l[1]; //high byte
	uint8_t lbyte = l[0]; //low byte

	PORTA ^= _BV(SCLK); //sclk up
	//sdata is read on the 2nd rising edge after LRCLK toggle

	//LRCLK is in left channel state	

	//compiler made slow code so unroll with out own macro
	WRITESAMPLEBIT(hbyte, 0x80);
	WRITESAMPLEBIT(hbyte, 0x40);
	WRITESAMPLEBIT(hbyte, 0x20);
	WRITESAMPLEBIT(hbyte, 0x10);
	WRITESAMPLEBIT(hbyte, 0x08);
	WRITESAMPLEBIT(hbyte, 0x04);
	WRITESAMPLEBIT(hbyte, 0x02);
	WRITESAMPLEBIT(hbyte, 0x01);

	WRITESAMPLEBIT(lbyte, 0x80);
	WRITESAMPLEBIT(lbyte, 0x40);
	WRITESAMPLEBIT(lbyte, 0x20);
	WRITESAMPLEBIT(lbyte, 0x10);
	WRITESAMPLEBIT(lbyte, 0x08);
	WRITESAMPLEBIT(lbyte, 0x04);
	WRITESAMPLEBIT(lbyte, 0x02);
	WRITESAMPLEBIT(lbyte, 0x01);

	//right channel
	hbyte = r[1]; //high byte
	lbyte = r[0]; //low byte

	PORTA ^= _BV(LRCLK); //switch to right channel
	PORTA &= ~(_BV(SCLK)); //bring sclock down
	PORTA |= _BV(SCLK); //sclk up

	WRITESAMPLEBIT(hbyte, 0x80);
	WRITESAMPLEBIT(hbyte, 0x40);
	WRITESAMPLEBIT(hbyte, 0x20);
	WRITESAMPLEBIT(hbyte, 0x10);
	WRITESAMPLEBIT(hbyte, 0x08);
	WRITESAMPLEBIT(hbyte, 0x04);
	WRITESAMPLEBIT(hbyte, 0x02);
	WRITESAMPLEBIT(hbyte, 0x01);

	WRITESAMPLEBIT(lbyte, 0x80);
	WRITESAMPLEBIT(lbyte, 0x40);
	WRITESAMPLEBIT(lbyte, 0x20);
	WRITESAMPLEBIT(lbyte, 0x10);
	WRITESAMPLEBIT(lbyte, 0x08);
	WRITESAMPLEBIT(lbyte, 0x04);
	WRITESAMPLEBIT(lbyte, 0x02);
	WRITESAMPLEBIT(lbyte, 0x01);

	//for testing ttoo highclock ticks
//	for (i=0;i<255;++i) NOOP;

	PORTA ^= _BV(LRCLK); //switch to left channel
	PORTA &= ~(_BV(SCLK)); //bring sclock down

#ifdef WATCHCLOCKS
	stillWritingBits = 0;
#endif

/*
	for (i=0x80;i>0;i>>=1) //funroll
	{
		PORTA &= ~(_BV(SCLK)); //lower sclk
		if ( (hbyte & i) > 0)
			PORTA |= _BV(SDATA); //set bit
		else
			PORTA &= ~(_BV(SDATA)); //clear
		
		PORTA |= _BV(SCLK); //sclk up
	}

	for (i=0x80;i>0;i>>=1) //funroll
	{
		PORTA &= ~(_BV(SCLK)); //lower sclk
		if ( (lbyte & i) > 0)
			PORTA |= _BV(SDATA); //set bit
		else
			PORTA &= ~(_BV(SDATA)); //clear
		
		PORTA |= _BV(SCLK); //sclk up
	}
*/
}

void WriteTest()
{
	uint8_t i = 0;

	PORTA |= 1<<SCLK; //sclk up

	for(i=0;i<16;++i)
	{
		PORTA &= ~(1<<SCLK); //sclk down
		PORTA |= (1<<SDATA); //set bit
		PORTA |= 1<<SCLK; //sclk up, data read in, set SDATA before this
	}
}

int main( void )
{
	cli();
	setup_clock();
	setup_pins();
	timer_init();
	sei();
//	setup_spi();

//	DDRB |= _BV(2);

//	PowerUpDac();
//		PORTA |= 1<<SDATA;

	int16_t l = 0x7fff;
	int16_t r = 0x7fff;

//	PORTA ^= _BV(LRCLK); //switch channel
//	PORTA ^= _BV(LRCLK); //switch channel

	while(1)
	{
		WriteSample((int8_t*)&l,(int8_t*)&r);
		while (WriteNewSample==0);
		WriteNewSample = 0;
	}

	PORTA = 0x0;

	return 0;
}

/*

Copyright (c) 2012 Joshua Allen

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
