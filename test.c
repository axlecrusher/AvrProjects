//Copyright 2012 Joshua Allen

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "usb.h"
#include "SPIPrinting.h"
#include <stdio.h>

#include <d4164.h>

//uint8_t dataString[257];

void TestWires()
{
	// Test DRAM wiring first. Can identify faulty address wires.
	uint8_t i;
	uint8_t r = 0;
	for (i = 0x01;;i<<=1)
	{
		WriteBit(0x00, 0x00, 0x00);
		WriteBit(i, 0x00, 0xFF);
		r = ReadBit(0x00, 0x00);

		if (r != 0x00)
		{
			printf("\nread failed at 0x00 0x00, corrupted by writing to %02X %02X\n", i, 0);
			PORTB &= ~_BV(4); //LED OFF
			for (;;);
		}
		r = ReadBit(i, 0x00);

		if (r != 0xFF)
		{
			printf("\nread failed at %02X %02X\n", i, 0);
			PORTB &= ~_BV(4); //LED OFF
			for (;;);
		}
	
		if (i >= 0x80) break;
	}
}


int main()
{
	cli(); //disable interrupts
//	DDRD = 1; //wtf?

	//Don't touch any PORTB below 4, those are for printf
	SetupPrintf();

//	USB_ZeroPrescaler();
//	USB_Init();

	//no clock divisor
        CLKPR=0x80;
        CLKPR=0x00;

	//set output pins
	DDRB |= _BV(7) | _BV(6) | _BV(4);
	DDRC |= 0xF0;
	DDRD |= 0x7F;

	//ras high
	//cas high
	//we high
	PORTC |= CAS | WE | RAS;

	PORTB |= _BV(4); //LEDON
	_delay_ms(50);

//	PORTB = 0x00; //all 
//	PORTC = 0x00; //all input
//	PORTD = 0x00; //all input
	
	sei();	//Safe to leave lockdown...
	uint8_t b,r;

	uint8_t ri,ci;
	ri=ci=0;


	TestWires();

	b = 0x01;

	for(;;)
	{
		//walking 1 algorithim
		b = (b >> 7) | (b << 1); // rotate left 

		for (ri = 0; ; ++ri)
		{
			WriteRow(ri,b);
			WriteRow(ri,~b); //bit inversion
			if (ri >= 0xFF) break;
		}

		for (ri = 0; ; ++ri)
		{
			ReadRow(ri,~b);
			if (ri >= 0xFF) break;
		}

		PORTB &= ~_BV(4); //LEDON
		_delay_ms(80);	
		PORTB |= _BV(4); //LEDON
	}

	return 0;
}
