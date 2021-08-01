EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Connector_Generic:Conn_02x05_Counter_Clockwise J?
U 1 1 6103FDC5
P 2600 2200
F 0 "J?" H 2650 2617 50  0000 C CNN
F 1 "Front Panel" H 2650 2526 50  0000 C CNN
F 2 "" H 2600 2200 50  0001 C CNN
F 3 "~" H 2600 2200 50  0001 C CNN
	1    2600 2200
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x02_Female J?
U 1 1 61040D5C
P 2700 2750
F 0 "J?" H 2728 2726 50  0000 L CNN
F 1 "Speaker" H 2728 2635 50  0000 L CNN
F 2 "" H 2700 2750 50  0001 C CNN
F 3 "~" H 2700 2750 50  0001 C CNN
	1    2700 2750
	1    0    0    -1  
$EndComp
$Comp
L Isolator:SFH617A-1 U?
U 1 1 61041940
P 3700 2200
F 0 "U?" H 3700 2525 50  0000 C CNN
F 1 "Power Optocoupler" H 3700 2434 50  0000 C CNN
F 2 "Package_DIP:DIP-4_W7.62mm" H 3500 2000 50  0001 L CIN
F 3 "http://www.vishay.com/docs/83740/sfh617a.pdf" H 3700 2200 50  0001 L CNN
	1    3700 2200
	-1   0    0    -1  
$EndComp
$Comp
L Isolator:SFH617A-1 U?
U 1 1 6104139D
P 1600 2200
F 0 "U?" H 1600 1883 50  0000 C CNN
F 1 "Reset Optocoupler" H 1600 1974 50  0000 C CNN
F 2 "Package_DIP:DIP-4_W7.62mm" H 1400 2000 50  0001 L CIN
F 3 "http://www.vishay.com/docs/83740/sfh617a.pdf" H 1600 2200 50  0001 L CNN
	1    1600 2200
	1    0    0    1   
$EndComp
Wire Wire Line
	1900 2300 2400 2300
Wire Wire Line
	2400 2200 1900 2200
Wire Wire Line
	1900 2200 1900 2100
Wire Wire Line
	3400 2300 2900 2300
Wire Wire Line
	3400 2100 3400 2200
Wire Wire Line
	3400 2200 2900 2200
$EndSCHEMATC
