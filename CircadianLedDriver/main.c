#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <string.h>
#include <avr/sfr_defs.h>

#include <avr/pgmspace.h>
#define SECOND 62500

//linear LED steps
char steps[] PROGMEM = { 1, 2, 3, 4, 6, 7, 12, 16, 23, 32, 45, 64, 90, 128, 180, 255 };

uint8_t offset = 0;

uint16_t cx = 0;

//TIM0_OVF_vect
ISR(TIMER0_OVF_vect)
{
	cx++;
	/*
	if(cx==SECOND/20) //seconds
	{
		cx = 0;
//		OCR0A = pgm_read_byte(steps+offset);
		OCR0A = offset;
		offset++;
		if (offset>=255) offset = 0;
	}
	*/
//	OCR0A++;
	/*
	OCR0A = pgm_read_byte(flicker+offset);
	++offset;
	if (offset>=sizeof(flicker)) offset = 0;
	*/
}

static void setup_clock()
{
	CLKPR = 0x80;	/*Setup CLKPCE to be receptive*/
	CLKPR = 0x00;// | _BV(CLKPS1); //no divisor
}

static void setup_pwm()
{
	//FAST PWM running at cpu speed
	TCCR0A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM01) | _BV(WGM00);
	TCCR0B = _BV(CS00);//  | _BV(CS01);

	//use overflow interrupt as timer to count time that has passed
	TIMSK0 = _BV(TOIE0);
	TIFR0 = _BV(TOV0);

	//seems to look like 6000K
	OCR0A = 39; //blue
	OCR0B = 90; //green
	OCR1A = 198; //red

	//FAST PWM at cpu speed, 8 bit
	TCCR1A = _BV(COM1A1) | _BV(COM1B1) | _BV(WGM10);
	TCCR1B = _BV(WGM12) | _BV(CS10);
	OCR1B = 0;
/*
	TCCR1A = 0;
	TCCR1B = _BV(WGM12) | _BV(CS10) | _BV(CS12);
	TIMSK1 = _BV(OCIE1A);
	OCR1A = 255;
	OCR1B = 255;
	*/
}

int main( void )
{
	cli();

	DDRB = _BV(PB7); //pwm output for LED
	DDRD = _BV(PD0); //pwm output for LED
	DDRC = _BV(PC6) | _BV(PC5); //pwm output for LED

	setup_clock();
	setup_pwm();

//	OCR0A = 0; //25%

	sei();

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
