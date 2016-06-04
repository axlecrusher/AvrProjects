#include <avr/interrupt.h>
#include "mytypes.h"

#include "defines.h"

extern vuint8_t direction; //why do I need this? Why not set the port directly? port can be overwritten depending on masks
extern vint8_t y_tmp;
extern vint8_t x_tmp;
extern vuint8_t MOTOR_PWM_MASK;

// two output pins need to be driven with pwm so do our own handling of pin toggling using counter interrupts
ISR(TIMER1_OVF_vect)
{
	//overflow interrupt used to turn on motor
	//interrupt to turn on PWM pulse
	PORTD = (PORTD & ~MOTORMASK) | (direction & MOTOR_PWM_MASK);
}

ISR(TIMER1_COMPA_vect)
{
	//declination PWM
	//interrupt to turn off PWM pulse

	PORTD = (PORTD & ~RA_MOTORMASK); //STOP
//	PORTD = (PORTD & ~RA_MOTORMASK) | DEC_BRAKE;
}

//XXXX compare value B still needs to be set up
ISR(TIMER1_COMPB_vect)
{
	//right ascension PWM
	//interrupt to turn off PWM pulse

	PORTD = (PORTD & ~DEC_MOTORMASK); //STOP
//	PORTD = (PORTD & ~DEC_MOTORMASK) | RA_BRAKE;
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
