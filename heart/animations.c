#include "animations.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <string.h>
#include <avr/sfr_defs.h>
#include <avr/pgmspace.h>

uint8_t* frame = 0x00;

extern void (*DoFrame)(void);
extern void (*DrawFunc)(void);
extern uint8_t done_animation;

uint8_t LightLED(uint8_t i);

uint8_t JCframes[] PROGMEM = {
14,		10,4,5,13,20,26,32,38,30,22,14,7,1,2, //heart outline
10,		8,9,10,11,12,17,24,31,30,22, //J
5,		10,16,17,18,24, //+
8,		9,10,11,16,23,30,31,32, //C
0xFF
};

//uint8_t HeartOutline[] PROGMEM = { 10,4,5,13,20,26,32,38,30,22,14,7,1,2 };

uint8_t FillHeartFrames[] PROGMEM = {
14,		10,4,5,13,20,26,32,38,30,22,14,7,1,2, //heart outline
24,		1,2,10,4,5,7,13,14,20,22,26,30,32,38,8,9,17,11,12,15,19,23,25,31,
27,		1,2,10,4,5,7,13,14,20,22,26,30,32,38,8,9,17,11,12,15,19,23,25,31,16,18,24,

13,		8,9,17,11,12,15,19,23,25,31,16,18,24,
3,		16,18,24,
0,
0,

0xFF //stop
};

volatile uint8_t iBegin = 0; //index of first LED code
volatile uint8_t iEnd = 0; //index of last LED code

//We could have hard coded the function order but i felt like doing it like this
//allows for some neat things like reusing animations in arbitrary order
//this uses 62 more bytes
volatile uint8_t iSequence = 0;
void (*AnimationSequence[]) (void) PROGMEM = { &SetupHeartOutline, &SetupJC, &SetupHeartOutline, &SetupHeartFill, 0x0000 };

void NextSequence()
{
	void (*f)(void) = pgm_read_word(AnimationSequence+iSequence);
	if (f == 0x0000)
	{
		done_animation=0xFF;
		iSequence = 0;
		f = pgm_read_word(AnimationSequence);
	}

	f();
	iSequence++;
}

void NextStaticFrame()
{
	frame += pgm_read_byte(frame)+1;

	if (pgm_read_byte(frame) == 0xFF)
	{
		NextSequence();
	}
	else
	{
		iBegin = 1;
		iEnd = pgm_read_byte(frame)+1;
	}
}

void HeartOutlineFrame()
{
	iEnd++;
	if (iEnd>15)
	{
		NextSequence();
	}
}

void SetupHeartOutline()
{
	cli();

	frame = FillHeartFrames;
	iBegin = 1;
	iEnd = 2;
	TCNT1 = 0;
	OCR1A = 976; //    1/20 second
	DoFrame = HeartOutlineFrame;

	sei();
}

void SetupHeartFill()
{
	cli();

	frame = FillHeartFrames;
	iBegin = 1;
	iEnd = pgm_read_byte(frame) +1;
	TCNT1 = 0;
	OCR1A = 19531; //    1/20 second
	DoFrame = NextStaticFrame;

	sei();
}

void SetupJC()
{
	cli();

	frame = JCframes;
	iBegin = 1;
	iEnd = pgm_read_byte(frame) +1;
	TCNT1 = 0;
	OCR1A = 19531*2; //    1/20 second
	DoFrame = NextStaticFrame;

	sei();
}

void ShowFrame()
{
	cli();
//	uint8_t* f = offset;
	uint8_t* f = frame;
//	uint8_t length = pgm_read_byte(animation+o) + 1;
	uint8_t b = iBegin;
	uint8_t l = iEnd;
	sei();


	uint8_t i;
	for (i=b; i<l; ++i)
	{
//		if (current_frame[i] < GRID_ARRAY_LENGTH)
			LightLED(pgm_read_byte(f+i));
	}
}

/*
//we can animate the frame in various ways if we want
void MoveFrameUp()
{
	uint8_t length = current_frame[0] + 1;
	uint8_t i;
	uint8_t d;
	for (i=1; i<length; ++i)
	{
		d = current_frame[i] - 7;
		if (d>82) d -= 255-82-1;
		current_frame[i]  = d;
	}
}
*/
