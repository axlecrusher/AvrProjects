all : program.hex burn program.lst
CC=avr-gcc

PART = atmega8u2
PROGPART = m8u2
CFLAGS = -Wall -Os -mmcu=$(PART) -DF_CPU=16000000UL

OBJS = test.o
SRCS = test.c

program.elf : $(OBJS)
	avr-gcc -I  $(CFLAGS) -Wl,-Map,program.map -o $@ $^ -L/usr/lib/binutils/avr/2.18

program.hex : program.elf
	avr-objcopy -j .text -j .data -O ihex program.elf program.hex 

program.lst : $(SRCS)
	avr-gcc -c -g -Wa,-a,-ad $(CFLAGS) $^ > $@

burn : program.hex
	avrdude -c usbtiny -p $(PROGPART) -U flash:w:program.hex

slowburn : program.hex
	avrdude -c usbtiny -p $(PROGPART) -B 1024 -U flash:w:program.hex

# if you get the part borked, type -B 1024 and it will go REALLY slow

#This make the part use an external crystal.
burnfuses :
	avrdude -c usbtiny -p $(PROGPART) -U lfuse:w:0x0e:m -U hfuse:w:0xd9:m -U efuse:w:0xf4:m

readfuses :
	avrdude -c usbtiny -p $(PROGPART) -U hfuse:r:high.txt:b -U lfuse:r:low.txt:b

clean : 
	rm -f *~ high.txt low.txt program.hex program.map program.elf *.o program.lst
