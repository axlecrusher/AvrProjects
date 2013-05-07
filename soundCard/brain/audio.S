#define _SFR_ASM_COMPAT 1
#define __SFR_OFFSET 0

#include <avr/io.h>
#include <avr/interrupt.h>

//#define RSEND r18
#define RTMP r19

.global ReadUSB_SendChannelData_asm
ReadUSB_SendChannelData_asm:

	;this function will read 16 bytes from USB into the circular buffer
	;and send 4 bytes from the circular buffer out over the SPI

	push RTMP ;2
	push YL
	push YH

;	char* rb = ab.buffer+ab.tail; 6 ticks
	lds YL,ab+2
	lds YH,ab+3
	subi YL,lo8(-(ab+6))
	sbci YH,hi8(-(ab+6))

;	SPDR = *rb; //send left MSB, 17 clock cycles
;	rb++;
	ld RTMP, Y+
	out SPDR,RTMP

	push XL ;2
	push XH ;2

;	char* wb = ab.buffer+ab.head;
	;this is the correct way to index into an array, 6 ticks
	lds XL,ab ;load index value
	lds XH,ab+1
	subi XL,lo8(-(ab+6)) ;immediate add pointer value of buffer
	sbci XH,hi8(-(ab+6)) ;immediate add with carry

;	*wb = UEDATX; wb++; 4 ticks
	lds RTMP,241
	st X+, RTMP
;	*wb = UEDATX; wb++;
	lds RTMP,241
	st X+, RTMP

.SPI_WAIT1:
;	while(!(SPSR & _BV(SPIF))); //wait for complete
	in RTMP,SPSR
	sbrs RTMP,SPIF
	rjmp .SPI_WAIT1



;	SPDR = *rb; //send left LSB, 17 clock cycles
;	rb++;
	ld RTMP, Y+
	out SPDR,RTMP

;	*wb = UEDATX; wb++;
	lds RTMP,241
	st X+, RTMP
;	*wb = UEDATX; wb++;
	lds RTMP,241
	st X+, RTMP

.SPI_WAIT2:
;	while(!(SPSR & _BV(SPIF))); //wait for complete
	in RTMP,SPSR
	sbrs RTMP,SPIF
	rjmp .SPI_WAIT2



;	SPDR = *rb; //send right MSB, 17 clock cycles
;	rb++;
	ld RTMP, Y+
	out SPDR,RTMP
;	out 78-32,RTMP

;	*wb = UEDATX; wb++;
	lds RTMP,241
	st X+, RTMP
;	*wb = UEDATX; wb++;
	lds RTMP,241
	st X+, RTMP

.SPI_WAIT3:
;	while(!(SPSR & _BV(SPIF))); //wait for complete
	in RTMP,SPSR
	sbrs RTMP,SPIF
	rjmp .SPI_WAIT3



;	SPDR = *rb; //send right LSB, 17 clock cycles
;	rb++;
	ld RTMP, Y+
	out SPDR,RTMP
;	out 78-32,RTMP

;	*wb = UEDATX; wb++;
	lds RTMP,241
	st X+, RTMP
;	*wb = UEDATX; wb++;
	lds RTMP,241
	st X+, RTMP

	;start popping things that we don't need early while we wait for SPI
	pop YH
	pop YL
	pop XH
	pop XL

.SPI_WAIT4:
;	while(!(SPSR & _BV(SPIF))); //wait for complete
	in RTMP,SPSR
	sbrs RTMP,SPIF
	rjmp .SPI_WAIT4

	pop RTMP

ret
