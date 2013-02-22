#define _SFR_ASM_COMPAT 1
#define __SFR_OFFSET 0

#include <avr/io.h>
#include <avr/interrupt.h>

#define REG_ZERO r0
#define REG_INDEX r20

#define PORTA 59-32
#define PINA 57-32
#define USIBR 48-32

#define REG_ONEBIT r21
#define REG_LOWBYTE r22
#define REG_HIGHBYTE r23

#define REG_LH r16
#define REG_LL r17
#define REG_RH r18
#define REG_RL r19

//USI_OVF_vect
.global __vector_16
__vector_16:

	sbi 47-32,6 ;clear oveflow flag, 2

set_pointer:
;	subi r30,lo8(-(lr_samples))	;1
;	sbci r31,hi8(-(lr_samples))	;1

store:
;	in REG_LH,USIBR				;1
;	st +Z,REG_LH				;2, store increment



	mov r30,REG_INDEX 			;1
	ldi r31,lo8(0)				;1
	in REG_LH,USIBR				;1
	subi r30,lo8(-(lr_samples))	;1
	sbci r31,hi8(-(lr_samples))	;1
	st Z,REG_LH					;2
	inc REG_INDEX				;1
	andi REG_INDEX,lo8(3)		;1






;	in REG_LH, USIBR
	
;	inc REG_INDEX
;	andi REG_INDEX,lo8(3)	;0-3 only
	
;	//*(lr_samples+wIndex) = USIBR;
;	lds r17,wIndex
;	mov r30,r17
;	ldi r31,lo8(0)
;	in r18,48-32
;	subi r30,lo8(-(lr_samples))
;	sbci r31,hi8(-(lr_samples))
;	st Z,r18
;	//wIndex++;
;	subi r17,lo8(-(1))
;	//wIndex &= 0x03; //0-3 only
;	andi r17,lo8(3)
;	sts wIndex,r17

reti

.global	WriteSample_asm
WriteSample_asm:

	clr REG_ZERO
	clr REG_INDEX
	ldi REG_ONEBIT,0x02 //SDATA high
	
	sbi 45-32,6		;Counter Overflow Interrupt Enable

//the rest of the program is in this loop forever
//who cares what registers we stop, we are never leaving this loop.

//	push REG_ONEBIT
//	push REG_LOWBYTE
//	push REG_HIGHBYTE
//	push REG_ZERO
//	clr REG_ZERO

.WRITELOOP:

.WAIT_FOR_LEFT:
	sbic PINA,7
	rjmp .WAIT_FOR_LEFT

cli
	//left channel first
	//uint8_t hbyte = lr_samples[0]; //high byte
	lds REG_HIGHBYTE,lr_samples

	//uint8_t lbyte = lr_samples[1]; //low byte
	lds REG_LOWBYTE,lr_samples+1
sei

	cbi PORTA,2 //sclock down
	sbi PORTA,2 //sclk up

	//do high byte
	out PORTA,REG_ZERO		//sclk down, 0 data
	sbrc REG_HIGHBYTE,7		//check bit
	out PORTA,REG_ONEBIT	//SDATA high if bit == 1
	sbi PORTA,2				//SCLK high

	out PORTA,REG_ZERO		//sclk down, 0 data
	sbrc REG_HIGHBYTE,6		//check bit
	out PORTA,REG_ONEBIT	//SDATA high if bit == 1
	sbi PORTA,2				//SCLK high

	out PORTA,REG_ZERO		//sclk down, 0 data
	sbrc REG_HIGHBYTE,5		//check bit
	out PORTA,REG_ONEBIT	//SDATA high if bit == 1
	sbi PORTA,2				//SCLK high

	out PORTA,REG_ZERO		//sclk down, 0 data
	sbrc REG_HIGHBYTE,4		//check bit
	out PORTA,REG_ONEBIT	//SDATA high if bit == 1
	sbi PORTA,2				//SCLK high

	out PORTA,REG_ZERO		//sclk down, 0 data
	sbrc REG_HIGHBYTE,3		//check bit
	out PORTA,REG_ONEBIT	//SDATA high if bit == 1
	sbi PORTA,2				//SCLK high

	out PORTA,REG_ZERO		//sclk down, 0 data
	sbrc REG_HIGHBYTE,2		//check bit
	out PORTA,REG_ONEBIT	//SDATA high if bit == 1
	sbi PORTA,2				//SCLK high

	out PORTA,REG_ZERO		//sclk down, 0 data
	sbrc REG_HIGHBYTE,1		//check bit
	out PORTA,REG_ONEBIT	//SDATA high if bit == 1
	sbi PORTA,2				//SCLK high

	out PORTA,REG_ZERO		//sclk down, 0 data
	sbrc REG_HIGHBYTE,0		//check bit
	out PORTA,REG_ONEBIT	//SDATA high if bit == 1
	sbi PORTA,2				//SCLK high


	//do low byte
	out PORTA,REG_ZERO		//sclk down, 0 data
	sbrc REG_LOWBYTE,7		//check bit
	out PORTA,REG_ONEBIT	//SDATA high if bit == 1
	sbi PORTA,2				//SCLK high

	out PORTA,REG_ZERO		//sclk down, 0 data
	sbrc REG_LOWBYTE,6		//check bit
	out PORTA,REG_ONEBIT	//SDATA high if bit == 1
	sbi PORTA,2				//SCLK high

	out PORTA,REG_ZERO		//sclk down, 0 data
	sbrc REG_LOWBYTE,5		//check bit
	out PORTA,REG_ONEBIT	//SDATA high if bit == 1
	sbi PORTA,2				//SCLK high

	out PORTA,REG_ZERO		//sclk down, 0 data
	sbrc REG_LOWBYTE,4		//check bit
	out PORTA,REG_ONEBIT	//SDATA high if bit == 1
	sbi PORTA,2				//SCLK high

	out PORTA,REG_ZERO		//sclk down, 0 data
	sbrc REG_LOWBYTE,3		//check bit
	out PORTA,REG_ONEBIT	//SDATA high if bit == 1
	sbi PORTA,2				//SCLK high

	out PORTA,REG_ZERO		//sclk down, 0 data
	sbrc REG_LOWBYTE,2		//check bit
	out PORTA,REG_ONEBIT	//SDATA high if bit == 1
	sbi PORTA,2				//SCLK high

	out PORTA,REG_ZERO		//sclk down, 0 data
	sbrc REG_LOWBYTE,1		//check bit
	out PORTA,REG_ONEBIT	//SDATA high if bit == 1
	sbi PORTA,2				//SCLK high

	out PORTA,REG_ZERO		//sclk down, 0 data
	sbrc REG_LOWBYTE,0		//check bit
	out PORTA,REG_ONEBIT	//SDATA high if bit == 1
	sbi PORTA,2				//SCLK high

	sbic 57-32,7
	rcall ErrorBlink

	//wait for right
.WAIT_FOR_RIGHT:
	sbis PINA,7
	rjmp .WAIT_FOR_RIGHT

cli
	//right channel
	//uint8_t hbyte = lr_samples[0]; //high byte
	lds REG_HIGHBYTE,lr_samples+2

	//uint8_t lbyte = lr_samples[1]; //low byte
	lds REG_LOWBYTE,lr_samples+3
sei

	cbi PORTA,2 //sclock down
	sbi PORTA,2 //sclk up

	//do high byte
	out PORTA,REG_ZERO		//sclk down, 0 data
	sbrc REG_HIGHBYTE,7		//check bit
	out PORTA,REG_ONEBIT	//SDATA high if bit == 1
	sbi PORTA,2				//SCLK high

	out PORTA,REG_ZERO		//sclk down, 0 data
	sbrc REG_HIGHBYTE,6		//check bit
	out PORTA,REG_ONEBIT	//SDATA high if bit == 1
	sbi PORTA,2				//SCLK high

	out PORTA,REG_ZERO		//sclk down, 0 data
	sbrc REG_HIGHBYTE,5		//check bit
	out PORTA,REG_ONEBIT	//SDATA high if bit == 1
	sbi PORTA,2				//SCLK high

	out PORTA,REG_ZERO		//sclk down, 0 data
	sbrc REG_HIGHBYTE,4		//check bit
	out PORTA,REG_ONEBIT	//SDATA high if bit == 1
	sbi PORTA,2				//SCLK high

	out PORTA,REG_ZERO		//sclk down, 0 data
	sbrc REG_HIGHBYTE,3		//check bit
	out PORTA,REG_ONEBIT	//SDATA high if bit == 1
	sbi PORTA,2				//SCLK high

	out PORTA,REG_ZERO		//sclk down, 0 data
	sbrc REG_HIGHBYTE,2		//check bit
	out PORTA,REG_ONEBIT	//SDATA high if bit == 1
	sbi PORTA,2				//SCLK high

	out PORTA,REG_ZERO		//sclk down, 0 data
	sbrc REG_HIGHBYTE,1		//check bit
	out PORTA,REG_ONEBIT	//SDATA high if bit == 1
	sbi PORTA,2				//SCLK high

	out PORTA,REG_ZERO		//sclk down, 0 data
	sbrc REG_HIGHBYTE,0		//check bit
	out PORTA,REG_ONEBIT	//SDATA high if bit == 1
	sbi PORTA,2				//SCLK high


	//do low byte
	out PORTA,REG_ZERO		//sclk down, 0 data
	sbrc REG_LOWBYTE,7		//check bit
	out PORTA,REG_ONEBIT	//SDATA high if bit == 1
	sbi PORTA,2				//SCLK high

	out PORTA,REG_ZERO		//sclk down, 0 data
	sbrc REG_LOWBYTE,6		//check bit
	out PORTA,REG_ONEBIT	//SDATA high if bit == 1
	sbi PORTA,2				//SCLK high

	out PORTA,REG_ZERO		//sclk down, 0 data
	sbrc REG_LOWBYTE,5		//check bit
	out PORTA,REG_ONEBIT	//SDATA high if bit == 1
	sbi PORTA,2				//SCLK high

	out PORTA,REG_ZERO		//sclk down, 0 data
	sbrc REG_LOWBYTE,4		//check bit
	out PORTA,REG_ONEBIT	//SDATA high if bit == 1
	sbi PORTA,2				//SCLK high

	out PORTA,REG_ZERO		//sclk down, 0 data
	sbrc REG_LOWBYTE,3		//check bit
	out PORTA,REG_ONEBIT	//SDATA high if bit == 1
	sbi PORTA,2				//SCLK high

	out PORTA,REG_ZERO		//sclk down, 0 data
	sbrc REG_LOWBYTE,2		//check bit
	out PORTA,REG_ONEBIT	//SDATA high if bit == 1
	sbi PORTA,2				//SCLK high

	out PORTA,REG_ZERO		//sclk down, 0 data
	sbrc REG_LOWBYTE,1		//check bit
	out PORTA,REG_ONEBIT	//SDATA high if bit == 1
	sbi PORTA,2				//SCLK high

	out PORTA,REG_ZERO		//sclk down, 0 data
	sbrc REG_LOWBYTE,0		//check bit
	out PORTA,REG_ONEBIT	//SDATA high if bit == 1
	sbi PORTA,2				//SCLK high

	sbis 57-32,7
	rcall ErrorBlink

rjmp .WRITELOOP

//	pop REG_ZERO
//	pop REG_HIGHBYTE
//	pop REG_LOWBYTE
//	pop REG_ONEBIT

ret

