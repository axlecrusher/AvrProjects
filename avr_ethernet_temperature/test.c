#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <string.h>
#include <avr/sfr_defs.h>
#include <avr_print.h>
#include <avr/pgmspace.h>

#include "ds18s20.h"

static void setup_clock()
{
	CLKPR = 0x80;	/*Setup CLKPCE to be receptive*/
	CLKPR = 0x00;// | _BV(CLKPS1); //no divisor
}

int main( void )
{
	cli();
	setup_clock();
	ds18s20_setup_pins();
	setup_spi();
	sei();

	while (1)
	{
		ReadTemp();
	}
	return 0;
}

void ReadTemp()
{
	struct ds18s20_memory mem;
	uint8_t* mPtr = (uint8_t*)(&mem);

	uint8_t r = ds18s20_init();
	if (r==0) sendstr("Found device\n");

	sendstr("T convert... ");
	ds18s20_send(0xcc); //skip ROM
	ds18s20_send(0x44); //T convert, 750ms max

	while (ds18s20_read()==0);
	sendstr("done!\n");

	char data[8*9];

	r = ds18s20_init();
//	if (r==0) sendstr("Found device\n");
	sendstr("Read temperature\n");
	ds18s20_send(0xcc); //skip ROM
	ds18s20_send(0xbe); //red scratchpad

	for (int i=0;i<8;++i)
		mPtr[i] = ds18s20_read_byte();

	int16_t t = mem.temperature/2; //celcius

	//compute the entended bits in fixed point with 4 decimal places
	//this probably only works for values greater than 0
	uint16_t b = mem.count_per_c - mem.count_remain;//0-15 as a remainder
	b*=666; // 1/15 = 0.066666...
//	if (t<0) { b-= 10000; b*=-1;}

//	printf("0x%x %d.%d C\n", mem.temperature, t, (mem.temperature&0x01)*5);
	printf("0x%x %d.%04d C\n", mem.temperature, t, b);
}


/*

Copyright (c) 2014 Joshua Allen

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
