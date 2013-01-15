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

void WriteSample();

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
PORTA = 0;
PORTB = 0;
	DDRA = (1<<SDATA) | (1<<SCLK) | (1<<LRCLK); //set output pins
//	PORTA = (1<<SCLK); //up

//	DDRB =  (1<<MCLK);
//	PORTB = 0x0;

//	PINA = 0x0;
//	PINB = 0x0;
}

void FastBlink()
{
	while(1)
	{
		PORTA ^= _BV(LRCLK); //switch channel
		delay_ms(100);
	}
}

void ErrorBlink()
{
	//slow system clock way down
	CLKPR = 0x80;	/*Setup CLKPCE to be receptive*/
	CLKPR = _BV(CLKPS1) | _BV(CLKPS0); //8 ticks

	TCCR0B = _BV(CS02) | _BV(CS00); // 1024 ticks
	OCR0A = 128; // timer at 256 ticks (counts from 0)
	while(1);
}

volatile int16_t mono = 0xffff;
volatile int16_t left = 0xffff;
volatile int16_t right = 0xffff;

ISR( TIM1_COMPA_vect )
{
cli();
	mono ^= 0x8000;
	left ^= 0x8000;
	right+=1000;
sei();
}

static void timer_init( void )
{
	//LRCK has to do a complete cycle every 512 clock ticks
	//we have just 256 clock ticks to work with.
	//setup timer for LRCLK
	TCCR0A = _BV(COM0B0) | _BV(WGM01); // CTC, PA7 toggle
	TCCR0B = _BV(CS01) | _BV(CS00); // 64 ticks
	OCR0A = 3; // timer at 256 ticks (counts from 0)
	TIMSK0 = 0x00; //no timer interrupts

	TCCR1A = 0x00; // CTC
	TCCR1B = _BV(WGM12) | _BV(CS10); // CTC, no prescaling
//	OCR1A = 4711; // toggle so we make almost 2600hz square wave
	OCR1A = 27840; //440 hz square
	TIMSK1 = _BV(OCIE1A); //no timer interrupts
//	TIFR1 = _BV(OCF1A);

	

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

#define RIGHTCHANNEL _BV(LRCLK)

void WriteSample()
{
	//left channel first
	uint8_t hbyte = ((int8_t*)&left)[1]; //high byte
	uint8_t lbyte = ((int8_t*)&left)[0]; //low byte

	while ((PINA & _BV(LRCLK)) == RIGHTCHANNEL); //wait for left channel

	PORTA &= ~_BV(SCLK); //bring sclock down
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
	hbyte = ((int8_t*)&right)[1]; //high byte
	lbyte = ((int8_t*)&right)[0]; //low byte

	while ((PINA & _BV(LRCLK)) != RIGHTCHANNEL); //wait for right channel

	PORTA &= ~_BV(SCLK); //bring sclock down
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

//	_delay_ms(5); //uncomment to test taking too long
	if ((PINA & _BV(LRCLK)) != RIGHTCHANNEL)
		ErrorBlink();
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
//	setup_data_spi();
//	setup_spi();

//	DDRB |= _BV(2);

//	PowerUpDac();
//		PORTA |= 1<<SDATA;

	int16_t l = 0xffff;
//	int16_t l = 0xffff;
	int16_t r = 0xffff;

	//wait for right channel just to force proper syncing of the first sample
	while ((PINA & _BV(LRCLK)) != RIGHTCHANNEL);

	while(1)
	{
		WriteSample();
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
