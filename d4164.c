#include <stdint.h>
#include <avr/io.h>

#include <d4164.h>

/*

DRAM to AVR mapping

A0 PD0
A1 PD1
A2 PD2
A3 PD3
A4 PD4
A5 PD5
A6 PD6
A7 PB6

//DIN PC4 //output to ram
DIN PB7 //output to ram
WE PC5
RAS PC6

CAS PC7
DOUT PC2 //input from ram
*/

inline void SetAddress(uint8_t addr)
{
	PORTD = (PORTD & _BV(7)) | (addr & ~_BV(7));
//	PORTB = (PORTB & ~_BV(7)) | (addr & _BV(7));
	addr >>= 1;
	PORTB = (PORTB & ~_BV(6)) | (addr & _BV(6));
}

inline void SetBit(uint8_t bit)
{
//	PORTC = (PORTC & ~_BV(4)) | (bit & _BV(4));
	PORTB = (PORTB & ~_BV(7)) | (bit & _BV(7));
}

void WriteBit(uint8_t ra, uint8_t ca, uint8_t bit)
{
	//set data bit
	SetBit(bit);
//	printf("D:%02X\n", PORTC);

	SetAddress(ra); //set row address

	//ras low
	PORTC &= ~_BV(6);
	//sleep 15ns for tRAH
	asm("nop"); //62ns

	//WE low
	PORTC &= ~WE;

	SetAddress(ca); //set column address

	//cas low
	PORTC &= ~CAS;
	//sleep .1us - 10usec for tCAS
	//.25usec total here
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	
	//ras high
	//cas high
	//we high
	PORTC |= CAS | WE | 0xE0;
}

char ReadBit(uint8_t ra, uint8_t ca)
{
	//WE high
	PORTC |= WE;

	SetAddress(ra); //set row address

	//ras low
	PORTC &= ~_BV(6);
	//sleep 15ns for tRAH
	asm("nop"); //62ns

	SetAddress(ca); //set column address

	//cas low
	PORTC &= ~CAS;
	//sleep .1us - 10usec for tCAS
	//.25usec total here
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");

	uint8_t bit = PINC & _BV(2);
	
	//ras high
	//cas high
	//we high
	PORTC |= CAS | WE | 0xE0;

	if (bit>0) return 255;
	return 0;
}

void WriteByte(uint8_t ra, uint8_t ca, uint8_t byte)
{
	uint8_t i;

	SetAddress(ra); //set row address

	//ras low
	PORTC &= ~_BV(6);

	//sleep 15ns for tRAH
//	asm("nop");

	for (i=0;i<8;++i)
	{
		//set data bit
		SetBit(byte);
//		PORTC = (PORTC & ~_BV(4)) | ((byte & 0x01)<<4);

		//WE low
		PORTC &= ~WE;
		//down for at least 55ns

		SetAddress(ca); //set column address

		//cas low
		PORTC &= ~CAS;
		//down for .1us - 10usec for tCAS
		//so do extra stuff here
		//.25usec total here
//		byte >>= 1;
		byte <<= 1;
		++ca;
		asm("nop");
		asm("nop");
//		asm("nop");
//		asm("nop");
		
	
		//cas high
		//we high
		PORTC |= CAS | WE | 0x80;
		//for 80ns
	}

	//ras high
	PORTC |= 0x60;
	//high for 120ns, tRP
}

char ReadByte(uint8_t ra, uint8_t ca)
{
	return 0;
}

