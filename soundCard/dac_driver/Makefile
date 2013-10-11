all : program.hex burn program.lst
PART=attiny44
PROGPART=t44

CFLAGS=-g -O2 -mmcu=$(PART) -DF_CPU=24500000UL -Wall
CC=avr-gcc

OBJS = test.o max5556.o
SRCS = test.c max5556.S

program.elf : $(OBJS)
	avr-gcc -I  $(CFLAGS) -mmcu=$(PART) -Wl,-Map,program.map -o $@ $^ -L/usr/lib/binutils/avr/2.18

program.hex : program.elf
	avr-objcopy -j .text -j .data -O ihex program.elf program.hex 

program.lst : $(SRCS)
	avr-gcc -c -g -Wa,-a,-ad $(CFLAGS) $^ > $@

burn : program.hex
	avrdude -c usbtiny -p $(PROGPART) -U flash:w:program.hex
# if you get the part borked, type -B 1024 and it will go REALLY slow

#This make the part use an external crystal.
burnfuses :
	avrdude -c usbtiny -p $(PROGPART) -U lfuse:w:0xAE:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m 

readfuses :
	avrdude -c usbtiny -p $(PROGPART) -U hfuse:r:high.txt:b -U lfuse:r:low.txt:b

clean : 
	rm -f *~ high.txt low.txt program.hex program.map program.elf *.o program.lst
