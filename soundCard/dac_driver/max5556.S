#define _SFR_ASM_COMPAT 1
#define __SFR_OFFSET 0

//#include <avr/io.h>

#define REG_ZERO 1
#define PORTA 59-32
#define PINA 57-32

#define REG_ONEBIT r21
#define REG_LOWBYTE r24
#define REG_HIGHBYTE r25

.global	WriteSample_asm
WriteSample_asm:

	push REG_ONEBIT
	push REG_LOWBYTE
	push REG_HIGHBYTE

	ldi REG_ONEBIT,0x02 //SDATA high

	//left channel first
	//uint8_t hbyte = lr_samples[0]; //high byte
	lds REG_HIGHBYTE,lr_samples

	//uint8_t lbyte = lr_samples[1]; //low byte
	lds REG_LOWBYTE,lr_samples+1

.WAIT_FOR_LEFT:
	sbic PINA,7
	rjmp .WAIT_FOR_LEFT

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

	//wait for right
.WAIT_FOR_RIGHT:
	sbis PINA,7
	rjmp .WAIT_FOR_RIGHT

	//right channel
	//uint8_t hbyte = lr_samples[0]; //high byte
	lds REG_HIGHBYTE,lr_samples+2

	//uint8_t lbyte = lr_samples[1]; //low byte
	lds REG_LOWBYTE,lr_samples+3

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

	pop REG_HIGHBYTE
	pop REG_LOWBYTE
	pop REG_ONEBIT

ret

