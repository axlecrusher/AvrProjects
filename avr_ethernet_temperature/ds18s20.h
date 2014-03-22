#ifndef DS18S20_H
#define DS18S20_H

#include <avr/io.h>

#define DS18S20_BUS_PIN PD6
#define DS18S20_BUS_DIR DDRD
#define DS18S20_BUS_IN PIND
#define DS18S20_BUS_OUT PORTD

static inline void ds18s20_low() { DS18S20_BUS_OUT &= ~_BV(DS18S20_BUS_PIN); }
static inline void ds18s20_high() { DS18S20_BUS_OUT |= _BV(DS18S20_BUS_PIN); }

static inline void ds18s20_txmode() { DS18S20_BUS_DIR |= _BV(DS18S20_BUS_PIN); }
static inline void ds18s20_rxmode() { DS18S20_BUS_DIR &= ~_BV(DS18S20_BUS_PIN); }

void ds18s20_setup_pins();

void ds18s20_reset();

//return 0 if devices found
uint8_t ds18s20_init();

uint8_t ds18s20_send(uint8_t hex);
uint8_t ds18s20_read();

uint8_t ds18s20_read_byte();

struct ds18s20_memory
{
	int16_t temperature; //Q14.1 fixed point format
	uint8_t th,tl;
	uint16_t reserved;
	uint8_t count_remain;
	uint8_t count_per_c;
	uint8_t crc;
} __attribute__ ((__packed__));

#endif

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
