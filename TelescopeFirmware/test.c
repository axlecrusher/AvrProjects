#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <string.h>
#include <avr/sfr_defs.h>
#include <stdlib.h>
//#include "avr_print.h"

#include "usb.h"
#include "avrUsbUtils.h"

#define STEPCOUNT 6
//uint8_t steps[STEPCOUNT] = { 0x01, 0x03, 0x02, 0x06, 0x04, 0x05 };
//uint8_t i = 0;

#define PWM_MAX 0xffff
#define PWM_MIN 2448
//#define PWM_MIN 4896

#define MOTORMASK 0x03
#define FORWARD 0x01
#define BACKWARD 0x02
#define STOP 0x00

volatile uint32_t rcount;
volatile uint8_t direction;

// two output pins need to be driven with pwm so do our own handling of pin toggling using counter interrupts
ISR(TIMER1_OVF_vect)
{
	//interrupt to turn on PWM pulse
	PORTD = (PORTD & ~MOTORMASK) | direction;
}

ISR(TIMER1_COMPA_vect)
{
	//interrupt to turn off PWM pulse
	PORTD = (PORTD & ~MOTORMASK) | 0x03; //brake mode
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

//0 to 65535
static void set_motor_pwm(uint32_t t)
{
	if (t==0)
	{
		//disable interrupt
		TIMSK1 &= ~_BV(TOIE1);
	}
	else if (t<PWM_MIN) 
	{
		OCR1A = PWM_MIN;
		TIMSK1 |= _BV(TOIE1);
	}
	else
	{
		OCR1A = t;		
		TIMSK1 |= _BV(TOIE1);
	}
}

static void setup_timers()
{
	//16mhz / 256
//	TCCR1A = _BV(WGM11) | _BV(WGM10);//Toggle OC1A
	TCCR1B =  _BV(CS11); //CTC, 256 divisor, 62.5kHz

	TIMSK1 = _BV(OCIE1A) | _BV(TOIE1);

	set_motor_pwm(0);
}

static void SetupDriverPins()
{
	DDRD = _BV(PD0) | _BV(PD1) | _BV(PD6);
	PORTD = 0;
}

void forward() {
//	PORTD &= ~_BV(PD1);
//	PORTD |= _BV(PD0);
	cli();
	direction = FORWARD;
	sei();
}

void backward() {
//	PORTD &= ~_BV(PD0);
//	PORTD |= _BV(PD1);
	cli();
	direction = BACKWARD;
	sei();
}

void stop() {
	PORTD &= ~(_BV(PD1)|_BV(PD2));
	direction = STOP;
}

void slew(int32_t *dx)
{
	uint32_t adx = labs(*dx);
/*
		sendhex8(dx);
		sendchr(':');
		sendhex8(&adx);
		sendchr('\n');
*/
	if (*dx < 0 )
		backward();
	else
		forward();

	if (adx<0xF)	
		set_motor_pwm(0);
	else if (adx<0x800)
		set_motor_pwm(PWM_MIN);
	else if (adx<0x1000)
		set_motor_pwm(PWM_MIN*2);
	else
		set_motor_pwm(0xffff);
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
//	setup_spi();

	USB_ZeroPrescaler();
	USB_Init();

	sei();

	uint32_t t = 0;
	uint32_t y_pos = 0x10000 * 2;
	forward();
//	while(1);
	set_motor_pwm(1);

	while(1)
	{
		UENUM = 1; //interrupts can change this
		if ( USB_READY(UEINTX) )
		{

		}


//		PORTD &= ~_BV(PD6);
		cli();
		t=rcount;
//		t &= 0xffffff00;
		sei();

//		if (t>0xff000000) t = 0;

		int32_t sy = (y_pos - t);
		slew(&sy);
/*
		sendhex8(&dy);
		sendchr('\n');
		*/
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
