//Copyright 2012 Joshua Allen

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
//#include "usb.h"
#include "SPIPrinting.h"
#include <stdio.h>

#include <d4164.h>

//uint8_t dataString[257];

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

	for(;;)
	{
		b=0x55;
		for (ri = 0; ; ++ri)
		{
			for (ci=0; ; ci += 8)
			{
				//the function overhead takes care of the tRC of at least 330ns
				WriteByte(ri,ci,b);
				b = ~b;
				if (ci >= 0xF8) break;
			}
			if (ri >= 0xFF) break;
		}

		b=0x55;
		for (ri = 0; ; ++ri)
		{
			for (ci=0; ; ci += 8)
			{
				r = ReadByte(ri,ci);
//				printf("read %02X at %02X %02X\n", r, ri, ci);
				if (r != b)
				{
//					printf("broken at %02X %02X\n", ri, ci);
					PORTB &= ~_BV(4); //LED off
					return 0;			
				}
				b = ~b;
				if (ci >= 0xF8) break;
			}
			if (ri >= 0xFF) break;
		}

		PORTB &= ~_BV(4); //LEDON
		_delay_ms(10);	
		PORTB |= _BV(4); //LEDON
	}

	return 0;
}
