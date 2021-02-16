EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 3 4
Title "AGCCS-Ctrl22"
Date "2021-02-15"
Rev "1.2"
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L smartevse:ATmega4808-XFR U10
U 1 1 5E9B0383
P 5750 3800
F 0 "U10" H 5750 5075 50  0000 C CNN
F 1 "ATmega4808-X" H 5750 4984 50  0000 C CNN
F 2 "Package_SO:SSOP-28_5.3x10.2mm_P0.65mm" H 5750 3800 50  0001 C CIN
F 3 "http://ww1.microchip.com/downloads/en/DeviceDoc/40002018A.pdf" H 5750 3800 50  0001 C CNN
F 4 "Mouser" H 5750 3800 50  0001 C CNN "Supplier"
	1    5750 3800
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR026
U 1 1 5E9B94B9
P 7350 3850
F 0 "#PWR026" H 7350 3600 50  0001 C CNN
F 1 "GND" V 7355 3722 50  0000 R CNN
F 2 "" H 7350 3850 50  0001 C CNN
F 3 "" H 7350 3850 50  0001 C CNN
	1    7350 3850
	0    -1   -1   0   
$EndComp
$Comp
L Device:C_Small C17
U 1 1 5E9B9A4C
P 7300 4000
F 0 "C17" H 7392 4046 50  0000 L CNN
F 1 "100n" H 7392 3955 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 7300 4000 50  0001 C CNN
F 3 "~" H 7300 4000 50  0001 C CNN
F 4 "" H 7300 4000 50  0001 C CNN "Part"
F 5 "963-EMF212B7104KGHT " H 7300 4000 50  0001 C CNN "PartNo"
F 6 "Mouser" H 7300 4000 50  0001 C CNN "Supplier"
	1    7300 4000
	1    0    0    -1  
$EndComp
Wire Wire Line
	7300 4150 7300 4100
Wire Wire Line
	7000 4150 7300 4150
Wire Wire Line
	7300 4150 7400 4150
Connection ~ 7300 4150
$Comp
L power:+3.3V #PWR027
U 1 1 5E9BBC72
P 7400 4150
F 0 "#PWR027" H 7400 4000 50  0001 C CNN
F 1 "+3.3V" V 7415 4278 50  0000 L CNN
F 2 "" H 7400 4150 50  0001 C CNN
F 3 "" H 7400 4150 50  0001 C CNN
	1    7400 4150
	0    1    1    0   
$EndComp
Wire Wire Line
	7300 3900 7300 3850
Connection ~ 7300 3850
Wire Wire Line
	7300 3850 7350 3850
Wire Wire Line
	7000 3850 7000 3950
Wire Wire Line
	7000 3850 7300 3850
Wire Wire Line
	6350 3950 7000 3950
Wire Wire Line
	7000 4150 7000 4100
Wire Wire Line
	7000 4100 6350 4100
Wire Wire Line
	6350 4850 6500 4850
Wire Wire Line
	6500 4850 6500 5000
$Comp
L Device:C_Small C15
U 1 1 5E9BECE4
P 5750 5000
F 0 "C15" V 5521 5000 50  0000 C CNN
F 1 "100n" V 5612 5000 50  0000 C CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 5750 5000 50  0001 C CNN
F 3 "~" H 5750 5000 50  0001 C CNN
F 4 "" H 5750 5000 50  0001 C CNN "Part"
F 5 "963-EMF212B7104KGHT " H 5750 5000 50  0001 C CNN "PartNo"
F 6 "Mouser" H 5750 5000 50  0001 C CNN "Supplier"
	1    5750 5000
	0    1    1    0   
$EndComp
Wire Wire Line
	4900 5000 5050 5000
Wire Wire Line
	5850 5000 6500 5000
$Comp
L power:GND #PWR023
U 1 1 5E9BF8B1
P 6500 5000
F 0 "#PWR023" H 6500 4750 50  0001 C CNN
F 1 "GND" H 6505 4827 50  0000 C CNN
F 2 "" H 6500 5000 50  0001 C CNN
F 3 "" H 6500 5000 50  0001 C CNN
	1    6500 5000
	1    0    0    -1  
$EndComp
Connection ~ 6500 5000
$Comp
L power:+3.3V #PWR019
U 1 1 5E9C0063
P 4900 4950
F 0 "#PWR019" H 4900 4800 50  0001 C CNN
F 1 "+3.3V" H 4915 5123 50  0000 C CNN
F 2 "" H 4900 4950 50  0001 C CNN
F 3 "" H 4900 4950 50  0001 C CNN
	1    4900 4950
	1    0    0    -1  
$EndComp
Wire Wire Line
	4900 4950 4900 5000
Wire Wire Line
	5150 4850 5050 4850
Wire Wire Line
	5050 4850 5050 5000
Connection ~ 5050 5000
Wire Wire Line
	5050 5000 5650 5000
Wire Wire Line
	6350 2750 6600 2750
Wire Wire Line
	6350 2900 6600 2900
Wire Wire Line
	6350 3050 6600 3050
Text HLabel 6900 3050 2    50   Output ~ 0
SSR_L3
Text HLabel 6900 2900 2    50   Output ~ 0
SSR_L2
Text HLabel 6900 2750 2    50   Output ~ 0
SSR_L1
$Comp
L Device:R_Small R8
U 1 1 5E9C3471
P 6700 2750
F 0 "R8" V 6750 2600 50  0000 C CNN
F 1 "100" V 6750 2900 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 6700 2750 50  0001 C CNN
F 3 "~" H 6700 2750 50  0001 C CNN
F 4 "" H 6700 2750 50  0001 C CNN "Part"
F 5 "594-MCT06030C1000FP5 " H 6700 2750 50  0001 C CNN "PartNo"
F 6 "Mouser" H 6700 2750 50  0001 C CNN "Supplier"
	1    6700 2750
	0    -1   -1   0   
$EndComp
$Comp
L Device:R_Small R9
U 1 1 5E9C406E
P 6700 2900
F 0 "R9" V 6750 2750 50  0000 C CNN
F 1 "100" V 6750 3050 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 6700 2900 50  0001 C CNN
F 3 "~" H 6700 2900 50  0001 C CNN
F 4 "" H 6700 2900 50  0001 C CNN "Part"
F 5 "594-MCT06030C1000FP5 " H 6700 2900 50  0001 C CNN "PartNo"
F 6 "Mouser" H 6700 2900 50  0001 C CNN "Supplier"
	1    6700 2900
	0    -1   -1   0   
$EndComp
Wire Wire Line
	6800 2750 6900 2750
Wire Wire Line
	6900 2900 6800 2900
$Comp
L Device:R_Small R10
U 1 1 5E9C5025
P 6700 3050
F 0 "R10" V 6750 2900 50  0000 C CNN
F 1 "100" V 6750 3200 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 6700 3050 50  0001 C CNN
F 3 "~" H 6700 3050 50  0001 C CNN
F 4 "" H 6700 3050 50  0001 C CNN "Part"
F 5 "594-MCT06030C1000FP5 " H 6700 3050 50  0001 C CNN "PartNo"
F 6 "Mouser" H 6700 3050 50  0001 C CNN "Supplier"
	1    6700 3050
	0    -1   -1   0   
$EndComp
Wire Wire Line
	6800 3050 6900 3050
$Comp
L smartevse:SN65HVD72C U7
U 1 1 5E9C58C2
P 2500 3900
F 0 "U7" H 2500 4508 69  0000 C CNN
F 1 "SN65HVD72C" H 2500 4388 69  0000 C CNN
F 2 "Package_SO:SOIC-8_3.9x4.9mm_P1.27mm" H 2500 3900 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/sn65hvd72.pdf?HQS=TI-null-null-mousermode-df-pf-null-wwe&ts=1590747515592" H 2500 3900 50  0001 C CNN
F 4 "Mouser" H 2500 3900 50  0001 C CNN "Supplier"
F 5 "595-SN65HVD72D" H 2500 3900 50  0001 C CNN "PartNo"
	1    2500 3900
	-1   0    0    -1  
$EndComp
Wire Wire Line
	3200 3700 3300 3700
Wire Wire Line
	3300 3700 3300 3300
$Comp
L power:+3.3V #PWR016
U 1 1 5E9CA545
P 3300 3300
F 0 "#PWR016" H 3300 3150 50  0001 C CNN
F 1 "+3.3V" H 3315 3473 50  0000 C CNN
F 2 "" H 3300 3300 50  0001 C CNN
F 3 "" H 3300 3300 50  0001 C CNN
	1    3300 3300
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C14
U 1 1 5E9CAC62
P 3400 3700
F 0 "C14" V 3629 3700 50  0000 C CNN
F 1 "100n" V 3538 3700 50  0000 C CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 3400 3700 50  0001 C CNN
F 3 "~" H 3400 3700 50  0001 C CNN
F 4 "" H 3400 3700 50  0001 C CNN "Part"
F 5 "963-EMF212B7104KGHT " H 3400 3700 50  0001 C CNN "PartNo"
F 6 "Mouser" H 3400 3700 50  0001 C CNN "Supplier"
	1    3400 3700
	0    -1   -1   0   
$EndComp
Connection ~ 3300 3700
Wire Wire Line
	3650 3700 3500 3700
$Comp
L power:GND #PWR018
U 1 1 5E9CBD4D
P 3650 3700
F 0 "#PWR018" H 3650 3450 50  0001 C CNN
F 1 "GND" H 3655 3527 50  0000 C CNN
F 2 "" H 3650 3700 50  0001 C CNN
F 3 "" H 3650 3700 50  0001 C CNN
	1    3650 3700
	1    0    0    -1  
$EndComp
Wire Wire Line
	3200 3900 3350 3900
Wire Wire Line
	3350 3900 3350 4000
Wire Wire Line
	3350 4000 3200 4000
Wire Wire Line
	3350 4000 4200 4000
Wire Wire Line
	4200 4000 4200 3350
Wire Wire Line
	4200 3350 5150 3350
Connection ~ 3350 4000
Wire Wire Line
	3200 4100 4000 4100
Wire Wire Line
	4000 4100 4000 2900
Wire Wire Line
	4000 2900 5150 2900
Wire Wire Line
	1800 3700 1700 3700
Wire Wire Line
	1700 3700 1700 3050
Wire Wire Line
	1700 3050 5150 3050
Wire Wire Line
	3350 4300 3200 4300
$Comp
L power:GND #PWR017
U 1 1 5E9D10DE
P 3350 4300
F 0 "#PWR017" H 3350 4050 50  0001 C CNN
F 1 "GND" H 3355 4127 50  0000 C CNN
F 2 "" H 3350 4300 50  0001 C CNN
F 3 "" H 3350 4300 50  0001 C CNN
	1    3350 4300
	1    0    0    -1  
$EndComp
Wire Wire Line
	1800 3900 1500 3900
Wire Wire Line
	1800 4000 1500 4000
Text Label 5100 3350 2    50   ~ 0
RS485_XDIR
Text Label 5100 3050 2    50   ~ 0
RS485_RX
Text Label 5100 2900 2    50   ~ 0
RS485_TX
$Comp
L smartevse:FAN3214TMX U8
U 1 1 5E9D4E3E
P 2800 6050
F 0 "U8" H 2800 6758 69  0000 C CNN
F 1 "FAN3214TMX" H 2800 6638 69  0000 C CNN
F 2 "Package_SO:SOIC-8_3.9x4.9mm_P1.27mm" H 2800 6050 50  0001 C CNN
F 3 "https://www.mouser.de/datasheet/2/308/FAN3214T_F085-D-1805909.pdf" H 2800 6050 50  0001 C CNN
F 4 "Mouser" H 2800 6050 50  0001 C CNN "Supplier"
F 5 "512-FAN3214TMX" H 2800 6050 50  0001 C CNN "PartNo"
	1    2800 6050
	1    0    0    -1  
$EndComp
Text Notes 2700 2750 2    50   ~ 0
RS485 Driver\n
$Comp
L Device:R_Small R3
U 1 1 5E9D7D11
P 1250 5550
F 0 "R3" V 1150 5400 50  0000 C CNN
F 1 "100" V 1145 5550 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 1250 5550 50  0001 C CNN
F 3 "~" H 1250 5550 50  0001 C CNN
F 4 "" H 1250 5550 50  0001 C CNN "Part"
F 5 "594-MCT06030C1000FP5 " H 1250 5550 50  0001 C CNN "PartNo"
F 6 "Mouser" H 1250 5550 50  0001 C CNN "Supplier"
	1    1250 5550
	0    1    1    0   
$EndComp
$Comp
L Device:R_Small R4
U 1 1 5E9D8F6A
P 1250 5750
F 0 "R4" V 1150 5600 50  0000 C CNN
F 1 "100" V 1145 5750 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 1250 5750 50  0001 C CNN
F 3 "~" H 1250 5750 50  0001 C CNN
F 4 "" H 1250 5750 50  0001 C CNN "Part"
F 5 "594-MCT06030C1000FP5 " H 1250 5750 50  0001 C CNN "PartNo"
F 6 "Mouser" H 1250 5750 50  0001 C CNN "Supplier"
	1    1250 5750
	0    1    1    0   
$EndComp
$Comp
L Device:R_Small R6
U 1 1 5E9D91A8
P 1250 6150
F 0 "R6" V 1150 6000 50  0000 C CNN
F 1 "100" V 1145 6150 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 1250 6150 50  0001 C CNN
F 3 "~" H 1250 6150 50  0001 C CNN
F 4 "" H 1250 6150 50  0001 C CNN "Part"
F 5 "594-MCT06030C1000FP5 " H 1250 6150 50  0001 C CNN "PartNo"
F 6 "Mouser" H 1250 6150 50  0001 C CNN "Supplier"
	1    1250 6150
	0    1    1    0   
$EndComp
$Comp
L Device:R_Small R5
U 1 1 5E9D94F8
P 1250 5950
F 0 "R5" V 1150 5800 50  0000 C CNN
F 1 "100" V 1145 5950 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 1250 5950 50  0001 C CNN
F 3 "~" H 1250 5950 50  0001 C CNN
F 4 "" H 1250 5950 50  0001 C CNN "Part"
F 5 "594-MCT06030C1000FP5 " H 1250 5950 50  0001 C CNN "PartNo"
F 6 "Mouser" H 1250 5950 50  0001 C CNN "Supplier"
	1    1250 5950
	0    1    1    0   
$EndComp
Wire Wire Line
	1400 5950 1350 5950
Wire Wire Line
	1150 5750 1050 5750
Wire Wire Line
	1050 5750 1050 5950
Wire Wire Line
	1050 5950 1150 5950
Wire Wire Line
	1050 5550 1050 5750
Connection ~ 1050 5750
$Comp
L power:+12V #PWR013
U 1 1 5E9DE707
P 650 5300
F 0 "#PWR013" H 650 5150 50  0001 C CNN
F 1 "+12V" H 665 5473 50  0000 C CNN
F 2 "" H 650 5300 50  0001 C CNN
F 3 "" H 650 5300 50  0001 C CNN
	1    650  5300
	1    0    0    -1  
$EndComp
Wire Wire Line
	2100 5550 2100 5750
$Comp
L Device:CP_Small C12
U 1 1 5E9E3618
P 1400 6600
F 0 "C12" H 1488 6646 50  0000 L CNN
F 1 "10000u16V" V 1250 6350 50  0000 L CNN
F 2 "Capacitor_THT:CP_Radial_D22.0mm_P10.00mm_SnapIn" H 1400 6600 50  0001 C CNN
F 3 "~" H 1400 6600 50  0001 C CNN
F 4 "" H 1400 6600 50  0001 C CNN "Part"
F 5 "871-B41231A4109M000" H 1400 6600 50  0001 C CNN "PartNo"
F 6 "Mouser" H 1400 6600 50  0001 C CNN "Supplier"
	1    1400 6600
	1    0    0    -1  
$EndComp
Connection ~ 2100 7000
$Comp
L power:GND #PWR015
U 1 1 5E9E6EF4
P 2100 7000
F 0 "#PWR015" H 2100 6750 50  0001 C CNN
F 1 "GND" H 2105 6827 50  0000 C CNN
F 2 "" H 2100 7000 50  0001 C CNN
F 3 "" H 2100 7000 50  0001 C CNN
	1    2100 7000
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C13
U 1 1 5E9E7944
P 1850 5650
F 0 "C13" H 1942 5696 50  0000 L CNN
F 1 "100n" V 1950 5450 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 1850 5650 50  0001 C CNN
F 3 "~" H 1850 5650 50  0001 C CNN
F 4 "" H 1850 5650 50  0001 C CNN "Part"
F 5 "963-EMF212B7104KGHT " H 1850 5650 50  0001 C CNN "PartNo"
F 6 "Mouser" H 1850 5650 50  0001 C CNN "Supplier"
	1    1850 5650
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR014
U 1 1 5E9EB17C
P 1850 5750
F 0 "#PWR014" H 1850 5500 50  0001 C CNN
F 1 "GND" H 1855 5577 50  0000 C CNN
F 2 "" H 1850 5750 50  0001 C CNN
F 3 "" H 1850 5750 50  0001 C CNN
	1    1850 5750
	1    0    0    -1  
$EndComp
Wire Wire Line
	2100 5950 1650 5950
Wire Wire Line
	1650 5950 1650 4600
Wire Wire Line
	1650 4600 4500 4600
Wire Wire Line
	4500 4600 4500 2750
Wire Wire Line
	4500 2750 5150 2750
Wire Wire Line
	5150 3200 4600 3200
Wire Wire Line
	4600 3200 4600 4700
Wire Wire Line
	4600 4700 1550 4700
Wire Wire Line
	1550 4700 1550 6050
Wire Wire Line
	1550 6050 2100 6050
Text Label 5100 3200 2    50   ~ 0
Lock_Drive_B
Text Label 5100 2750 2    50   ~ 0
Lock_Drive_A
$Comp
L Diode:BAT54S D2
U 1 1 5E9F9C87
P 3800 6300
F 0 "D2" V 3846 6388 50  0000 L CNN
F 1 "BAT54S" V 3755 6388 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23_Handsoldering" H 3875 6425 50  0001 L CNN
F 3 "https://www.diodes.com/assets/Datasheets/ds11005.pdf" H 3680 6300 50  0001 C CNN
F 4 "" H 3800 6300 50  0001 C CNN "Part"
F 5 "583-BAT54S " H 3800 6300 50  0001 C CNN "PartNo"
F 6 "Mouser" H 3800 6300 50  0001 C CNN "Supplier"
	1    3800 6300
	0    1    -1   0   
$EndComp
$Comp
L Diode:BAT54S D3
U 1 1 5E9FC0D6
P 4300 6300
F 0 "D3" V 4346 6388 50  0000 L CNN
F 1 "BAT54S" V 4255 6388 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23_Handsoldering" H 4375 6425 50  0001 L CNN
F 3 "https://www.diodes.com/assets/Datasheets/ds11005.pdf" H 4180 6300 50  0001 C CNN
F 4 "" H 4300 6300 50  0001 C CNN "Part"
F 5 "583-BAT54S " H 4300 6300 50  0001 C CNN "PartNo"
F 6 "Mouser" H 4300 6300 50  0001 C CNN "Supplier"
	1    4300 6300
	0    1    -1   0   
$EndComp
Wire Wire Line
	4300 6600 4300 7000
Wire Wire Line
	2100 7000 3800 7000
Wire Wire Line
	3800 7000 3800 6600
Connection ~ 3800 7000
Wire Wire Line
	3800 7000 4300 7000
Wire Wire Line
	3600 6300 3600 5850
Wire Wire Line
	3600 5850 3500 5850
Wire Wire Line
	3500 5750 4100 5750
Connection ~ 3600 5850
Wire Wire Line
	4500 5750 4100 5750
Connection ~ 4100 5750
Wire Wire Line
	3600 5850 4500 5850
Wire Wire Line
	4100 5750 4100 6300
Wire Wire Line
	2100 5550 2100 5250
Wire Wire Line
	2100 5250 3800 5250
Wire Wire Line
	3800 5250 3800 6000
Connection ~ 2100 5550
Wire Wire Line
	3800 5250 4300 5250
Wire Wire Line
	4300 5250 4300 6000
Connection ~ 3800 5250
Wire Notes Line
	550  5000 550  7250
Wire Notes Line
	550  7250 4450 7250
Wire Notes Line
	4450 7250 4450 5000
Wire Notes Line
	4450 5000 550  5000
Wire Notes Line
	3850 4550 1000 4550
Wire Notes Line
	1000 4550 1000 2650
Wire Notes Line
	1000 2650 3850 2650
Wire Notes Line
	3850 2650 3850 4550
Text Notes 2850 5150 2    50   ~ 0
Lock Actuator Driver\n
Text HLabel 1500 3900 0    50   BiDi ~ 0
RS485_A
Text HLabel 1500 4000 0    50   BiDi ~ 0
RS485_B
Text HLabel 4500 5750 2    50   Output ~ 0
Lock_W
Text HLabel 4500 5850 2    50   Output ~ 0
Lock_R
Wire Wire Line
	6900 5200 7500 5200
$Comp
L Diode:BAT54S D4
U 1 1 5EA24B11
P 7800 5750
F 0 "D4" V 7846 5838 50  0000 L CNN
F 1 "BAT54S" V 7755 5838 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23_Handsoldering" H 7875 5875 50  0001 L CNN
F 3 "https://www.diodes.com/assets/Datasheets/ds11005.pdf" H 7680 5750 50  0001 C CNN
F 4 "" H 7800 5750 50  0001 C CNN "Part"
F 5 "583-BAT54S " H 7800 5750 50  0001 C CNN "PartNo"
F 6 "Mouser" H 7800 5750 50  0001 C CNN "Supplier"
	1    7800 5750
	0    1    -1   0   
$EndComp
$Comp
L power:+3.3V #PWR025
U 1 1 5EA2A4B6
P 7800 5450
F 0 "#PWR025" H 7800 5300 50  0001 C CNN
F 1 "+3.3V" H 7815 5623 50  0000 C CNN
F 2 "" H 7800 5450 50  0001 C CNN
F 3 "" H 7800 5450 50  0001 C CNN
	1    7800 5450
	1    0    0    -1  
$EndComp
Wire Wire Line
	7600 5750 7500 5750
Wire Wire Line
	7500 5200 7500 5750
$Comp
L Device:R_Small R11
U 1 1 5EA33196
P 7500 5950
F 0 "R11" H 7559 5996 50  0000 L CNN
F 1 "10k" H 7559 5905 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 7500 5950 50  0001 C CNN
F 3 "~" H 7500 5950 50  0001 C CNN
F 4 "" H 7500 5950 50  0001 C CNN "Part"
F 5 "594-MCT06030C1002FP5 " H 7500 5950 50  0001 C CNN "PartNo"
F 6 "Mouser" H 7500 5950 50  0001 C CNN "Supplier"
	1    7500 5950
	1    0    0    -1  
$EndComp
Wire Wire Line
	7500 5850 7500 5750
Connection ~ 7500 5750
Wire Wire Line
	7500 6050 7500 6150
Wire Wire Line
	7500 6150 7800 6150
Wire Wire Line
	7800 6150 7800 6050
$Comp
L power:GND #PWR024
U 1 1 5EA39029
P 7500 6150
F 0 "#PWR024" H 7500 5900 50  0001 C CNN
F 1 "GND" H 7505 5977 50  0000 C CNN
F 2 "" H 7500 6150 50  0001 C CNN
F 3 "" H 7500 6150 50  0001 C CNN
	1    7500 6150
	1    0    0    -1  
$EndComp
Connection ~ 7500 6150
Wire Wire Line
	7500 5200 8000 5200
Connection ~ 7500 5200
$Comp
L Device:R_Small R12
U 1 1 5EA3C491
P 8100 5200
F 0 "R12" V 7904 5200 50  0000 C CNN
F 1 "31.6k" V 7995 5200 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 8100 5200 50  0001 C CNN
F 3 "~" H 8100 5200 50  0001 C CNN
F 4 "" H 8100 5200 50  0001 C CNN "Part"
F 5 "594-MCT06030C3162FP5" H 8100 5200 50  0001 C CNN "PartNo"
F 6 "Mouser" H 8100 5200 50  0001 C CNN "Supplier"
	1    8100 5200
	0    1    1    0   
$EndComp
Wire Wire Line
	8200 5200 8400 5200
Text HLabel 8400 5200 2    50   Input ~ 0
Lock_B
Wire Wire Line
	5150 4100 4700 4100
$Comp
L Transistor_FET:BSS138 Q1
U 1 1 5EA4C2D0
P 5350 5450
F 0 "Q1" H 5554 5496 50  0000 L CNN
F 1 "BSS138" H 5554 5405 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23_Handsoldering" H 5550 5375 50  0001 L CIN
F 3 "https://www.fairchildsemi.com/datasheets/BS/BSS138.pdf" H 5350 5450 50  0001 L CNN
F 4 "" H 5350 5450 50  0001 C CNN "Part"
F 5 "512-BSS138 " H 5350 5450 50  0001 C CNN "PartNo"
F 6 "Mouser" H 5350 5450 50  0001 C CNN "Supplier"
	1    5350 5450
	1    0    0    -1  
$EndComp
Wire Wire Line
	5150 5450 5050 5450
Wire Wire Line
	4700 4100 4700 5450
$Comp
L Device:R_Small R7
U 1 1 5EA5635B
P 5050 5700
F 0 "R7" H 5109 5746 50  0000 L CNN
F 1 "10k" H 5109 5655 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 5050 5700 50  0001 C CNN
F 3 "~" H 5050 5700 50  0001 C CNN
F 4 "" H 5050 5700 50  0001 C CNN "Part"
F 5 "594-MCT06030C1002FP5 " H 5050 5700 50  0001 C CNN "PartNo"
F 6 "Mouser" H 5050 5700 50  0001 C CNN "Supplier"
	1    5050 5700
	1    0    0    -1  
$EndComp
Wire Wire Line
	5050 5600 5050 5450
Connection ~ 5050 5450
Wire Wire Line
	5050 5450 4700 5450
Wire Wire Line
	5050 5800 5050 5950
Wire Wire Line
	5050 5950 5450 5950
Wire Wire Line
	5450 5950 5450 5650
$Comp
L power:GND #PWR020
U 1 1 5EA5D51E
P 5050 5950
F 0 "#PWR020" H 5050 5700 50  0001 C CNN
F 1 "GND" H 5055 5777 50  0000 C CNN
F 2 "" H 5050 5950 50  0001 C CNN
F 3 "" H 5050 5950 50  0001 C CNN
	1    5050 5950
	1    0    0    -1  
$EndComp
Connection ~ 5050 5950
Wire Wire Line
	5450 5250 5650 5250
Text HLabel 5650 5250 2    50   Output ~ 0
LED
Wire Wire Line
	5150 4400 5000 4400
Wire Wire Line
	5150 4550 5000 4550
Wire Wire Line
	5150 4700 5000 4700
Text HLabel 5000 4400 0    50   Input ~ 0
CT0
Text HLabel 5000 4550 0    50   Input ~ 0
CT1
Text HLabel 5000 4700 0    50   Input ~ 0
CT2
Wire Wire Line
	5150 4250 5000 4250
Text HLabel 5000 4250 0    50   Input ~ 0
Button
Text Label 7000 5200 0    50   ~ 0
Lock_State
Wire Wire Line
	5150 3500 5000 3500
Wire Wire Line
	5150 3650 5000 3650
Text HLabel 5000 3500 0    50   Input ~ 0
PP
Text HLabel 5000 3650 0    50   Input ~ 0
CP
Wire Wire Line
	6350 3350 6900 3350
Text HLabel 6900 3350 2    50   Output ~ 0
PWM_Out
Wire Wire Line
	6350 3200 6900 3200
Text HLabel 6900 3200 2    50   Output ~ 0
Signal_Relay
Wire Wire Line
	6350 4250 7900 4250
Wire Wire Line
	6350 3500 6900 3500
Wire Wire Line
	6350 3650 6900 3650
Text Label 6900 4400 2    50   ~ 0
ATmega_RESET
Text Label 6900 3500 2    50   ~ 0
ATmega_RX0
Text Label 6900 3650 2    50   ~ 0
ATmega_TX0
$Comp
L RF_Module:ESP32-WROOM-32 U9
U 1 1 5EC08E25
P 9550 4400
F 0 "U9" H 9550 6100 50  0000 C CNN
F 1 "ESP32-WROOM-32" H 9500 6000 50  0000 C CNN
F 2 "RF_Module:ESP32-WROOM-32" H 9550 2900 50  0001 C CNN
F 3 "https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32_datasheet_en.pdf" H 9250 4450 50  0001 C CNN
F 4 "Mouser" H 9550 4400 50  0001 C CNN "Supplier"
	1    9550 4400
	-1   0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_02x04_Top_Bottom J5
U 1 1 5EC1F0B2
P 9100 2100
F 0 "J5" H 9150 1800 50  0000 C CNN
F 1 "Conn_02x04_Top_Bottom" H 9150 2326 50  0000 C CNN
F 2 "atmevse-footprints:PROGRAM-Header" H 9100 2100 50  0001 C CNN
F 3 "~" H 9100 2100 50  0001 C CNN
F 4 "" H 9100 2100 50  0001 C CNN "Part"
F 5 "710-61300821121" H 9100 2100 50  0001 C CNN "PartNo"
F 6 "Mouser" H 9100 2100 50  0001 C CNN "Supplier"
	1    9100 2100
	1    0    0    -1  
$EndComp
Wire Wire Line
	10150 3200 10500 3200
Wire Wire Line
	10500 3200 10500 2300
Wire Wire Line
	8950 3200 8700 3200
Wire Wire Line
	9650 2000 9400 2000
$Comp
L power:GND #PWR033
U 1 1 5EC2FAAE
P 9650 2000
F 0 "#PWR033" H 9650 1750 50  0001 C CNN
F 1 "GND" V 9655 1872 50  0000 R CNN
F 2 "" H 9650 2000 50  0001 C CNN
F 3 "" H 9650 2000 50  0001 C CNN
	1    9650 2000
	0    -1   -1   0   
$EndComp
$Comp
L power:GND #PWR032
U 1 1 5EC306B7
P 9750 750
F 0 "#PWR032" H 9750 500 50  0001 C CNN
F 1 "GND" V 9755 622 50  0000 R CNN
F 2 "" H 9750 750 50  0001 C CNN
F 3 "" H 9750 750 50  0001 C CNN
	1    9750 750 
	0    -1   -1   0   
$EndComp
$Comp
L power:GND #PWR030
U 1 1 5EC30AAE
P 8700 750
F 0 "#PWR030" H 8700 500 50  0001 C CNN
F 1 "GND" V 8705 622 50  0000 R CNN
F 2 "" H 8700 750 50  0001 C CNN
F 3 "" H 8700 750 50  0001 C CNN
	1    8700 750 
	0    1    1    0   
$EndComp
Text Label 8300 900  0    50   ~ 0
ATmega_TX0
Text Label 8300 1100 0    50   ~ 0
ATmega_RX0
Text Label 8300 1300 0    50   ~ 0
ATmega_RESET
Text Label 8350 3300 0    50   ~ 0
ESP_TX0
Text Label 8350 3500 0    50   ~ 0
ESP_RX0
Wire Wire Line
	8300 3300 8950 3300
Wire Wire Line
	8200 3500 8950 3500
Wire Wire Line
	8950 3800 8200 3800
Text Label 8350 3800 0    50   ~ 0
ESP_IO12
Wire Wire Line
	8950 4200 8200 4200
Wire Wire Line
	8950 4300 8200 4300
Text Label 8350 4200 0    50   ~ 0
ESP_RX2
Text Label 8350 4300 0    50   ~ 0
ESP_TX2
Wire Wire Line
	9500 900  10100 900 
Wire Wire Line
	9500 1100 10100 1100
Wire Wire Line
	10100 1300 9500 1300
Text Label 9750 1300 0    50   ~ 0
ESP_IO12
Text Label 9750 1100 0    50   ~ 0
ESP_TX2
Text Label 9750 900  0    50   ~ 0
ESP_RX2
Text Label 7900 1700 0    50   ~ 0
ATmega_UDPI
$Comp
L power:+3.3V #PWR034
U 1 1 5ECB15D8
P 10000 2000
F 0 "#PWR034" H 10000 1850 50  0001 C CNN
F 1 "+3.3V" H 10015 2173 50  0000 C CNN
F 2 "" H 10000 2000 50  0001 C CNN
F 3 "" H 10000 2000 50  0001 C CNN
	1    10000 2000
	1    0    0    -1  
$EndComp
Text Notes 8450 650  0    50   ~ 0
ATmega/ESP Programming & Serial Comms\n
$Comp
L power:GND #PWR031
U 1 1 5ECB99BA
P 9550 5800
F 0 "#PWR031" H 9550 5550 50  0001 C CNN
F 1 "GND" H 9555 5627 50  0000 C CNN
F 2 "" H 9550 5800 50  0001 C CNN
F 3 "" H 9550 5800 50  0001 C CNN
	1    9550 5800
	1    0    0    -1  
$EndComp
Text Label 8750 3200 0    50   ~ 0
IO0
Text Label 10200 3200 0    50   ~ 0
EN
Text Label 9800 2950 0    50   ~ 0
ESP_VDD
Wire Wire Line
	9550 3000 9550 2950
Wire Wire Line
	9800 2950 9550 2950
$Comp
L Device:R R25
U 1 1 5EBDA7D4
P 10800 2950
F 0 "R25" H 10870 2996 50  0000 L CNN
F 1 "12k" H 10870 2905 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" V 10730 2950 50  0001 C CNN
F 3 "~" H 10800 2950 50  0001 C CNN
F 4 "" H 10800 2950 50  0001 C CNN "Part"
F 5 "594-MCT06030C1202FP5 " H 10800 2950 50  0001 C CNN "PartNo"
F 6 "Mouser" H 10800 2950 50  0001 C CNN "Supplier"
	1    10800 2950
	1    0    0    -1  
$EndComp
$Comp
L Device:C C21
U 1 1 5EBDAC66
P 10800 3450
F 0 "C21" H 10915 3496 50  0000 L CNN
F 1 "1n / 50V" H 10915 3405 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 10838 3300 50  0001 C CNN
F 3 "~" H 10800 3450 50  0001 C CNN
F 4 "" H 10800 3450 50  0001 C CNN "Part"
F 5 "77-VJ0805A102KXAAC" H 10800 3450 50  0001 C CNN "PartNo"
F 6 "Mouser" H 10800 3450 50  0001 C CNN "Supplier"
	1    10800 3450
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR036
U 1 1 5EBDB705
P 10800 3600
F 0 "#PWR036" H 10800 3350 50  0001 C CNN
F 1 "GND" H 10805 3427 50  0000 C CNN
F 2 "" H 10800 3600 50  0001 C CNN
F 3 "" H 10800 3600 50  0001 C CNN
	1    10800 3600
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR035
U 1 1 5EBDC16A
P 10800 2800
F 0 "#PWR035" H 10800 2650 50  0001 C CNN
F 1 "+3.3V" H 10815 2973 50  0000 C CNN
F 2 "" H 10800 2800 50  0001 C CNN
F 3 "" H 10800 2800 50  0001 C CNN
	1    10800 2800
	1    0    0    -1  
$EndComp
Wire Wire Line
	10800 3100 10800 3200
Wire Wire Line
	10800 3200 10500 3200
Connection ~ 10800 3200
Wire Wire Line
	10800 3200 10800 3300
Connection ~ 10500 3200
$Comp
L Device:CP_Small C16
U 1 1 5EC15E77
P 10650 4600
F 0 "C16" V 10550 4700 50  0000 L CNN
F 1 "100u / 6.3V" V 10750 4350 50  0000 L CNN
F 2 "Capacitor_Tantalum_SMD:CP_EIA-2012-15_AVX-P_Pad1.30x1.05mm_HandSolder" H 10650 4600 50  0001 C CNN
F 3 "~" H 10650 4600 50  0001 C CNN
F 4 "" H 10650 4600 50  0001 C CNN "Part"
F 5 "581-TLCR107M006RTA " H 10650 4600 50  0001 C CNN "PartNo"
F 6 "Mouser" H 10650 4600 50  0001 C CNN "Supplier"
	1    10650 4600
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C22
U 1 1 5EC16D76
P 10950 4600
F 0 "C22" V 10850 4700 50  0000 L CNN
F 1 "1u / 10V" V 11050 4450 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 10950 4600 50  0001 C CNN
F 3 "~" H 10950 4600 50  0001 C CNN
F 4 "" H 10950 4600 50  0001 C CNN "Part"
F 5 "77-VJ805Y105KXARW1BC " H 10950 4600 50  0001 C CNN "PartNo"
F 6 "Mouser" H 10950 4600 50  0001 C CNN "Supplier"
	1    10950 4600
	1    0    0    -1  
$EndComp
Wire Wire Line
	10650 4700 10650 4900
Wire Wire Line
	10650 4900 10800 4900
Wire Wire Line
	10950 4900 10950 4700
Wire Wire Line
	10800 4900 10800 5000
Connection ~ 10800 4900
Wire Wire Line
	10800 4900 10950 4900
$Comp
L power:GND #PWR038
U 1 1 5EC27338
P 10800 5000
F 0 "#PWR038" H 10800 4750 50  0001 C CNN
F 1 "GND" H 10805 4827 50  0000 C CNN
F 2 "" H 10800 5000 50  0001 C CNN
F 3 "" H 10800 5000 50  0001 C CNN
	1    10800 5000
	1    0    0    -1  
$EndComp
Wire Wire Line
	10650 4500 10650 4300
Wire Wire Line
	10650 4300 10800 4300
Wire Wire Line
	10950 4300 10950 4500
Wire Wire Line
	10800 4300 10800 4200
Connection ~ 10800 4300
Wire Wire Line
	10800 4300 10950 4300
$Comp
L power:+3.3V #PWR037
U 1 1 5EC38273
P 10800 4200
F 0 "#PWR037" H 10800 4050 50  0001 C CNN
F 1 "+3.3V" H 10815 4373 50  0000 C CNN
F 2 "" H 10800 4200 50  0001 C CNN
F 3 "" H 10800 4200 50  0001 C CNN
	1    10800 4200
	1    0    0    -1  
$EndComp
Text Label 10800 4300 2    50   ~ 0
ESP_VDD
NoConn ~ 8950 3600
NoConn ~ 8950 3700
NoConn ~ 8950 3900
NoConn ~ 8950 4000
NoConn ~ 8950 4400
NoConn ~ 8950 4500
NoConn ~ 8950 4600
NoConn ~ 8950 4700
NoConn ~ 8950 4800
NoConn ~ 8950 4900
NoConn ~ 8950 5000
NoConn ~ 8950 5100
NoConn ~ 8950 5200
NoConn ~ 8950 5300
NoConn ~ 8950 5400
NoConn ~ 8950 5500
NoConn ~ 10150 4900
NoConn ~ 10150 4800
NoConn ~ 10150 4700
NoConn ~ 10150 4600
NoConn ~ 10150 4500
NoConn ~ 10150 4400
NoConn ~ 10150 3400
NoConn ~ 10150 3500
NoConn ~ 8950 3400
Text Notes 5400 2250 0    50   ~ 0
ATmega UART:\n0 - PA0, PA1\n1 - PC0, PC1\n2 - PF0, PF1
Wire Notes Line
	5350 1900 5350 2300
Wire Notes Line
	5350 2300 6000 2300
Wire Notes Line
	6000 2300 6000 1900
Wire Notes Line
	6000 1900 5350 1900
Text Label 5050 3950 2    50   ~ 0
Lock_State
Wire Wire Line
	5050 3950 5150 3950
Wire Wire Line
	6350 4550 6900 4550
Wire Wire Line
	6350 4700 6900 4700
Text Label 6450 4550 0    50   ~ 0
ATmega_RX2
Text Label 6450 4700 0    50   ~ 0
ATmega_TX2
$Comp
L Connector_Generic:Conn_01x04 J3
U 1 1 5EC87485
P 5750 1200
F 0 "J3" H 5830 1192 50  0000 L CNN
F 1 "Conn_01x04" H 5830 1101 50  0000 L CNN
F 2 "atmevse-footprints:UART2-Header" H 5750 1200 50  0001 C CNN
F 3 "~" H 5750 1200 50  0001 C CNN
F 4 "" H 5750 1200 50  0001 C CNN "Part"
F 5 "710-61304011121" H 5750 1200 50  0001 C CNN "PartNo"
F 6 "Mouser" H 5750 1200 50  0001 C CNN "Supplier"
	1    5750 1200
	1    0    0    -1  
$EndComp
Wire Wire Line
	5550 1100 5300 1100
Wire Wire Line
	5300 1400 5550 1400
$Comp
L power:+12V #PWR021
U 1 1 5ECA8BCC
P 5300 1100
F 0 "#PWR021" H 5300 950 50  0001 C CNN
F 1 "+12V" H 5315 1273 50  0000 C CNN
F 2 "" H 5300 1100 50  0001 C CNN
F 3 "" H 5300 1100 50  0001 C CNN
	1    5300 1100
	1    0    0    -1  
$EndComp
Text Label 5100 1200 0    50   ~ 0
ATmega_RX2
Wire Wire Line
	5100 1200 5550 1200
Text Label 5100 1300 0    50   ~ 0
ATmega_TX2
Wire Wire Line
	5100 1300 5550 1300
$Comp
L power:GND #PWR022
U 1 1 5ECB9625
P 5300 1400
F 0 "#PWR022" H 5300 1150 50  0001 C CNN
F 1 "GND" H 5305 1227 50  0000 C CNN
F 2 "" H 5300 1400 50  0001 C CNN
F 3 "" H 5300 1400 50  0001 C CNN
	1    5300 1400
	1    0    0    -1  
$EndComp
Wire Notes Line
	5000 750  6300 750 
Wire Notes Line
	6300 750  6300 1700
Wire Notes Line
	6300 1700 5000 1700
Wire Notes Line
	5000 1700 5000 750 
Text Notes 5200 700  0    50   ~ 0
ATmega UART2 Connector\n
Wire Wire Line
	1050 5950 1050 6150
Wire Wire Line
	1050 6150 1150 6150
Connection ~ 1050 5950
Wire Wire Line
	1050 5550 1150 5550
Wire Wire Line
	1350 6150 1400 6150
Text Label 8350 4100 0    50   ~ 0
ESP_IO15
$Comp
L Connector_Generic:Conn_01x04 J4
U 1 1 5F466074
P 9300 1550
F 0 "J4" H 9450 1350 50  0000 C CNN
F 1 "Conn_01x04" H 9600 1450 50  0000 C CNN
F 2 "atmevse-footprints:UART0-Header" H 9300 1550 50  0001 C CNN
F 3 "~" H 9300 1550 50  0001 C CNN
	1    9300 1550
	1    0    0    -1  
$EndComp
Wire Wire Line
	9100 750  9750 750 
Connection ~ 9100 750 
$Comp
L Device:R_Small R34
U 1 1 5F47B25C
P 9400 900
F 0 "R34" V 9350 1050 50  0000 C CNN
F 1 "1k" V 9295 900 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 9400 900 50  0001 C CNN
F 3 "~" H 9400 900 50  0001 C CNN
	1    9400 900 
	0    1    1    0   
$EndComp
$Comp
L Device:R_Small R37
U 1 1 5F47BC59
P 9400 1100
F 0 "R37" V 9350 1250 50  0000 C CNN
F 1 "1k" V 9295 1100 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 9400 1100 50  0001 C CNN
F 3 "~" H 9400 1100 50  0001 C CNN
	1    9400 1100
	0    1    1    0   
$EndComp
$Comp
L Device:R_Small R38
U 1 1 5F47BE53
P 9400 1300
F 0 "R38" V 9350 1450 50  0000 C CNN
F 1 "1k" V 9295 1300 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 9400 1300 50  0001 C CNN
F 3 "~" H 9400 1300 50  0001 C CNN
	1    9400 1300
	0    1    1    0   
$EndComp
Wire Wire Line
	8300 900  9000 900 
Wire Wire Line
	8300 1100 8900 1100
Wire Wire Line
	8300 1300 8800 1300
Wire Wire Line
	9100 750  9100 1450
Wire Wire Line
	9000 1550 9100 1550
Wire Wire Line
	8700 750  9100 750 
Wire Wire Line
	9000 900  9000 1550
Connection ~ 9000 900 
Wire Wire Line
	9000 900  9300 900 
Wire Wire Line
	8900 1100 8900 1650
Wire Wire Line
	8900 1650 9100 1650
Connection ~ 8900 1100
Wire Wire Line
	8900 1100 9300 1100
Wire Wire Line
	8800 1300 8800 1750
Wire Wire Line
	8800 1750 9100 1750
Connection ~ 8800 1300
Wire Wire Line
	8800 1300 9300 1300
Wire Wire Line
	10000 2000 10000 2100
Wire Wire Line
	10000 2100 9400 2100
Wire Wire Line
	8900 2000 7900 2000
Wire Wire Line
	7900 2000 7900 4250
Wire Wire Line
	8900 2100 8300 2100
Wire Wire Line
	8300 2100 8300 3300
Wire Wire Line
	8900 2200 8200 2200
Wire Wire Line
	8200 2200 8200 3500
Wire Wire Line
	8950 4100 8200 4100
Wire Wire Line
	8900 2300 8700 2300
Wire Wire Line
	8700 2300 8700 3200
Wire Wire Line
	9400 2300 10500 2300
Text Label 9700 2200 0    50   ~ 0
ESP_IO15
Wire Wire Line
	9700 2200 9400 2200
Wire Wire Line
	6350 4400 6900 4400
Wire Wire Line
	2100 7000 2100 6550
Wire Wire Line
	1400 7000 2100 7000
Wire Wire Line
	1400 6700 1400 7000
Wire Wire Line
	1850 5550 2100 5550
Connection ~ 1850 5550
Wire Wire Line
	1400 5550 1850 5550
Wire Wire Line
	1400 5550 1400 5750
Wire Wire Line
	1350 5750 1400 5750
$Comp
L Diode:B220 D12
U 1 1 603DC56F
P 850 5550
F 0 "D12" H 850 5333 50  0000 C CNN
F 1 "B220" H 850 5424 50  0000 C CNN
F 2 "Diode_SMD:D_SMB_Handsoldering" H 850 5375 50  0001 C CNN
F 3 "http://www.jameco.com/Jameco/Products/ProdDS/1538777.pdf" H 850 5550 50  0001 C CNN
	1    850  5550
	-1   0    0    1   
$EndComp
Text Label 3250 5250 0    50   ~ 0
12V_sep
Connection ~ 1400 5750
Wire Wire Line
	1400 5750 1400 5950
Connection ~ 1400 5950
Wire Wire Line
	1400 5950 1400 6150
Connection ~ 1400 6150
Wire Wire Line
	1400 6150 1400 6500
Wire Wire Line
	1350 5550 1400 5550
Connection ~ 1400 5550
Wire Wire Line
	1000 5550 1050 5550
Connection ~ 1050 5550
Wire Wire Line
	700  5550 650  5550
Wire Wire Line
	650  5550 650  5300
$EndSCHEMATC
