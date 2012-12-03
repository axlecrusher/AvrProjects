#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <string.h>
#include <avr/sfr_defs.h>
#include "avr_print.h"

#define SDATA PA1
#define SCLK PA2
#define LRCLK PA7
#define MCLK PB2 //Fuses burned to system clock output

#define WATCHCLOCKS

uint8_t offset = 0;
//volatile uint8_t WriteNewSample = 0x00;

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
//	CLKPR = 0x03;	/*No scalar, set clock divisor here*/
//	CKSEL = 0x02;
}

void setup_pins()
{
	DDRA = 0;
	DDRB = 0;

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

ISR( TIM1_COMPA_vect )
{
	//sclock need to be left in the up state before the callback
//	PORTA ^= _BV(LRCLK); //switch to left channel

/*
TCCR1B ^= 0x01;
printf("tick %d\n", ticksSinceInterrupt);
TCCR1B ^= 0x01;
*/
#ifdef WATCHCLOCKS
	if (stillWritingBits>0) TooSlow();
#endif

//	PORTA ^= _BV(LRCLK); //switch channel
//	PORTA &= ~(_BV(SCLK)); //bring sclock down
//	WriteNewSample = 0x01;
}

static void timer_init( void )
{
	//LRCK has to do a copmlete cycle ever 512 clock ticks
	//we have just 256 clock ticks to work with.

	TCCR0A = 0x12; /* CTC, PA7 toggle */
	TCCR0B = 0x03; /* 64 ticks */
	OCR0A = 3; // timer at 256 ticks (counts from 0)
	TIMSK0 = 0x00; //no timer interrupts
}

//#define WRITEBIT_FAST

#ifndef WRITEBIT_FAST

//8 tick worst case, 7 best case
#define WRITESAMPLEBIT(DATA, BIT) { PORTA &= ~(_BV(SCLK) | _BV(SDATA)); /*lower sclk*/ \
		if ( ((DATA) & (BIT)) > 0) { PORTA |= _BV(SDATA); } /* 1 data, 2 or 3 clock ticks */ \
		PORTA |= _BV(SCLK); } /* sclk up */

#else

//6 tick worst case, 5 best case (DANGEROUS STOMPS PA REGISTER)
#define WRITESAMPLEBIT(DATA, BIT) { PORTA = 0; /*low sclk, low data*/ \
		if ( ((DATA) & (BIT)) > 0) { PORTA |= _BV(SDATA); } /* 1 data, 2 or 3 clock ticks */ \
		PORTA |= _BV(SCLK); } /* sclk up */

#endif

//9 clock ticks, CAREFUL!!! CONSTANT TIMING REGARDLESS OF DATA
//compiler makes this into bullcrap asm with lots of relative jumps
#define WRITESAMPLEBIT9(DATA, BIT) { PORTA &= ~(_BV(SCLK)); /*lower sclk*/ \
		if ( ((DATA) & (BIT)) == 0) { PORTA &= ~_BV(SDATA); } /* 1 data, 3 or 2 clock ticks */ \
		if ( ((DATA) & (BIT)) > 0) { PORTA |= _BV(SDATA); } /* 1 data, 2 or 3 clock ticks (inverse of above) */ \
		PORTA |= _BV(SCLK); } /* sclk up */

//9 clock ticks, CAREFUL!!! CONSTANT TIMING REGARDLESS OF DATA
//assembly implementation of above because gcc botches it up
#define WRITESAMPLEBIT9ASM(DATA, BIT) { PORTA &= ~(_BV(SCLK)); /*lower sclk*/ \
		/* if ( ((DATA) & (BIT)) == 0) { PORTA &= ~_BV(SDATA); } 1 data, 3 or 2 clock ticks */ \
		__asm__ __volatile__ ("sbrs %0,%1\n\t" : "+r"(DATA) : "i"(BIT) ); \
		__asm__ __volatile__ ("cbi 59-32,1\n\t" ); \
		/* if ( ((DATA) & (BIT)) > 0) { PORTA |= _BV(SDATA); } 1 data, 2 or 3 clock ticks (inverse of above) */ \
		__asm__ __volatile__ ("sbrc %0,%1\n\t" : "+r"(DATA) : "i"(BIT) ); \
		__asm__ __volatile__ ("sbi 59-32,1\n\t" ); \
		PORTA |= _BV(SCLK); } /* sclk up */

void WriteSample(int8_t* l, int8_t* r)
{
	//left channel first
	uint8_t hbyte = l[1]; //high byte
	uint8_t lbyte = l[0]; //low byte

	//LRCLK is in left channel state	
//	PORTA &= ~_BV(LRCLK); //switch to left channel

	while ((PINA & _BV(LRCLK)) > 0); //wait for left channel

	PORTA &= ~(_BV(SCLK)); //bring sclock down
	PORTA |= _BV(SCLK); //sclk up

	//compiler made slow code so unroll with our own macro
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

	while ((PINA & _BV(LRCLK)) == 0); //wait for right channel

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
}

void WriteSampleFromSPI()
{
	//left channel first
	uint8_t sbyte = 0x00;

	while ((PINA & _BV(LRCLK)) > 0); //wait for left channel

	PORTA &= ~(_BV(SCLK)); //bring sclock down
	PORTA |= _BV(SCLK); //sclk up
	
	//read high byte
	while ((USISR & (1<<USIOIF)) == 0);
	USISR |= 1<<USIOIF; //clear interrupt
	sbyte = USIBR; //read buffered data

	//compiler made slow code so unroll with our own macro
	WRITESAMPLEBIT(sbyte, 0x80);
	WRITESAMPLEBIT(sbyte, 0x40);
	WRITESAMPLEBIT(sbyte, 0x20);
	WRITESAMPLEBIT(sbyte, 0x10);
	WRITESAMPLEBIT(sbyte, 0x08);
	WRITESAMPLEBIT(sbyte, 0x04);
	WRITESAMPLEBIT(sbyte, 0x02);
	WRITESAMPLEBIT(sbyte, 0x01);

	//read low byte
	while ((USISR & (1<<USIOIF)) == 0);
	USISR |= 1<<USIOIF; //clear interrupt
	sbyte = USIBR; //read buffered data

	WRITESAMPLEBIT(sbyte, 0x80);
	WRITESAMPLEBIT(sbyte, 0x40);
	WRITESAMPLEBIT(sbyte, 0x20);
	WRITESAMPLEBIT(sbyte, 0x10);
	WRITESAMPLEBIT(sbyte, 0x08);
	WRITESAMPLEBIT(sbyte, 0x04);
	WRITESAMPLEBIT(sbyte, 0x02);
	WRITESAMPLEBIT(sbyte, 0x01);

	while ((PINA & _BV(LRCLK)) == 0); //wait for right channel

	PORTA &= ~(_BV(SCLK)); //bring sclock down
	PORTA |= _BV(SCLK); //sclk up

	//read high byte
	while ((USISR & (1<<USIOIF)) == 0);
	USISR |= 1<<USIOIF; //clear interrupt
	sbyte = USIBR; //read buffered data

	WRITESAMPLEBIT(sbyte, 0x80);
	WRITESAMPLEBIT(sbyte, 0x40);
	WRITESAMPLEBIT(sbyte, 0x20);
	WRITESAMPLEBIT(sbyte, 0x10);
	WRITESAMPLEBIT(sbyte, 0x08);
	WRITESAMPLEBIT(sbyte, 0x04);
	WRITESAMPLEBIT(sbyte, 0x02);
	WRITESAMPLEBIT(sbyte, 0x01);

	//read low byte
	while ((USISR & (1<<USIOIF)) == 0);
	USISR |= 1<<USIOIF; //clear interrupt
	sbyte = USIBR; //read buffered data

	WRITESAMPLEBIT(sbyte, 0x80);
	WRITESAMPLEBIT(sbyte, 0x40);
	WRITESAMPLEBIT(sbyte, 0x20);
	WRITESAMPLEBIT(sbyte, 0x10);
	WRITESAMPLEBIT(sbyte, 0x08);
	WRITESAMPLEBIT(sbyte, 0x04);
	WRITESAMPLEBIT(sbyte, 0x02);
	WRITESAMPLEBIT(sbyte, 0x01);
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

void setup_data_spi()
{
	DDRA &= 0xAF; //clear PA6, PA4
	DDRA |= _BV(PA5); //MISO
	USICR = (1<<USIWM0)|(1<<USICS1); //slave device
	USISR |= 1<<USIOIF; //overflow flag for signal end of transmission
}

int main( void )
{
	cli();
	setup_clock();
	setup_pins();
	timer_init();
	sei();
	setup_data_spi();
//	setup_spi();

//	DDRB |= _BV(2);

//	PowerUpDac();
//		PORTA |= 1<<SDATA;

	int16_t l = 0x7fff;
//	int16_t l = 0xffff;
	int16_t r = 0xffff;

	//wait for right channel just to force proper syncing of the first sample
	while ((PINA & _BV(LRCLK)) == 0);

	while(1)
	{
		WriteSample((int8_t*)&l,(int8_t*)&r);
//		WriteSampleFromSPI();
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
