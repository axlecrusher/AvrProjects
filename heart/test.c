#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <string.h>
#include <avr/sfr_defs.h>
#include <avr/pgmspace.h>

#include "animations.h"

#define GRID_ARRAY_LENGTH 42
static const uint8_t DDRA_map[] =  {0x80,0x06,0x06,0x80,0x0C,0x0C,0x80,0x22,0x24,0x24,0x05,0x14,0x14,0x18,0x22,0x03,0x03,0x05,0x09,0x09,0x18,0x80,0x21,0x21,0x0A,0x11,0x11,0x80,0x80,0x80,0x30,0x0A,0x30,0x80,0x80,0x80,0x80,0x80,0x28,0x80,0x80,0x80};
static const uint8_t PORTA_map[] = {0x80,0x02,0x04,0x80,0x08,0x04,0x80,0x20,0x20,0x04,0x04,0x10,0x04,0x08,0x02,0x02,0x01,0x01,0x08,0x01,0x10,0x80,0x20,0x01,0x08,0x10,0x01,0x80,0x80,0x80,0x20,0x02,0x10,0x80,0x80,0x80,0x80,0x80,0x20,0x80,0x80,0x80};

//uint8_t current_frame[GRID_ARRAY_LENGTH+1] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

//uint8_t current_frame[GRID_ARRAY_LENGTH+1] = {10,8,9,10,11,12,17,24,31,30,22};


void (*DoFrame)(void) = NULL;
void (*DrawFunc)(void) = NULL;

ISR(INT0_vect)
{
	GIMSK = 0;
}

ISR( TIM1_COMPA_vect )
{
	DoFrame();
}

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
}

void setup_pins()
{
	DDRA = 0x3F; //set output pins, 24
	DDRB = 0;

	PORTA = 0;
	PORTB = 0;

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
		PORTA ^= _BV(PA0); //switch channel
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

uint8_t LightLED(uint8_t i)
{	
	PORTA = PORTA_map[i];
	DDRA = DDRA_map[i];

	_delay_loop_2(100); //25us

	DDRA = 0;  //put all pins into input mode, all LEDs off

	return (PORTA_map[i] != 0x80);
}

void TestLEDs()
{
	uint8_t i;
	for (i=0; i < GRID_ARRAY_LENGTH; ++i)
	{
		if ( LightLED(i) ) delay_ms(10);
	}
}

static void timer_init( void )
{
	TCCR1A = 0x00; // CTC
	TCCR1B = _BV(WGM12) | _BV(CS12) | _BV(CS10); // CTC, no prescaling
//	OCR1A = 4711; // toggle so we make almost 2600hz square wave
	OCR1A = 19531; //1 second
	TIMSK1 = _BV(OCIE1A); //no timer interrupts
//	TIFR1 = _BV(OCF1A);
}

/*
void CopyFrame(const uint8_t* p)
{
	uint8_t i;
	uint8_t length = p[0];
	for (i=0;i<=length; ++i)
		current_frame[i] = p[i];
}
*/

int main( void )
{
	cli();
	setup_clock();

	DIDR0 = 0xFF; //disable digital input

	setup_pins();
	timer_init();

	sei();

//	USIDR = 0x00;

//	setup_spi();

//	DDRB |= _BV(2);

//	PowerUpDac();
//		PORTA |= 1<<SDATA;

	TestLEDs();

	PORTA = 0x0;

//	offset = 0;
//	memcpy_P(current_frame, animation+offset, pgm_read_byte(animation+offset)+1);

	DrawFunc = &ShowFrame;
	NextSequence();

	while (1)
	{
		DrawFunc();
	}

	PORTA = 0x0;

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
