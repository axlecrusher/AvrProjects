#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <string.h>
#include <avr/sfr_defs.h>

//jitter seems to be 4237hz fast
const uint16_t SECONDTICKS = 62500;// + 17;

//clock seems to 102hz slower than it should
//jump one timer tick every 2.5 seconds
//or jump ticks 2 ever 5 seconds
uint8_t seconds = 0;
uint16_t drifted = 0;

ISR( TIMER1_COMPA_vect )
{
	if ((PINC & _BV(PC6)) > 0)
	{
		//light toggled on, prepare to toggle off
		TCCR1B &= ~_BV(WGM12); //CTC off
		OCR1A = 12500; //200ms
		seconds++;
	}
	else
	{
		//after light is toggled off, prepare to toggle it on again
		TCCR1B |= _BV(WGM12); //CTC on
		OCR1A = SECONDTICKS;
/*
		drifted += 102;
		if (drifted >= 256)
		{
			drifted -= 256;
			OCR1A++;
		}
*/

		if (seconds == 5)
		{
			OCR1A+=2;
			seconds = 0;
		}

	}
}

int main( void )
{
	cli();

	CLKPR = 0x80;	/*Setup CLKPCE to be receptive*/
	CLKPR = 0x00;	/*No scalar, set clock divisor here*/

	DDRC = _BV(PC6); //output on PC6

	//16Mhz with clock scalar at 256
	//every 31250 clock ticks is 1/2 second
	//toggle PC6 on every counter match
	//on exactly at 1 second intervals
	TCCR1A = _BV(COM1A0); //toggle OC1A (PC6)
	TCCR1B = _BV(CS12) | _BV(WGM12); // CTC, 256 scalar
	OCR1A = SECONDTICKS; //clock ticks
	TIMSK1 = _BV(OCIE1A);


	sei();

	while(1);

	return 0;
}

/*

Copyright (c) 2012 Joshua Allen

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
