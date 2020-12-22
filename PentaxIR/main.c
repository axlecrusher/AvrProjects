#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <string.h>
#include <avr/sfr_defs.h>

/*khz*/
#define FREQ 38
#define EXPOSURE_DELAY 6000  //time to wait between EndExposure and BeginExposure

#define TRUE 1
#define FALSE 0

#define DEBOUNCE_MS 20

#define LED_ON PORTB |= _BV(PB0);
#define LED_OFF PORTB &= ~_BV(PB0);

/* variables that can be changed by interrupts */
volatile uint32_t msec_time = 0; //can store 1193.0464708333 hours
volatile uint16_t button_press_ms = 0; //number of ms button was held down
volatile uint8_t flags = 0;
volatile uint8_t squential_presses = 0; //number of sequential presses

/* variables only changed by main loop */
uint32_t exposure_ms = 1000;
uint32_t shutter_event_time = 0; //timestamp of next shutter event

void (*DoShutterEvent)() = NULL;

enum ButtonState
{
	BUTTON_UP = 0,
	BUTTON_DOWN,
	BUTTON_RELEASED,
	BUTTON_UPDATE //indicates that interrupt has read the button and set the flags
};

enum OperationMode
{
	MODE_BULB = 0,
	MODE_DELAY
};

uint8_t mode = MODE_BULB;

//Interrupt set to execute every 1ms
ISR(TIMER0_COMPA_vect)
{
	static uint32_t last_release_time = 0;
	static uint16_t stateDuration_ms = 0; //time in the current state
	static uint8_t buttonState = BUTTON_UP; //track button state

	++msec_time;

	//read button state
	if ((PINB & _BV(PB1))==0) //button down, pin pulled low
	{
		if (buttonState == BUTTON_UP)
		{
			//state changed, reset time
			stateDuration_ms = 0;
		}

		buttonState = BUTTON_DOWN;
	}
	else //button up
	{
		if (buttonState == BUTTON_DOWN)
		{
			stateDuration_ms = 0; //state changed, reset time
		}

		buttonState = BUTTON_UP;
	}

	//increment but don't overflow
	if (stateDuration_ms < 65000) //65 seconds max
	{
		++stateDuration_ms;
	}

	//debounce, consider state only after some time has passed
	if (stateDuration_ms >= DEBOUNCE_MS)
	{
		flags |= _BV(BUTTON_UPDATE); //signal button has been read

		if (buttonState == BUTTON_DOWN)
		{
			//this code needs to update every tick the button is down
			//because button_press_ms needs to be updated each time
			button_press_ms = stateDuration_ms;
			flags |= _BV(BUTTON_DOWN); //set button down
		}
		else if (buttonState == BUTTON_UP)
		{
			flags &= ~_BV(BUTTON_DOWN); //clear button down

			//this code must only run the one tick the button is released
			if (stateDuration_ms == DEBOUNCE_MS)
			{
				const uint32_t delta = msec_time - last_release_time;
				last_release_time = msec_time;
				flags |= _BV(BUTTON_RELEASED); //signal button released

				if (delta < 1000) //fast press
				{
					++squential_presses;
//					LED_ON;
				}
				else
				{
					squential_presses = 1;
//					LED_OFF;
				}
			}
		}
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
//	DDRB = _BV(PB0); //set pin as output (LED)
	DDRB = _BV(DDB0); //set pin as output (LED)
//	PORTB = 0;

	//DDRB is 0 (input), button input
	PORTB = _BV(PB1); //set pullup resistor for input pin
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

uint8_t AtomicRead8(volatile uint8_t* x)
{
	uint8_t a;
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
		LED_ON;
		_delay_us(pause);
		LED_OFF;
		_delay_us(pause);
	}
}

void shutterNow()
{
	uint8_t i;

	//disable interrupts while signaling shutter
	cli();

	high(13);
	_delay_ms(3);
	for (i=0;i<7;i++) {
		high(1);
		_delay_ms(1);
	};

	sei();
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

	uint32_t exposeTime = exposure_ms;

	if (mode == MODE_BULB)
	{
		exposeTime /= 1000; //to seconds
		exposeTime *= 60000; //to minutes

		shutter_event_time = AtomicRead32(&msec_time) + exposeTime;
		DoShutterEvent = EndExposure;
	}
	else if (mode == MODE_DELAY)
	{
		exposeTime /= 1000; //to seconds
		exposeTime *= 1000; //back to ms (whole seconds)

		//MODE_DELAY takes a photo every exposure_ms.
		shutter_event_time = AtomicRead32(&msec_time) + exposeTime;
		DoShutterEvent = BeginExposure;
	}
}

void EndExposure()
{
	shutterNow();
	shutter_event_time = AtomicRead32(&msec_time) + EXPOSURE_DELAY;
	DoShutterEvent = BeginExposure;
}

typedef struct
{
	uint16_t press_ms;
	uint8_t squential_presses;
	uint8_t flags;
} buttonState;

//Read button state into struct, avoid race conditions
void readButtonState(buttonState* state)
{
	//read all the state info we need and reset while blocking interrupts...
	//avoid race condition
	cli();

	state->press_ms = button_press_ms;
	state->squential_presses = squential_presses;
	state->flags = flags;

	//clear button update notifications
	flags &= ~_BV(BUTTON_UPDATE);
	flags &= ~_BV(BUTTON_RELEASED);

	sei();
}

void ButtonFeedback(buttonState* state)
{
	uint16_t bt = state->press_ms;

	/* illuminate for first half of each elasped second */
	if ((bt>1000) & ((bt&1023)<500))
	{
		LED_ON;
	}
	else
	{
		LED_OFF;
	}
}

void FlashLED(uint8_t x)
{
	for (uint8_t i = 0; i < x; i++)
	{
		LED_ON;
		_delay_ms(100);
		LED_OFF;
		_delay_ms(100);
	}
}

//Called after button has been released
void ProcessButton(buttonState* state)
{
	uint16_t timeDown = state->press_ms;
	uint8_t pressCount = state->squential_presses;

	if (timeDown == 0) return;

	/* Less than 1 second button press starts/restarts exposure sequence, anything longers sets the exposure time.
	Every second of depressed time maps to one minute of exposure time.
	After the exposure time is set, the button must be pressed once for less than a second to start the exposure process. */
	if (pressCount == 1)
	{
		if (timeDown < 1000)
		{
			BeginExposure();
		}
		else
		{
			exposure_ms = timeDown;
			DoShutterEvent = NULL;
		}
	}
	else if (pressCount == 2)
	{
		//put into bulb mode
		mode = MODE_BULB;
		FlashLED(2);
	}
	else if (pressCount == 3)
	{
		//put into delay mode
		mode = MODE_DELAY;
		FlashLED(3);
	}
}

int main( void )
{
	buttonState state;

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

		readButtonState(&state);

		if ((state.flags & _BV(BUTTON_UPDATE))>0) //button state has been updated
		{
			if ((state.flags & _BV(BUTTON_DOWN))>0) ButtonFeedback(&state);
			if ((state.flags & _BV(BUTTON_RELEASED))>0) ProcessButton(&state); //BUTTON_RELEASED set when button changes from down to up
		}
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
