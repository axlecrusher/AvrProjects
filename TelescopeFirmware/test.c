#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <string.h>
#include <avr/sfr_defs.h>
#include <stdlib.h>
#include <util/atomic.h>
#include "avr_print.h"

#include "usb.h"
#include "avrUsbUtils.h"

#include "mytypes.h"
#include "defines.h"

#include "motor_functions.h"

vuint8_t direction; //why do I need this? Why not set the port directly? port can be overwritten depending on masks

vuint8_t MOTOR_PWM_MASK = 0; //mask to enable specific motors

 //AVR has 512 bytes os ram available

// 0xf8900 encoder ticks

extern vuint8_t usbHasEvent;

vuint32_t x_pos = 0x0; //right ascension
vuint32_t x_dest = 0x0; //right ascension

vuint32_t y_pos = 0x0; //declination
vuint32_t y_dest = 0x0; //declination
vuint32_t gtmp1 = 0xdeadbeef;

vint8_t jog_value_dec = 0x00;
vint8_t jog_value_ra = 0x00;

// 8bit variables for faster interrupt math
vint8_t y_tmp = 0x00;
vint8_t x_tmp = 0x00;


static void SetupInterruptPins()
{
	EICRA = _BV(ISC01) | _BV(ISC00) | _BV(ISC11) | _BV(ISC10) | _BV(ISC21) | _BV(ISC20) | _BV(ISC31) | _BV(ISC30);
	EIMSK = _BV(INT2) | _BV(INT3) | _BV(INT0) | _BV(INT1);
}

static void SetupDriverPins()
{
//	DDRD = _BV(PD0) | _BV(PD1);// | _BV(PD6);
	DDRD = _BV(PD6) | _BV(PD7) | _BV(PD5) | _BV(PD4);
	PORTD = 0;
}

static void setup_clock()
{
	CLKPR = 0x80;	/*Setup CLKPCE to be receptive*/
	CLKPR = 0x00; //no divisor
}

static void setup_timers()
{
	//16mhz / 256
//	TCCR1A = _BV(WGM11) | _BV(WGM10);//Toggle OC1A
	TCCR1B =  _BV(CS11); //CTC, 256 divisor, 62.5kHz

	TIMSK1 = _BV(OCIE1A) | _BV(TOIE1) | _BV(OCIE1B);

	set_ra_pwm(0);
	set_dec_pwm(0);
}

int32_t ComputeOffset(vuint32_t* pos, vuint32_t* dest )
{
	uint32_t p,d;

	cli();
		p = *pos;
		d = *dest;
	sei();

	return (d - p);
}

int main( void )
{
	int32_t tmp = 0x0;
//	uint8_t tmp8;

	cli();

	SetupInterruptPins();

	y_pos = 0;
	x_pos = 0;

//	DDRC = _BV(PC6); //output pin
	SetupDriverPins();

	setup_clock();
	setup_timers();
	SetupPrintf();

	USB_ZeroPrescaler();
	USB_Init();

	sei();

	printf("hello world");

//	motorflags |= 0x01;
//	set_motor_pwm(PWM_MAX);
//	forward();
//	while(1);

//AutoSlop();

/*
AutoSlop();
while(1);
*/

//		dec_backward();
//		set_dec_pwm(0x1fff);
//		ra_forward();
//		set_ra_pwm(0x3fff);
	while(1)
	{
		cli();
//		tmp8 = usbHasEvent;
		//add accumulated radial encoder output
		y_pos += y_tmp;
		y_tmp = 0;

		x_pos += x_tmp;
		x_tmp = 0;
		sei();

		if (usbHasEvent) ProcessUSB();

		if (jog_value_dec == 0) {
			tmp = ComputeOffset(&y_pos,&y_dest);
			slew_dec(&tmp);
		}

		if (jog_value_ra == 0) {
			tmp = ComputeOffset(&x_pos,&x_dest);
			slew_ra(&tmp);
		}
	}
	return 0;
}

/*

Copyright (c) 2015 Joshua Allen

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
