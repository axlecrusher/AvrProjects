#include "avrUsbUtils.h"
#include "usb_handler.h"

#include "usb_protocol.h"
#include "mytypes.h"

#define MOTOR_FLAG_X_ON 0x01;
#define MOTOR_FLAG_Y_ON 0x02;

extern vuint8_t motorflags;
extern vuint32_t x_pos = 0x0;
extern vuint32_t y_pos = 0x0;
extern vuint32_t x_dest = 0x0;
extern vuint32_t y_dest = 0x0;

uint32_t ReadSlewDest();

void VendorRequest(uint8_t bRequest) {
	switch(bRequest) {
		case PING:
			usb_wait_in_ready();
			usb_write_str("PONG");
			usb_send_in();
			break;
		case MOTOR_INFO:
			usb_wait_in_ready();
//			usb_write(&x_pos,sizeof(x_pos));
			usb_write(&y_pos,sizeof(y_pos));
			usb_send_in();
			break;
		case MOTORS_ON:
			usb_wait_in_ready();
			motorflags |= MOTOR_FLAG_Y_ON;
			usb_write_str("OK");
			usb_send_in();
			break;
		case GOTO_Y:
			//LOOK AT
			//Control-Out (CPU To Us)
			usb_wait_receive_out();
			y_dest = ReadSlewDest();
			usb_ack_out();
//			usb_write_str("OK");
			usb_send_in();
//			PORTD =  _BV(PD6);
			break;
		case GOTO_X:
			//LOOK AT
			//Control-Out (CPU To Us)
			usb_wait_receive_out();
			x_dest = ReadSlewDest();
			usb_ack_out();
//			usb_write_str("OK");
			usb_send_in();
			break;
	}
}

uint32_t ReadSlewDest()
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
