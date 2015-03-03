#include "avrUsbUtils.h"
#include "usb_handler.h"

#define PING 0xA1
#define PONG 0xA2
#define MOTOR_INFO 0xA3
#define MOTORS_ON 0xA4
#define MOTORS_OFF 0xA5
#define GOTO_Y 0xA6
#define GOTO_X 0xA7

#define MOTOR_FLAG_ON 0x01;

extern volatile uint32_t rcount;
extern volatile uint8_t motorflags;
extern volatile uint32_t y_pos;

uint32_t RewadSLewDest();

void VendorRequest(uint8_t bRequest) {
	char* data;


	switch(bRequest) {
		case PING:
			usb_wait_in_ready();
			usb_write_str("PONG");
			usb_send_in();
			break;
		case MOTOR_INFO:
			usb_wait_in_ready();
//			usb_write(&rcount,sizeof(rcount));
			usb_write(&y_pos,sizeof(y_pos));
			usb_send_in();
			break;
		case MOTORS_ON:
			usb_wait_in_ready();
//			usb_write(&rcount,sizeof(rcount));
			motorflags |= MOTOR_FLAG_ON;
			usb_write_str("OK");
			usb_send_in();
			break;
		case GOTO_Y:
			usb_wait_in_ready();
//			usb_write(&rcount,sizeof(rcount));
//			motorflags |= MOTOR_FLAG_ON;
			y_pos = RewadSLewDest();
//			y_pos = 0xf000;
//usb_ack_out();
//			usb_send_in();
			usb_write_str("OK");
			usb_send_in();
			break;
	}
}

uint32_t RewadSLewDest()
{
	uint8_t i;
	uint32_t d = 0;
	uint8_t* data = &d;

	for( i = 0; (i < 4) && USB_HAS_SPACE(UEINTX); i++ )
		data[i] = UEDATX;

	return d;
}

/*

Copyright (c) 2015, Joshua Allen
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
