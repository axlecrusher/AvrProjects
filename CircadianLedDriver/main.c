#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <string.h>
#include <avr/sfr_defs.h>

#include <avr/pgmspace.h>

//linear LED steps
char steps[] PROGMEM = { 1, 2, 3, 4, 6, 7, 12, 16, 23, 32, 45, 64, 90, 128, 180, 255 };

/*
White Light
39 blue
90 green
198 red
*/

uint8_t offset = 0;
uint8_t drift = 0;
uint16_t cx = 0;
volatile uint32_t time = 0; //time of day in seconds
volatile char updateAnimate = 0; //time of day in seconds

//use PWM counter overflow to keep track of time
ISR(TIM0_OVF_vect)
{
	//triggers every 256 clock ticks
	//PWM clock runs at 2.5Mhz
	++cx;
	if (cx >= 9765) //overflows per second 9765.625
	{
		cx = 0;
		++time;
		++drift;
		if (drift>=8)
		{
			//account for drift from uneven division of HZ
			cx+=5;
//			++time;
			drift = 0;
		}
		if (time >= 86400) time = 0; //reset time ever 24 hours
		updateAnimate = 1;
//		uint8_t i = (uint8_t)(time);
//		SetR(i);
	}
}

inline uint8_t interpolate(uint8_t start, uint8_t end, uint8_t p) {
	uint16_t x= (end*p)>>8;
	uint16_t y= (start * (256-p))>>8;
	return x+y;
}

uint8_t ComputePercent(uint32_t x, uint32_t y, uint32_t z)
{
	y -= x;
	z -= x; //geater than 0
	uint32_t p = (z<<8)/y;//conver to a range between 0 and 255
	return (uint8_t)p;	
}

uint8_t ComputeTween(uint8_t begin, uint8_t end, uint32_t startTime, uint32_t endTime)
{
	if (time<=startTime) return begin;
	if (time>=endTime) return end;

	uint8_t p = ComputePercent(startTime,endTime,time);

	return interpolate(begin,end,p);
}


void Animate()
{
	SetR(ComputeTween(0,255,0,600));
	SetG(ComputeTween(0,255,600,601));
	updateAnimate = 0;
}

static void setup_clock()
{
	CLKPR = 0x80;	/*Setup CLKPCE to be receptive*/
//	CLKPR = _BV(CLKPS1); //4
	CLKPR = 0x00; //fullspeed
}

void DisablePWM()
{
	if (OCR0A == 0)
	{
		TCCR0A &= ~(_BV(COM0A1)|_BV(COM0A0));
	}
	else
	{
		TCCR0A &= ~(_BV(COM0A1)|_BV(COM0A0));
		TCCR0A |= _BV(COM0A1);
	}
}

void SetR(uint8_t x)
{
	int r = TCCR0A;
	r &= ~(_BV(COM0A0) | _BV(COM0A1));
	if (x>0) r |= _BV(COM0A1);
	OCR0A = x;
	TCCR0A = r;
}

void SetG(uint8_t x)
{
	int r = TCCR0A;
	r &= ~(_BV(COM0B0) | _BV(COM0B1));
	if (x>0) r |= _BV(COM0B1);
	OCR0B = x;
	TCCR0A = r;
}

void SetB(uint8_t x)
{
	int r = TCCR1A;
	r &= ~(_BV(COM1B0) | _BV(COM1B1));
	if (x>0) r |= _BV(COM1B1);
	OCR1B = x;
	TCCR1A = r;
}

void SetUV(uint8_t x)
{
	int r = TCCR1A;
	r &= ~(_BV(COM1A0) | _BV(COM1A1));
	if (x>0) r |= _BV(COM1A1);
	OCR1A = x;
	TCCR1A = r;
}

static void setup_pwm()
{
	//FAST PWM running at cpu speed
	TCCR0A = _BV(WGM01) | _BV(WGM00);
//	TCCR0B = _BV(CS00);//  | _BV(CS01);
	TCCR0B = _BV(CS01);

	//use overflow interrupt as timer to count time that has passed
	TIMSK0 = _BV(TOIE0);
	TIFR0 = _BV(TOV0);

	//seems to look like 6000K
//	OCR0A = 198; //red
//	OCR0B = 90; //green
//	OCR1A = 0; //uv

	OCR0A = 0; //red
	OCR0B = 0; //green
	OCR1B = 0; //blue
	OCR1A = 0;

/*
OCR0A = 0;

	OCR0B = 100; //green
	OCR1A = 200; //red
*/
/*
	OCR0A = 0; //blue
	OCR0B = 255; //green
	OCR1A = 0; //red
*/
	//FAST PWM at cpu speed, 8 bit
	TCCR1A = _BV(WGM10);
//	TCCR1B = _BV(WGM12) | _BV(CS10);
	TCCR1B = _BV(WGM12) | _BV(CS11);
//	OCR1B = 0;
/*
	TCCR1A = 0;
	TCCR1B = _BV(WGM12) | _BV(CS10) | _BV(CS12);
	TIMSK1 = _BV(OCIE1A);
	OCR1A = 255;
	OCR1B = 255;
	*/
/*
	SetR(255);
	SetG(255);
	SetB(255);
	*/
}

int main( void )
{
	cli();

	DDRB = _BV(PB2); //pwm output for LED
	DDRA = _BV(PA6) | _BV(PA7) | _BV(PA5) ; //pwm output for LED

	setup_clock();
	setup_pwm();

//	OCR0A = 0; //25%

	sei();

	while(1)
	{
		if (updateAnimate)
		{
			Animate();
		}
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
