#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <string.h>
#include <avr/sfr_defs.h>

ISR(TIM1_COMPA_vect)
{
	TCCR0A = _BV(COM0B1) | _BV(WGM01); //clear on match
	TCCR0B |= _BV(FOC0B); //force match to force pin low, strobe on
	TCCR0A = _BV(COM0B1) | _BV(COM0B0) | _BV(WGM01); //set high on match
	TCNT0 = 0; //simulate ctc
}

static void setup_clock()
{
//	CLKPR = 0x80;	/*Setup CLKPCE to be receptive*/
//	CLKPR = 0x00; //no divisor
	/* 1 Mhz clock */
}

static void setup_timers()
{
/*
	TCCR1A = _BV(COM1A0);//Toggle OC1A
	TCCR1B = _BV(WGM12) | _BV(CS12); //CTC, 256 divisor
	OCR1A = 1041; //60hz
	TIMSK1 = 0x00; //no interrupts
*/

	/* toggle OC1B, CK/4096, CTC */
//	TCCR1 = _BV(CS13) | _BV(CS12) | _BV(CS10) | _BV(CTC1);

	/* strobe on timer */
	/* OC0B, CTC, clear OC0B */
	TCCR0A = _BV(COM0B1) | _BV(COM0B0) | _BV(WGM01); //set high on match
	TCCR0B = _BV(CS02) | _BV(CS00);
//	TIMSK = 0;
	OCR0A = 9; //1/100th second

	/* timer for firing strobe */
	TCCR1 = _BV(CTC1) | _BV(CS12) | _BV(CS13)| _BV(CS10);
	TIMSK = _BV(OCIE1A);
	OCR1C = 25;
}

int main( void )
{
	cli();


	setup_clock();
	setup_timers();
	DDRB = _BV(PB1) | _BV(PB4); //output pin

	sei();

//PORTB = _BV(PB3);
	while(1);

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
