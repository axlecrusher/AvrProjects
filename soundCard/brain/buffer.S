#include <avr/sfr_defs.h>

#define SPDR 78-32
#define UEDATX 241

#define REG_TMP r24
#define REG_INDEX r16

#define UEINTX 232
#define UENUM 233

#define PORTD 43-32

.global	ReadFromUSB_asm
ReadFromUSB_asm:

;load pointer into special register X
lds r26,writePtr
lds r27,writePtr+1

;load pointer into special register Y
lds r28,readPtr
lds r29,readPtr+1

cbi PORTD,6 ;clear LED

;UENUM=4
ldi REG_TMP,lo8(4)
sts UENUM,REG_TMP

;clear RXOUTI
lds REG_TMP,UEINTX
andi REG_TMP,lo8(-5)
sts UEINTX,REG_TMP

;if ( USB_READY(UEINTX) && BUFFER_HAS_ROOM(DATAGRAM_SIZE) )
;need to do this

clr REG_INDEX

USB_SPI_LOOP:
;write data value to SPI, increment pointer
ld REG_TMP,Y+
out SPDR,REG_TMP

;read UEDATX into memory at writePtr, increment writePtr
lds REG_TMP,UEDATX
st X+,REG_TMP

lds REG_TMP,UEDATX
st X+,REG_TMP

lds REG_TMP,UEDATX
st X+,REG_TMP

lds REG_TMP,UEDATX
st X+,REG_TMP

inc REG_INDEX
cpi REG_INDEX,lo8(4)
brne .USB_SPI_LOOP

clr REG_INDEX

;sending to SPI is done, read the rest of USB into ram
USB_LOOP:
lds REG_TMP,UEDATX
;movw r30,r26
st X+,REG_TMP
inc REG_INDEX
cpi REG_INDEX,lo8(48)
brne .USB_LOOP

;clear FIFOCON
lds REG_TMP,UEINTX
andi REG_TMP,lo8(127)
sts UEINTX,REG_TMP

;deal with looping the buffers around

;save current write pointer
sts writePtr,r26
sts writePtr+1,r27

;save current read pointer
sts readPtr,r28
sts readPtr+1,r29

lds REG_TMP, samplesBuffered
add REG_TMP, 15 ;buffered 16 samples but played 1
sts samplesBuffered, REG_TMP

ret
