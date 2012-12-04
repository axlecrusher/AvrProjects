#include <avr/interrupt.h>

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
	TCCR1A = _BV(COM1A0);
	TCCR1B = _BV(CS12) | _BV(WGM12); // CTC, 256 scalar
	OCR1A = 31250; //clock ticks
	TCNT1 = 31250;

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
