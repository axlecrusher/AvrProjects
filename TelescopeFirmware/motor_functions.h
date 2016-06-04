#ifndef MOTORFUNCTIONS_H
#define MOTORFUNCTIONS_H


void dec_forward();
void dec_backward();
void dec_stop();
void ra_forward();
void ra_backward();
void ra_stop();
void set_ra_pwm(uint16_t t);
void set_dec_pwm(uint16_t t);
void slew_ra(int32_t *dx);
void slew_dec(int32_t *dx);
void jog_ra(int8_t x);
void jog_dec(int8_t x);

#endif