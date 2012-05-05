//Copyright 2011 Charles Lohr under the MIT/X11 License.

#include "SPIPrinting.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

volatile char SPIBuffer[PRINTF_BUFFER_SIZE];
volatile unsigned char SPIHead = 0;
volatile unsigned char SPITail = 0;

static int SPIPutCharInternal(char c, FILE *stream);

static FILE mystdout = FDEV_SETUP_STREAM( SPIPutCharInternal, NULL, _FDEV_SETUP_WRITE );

void SetupPrintf()
{
	SPCR = (1<<SPIE)|(1<<SPE);
	DDRB |= _BV(3);
	DDRB &= ~(_BV(0)|_BV(1)|_BV(2));
	stdout = &mystdout;
}

static int SPIPutCharInternal(char c, FILE *stream)
{
	if( ( SPIHead + 1 ) == SPITail ) return -1;//Overflow.
	SPIBuffer[SPIHead] = c;
	SPIHead++;
	if( SPIHead == PRINTF_BUFFER_SIZE ) SPIHead = 0;
	return 0;
}

void SPIPutChar( char c )
{
	SPIPutCharInternal( c, 0 );
}

ISR( SPI_STC_vect )
{
//	volatile unsigned char cx = SPDR;
//	If you want to get input from the computer, read SPDR.

	if( SPITail != SPIHead )
		SPDR = SPIBuffer[SPITail++];
	else
		SPDR = 0;

	if( SPITail == PRINTF_BUFFER_SIZE )
		SPITail = 0;
}


