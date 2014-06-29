EESchema Schematic File Version 2  date Sun 29 Jun 2014 05:42:30 PM EDT
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:special
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:attinyx4
LIBS:attiny
LIBS:PentaxIR-cache
EELAYER 25  0
EELAYER END
$Descr A4 11700 8267
encoding utf-8
Sheet 1 1
Title ""
Date "28 jun 2014"
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Connection ~ 8350 2350
Wire Wire Line
	8850 2350 8850 2500
Wire Wire Line
	7900 2350 8850 2350
Connection ~ 8150 2350
Wire Wire Line
	7900 2900 7900 2750
Wire Wire Line
	7650 3500 8350 3500
Wire Wire Line
	4000 3750 4000 4350
Wire Wire Line
	4000 4350 4500 4350
Wire Wire Line
	4500 4550 4000 4550
Wire Wire Line
	4000 4550 4000 4700
Wire Wire Line
	7900 3400 7900 3500
Connection ~ 7900 3500
Wire Wire Line
	8350 3500 8350 3400
Wire Wire Line
	8350 2900 8350 2750
Wire Wire Line
	8150 2350 8150 2150
Wire Wire Line
	8850 3150 8850 3700
Wire Wire Line
	8850 3700 7650 3700
$Comp
L SW_PUSH SW?
U 1 1 53AEFC30
P 8850 2800
F 0 "SW?" H 9000 2910 50  0000 C CNN
F 1 "SW_PUSH" H 8850 2720 50  0000 C CNN
	1    8850 2800
	0    1    1    0   
$EndComp
$Comp
L LED D?
U 1 1 53AEFBD3
P 8350 2550
F 0 "D?" H 8350 2650 50  0000 C CNN
F 1 "LED" H 8350 2450 50  0000 C CNN
	1    8350 2550
	0    -1   -1   0   
$EndComp
$Comp
L GND #PWR?
U 1 1 53AEFBBA
P 8150 2150
F 0 "#PWR?" H 8150 2150 30  0001 C CNN
F 1 "GND" H 8150 2080 30  0001 C CNN
	1    8150 2150
	-1   0    0    1   
$EndComp
$Comp
L LED D?
U 1 1 53AEFB9C
P 7900 2550
F 0 "D?" H 7900 2650 50  0000 C CNN
F 1 "LED" H 7900 2450 50  0000 C CNN
	1    7900 2550
	0    -1   -1   0   
$EndComp
$Comp
L R R?
U 1 1 53AEFB58
P 8350 3150
F 0 "R?" V 8430 3150 50  0000 C CNN
F 1 "220" V 8350 3150 50  0000 C CNN
	1    8350 3150
	1    0    0    -1  
$EndComp
$Comp
L R R?
U 1 1 53AEFB50
P 7900 3150
F 0 "R?" V 7980 3150 50  0000 C CNN
F 1 "220" V 7900 3150 50  0000 C CNN
	1    7900 3150
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR?
U 1 1 53AEFB1F
P 4000 4700
F 0 "#PWR?" H 4000 4700 30  0001 C CNN
F 1 "GND" H 4000 4630 30  0001 C CNN
	1    4000 4700
	1    0    0    -1  
$EndComp
$Comp
L +5V #PWR?
U 1 1 53AEFB09
P 4000 3750
F 0 "#PWR?" H 4000 3840 20  0001 C CNN
F 1 "+5V" H 4000 3840 30  0000 C CNN
	1    4000 3750
	1    0    0    -1  
$EndComp
$Comp
L ATTINY25-45-85/DIP-SO ~U?
U 1 1 53AEFAED
P 6050 4000
F 0 " U?" H 6025 4625 60  0000 C CNN
F 1 "ATTINY25-45-85/DIP-SO" H 6050 4775 60  0000 C CNN
	1    6050 4000
	1    0    0    -1  
$EndComp
$EndSCHEMATC
