#ifndef PROJDEFINES_H
#define PROJDEFINES_H

//1015697 //y,declination ticks per 360 degrees

#define STEPCOUNT 6
//uint8_t steps[STEPCOUNT] = { 0x01, 0x03, 0x02, 0x06, 0x04, 0x05 };
//uint8_t i = 0;

#define SLOP_PWM_MIN 1024

#define PWM_MAX 0xffff
#define PWM_MIN 2448
//#define PWM_MIN 4896

//_BV(PD0) | _BV(PD1)
//#define MOTORMASK (_BV(PD0) | _BV(PD1))

//_BV(PD6) | _BV(PD7)

#define RA_MOTORMASK (_BV(PD4) | _BV(PD5))
#define DEC_MOTORMASK (_BV(PD6) | _BV(PD7))

#define MOTORMASK (_BV(PD4) | _BV(PD5) | _BV(PD6) | _BV(PD7))

#define DEC_BRAKE (_BV(PD6) | _BV(PD7))
#define RA_BRAKE (_BV(PD4) | _BV(PD5))

#define STOP 0x00

//#define DEC_STOP PORTD &= ~DEC_MOTORMASK
//#define RA_STOP  PORTD &= ~RA_MOTORMASK

#define DEC_FORWARD _BV(PD6)
#define DEC_BACKWARD _BV(PD7)

#define RA_FORWARD _BV(PD4)
#define RA_BACKWARD _BV(PD5)

#endif