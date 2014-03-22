#include <util/delay.h>
#include "ds18s20.h"

//would be great if we can use a timer rather than cpu delay

void ds18s20_setup_pins()
{
	ds18s20_txmode(); //output mode
	ds18s20_high();
}

void ds18s20_reset()
{
	ds18s20_txmode();
	ds18s20_low();
	_delay_us(480);
	ds18s20_high();
}

uint8_t ds18s20_init()
{
	ds18s20_reset();
	ds18s20_rxmode();
	_delay_us(60); //wait for device reply to start

	//check for low signal from device
	uint8_t r = DS18S20_BUS_IN & _BV(DS18S20_BUS_PIN);

	_delay_us(420);

	return r;
}

void ds18s20_write0()
{
	ds18s20_txmode();
	ds18s20_low();
	_delay_us(60);
	ds18s20_high();
	_delay_us(1); //1us recovery
}

void ds18s20_write1()
{
	ds18s20_txmode();
	ds18s20_low();
	_delay_us(5);
	ds18s20_high();
	_delay_us(56); //55 + 1us recovery
}

uint8_t ds18s20_read()
{
	ds18s20_txmode();
	ds18s20_low();
	_delay_us(1);
	ds18s20_high();
	ds18s20_rxmode();

	uint8_t r = DS18S20_BUS_IN & _BV(DS18S20_BUS_PIN);
	_delay_us(60); //finish 60us minimum plus 1us recover

	if( r>0 ) return 1;
	return 0;
}

uint8_t ds18s20_read_byte()
{
	uint8_t b = 0;
	if (ds18s20_read()>0) b|=0x01;
	if (ds18s20_read()>0) b|=0x02;
	if (ds18s20_read()>0) b|=0x04;
	if (ds18s20_read()>0) b|=0x08;
	if (ds18s20_read()>0) b|=0x10;
	if (ds18s20_read()>0) b|=0x20;
	if (ds18s20_read()>0) b|=0x40;
	if (ds18s20_read()>0) b|=0x80;
	return b;
}

uint8_t ds18s20_send(uint8_t hex)
{
	//least signigicant bit first
	for (int i = 0; i < 8; ++i)
	{
		if ((hex & 0x01)>0)
			ds18s20_write1();
		else
			ds18s20_write0();
		hex>>=1;
	}
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
