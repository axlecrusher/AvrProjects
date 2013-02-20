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

volatile int8_t lr_samples[4];
uint8_t wIndex = 0;

uint8_t offset = 0;
//volatile uint8_t WriteNewSample = 0x00;

void WriteSample();

void SetupBuffers()
{
	lr_samples[0] = 0;
	lr_samples[1] = 0;
	lr_samples[2] = 0;
	lr_samples[3] = 0;
}
/*
inline static void SwapBuffers()
{
	//16 ticks
	int8_t* i = backBuffer;
	backBuffer = frontBuffer;
	frontBuffer = i;
}
*/
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
	CLKPR = 0x80;	//Setup CLKPCE to be receptive
	CLKPR = _BV(CLKPS1) | _BV(CLKPS0); //8 ticks

	TCCR0B = _BV(CS02) | _BV(CS00); // 1024 ticks
	OCR0A = 128; // timer at 256 ticks (counts from 0)
	while(1);
}

/*
volatile int16_t mono = 0xffff;
volatile int16_t left = 0xffff;
volatile int16_t right = 0xffff;
*/

/*
ISR( TIM1_COMPA_vect )
{
cli();
	mono ^= 0x4000;
	left ^= 0x4000;
	right+=1000;
sei();
}
*/


//11 clock cycles to enter and leave the interrupt
ISR(USI_OVF_vect)
{
	USIDR |= _BV(USIOIF); //clear oveflow flag
	*(lr_samples+wIndex) = USIBR;
	wIndex++;
	wIndex &= 0x03; //0-3 only
//	if(wIndex==0) SwapBuffers();
}

/*
void WaitForSPIByte()
{
	while (!(USISR & _BV(USIOIF)));
	USIDR |= _BV(USIOIF); //clear oveflow flag	
}
*/
/*
inline static void ReadSPI()
{
	if (USISR & _BV(USIOIF))
	{
		USIDR |= _BV(USIOIF); //clear oveflow flag
		backBuffer[wIndex] = USIBR;
		wIndex++;
		wIndex &= 0x03; //0-3 only
		if(wIndex==0) SwapBuffers();
	}
}
*/
static void timer_init( void )
{
	//LRCK has to do a complete cycle every 512 clock ticks
	//we have just 256 clock ticks to work with.
	//setup timer for LRCLK
	TCCR0A = _BV(COM0B0) | _BV(WGM01); // CTC, PA7 toggle
	TCCR0B = _BV(CS01) | _BV(CS00); // 64 ticks
	OCR0A = 3; // timer at 256 ticks (counts from 0)
	TIMSK0 = 0x00; //no timer interrupts
/*
	TCCR1A = 0x00; // CTC
	TCCR1B = _BV(WGM12) | _BV(CS10); // CTC, no prescaling
//	OCR1A = 4711; // toggle so we make almost 2600hz square wave
	OCR1A = 27840; //440 hz square
	TIMSK1 = _BV(OCIE1A); //no timer interrupts
//	TIFR1 = _BV(OCF1A);
*/
	

}

//#define FASTWRITE

#ifndef FASTWRITE
//8 tick worst case, 7 best case
#define WRITESAMPLEBIT(DATA, BIT) { PORTA &= ~(_BV(SCLK) | _BV(SDATA)); /*lower sclk*/ \
		if ( ((DATA) & (BIT)) > 0) { PORTA |= _BV(SDATA); } /* 1 data, 2 or 3 clock ticks */ \
		PORTA |= _BV(SCLK); } /* sclk up */

#else

#define WRITESAMPLEBIT(DATA, BIT) { PORTA = 0; /*lower sclk*/ \
		if ( ((DATA) & (BIT)) > 0) { PORTA = _BV(SDATA); } /* 1 data, 2 or 3 clock ticks */ \
		PORTA |= _BV(SCLK); } /* sclk up */

#endif

#define RIGHTCHANNEL _BV(LRCLK)

void WriteSample()
{
	//left channel first
	uint8_t hbyte = lr_samples[0]; //high byte
	uint8_t lbyte = lr_samples[1]; //low byte

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
	hbyte = lr_samples[2]; //high byte
	lbyte = lr_samples[3]; //low byte

	//check to see if we take too long
	if ((PINA & _BV(LRCLK)) == RIGHTCHANNEL)
		ErrorBlink();

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
	//check to see if we take too long
	if ((PINA & _BV(LRCLK)) != RIGHTCHANNEL)
		ErrorBlink();
}

void setup_data_spi()
{
	DDRA &= 0xAF; //clear PA6, PA4
	DDRA |= _BV(PA5); //MISO
	USICR = _BV(USIWM0) | _BV(USICS1) | _BV(USIOIE); //3 wire slave device, interrupt
	USISR |= _BV(USIOIF); //overflow flag for signal end of transmission
}

int main( void )
{
	cli();
	setup_clock();

	SetupBuffers();

	setup_pins();
	timer_init();

	setup_data_spi();
	sei();

//	USIDR = 0x00;

//	setup_spi();

//	DDRB |= _BV(2);

//	PowerUpDac();
//		PORTA |= 1<<SDATA;

//	int16_t l = 0xffff;
//	int16_t l = 0xffff;
//	int16_t r = 0xffff;

	//wait for right channel just to force proper syncing of the first sample
	while ((PINA & _BV(LRCLK)) != RIGHTCHANNEL);

//	while (1) WriteSample();
	WriteSample_asm();

/*
uint8_t i = 0;
	while(1)
	{
		if (i >= 109)
		{
			left ^= 0x8000;
right+=1000;
			i = 0;
		}
		WriteSample();
//		WriteSampleFromSPI();
		++i;
	}
*/
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
