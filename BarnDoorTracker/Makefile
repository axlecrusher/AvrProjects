
all : program.lst main.hex burn
PART=attiny44
PROGPART=t44

CFLAGS=-g -Wall -pedantic -Os -mmcu=$(PART) -DF_CPU=20000000UL -fwhole-program
CC=avr-gcc

main.elf : main.c
	avr-gcc -I $(CFLAGS) -Wl,-Map,main.map -o $@ $^ -L/usr/lib/binutils/avr/2.18

main.hex : main.elf
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex 

burn : main.hex
	avrdude -c usbtiny -p $(PROGPART) -U flash:w:main.hex
# if you get the part borked, type -B 1024 and it will go REALLY slow


readfuses :
	avrdude -c usbtiny -p $(PROGPART) -U hfuse:r:high.txt:b -U lfuse:r:low.txt:b

clean : 
	rm -f *~ high.txt low.txt main.hex main.map main.elf *.o

burnfuses :
	avrdude -c usbtiny -p t44 -U lfuse:w:0xfe:m -U hfuse:w:0xdf:m

program.lst : main.c Functions.c
	avr-gcc -c -g -Wa,-a,-ad $(CFLAGS) $^ > $@

