#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <string.h>
#include <avr/sfr_defs.h>

/*khz*/
#define FREQ 38
#define EXPOSURE_DELAY 6000

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

	if ((PINB & _BV(PB1))==0) //button down
	{
		++button_press_ms;
		flags |= _BV(BUTTON_UPDATE);
	}
	else if (button_press_ms > 0) //button released, remember to clear button_press_ms later
	{
		flags |= _BV(BUTTON_RELEASED);
	}
}

static void setup_clock()
{
	CLKPR = 0x80;	/*Setup CLKPCE to be receptive*/
//	CLKPR = _BV(CLKPS0); /* 1/2 clock speed, 8mhz */
	CLKPR = 0x00; //no divisor, 8mhz attiny25

	OSCCAL=0x5B; //set ocillator calibration
}

static void setup_timers()
{
	TCCR0A = _BV(WGM01);
	TCCR0B = _BV(CS01) | _BV(CS00); /* x/64 */
	OCR0A = 125; /* 1ms */
	TIMSK = _BV(OCIE0A);
}
/*
void ReportCalibration()
{
	sendhex2(OSCCAL);
	sendchr('\n');
}
*/
static void SetupPins()
{
	DDRB = _BV(PB0);
//	PORTB = 0;

	//button input
	PORTB = _BV(PB1);
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
		PORTB |= _BV(PB0);
		_delay_us(pause);
		PORTB &= ~_BV(PB0);
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
	shutter_event_time = AtomicRead32(&msec_time) + EXPOSURE_DELAY;
	DoShutterEvent = BeginExposure;
}

void ButtonFeedback()
{
	uint16_t bt = AtomicRead16(&button_press_ms);

	cli();
	flags &= ~_BV(BUTTON_UPDATE);
	sei();

	/* illuminate for first half of each elasped second */
	if ((bt>1000) & ((bt&1023)<500))
	{
		PORTB |= _BV(PB0);
	}
	else
	{
		PORTB &= ~_BV(PB0);
	}
}

void ProcesButton()
{
	uint16_t bt = AtomicRead16(&button_press_ms);

	cli();
	flags &= ~_BV(BUTTON_RELEASED);
	sei();

	if (bt == 0) return;

	/* Less than 1 second button press starts/restarts exposure sequence, anything longers sets the exposure time.
	Every second of depressed time maps to one minute of exposure time.
	After the exposure time is set, the button must be pressed once for less than a second to start the exposure process. */
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

//	setup_spi();
	sei();

	while(1)
	{
//		ReportCalibration();
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
