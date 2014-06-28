#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <string.h>
#include <avr/sfr_defs.h>

/*khz*/
#define FREQ 38

/* variables that can be changed by interrupts */
volatile uint32_t msec_time = 0;
volatile uint16_t button_press_ms = 0;
volatile uint8_t flags = 0;

/* variables only changed by main loop */
uint32_t exposure_ms = 1000;
uint32_t shutter_event_time = 0;

void (*DoShutterEvent)() = NULL;

enum
{
	BUTTON_UPDATE = 0,
	BUTTON_RELEASED,
	EXPOSING
};

ISR(TIMER0_COMPA_vect)
{
	++msec_time;

	if ((PINB & _BV(PB7))==0) //button down
	{
		++button_press_ms;
		flags |= _BV(BUTTON_UPDATE);
	}
	else if (button_press_ms > 0) //button released
	{
		flags |= _BV(BUTTON_RELEASED);
	}
}

static void setup_clock()
{
	CLKPR = 0x80;	/*Setup CLKPCE to be receptive*/
	CLKPR = _BV(CLKPS0); /* 1/2 clock speed, 8mhz */
//	CLKPR = 0x00; //no divisor
}

static void setup_timers()
{
	TCCR0A = _BV(WGM01);
	TCCR0B = _BV(CS01) | _BV(CS00);
	OCR0A = 125; /* 1ms */
	TIMSK0 = _BV(OCIE0A);
}

static void SetupPins()
{
	DDRD = _BV(PD5);
	PORTD = 0;

	//button input
//	DDRB = _BV(PB7);
//	DDRD = _BV(PD5);
	PORTB = _BV(PB7);
}

uint32_t AtomicRead32(volatile uint32_t* x)
{
	uint32_t a;
	cli();
	a = *x;
	sei();
	return a;
}

uint16_t AtomicRead16(volatile uint16_t* x)
{
	uint16_t a;
	cli();
	a = *x;
	sei();
	return a;
}

void high(uint16_t msec) {
	const double pause = (1000/FREQ/2);
	uint32_t t = msec;
	t+= AtomicRead32(&msec_time);

	while(AtomicRead32(&msec_time) < t)
	{
		PORTD |= _BV(PD5);
		_delay_us(pause);
		PORTD &= ~_BV(PD5);
		_delay_us(pause);
	}
}

void shutterNow()
{
	uint8_t i;
	high(13);
	_delay_ms(3);
	for (i=0;i<7;i++) {
		high(1);
		_delay_ms(1);
	};
}

void TryShutterEvent()
{
	uint32_t t = AtomicRead32(&msec_time);
	if (shutter_event_time < t) DoShutterEvent();
}

void EndExposure();

void BeginExposure()
{
	shutterNow();
	shutter_event_time = AtomicRead32(&msec_time) + exposure_ms;
	DoShutterEvent = EndExposure;
}

void EndExposure()
{
	shutterNow();
	shutter_event_time = AtomicRead32(&msec_time) + 6000;
	DoShutterEvent = BeginExposure;
}

void ButtonFeedback()
{
	uint16_t bt = AtomicRead16(&button_press_ms);

	cli();
	flags &= ~_BV(BUTTON_UPDATE);
	sei();

	if ((bt>1000) & ((bt&1023)<500))
	{
		PORTD |= _BV(PD5);
	}
	else
	{
		PORTD &= ~_BV(PD5);
	}
}

void ProcesButton()
{
	uint16_t bt = AtomicRead16(&button_press_ms);

	cli();
	flags &= ~_BV(BUTTON_RELEASED);
	sei();

	if (bt == 0) return;

	/* Less than 1 second button press starts/restarts exposure sequence, anything longers sets the exposure time */
	if (bt < 1000)
	{
		BeginExposure();
	}
	else
	{
		exposure_ms = bt;
		exposure_ms /= 1000;
		exposure_ms *= 60000;
		DoShutterEvent = NULL;
	}

	cli();
	button_press_ms = 0;
	sei();
}

int main( void )
{
	cli();

	SetupPins();

	setup_clock();
	setup_timers();

	sei();

	while(1)
	{
		if (DoShutterEvent != NULL) TryShutterEvent();
		if ((flags & _BV(BUTTON_UPDATE))>0) ButtonFeedback();
		if ((flags & _BV(BUTTON_RELEASED))>0) ProcesButton();
	}

	return 0;
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
