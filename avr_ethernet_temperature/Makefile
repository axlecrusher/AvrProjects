all : program.hex burn program.lst
CC = avr-gcc

SRCS = test.c avr_print.c ds18s20.c

PROCESSOR=atmega328
PROGRAMCODE=m328
#CPUFREQ=28636363UL
#CPUFREQ=20000000UL
CPUFREQ=8000000UL
COMMON_FLAGS= -mmcu=$(PROCESSOR) -I.
CFLAGS = -std=gnu99 -O1 $(COMMON_FLAGS) -DF_CPU=$(CPUFREQ) #-DNO_HTTP #-DMUTE_PRINTF #-DNO_HTTP #-Wall
CFLAGS:=$(CFLAGS) #-flto -fwhole-program 
AS = avr-gcc
ASFLAGS = $(COMMON_FLAGS) -c
EXTRA_LD_FLAGS=-Wl,--relax -Wa,-a,-ad -mcall-prologues 

program.lst : $(SRCS)
	avr-gcc -c -g -Wa,-a,-ad $(CFLAGS) $^ > $@

program.elf : $(SRCS)
	avr-gcc -I -mmcu=$(PROCESSOR) $(CFLAGS) -o $@ $^ -L /usr/lib64/binutils/avr/2.19.1 $(EXTRA_LD_FLAGS) > program.lst
	avr-size -A $@
	@echo "Total = .data + .text"
	avr-objdump program.elf -t > program.map

program.hex : program.elf
	avr-objcopy -j .text -j .data -O ihex program.elf program.hex 

burn : program.hex
	avrdude -c usbtiny -p $(PROGRAMCODE) -V -U flash:w:program.hex 
#-V 	

readfuses :
	avrdude -c usbtiny -p $(PROGRAMCODE) -U hfuse:r:high.txt:b -U lfuse:r:low.txt:b

#careful here...
#burnfuses :
#	sudo avrdude -c usbtiny -p $(PROGRAMCODE) -U lfuse:w:0xE6:m -U hfuse:w:0xDF:m
	#use crystal on mega48/168
burnfuses :
	avrdude -c usbtiny -p $(PROGRAMCODE) -U lfuse:w:0xe2:m -U hfuse:w:0xdf:m
#	avrdude -c usbtiny -p $(PROGRAMCODE) -U lfuse:w:0xE6:m -U hfuse:w:0xDF:m

clean : 
	rm -f *~ high.txt low.txt program.hex program.map program.elf $(OBJS) *.o usbdrv/*.o usbdrv/*~ program.lst

