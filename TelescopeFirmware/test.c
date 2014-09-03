#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <string.h>
#include <avr/sfr_defs.h>
#include "avr_print.h"

#define STEPCOUNT 6
//uint8_t steps[STEPCOUNT] = { 0x01, 0x03, 0x02, 0x06, 0x04, 0x05 };
//uint8_t i = 0;

#define WAIT 1562

#define FORWARD 0x01
#define BACKWARD 0x02
#define STOP 0x00

volatile uint32_t rcount;
volatile uint8_t direction;

ISR(TIMER1_COMPA_vect)
{

//	PORTD = steps[i];
//	++i;
//	if (i>=STEPCOUNT) i = 0;

//	PORTD ^= 0x01;

	if (OCR1A == WAIT)
	{

		OCR1A = 400; //might be lower limit for movement (at least visible)
		PORTD = (PORTD & ~0x03) | direction;
	}
	else
	{
		PORTD = (PORTD & ~0x03) | 0x00;
		OCR1A = WAIT;
	}
}

ISR(INT2_vect)
{
	uint8_t t = PIND & (_BV(PD2) | _BV(PD3));
	if (t == _BV(PD2))
		rcount++;
}

ISR(INT3_vect)
{

	uint8_t t1 = PIND & (_BV(PD2) | _BV(PD3));
	/*
	uint8_t t2 = PIND & (_BV(PD2) | _BV(PD3));
	do 
	{
		t1=t2;
		_delay_us (2.0f);
		t2 = PIND & (_BV(PD2) | _BV(PD3));
	}
	while(t1 != t2);
*/
	if (t1 == _BV(PD3))
		rcount--;
}

static void setup_clock()
{
	CLKPR = 0x80;	/*Setup CLKPCE to be receptive*/
	CLKPR = 0x00; //no divisor
}

static void setup_timers()
{

	TCCR1A = _BV(COM1A0);//Toggle OC1A
	TCCR1B = _BV(WGM12) | _BV(CS12); //CTC, 256 divisor
	OCR1A = WAIT; //60hz
	TIMSK1 = 0x00; //no interrupts
	TIMSK1 = OCIE1A<<1;
}

static void SetupDriverPins()
{
	DDRD = _BV(PD0) | _BV(PD1) | _BV(PD6);
	PORTD = 0;
}

void forward() {
//	PORTD &= ~_BV(PD1);
//	PORTD |= _BV(PD0);
	direction = FORWARD;
}

void backward() {
//	PORTD &= ~_BV(PD0);
//	PORTD |= _BV(PD1);
	direction = BACKWARD;
}

void stop() {
	PORTD &= ~(_BV(PD1)|_BV(PD2));
	direction = STOP;
}

int main( void )
{
	cli();

	EICRA = _BV(ISC21) | _BV(ISC20) | _BV(ISC31) | _BV(ISC30);
	EIMSK = _BV(INT2) | _BV(INT3);

rcount = 0;

//	DDRC = _BV(PC6); //output pin
	SetupDriverPins();

	setup_clock();
	setup_timers();
	setup_spi();

	sei();

uint32_t t = 0;
	while(1)
	{
//		PORTD &= ~_BV(PD6);
		cli();
		t=rcount;
		t &= 0xffffff00;
		sei();
if (t>0xff000000) 
	t = 0;

		if (t<0x10000) {
			forward();
		}
		else if (t>0x10000) {
			backward();
		}
		else
		{
			stop();
		}
		sendhex8(&t);
		sendchr('\n');
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
