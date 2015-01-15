EESchema Schematic File Version 2  date Fri 19 Dec 2014 03:22:09 PM EST
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
LIBS:aa_Drain-cache
EELAYER 25  0
EELAYER END
$Descr A4 11700 8267
encoding utf-8
Sheet 1 1
Title ""
Date "19 dec 2014"
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L GND #PWR?
U 1 1 54922724
P 6450 3450
F 0 "#PWR?" H 6450 3450 30  0001 C CNN
F 1 "GND" H 6450 3380 30  0001 C CNN
	1    6450 3450
	0    -1   -1   0   
$EndComp
$Comp
L LED D?
U 1 1 5492271A
P 6250 3450
F 0 "D?" H 6250 3550 50  0000 C CNN
F 1 "LED" H 6250 3350 50  0000 C CNN
	1    6250 3450
	1    0    0    -1  
$EndComp
$Comp
L R R?
U 1 1 54922705
P 6050 3200
F 0 "R?" V 6130 3200 50  0000 C CNN
F 1 "220" V 6050 3200 50  0000 C CNN
	1    6050 3200
	1    0    0    -1  
$EndComp
$Comp
L LM358N U?
U 1 1 5491F254
P 5550 2950
F 0 "U?" H 5500 3150 60  0000 L CNN
F 1 "LM358N" H 5500 2700 60  0000 L CNN
	1    5550 2950
	1    0    0    -1  
$EndComp
$Comp
L NPN Q?
U 1 1 5491F26D
P 6750 2950
F 0 "Q?" H 6750 2800 50  0000 R CNN
F 1 "PN2222A" H 6750 3100 50  0000 R CNN
F 2 "~" H 6750 2950 60  0000 C CNN
F 3 "~" H 6750 2950 60  0000 C CNN
	1    6750 2950
	1    0    0    -1  
$EndComp
$Comp
L +BATT #PWR?
U 1 1 5491F286
P 6850 2250
F 0 "#PWR?" H 6850 2200 20  0001 C CNN
F 1 "+BATT" H 6850 2350 30  0000 C CNN
	1    6850 2250
	1    0    0    -1  
$EndComp
$Comp
L R R?
U 1 1 5491F295
P 6750 2500
F 0 "R?" V 6830 2500 40  0000 C CNN
F 1 "10" V 6757 2501 40  0000 C CNN
F 2 "~" V 6680 2500 30  0000 C CNN
F 3 "~" H 6750 2500 30  0000 C CNN
	1    6750 2500
	1    0    0    -1  
$EndComp
Wire Wire Line
	5050 2850 5050 2250
Wire Wire Line
	5050 2250 6950 2250
$Comp
L GND #PWR?
U 1 1 5491F2B9
P 6850 3250
F 0 "#PWR?" H 6850 3250 30  0001 C CNN
F 1 "GND" H 6850 3180 30  0001 C CNN
	1    6850 3250
	1    0    0    -1  
$EndComp
$Comp
L R R?
U 1 1 5491F40F
P 6950 2500
F 0 "R?" V 7030 2500 40  0000 C CNN
F 1 "10" V 6957 2501 40  0000 C CNN
F 2 "~" V 6880 2500 30  0000 C CNN
F 3 "~" H 6950 2500 30  0000 C CNN
	1    6950 2500
	1    0    0    -1  
$EndComp
Connection ~ 6850 2250
Wire Wire Line
	6750 2750 6950 2750
Connection ~ 6850 2750
$Comp
L R R?
U 1 1 5491F6E5
P 6300 2950
F 0 "R?" V 6380 2950 40  0000 C CNN
F 1 "220" V 6307 2951 40  0000 C CNN
F 2 "~" V 6230 2950 30  0000 C CNN
F 3 "~" H 6300 2950 30  0000 C CNN
	1    6300 2950
	0    -1   -1   0   
$EndComp
Wire Wire Line
	6850 3150 6850 3250
$Comp
L R R?
U 1 1 5491F797
P 5050 3300
F 0 "R?" V 5130 3300 40  0000 C CNN
F 1 "5.6k" V 5057 3301 40  0000 C CNN
F 2 "~" V 4980 3300 30  0000 C CNN
F 3 "~" H 5050 3300 30  0000 C CNN
	1    5050 3300
	1    0    0    -1  
$EndComp
$Comp
L R R?
U 1 1 5491F7A6
P 4800 3050
F 0 "R?" V 4880 3050 40  0000 C CNN
F 1 "10k" V 4807 3051 40  0000 C CNN
F 2 "~" V 4730 3050 30  0000 C CNN
F 3 "~" H 4800 3050 30  0000 C CNN
	1    4800 3050
	0    -1   -1   0   
$EndComp
$Comp
L +5V #PWR?
U 1 1 5491F80D
P 4500 2550
F 0 "#PWR?" H 4500 2640 20  0001 C CNN
F 1 "+5V" H 4500 2640 30  0000 C CNN
	1    4500 2550
	0    -1   -1   0   
$EndComp
Wire Wire Line
	5450 2550 4500 2550
$Comp
L GND #PWR?
U 1 1 5491F823
P 5450 3550
F 0 "#PWR?" H 5450 3550 30  0001 C CNN
F 1 "GND" H 5450 3480 30  0001 C CNN
	1    5450 3550
	1    0    0    -1  
$EndComp
Wire Wire Line
	5450 3350 5450 3550
$Comp
L GND #PWR?
U 1 1 5491F844
P 5050 3550
F 0 "#PWR?" H 5050 3550 30  0001 C CNN
F 1 "GND" H 5050 3480 30  0001 C CNN
	1    5050 3550
	1    0    0    -1  
$EndComp
$Comp
L +2.5V #PWR?
U 1 1 54923E1E
P 4550 3050
F 0 "#PWR?" H 4550 3000 20  0001 C CNN
F 1 "+2.5V" H 4550 3150 30  0000 C CNN
	1    4550 3050
	0    -1   -1   0   
$EndComp
$EndSCHEMATC