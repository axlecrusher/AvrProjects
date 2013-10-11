#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <string.h>
#include <avr/sfr_defs.h>
#include "usb.h"

#include "avrUsbUtils.h"

#include "SPIPrinting.h"

#define DATAGRAM_SIZE 64

#include "buffer.h"

struct CircleBuffer ab;

static void setup_clock()
{
	CLKPR = 0x80;	/*Setup CLKPCE to be receptive*/
	CLKPR = 0x00; //no divisor
}

/*
static void setup_timers()
{
	TCCR1A = 0x00;
	TCCR1B = _BV(WGM12) | _BV(CS10); //CTC, no divisor
	TIMSK1 = 0x00 | _BV(OCIE1A); //TIMER1_COMPA_vect
	OCR1A = 333; //333.33333333333 we will have to make up for this
}
*/

uint8_t bytesRead = 0;

void SendChannelData();
void ReadUSB_SendChannelData();
void ReadUSB_SendChannelData_asm();

volatile char needAudioFlag = 0;
char dataState = 0;

ISR(INT4_vect)
{
	//signal the main loop that audio data needs to be sent over SPI
	//let the main loop do all the heavy lifting so that we can avoid
	//having to push 15 registers onto the stack when running the interrupt
	//need to get to sending the audio as fast as possible
	needAudioFlag = 1;
}

void ReadUSB_SendChannelData()
{
/*
	char* wb = ab.buffer+ab.head;
	char* rb = ab.buffer+ab.tail;

	SPDR = *rb; //send left MSB, 17 clock cycles
	rb++;
	*wb = UEDATX; wb++;
	*wb = UEDATX; wb++;
	while(!(SPSR & _BV(SPIF))); //wait for complete
	
	//send left LSB
	SPDR = *rb; //17 clock cycles for this to send
	rb++;
	*wb = UEDATX; wb++;
	*wb = UEDATX; wb++;
	while(!(SPSR & _BV(SPIF))); //wait for complete

	//send right MSB
	SPDR = *rb; //17 clock cycles for this to send
	rb++;
	*wb = UEDATX; wb++;
	*wb = UEDATX; wb++;
	while(!(SPSR & _BV(SPIF))); //wait for complete

	//send right LSB
	SPDR = *rb; //17 clock cycles for this to send
	*wb = UEDATX; wb++;
	*wb = UEDATX; wb++;
	while(!(SPSR & _BV(SPIF))); //wait for complete
*/
	ReadUSB_SendChannelData_asm();

	MoveBufferHead(&ab, 8);
	MoveBufferTail(&ab, 4);

	if (bytesRead == 0)
		UEINTX &= ~_BV(RXOUTI); //ack

	bytesRead += 8;
	if (bytesRead >= DATAGRAM_SIZE)
	{
		UEINTX &= ~_BV(FIFOCON); //reset USB packet
		bytesRead = 0;
	}
}

void SendChannelData()
{
	char* rb = (char*)(ab.buffer+ab.tail);

	//send left MSB
	SPDR = *rb;
	rb++;
	while(!(SPSR & _BV(SPIF))); //wait for complete

	//send left LSB
	SPDR = *rb;
	rb++;
	while(!(SPSR & _BV(SPIF))); //wait for complete

	//send right MSB
	SPDR = *rb;
	rb++;
	while(!(SPSR & _BV(SPIF))); //wait for complete

	//send right LSB
	SPDR = *rb;
	MoveBufferTail(&ab, 4);
	while(!(SPSR & _BV(SPIF))); //wait for complete
}

void setup_lr_interrupt()
{
	//setup interrupt on rising edge.
	//Rising edge indicates right channel processing by attiny
	//Since we send the left channel over SPI first
	//there is no danger of writing over the buffer being sent to the DAC
	//SPI should always be one channel ahead of the DAC

	EICRB = _BV(ISC41) | _BV(ISC40); //rising edge
	EIMSK = _BV(INT4);
}


void SPI_MasterInit(void)
{
	/* Set MOSI and SCK output, all others input */
	DDRB = (1<<PB2)|(1<<PB1) | _BV(PB0);
	/* Enable SPI, Master, set clock rate fck/2 */
	SPCR = (1<<SPE)|(1<<MSTR);
	SPSR = _BV(SPI2X);

	//drive SS high
//	PORTB |= _BV(PB0);
}

static void DoAudioState()
{
	char t = (char)dataState;
	if (t==1)
	{
		PORTD &= ~_BV(PD6); //LED off
		ReadUSB_SendChannelData();
	}
	else if (t==2)
	{
		PORTD &= ~_BV(PD6); //LED off
		SendChannelData();
	}
	else
		PORTD |= _BV(PD6); //LED on
}

int main( void )
{
	cli();

	setup_clock();

	InitBuffer(&ab);

	//Don't touch any PORTB below 4, those are for printf
//	SetupPrintf();

	USB_ZeroPrescaler();
	USB_Init();

	DDRC = 0x0;

	SPI_MasterInit();

//	_delay_ms(10); //wait for tiny 44 to be ready for data
//	setup_timers();

	setup_lr_interrupt();

	DDRD |= _BV(PD6);

	sei();

//	testAsm();
//	if (testPtr == 'w') PORTD |= _BV(PD6);

	while(1)
	{
		UENUM = 4; //interrupts can change this

		//these if statements can take a bit of time so run them and save the
		//result for future use
		if ( USB_READY(UEINTX) && (BytesFree(&ab) >= 8) )
			dataState=1;
		else if ( BytesUsed(&ab) >= 2)
			dataState=2;
		else
			dataState=0;

		while (needAudioFlag==0);
cli();
		needAudioFlag=0;

		DoAudioState();
sei();
	}

	return 0;
}

/*

Copyright (c) 2013 Joshua Allen

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
