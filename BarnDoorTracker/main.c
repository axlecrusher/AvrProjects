#include "Functions.h"

#define DEBUG_SPEED 0

/* inches */
#define BOARD_LENGTH 18.8125f

/* radians per 1/100th second */
#define RAD_TICK 0.000000729f

/* stepper motor steps per inch, 20 threads/inch, 200 steps/thread */
#define STEPS_INCH 4000

uint32_t time_ticks = 0; /* 1/100th seconds */
uint32_t completed_steps = 0;
uint32_t total_steps = 0;

ISR( TIM1_COMPA_vect )
{
	++time_ticks;
	PORTA ^= LED;
	if (completed_steps<total_steps)
	{
		FullStep(FORWARD);
		++completed_steps;
	}
}

static void timer_init( void )
{
	TCCR1A = _BV(WGM10); /* CTC */
	TCCR1B = _BV(CS11) | _BV(CS10); /* 64 divisor */
	OCR1A = 3125; /* 1/100th of a second */
	TIMSK1 = _BV(OCIE1A); /* enable interrupt */
}

static void setup_clock( void )
{
	/*Examine Page 33*/

	CLKPR = 0x80;	/*Setup CLKPCE to be receptive*/
	CLKPR = 0x00;	/*No scalar*/
}

void PowerOnLEDSequence()
{
	unsigned char i;
	PORTA |= LED;
	for (i=0; i<4;++i) _delay_ms(250);
	PORTA &= ~LED;
}

void CheckSwitches()
{
	if ( CheckSwitch(PINA, 1<<PA7) == true )
		MotorCallback = RewindClbk;			
	else
		MotorCallback = SideRealClbk;

	/* Stub for checking the 2nd switch. */
	if ( CheckSwitch(PINB, 1<<PB2) == true )
		PORTA |= LED;
	else
		PORTA &= ~LED;
}

float ComputeScrewLength(float radians)
{
	return sqrt((BOARD_LENGTH*BOARD_LENGTH) + (BOARD_LENGTH*BOARD_LENGTH) - (2*BOARD_LENGTH*BOARD_LENGTH*cos(radians)));
}

int main( void )
{
	float l,steps;
	cli();  /* disable interupts */
	DDRA = 0x2f;  /* do not change */
	DDRB = 0x00;  /* do not change */
	PORTA = 0x00;

	setup_clock();
/*	sei(); */

	PowerOnLEDSequence();

	cli();
	timer_init();
	sei();  /* enable interupts */

	while (1)
	{
		l = ComputeScrewLength(RAD_TICK*(time_ticks+1)); //compute 1 timer tick ahead
		steps = round(l*STEPS_INCH);
		if (steps>total_steps) total_steps = steps;
	}

	return 0;
}

/*

Copyright (c) 2014, Joshua Allen
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Joshua Allen nor the names of other contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
