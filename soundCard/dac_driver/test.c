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

static uint16_t sineTable[] = {32768, 33625, 34482, 35338, 36193, 37045, 37893, 38739, 39580, 40417, 41248, 42074, 42893, 43706, 44510, 45307, 46095, 46874, 47644, 48403, 49151, 49888, 50614, 51327, 52028, 52715, 53389, 54048, 54693, 55323, 55938, 56536, 57118, 57684, 58233, 58764, 59277, 59772, 60249, 60706, 61145, 61564, 61964, 62343, 62702, 63041, 63359, 63656, 63931, 64186, 64418, 64630, 64819, 64986, 65132, 65255, 65355, 65434, 65490, 65524, 65535, 65524, 65490, 65434, 65355, 65255, 65132, 64986, 64819, 64630, 64418, 64186, 63931, 63656, 63359, 63041, 62702, 62343, 61964, 61564, 61145, 60706, 60249, 59772, 59277, 58764, 58233, 57684, 57118, 56536, 55938, 55323, 54693, 54048, 53389, 52715, 52028, 51327, 50614, 49888, 49151, 48403, 47644, 46874, 46095, 45307, 44510, 43706, 42893, 42074, 41248, 40417, 39580, 38739, 37893, 37045, 36193, 35338, 34482, 33625, 32768, 31910, 31053, 30197, 29342, 28490, 27642, 26796, 25955, 25118, 24287, 23461, 22642, 21829, 21025, 20228, 19440, 18661, 17891, 17132, 16384, 15647, 14921, 14208, 13507, 12820, 12146, 11487, 10842, 10212, 9597, 8999, 8417, 7851, 7302, 6771, 6258, 5763, 5286, 4829, 4390, 3971, 3571, 3192, 2833, 2494, 2176, 1879, 1604, 1349, 1117, 905, 716, 549, 403, 280, 180, 101, 45, 11, 0, 11, 45, 101, 180, 280, 403, 549, 716, 905, 1117, 1349, 1604, 1879, 2176, 2494, 2833, 3192, 3571, 3971, 4390, 4829, 5286, 5763, 6258, 6771, 7302, 7851, 8417, 8999, 9597, 10212, 10842, 11487, 12146, 12820, 13507, 14208, 14921, 15647, 16384, 17132, 17891, 18661, 19440, 20228, 21025, 21829, 22642, 23461, 24287, 25118, 25955, 26796, 27642, 28490, 29342, 30197, 31053, 31910};

uint8_t offset = 0;
volatile uint8_t WriteNewSample = 0x00;

void WriteSample(int16_t* l, int16_t* r);

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
	PORTA = (1<<SCLK); //up

//	DDRB =  (1<<MCLK);
//	PORTB = 0x0;

//	PINA = 0x0;
//	PINB = 0x0;
}

void PowerUpDac()
{
	uint8_t i = 0;
	//MCLK by clock input
	PORTA |= (1<<LRCLK); //up
	PORTA &= ~(1<<LRCLK); //down
}

uint8_t ticksSinceInterrupt;

ISR( TIM0_COMPA_vect )
{
	//sclock need to be left in the up state before the callback
	PORTA ^= _BV(LRCLK); //switch channel
	PORTA &= ~(_BV(SCLK)); //bring sclock down
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

#define WRITESAMPLEBIT(DATA, BIT) { PORTA &= ~(_BV(SCLK)); /*lower sclk*/ \
		PORTA &= ~(_BV(SDATA)); /* 0 data, do not put in else or compiler makes lots of jumps */ \
		if ( ((DATA) & (BIT)) > 0) { PORTA |= _BV(SDATA); } /* 1 data */ \
		PORTA |= _BV(SCLK); } /* sclk up */

void WriteSample(int16_t* l, int16_t* r)
{
	uint8_t i;

	//left channel first
	uint8_t hbyte = *(l+1); //high byte
	uint8_t lbyte = *(l); //low byte

	PORTA |= 1<<SCLK; //sclk up
	//sdata is read on the 2nd rising edge after LRCLK toggle 

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
	hbyte = *(r+1); //high byte
	lbyte = *(r); //low byte

	PORTA ^= _BV(LRCLK); //switch to right channel
	PORTA &= ~(_BV(SCLK)); //bring sclock down
	PORTA |= 1<<SCLK; //sclk up

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
	uint16_t v = 0x7FFF;

	cli();
	setup_clock();
	setup_pins();
	timer_init();
	sei();
//	setup_spi();

//	DDRB |= _BV(2);

//	PowerUpDac();
//		PORTA |= 1<<SDATA;

	int16_t s = 0x7fff;

	while(1)
	{
		while (WriteNewSample==0);
		WriteNewSample = 0;
//		WriteTest();

//		printf("clock tick %d\n", ticksSinceInterrupt);
		
//		while (WriteNewSample == 0);
//		WriteNewSample = 0;

		WriteSample(&s,&s);
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
