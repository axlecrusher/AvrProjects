//Copyright 2012 Joshua Allen

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
//#include "usb.h"
#include "SPIPrinting.h"
#include <stdio.h>

#include <d4164.h>

int main()
{
	cli(); //disable interrupts
//	DDRD = 1; //wtf?

	//Don't touch any PORTB below 4, those are for printf
	SetupPrintf();

//	USB_ZeroPrescaler();
//	USB_Init();

	//set output pins
	DDRB |= _BV(7) | _BV(6) | _BV(4);
	DDRC |= 0xF0;
	DDRD |= 0x7F;

	//ras high
	//cas high
	//we high
	PORTC |= CAS | WE | 0xE0;

	PORTB |= _BV(4); //LEDON
	_delay_ms(50);

//	PORTB = 0x00; //all 
//	PORTC = 0x00; //all input
//	PORTD = 0x00; //all input
	
	sei();	//Safe to leave lockdown...
	uint8_t b,r;
	b=0;

	for(;;)
	{
//		WriteBit(0,0,b);
		WriteByte(0,0,b);

		//a few instructions are needed before the next read or write fot tRC 330ns
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");

		r = ReadBit(0,0);
//		printf("w %02X r %02X\n", b, r);
/*
		if (r == b) 
			PORTB |= _BV(4);
		else
			PORTB &= ~_BV(4); //LED off
*/

		if (r != b)
		{
//			printf("w %02X r %02X\n", b, r);
			PORTB &= ~_BV(4); //LED off
			return 0;			
		}

		b = ~b;
	}

	return 0;
}
