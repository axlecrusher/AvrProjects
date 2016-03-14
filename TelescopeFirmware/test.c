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

//1015697 //y,declination ticks per 360 degrees

#define STEPCOUNT 6
//uint8_t steps[STEPCOUNT] = { 0x01, 0x03, 0x02, 0x06, 0x04, 0x05 };
//uint8_t i = 0;

#define SLOP_PWM_MIN 1024

#define PWM_MAX 0xffff
#define PWM_MIN 2448
//#define PWM_MIN 4896

//_BV(PD0) | _BV(PD1)
//#define MOTORMASK (_BV(PD0) | _BV(PD1))

//_BV(PD6) | _BV(PD7)

#define MOTORMASK (_BV(PD4) | _BV(PD5) | _BV(PD6) | _BV(PD7))

#define RA_MOTORMASK (_BV(PD4) | _BV(PD5))
#define DEC_MOTORMASK (_BV(PD6) | _BV(PD7))

#define DEC_BRAKE (_BV(PD6) | _BV(PD7))
#define RA_BRAKE (_BV(PD4) | _BV(PD5))

#define STOP 0x00

//#define DEC_STOP PORTD &= ~DEC_MOTORMASK
//#define RA_STOP  PORTD &= ~RA_MOTORMASK

#define DEC_FORWARD _BV(PD6)
#define DEC_BACKWARD _BV(PD7)

#define RA_FORWARD _BV(PD4)
#define RA_BACKWARD _BV(PD5)

vuint8_t direction; //why do I need this? Why not set the port directly?

#define MOTOR_FLAG_ON 0x01
vuint8_t motorflags = 0;

 //AVR has 512 bytes os ram available

// 0xf8900 encoder ticks

extern vuint8_t usbHasEvent;

vuint32_t x_pos = 0x0;
vuint32_t y_pos = 0x0;
vuint32_t x_dest = 0x0;
vuint32_t y_dest = 0x0;
vuint32_t gtmp1 = 0;
vint8_t jog_value_dec = 0x00;
vint8_t jog_value_ra = 0x00;

// 8bit variables for faster interrupt math
vint8_t y_tmp = 0x00;
vint8_t x_tmp = 0x00;

// two output pins need to be driven with pwm so do our own handling of pin toggling using counter interrupts
ISR(TIMER1_OVF_vect)
{
	//interrupt to turn on PWM pulse
	PORTD = (PORTD & ~MOTORMASK) | direction;
}

ISR(TIMER1_COMPA_vect)
{
	//declination PWM
	//interrupt to turn off PWM pulse

//	PORTD = (PORTD & ~RA_MOTORMASK); //STOP
	PORTD = (PORTD & ~DEC_MOTORMASK) | DEC_BRAKE;
}

//XXXX compare value B still needs to be set up
ISR(TIMER1_COMPB_vect)
{
	//right ascension PWM
	//interrupt to turn off PWM pulse

//	PORTD = (PORTD & ~RA_MOTORMASK); //STOP
	PORTD = (PORTD & ~RA_MOTORMASK) | RA_BRAKE;
}

ISR(INT0_vect)
{
	if ((PIND & (_BV(PD0) | _BV(PD1))) == _BV(PD0))
		x_tmp++;
}

ISR(INT1_vect)
{
	if ((PIND & (_BV(PD0) | _BV(PD1))) == _BV(PD1))
		x_tmp--;
}

ISR(INT2_vect)
{
	//declination
	if ((PIND & (_BV(PD2) | _BV(PD3))) == _BV(PD2))
		y_tmp++;
}

ISR(INT3_vect)
{
	//declination
	if ((PIND & (_BV(PD2) | _BV(PD3))) == _BV(PD3))
		y_tmp--;
}

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

//0 to 65535
void set_motor_pwm(uint16_t t)
{
	if (!(motorflags && MOTOR_FLAG_ON))
	{
		//disable interrupt
		TIMSK1 &= ~_BV(TOIE1);
	}
	else if (t==0)
	{
		//disable interrupt
		TIMSK1 &= ~_BV(TOIE1);
	}
	/*
	else if (t<SLOP_PWM_MIN) 
	{
		OCR1A = SLOP_PWM_MIN;
		TIMSK1 |= _BV(TOIE1);
	}
	*/
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

void dec_forward() {
	cli();
		PORTD &= ~DEC_MOTORMASK; //per motor controller instrucionts, STOP before switching inputs
		direction |= DEC_FORWARD;
	sei();
}

void dec_backward() {
	cli();
		PORTD &= ~DEC_MOTORMASK; //per motor controller instrucionts, STOP before switching inputs
		direction |= DEC_BACKWARD;
	sei();
}

void dec_stop() {
	cli();
		direction &= ~DEC_MOTORMASK;
	sei();
}

void ra_forward() {
	cli();
		PORTD &= ~RA_MOTORMASK; //per motor controller instrucionts, STOP before switching inputs
		direction |= RA_FORWARD;
	sei();
}

void ra_backward() {
	cli();
		PORTD &= ~RA_MOTORMASK; //per motor controller instrucionts, STOP before switching inputs
		direction |= RA_BACKWARD;
	sei();
}

void ra_stop() {
	cli();
		direction &= ~RA_MOTORMASK;
	sei();
}

void slew(int32_t *dx)
{
	uint32_t adx = labs(*dx);
 
	if (*dx < 0 )
		backward();
	else
		forward();

	if (adx<0xF)	
		set_motor_pwm(0);
	else if (adx<0x2000)
		set_motor_pwm(PWM_MIN*3);
	else if (adx<0x10000)
		set_motor_pwm(PWM_MIN*8);
	else
		set_motor_pwm(0xffff);
		
}

void jog(int8_t dec, int8_t ra)
{
	direction=0x0;
	if (dec < 0)
		dec_backward();
	else
		dec_forward();
/*
	if (y>0) {
		cli();
		direction = _BV(PD5);
		sei();
	}
	else {
		cli();
		direction = _BV(PD4);
		sei();

	}
*/
	int16_t t = abs(x);
	int16_t t2 = abs(y);
	if (t2>t) t = t2;

	t <<= 9;
	t |= 0x1FF;

	gtmp1 = t;

	set_motor_pwm(t);
}
/*
void FindBackwardSlop() {
	forward();
	FindSlopRun();
}

void FindForwardSlop() {
	backward();
	FindSlopRun();
}

void FindSlopRun() {
	set_motor_pwm(PWM_MAX);
	_delay_ms(250);
	set_motor_pwm(PWM_MIN*8);
	_delay_ms(250);
	set_motor_pwm(PWM_MIN*3);
	_delay_ms(250);
	set_motor_pwm(PWM_MIN);
	_delay_ms(250);
	set_motor_pwm(0);
	_delay_ms(250);
	y_pos = 0;

	//switch direction
	if (direction==DEC_FORWARD)
		backward();
	else
		forward();

	_delay_ms(250);
	set_motor_pwm(500); //backtrack slowly
	_delay_ms(1000);
	set_motor_pwm(0);
}

void AutoSlop()
{
	motorflags |= 0x01;

	FindForwardSlop();
}
*/
int32_t ComputeOffset(vuint32_t* pos, vuint32_t* dest )
{
	uint32_t p,d;

	cli();
		p = *pos;
		d = *dest;
	sei();

	return (d - p);
}

void UpdateYPos() {

}

int main( void )
{
	int32_t tmp = 0x0;
	uint8_t tmp8;

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

//	motorflags |= 0x01;
//	set_motor_pwm(PWM_MAX);
//	forward();
//	while(1);

//AutoSlop();

/*
AutoSlop();
while(1);
*/
	while(1)
	{
		cli();
//		tmp8 = usbHasEvent;
		//add accumulated radial encoder output
		y_pos += y_tmp;
		y_tmp = 0;

		x_pos += x_tmp;
		x_tmp = 0;
//		x_pos=4321;
		sei();

		if (usbHasEvent) ProcessUSB();

		tmp = ComputeOffset(&y_pos, &y_dest);

		if ((jog_value_dec == 0) && (jog_value_ra ==0))
		{
			gtmp1 = tmp;
			slew(&tmp);
		}
		else
		{
			jog(jog_value_dec,jog_value_ra);
		}
/*
		sendhex8(&dy);
		sendchr('\n');
		*/
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
