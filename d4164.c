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

DIN PB7 //output to ram
WE PC5
RAS PC6

CAS PC7
DOUT PC2 //input from ram
*/

inline void SetAddress(uint8_t addr) __attribute__((always_inline));
inline void SetAddress(uint8_t addr)
{
	//7 clocks
//	PORTD = addr & ~_BV(7); //3 clocks
	PORTD = addr; //don't care about BV(7), its always high on the chip, 1 clock
	addr >>= 1;
	PORTB = (PORTB & ~_BV(6)) | (addr & _BV(6)); //6 clocks
//	PORTB = addr & _BV(6);
}

inline void SetAddressDestructive(uint8_t addr) __attribute__((always_inline));
inline void SetAddressDestructive(uint8_t addr)
{
	//5 clocks
	PORTD = addr; //don't care about BV(7), its always high on the chip, 1 clock

	//4 clocks
	addr >>= 1;
	PORTB = (addr & _BV(6)) | _BV(4);
}

inline void SetBit(uint8_t addr) __attribute__((always_inline));
inline void SetBit(uint8_t bit)
{
	PORTB = (PORTB & ~_BV(7)) | (bit & _BV(7)); //6 clocks
}

inline void SetBitDestructive(uint8_t addr) __attribute__((always_inline));
inline void SetBitDestructive(uint8_t bit)
{
//	PORTB = bit; //destructive to address and LED, 1 clock
	PORTB = bit | _BV(4); //destructive to address, 2 clocks
//	PORTB = (PORTB & ~_BV(7)) | (bit & _BV(7)); //6 clocks
}

void WriteBit(uint8_t ra, uint8_t ca, uint8_t bit)
{
	//set data bit
	SetBitDestructive(bit);
//	printf("D:%02X\n", PORTC);

	SetAddress(ra); //set row address

	//ras low
	PORTC &= ~RAS;
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
	PORTC |= CAS | WE | RAS;
}

char ReadBit(uint8_t ra, uint8_t ca)
{
	//WE high
	PORTC |= WE;

	SetAddressDestructive(ra); //set row address

	//ras low
	PORTC &= ~RAS;
	//sleep 15ns for tRAH
	asm("nop"); //62ns

	SetAddressDestructive(ca); //set column address

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
	PORTC |= CAS | WE | RAS;

	if (bit>0) return 255;
	return 0;
}

void WriteByte(uint8_t ra, uint8_t ca, uint8_t byte)
{
	uint8_t i;

	SetAddressDestructive(ra); //set row address

	//ras low
	PORTC &= ~RAS; //2 clocks

	//sleep 15ns for tRAH
//	asm("nop");

	for (i=0;i<8;++i)
	{
		//set data bit
		SetBitDestructive(byte);
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
		PORTC |= CAS | WE;
		//for 80ns
	}

	//ras high
	PORTC |= RAS;
	//high for 120ns, tRP
}

char ReadByte(uint8_t ra, uint8_t ca)
{
	uint8_t i, byte;
	byte = 0;

	SetAddressDestructive(ra); //set row address

	//ras low
	PORTC &= ~RAS;

	//sleep 15ns for tRAH
//	asm("nop");

	for (i=0;i<8;++i)
	{
		//WE high
		PORTC |= WE;
		//down for at least 55ns

		SetAddressDestructive(ca); //set column address

		//cas low
		PORTC &= ~CAS;

		//wait 100ns before reading data, tCAC  (2 instructions)
		asm("nop");
		asm("nop");

		byte <<= 1; //move last data read over 1 bit

		byte |= (PINC & _BV(2)) >> 2;

		//cas high
		//we high
		PORTC |= CAS | WE;
		//wait tCP, 80ns (2 instructions) (loop overhead)

		++ca;
//		asm("nop");		
	}

	//ras high
	PORTC |= RAS;
	//high for 120ns, tRP

	return byte;
}

