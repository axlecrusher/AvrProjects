#include <stdlib.h>
#include <avr/interrupt.h>
#include "mytypes.h"
#include "SPIPrinting.h"
#include "defines.h"

#include "motor_functions.h"

extern vuint8_t direction; //why do I need this? Why not set the port directly? port can be overwritten depending on masks
//extern vint8_t y_tmp;
//extern vint8_t x_tmp;
extern vuint8_t MOTOR_PWM_MASK;
extern vuint32_t gtmp1;


void dec_forward() {
	if ((direction & DEC_FORWARD)>0) return;
	cli();
		PORTD &= ~DEC_MOTORMASK; //per motor controller instructions, STOP before switching inputs
		direction &= ~DEC_MOTORMASK;
		direction |= DEC_FORWARD;
	sei();
}

void dec_backward() {
	if ((direction & DEC_BACKWARD)>0) return;
	cli();
		PORTD &= ~DEC_MOTORMASK; //per motor controller instructions, STOP before switching inputs
		direction &= ~DEC_MOTORMASK;
		direction |= DEC_BACKWARD;
	sei();
}

void dec_stop() {
	cli();
		PORTD &= ~DEC_MOTORMASK; //stop motor now
		MOTOR_PWM_MASK &= ~DEC_MOTORMASK;
	sei();
}

void ra_forward() {
	if ((direction & RA_FORWARD)>0) return;
	cli();
		PORTD &= ~RA_MOTORMASK; //per motor controller instructions, STOP before switching inputs
		direction &= ~RA_MOTORMASK;
		direction |= RA_FORWARD;
	sei();
}

void ra_backward() {
	if ((direction & RA_BACKWARD)>0) return;
	cli();
		PORTD &= ~RA_MOTORMASK; //per motor controller instructions, STOP before switching inputs
		direction &= ~RA_MOTORMASK;
		direction |= RA_BACKWARD;
	sei();
}

void ra_stop() {
	cli();
		PORTD &= ~RA_MOTORMASK; //stop motor now
		MOTOR_PWM_MASK &= ~RA_MOTORMASK;
	sei();
}

//0 to 65535
void set_ra_pwm(uint16_t t)
{
	if (t==0) {
		ra_stop();
		OCR1A = 0;//Output Compare Register 1 A
	}
	else {
		OCR1A = t;//Output Compare Register 1 A
		cli();
			MOTOR_PWM_MASK |= RA_MOTORMASK;
		sei();
	}
}

void set_dec_pwm(uint16_t t)
{
	if (t==0) {
		dec_stop();
		OCR1B = 0;//Output Compare Register 1 B
	}
	else {
		//SPIPutChar('b');	
		OCR1B = t;//Output Compare Register 1 B
		cli();
			MOTOR_PWM_MASK |= DEC_MOTORMASK;
		sei();
	}
}

void slew_ra(int32_t *dx)
{
	uint32_t adx = labs(*dx);
 
	if (*dx < 0 )
		ra_backward();
	else
		ra_forward();

	if (adx<0xF)	
		set_ra_pwm(0);
	else if (adx<0x2000)
		set_ra_pwm(PWM_MIN*3);
	else if (adx<0x10000)
		set_ra_pwm(PWM_MIN*8);
	else
		set_ra_pwm(0xffff);
		
}

void slew_dec(int32_t *dx)
{
	uint32_t adx = labs(*dx);

	if (*dx < 0 )
		dec_backward();
	else
		dec_forward();

	if (adx<0xF)	
		set_dec_pwm(0);
	else if (adx<0x2000)
		set_dec_pwm(PWM_MIN*3);
	else if (adx<0x10000)
		set_dec_pwm(PWM_MIN*8);
	else
		set_dec_pwm(0xffff);
}

void jog_ra(int8_t x)
{
	int16_t t = abs(x);
	if (x==0) { 
		set_ra_pwm(0);
		return;
	}

	if (x < 0)
		ra_backward();
	else
		ra_forward();

	t <<= 9;
	t |= 0x1FF;

	set_ra_pwm(t);
}

void jog_dec(int8_t x)
{
	int16_t t = abs(x);

	if (x==0) {
		set_dec_pwm(0);
		return;
	}

	if (x < 0)
		dec_backward();
	else
		dec_forward();

	t <<= 9;
	t |= 0x1FF;

	set_dec_pwm(t);
}