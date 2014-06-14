#include "Functions.h"

const char ALLOWMOTOR = 0x0F;

const char MINDELAY = 2;
const unsigned char MAXSTEPS = 200;
const char STEPORDER4[4] = {1,8,2,4};
const char STEPORDER8[8] = {1,9,8,10,2,6,4,5};
/*const int STEPPATTERN[2] = {9503,9503}; Average step time of 95.03ms */

#define STEPPATTERNLENGTH 7
const int STEPPATTERN[STEPPATTERNLENGTH] = {9138,9139,9138,9138,9138,9138,9138}; /* Average step time of 91.381428571429ms, really close to the required 91.38138124222ms */

unsigned char STEPPOSITION = 0;
 void (*MotorCallback)() = 0;

unsigned char HalfStep(unsigned char direction)
{
	STEPPOSITION  &= 7;
	PORTA = (PORTA & 0xf0) | (STEPORDER8[STEPPOSITION] & ALLOWMOTOR);
	STEPPOSITION = direction==FORWARD?STEPPOSITION-1:STEPPOSITION+1;
	return PORTA & 0x0f;
}

unsigned char FullStep(unsigned char direction)
{
	STEPPOSITION  &= 3;
	PORTA = (PORTA & 0xf0) | (STEPORDER4[STEPPOSITION] & ALLOWMOTOR);
	STEPPOSITION = direction==FORWARD?STEPPOSITION-1:STEPPOSITION+1;
	return PORTA & 0x0f;
}

void PowerDownMotor()
{
/*	PORTA &= ~LED; */
	PORTA = (PORTA & 0xf0) | (0 & ALLOWMOTOR);
}

void PowerUpMotor(unsigned char bits)
{
/*	PORTA |= LED; */
	PORTA = (PORTA & 0xf0) | (bits & ALLOWMOTOR);
}

void BlinkLight(unsigned int ms)
{
	static unsigned int c = 0;
	if (c==ms)
	{
		PORTA ^= LED;
		c = 0;
	}
	++c;
}

void SideRealClbk()
{
	/* runs every 10 microseconds so this needs to be as fast as possible */
	/* 10 milliseconds seems to be the amount of time needed to power up the motor */
	static unsigned char pidx = 0;
	static unsigned int tick = 0;
	static unsigned char lastStep = 0;

	++tick;

	if (tick == 1000)
	{
		PowerDownMotor();
	}
	else if (tick+1000 == STEPPATTERN[pidx])
	{
		PowerUpMotor(lastStep);
	}
	else if (tick == STEPPATTERN[pidx])
	{
		lastStep = HalfStep(FORWARD);
		pidx = (pidx+1)%STEPPATTERNLENGTH;
		tick = 0;
	}
}

void RewindClbk()
{
	/* runs every 10 microseconds */
	/* Only rewind every 2ms */
	static unsigned char tick = 0;
	if ((++tick)==200)
	{
		HalfStep(BACKWARDS);
		tick = 0;
	}
}

bool CheckSwitch(unsigned char sw, unsigned char bit)
{
	unsigned char counter = 0;
	while (sw & bit)
	{
		++counter;
		if (counter >= 10) return true;
		_delay_ms(1);
	}
	return false;
}

void TooSlowClbk()
{
	PORTA |= LED;
}

/*

Copyright (c) 2011, Joshua Allen
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Joshua Allen nor the names of other contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
