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
	6350 4850 6400 4850
Wire Wire Line
	6400 4850 6400 5000
Wire Wire Line
	4900 5000 5100 5000
Wire Wire Line
	5850 5000 6400 5000
$Comp
L power:GND #PWR023
U 1 1 5E9BF8B1
P 6400 5000
F 0 "#PWR023" H 6400 4750 50  0001 C CNN
F 1 "GND" H 6405 4827 50  0000 C CNN
F 2 "" H 6400 5000 50  0001 C CNN
F 3 "" H 6400 5000 50  0001 C CNN
	1    6400 5000
	1    0    0    -1  
$EndComp
Connection ~ 6400 5000
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
	5150 4850 5100 4850
Wire Wire Line
	5100 4850 5100 5000
Connection ~ 5100 5000
Wire Wire Line
	5100 5000 5650 5000
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
Wire Wire Line
	6800 2750 6900 2750
Wire Wire Line
	6900 2900 6800 2900
Wire Wire Line
	6800 3050 6900 3050
Wire Wire Line
	2525 1425 2625 1425
Wire Wire Line
	2625 1425 2625 800 
$Comp
L power:+3.3V #PWR016
U 1 1 5E9CA545
P 2625 800
F 0 "#PWR016" H 2625 650 50  0001 C CNN
F 1 "+3.3V" H 2640 973 50  0000 C CNN
F 2 "" H 2625 800 50  0001 C CNN
F 3 "" H 2625 800 50  0001 C CNN
	1    2625 800 
	1    0    0    -1  
$EndComp
Connection ~ 2625 1425
Wire Wire Line
	2975 1425 2825 1425
$Comp
L power:GND #PWR018
U 1 1 5E9CBD4D
P 2975 1425
F 0 "#PWR018" H 2975 1175 50  0001 C CNN
F 1 "GND" H 2980 1252 50  0000 C CNN
F 2 "" H 2975 1425 50  0001 C CNN
F 3 "" H 2975 1425 50  0001 C CNN
	1    2975 1425
	1    0    0    -1  
$EndComp
Wire Wire Line
	2525 1625 2675 1625
Wire Wire Line
	2675 1625 2675 1725
Wire Wire Line
	2675 1725 2525 1725
Wire Wire Line
	3900 3350 5150 3350
Wire Wire Line
	2675 2025 2525 2025
$Comp
L power:GND #PWR017
U 1 1 5E9D10DE
P 2675 2025
F 0 "#PWR017" H 2675 1775 50  0001 C CNN
F 1 "GND" H 2680 1852 50  0000 C CNN
F 2 "" H 2675 2025 50  0001 C CNN
F 3 "" H 2675 2025 50  0001 C CNN
	1    2675 2025
	1    0    0    -1  
$EndComp
Wire Wire Line
	1125 1625 1000 1625
Wire Wire Line
	1125 1725 1000 1725
Text Label 5100 3350 2    50   ~ 0
RS485_XDIR
Text Label 5100 3050 2    50   ~ 0
AVR_RX1
Text Label 5100 2900 2    50   ~ 0
AVR_TX1
Text Notes 1100 675  2    50   ~ 0
RS485 Driver\n
Wire Wire Line
	1250 5625 1250 5850
Connection ~ 1250 7100
$Comp
L power:GND #PWR015
U 1 1 5E9E6EF4
P 1250 7100
F 0 "#PWR015" H 1250 6850 50  0001 C CNN
F 1 "GND" H 1255 6927 50  0000 C CNN
F 2 "" H 1250 7100 50  0001 C CNN
F 3 "" H 1250 7100 50  0001 C CNN
	1    1250 7100
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR014
U 1 1 5E9EB17C
P 1000 5825
F 0 "#PWR014" H 1000 5575 50  0001 C CNN
F 1 "GND" H 1005 5652 50  0000 C CNN
F 2 "" H 1000 5825 50  0001 C CNN
F 3 "" H 1000 5825 50  0001 C CNN
	1    1000 5825
	1    0    0    -1  
$EndComp
Wire Wire Line
	1250 6050 800  6050
Wire Wire Line
	4500 2750 5150 2750
Wire Wire Line
	700  6150 1250 6150
Text Label 5100 3200 2    50   ~ 0
Lock_Drive_B
Text Label 5100 2750 2    50   ~ 0
Lock_Drive_A
Wire Wire Line
	3450 6700 3450 7100
Wire Wire Line
	1250 7100 2950 7100
Wire Wire Line
	2950 7100 2950 6700
Connection ~ 2950 7100
Wire Wire Line
	2950 7100 3450 7100
Wire Wire Line
	2750 6400 2750 5950
Wire Wire Line
	2750 5950 2650 5950
Wire Wire Line
	2650 5850 3250 5850
Connection ~ 2750 5950
Wire Wire Line
	3650 5850 3250 5850
Connection ~ 3250 5850
Wire Wire Line
	2750 5950 3650 5950
Wire Wire Line
	3250 5850 3250 6400
Wire Wire Line
	1250 5625 1250 5350
Wire Wire Line
	1250 5350 2950 5350
Wire Wire Line
	2950 5350 2950 6100
Connection ~ 1250 5625
Wire Wire Line
	2950 5350 3450 5350
Wire Wire Line
	3450 5350 3450 6100
Connection ~ 2950 5350
Text HLabel 1000 1625 0    50   BiDi ~ 0
RS485_A
Text HLabel 1000 1725 0    50   BiDi ~ 0
RS485_B
Text HLabel 3650 5850 2    50   Output ~ 0
Lock_W
Text HLabel 3650 5950 2    50   Output ~ 0
Lock_R
Wire Wire Line
	4975 5950 4975 6500
Wire Wire Line
	4975 6600 4975 6500
Connection ~ 4975 6500
Wire Wire Line
	4975 5950 4825 5950
Connection ~ 4975 5950
Wire Wire Line
	4625 5950 4425 5950
Text HLabel 4425 5950 0    50   Input ~ 0
Lock_B
Wire Wire Line
	1625 2850 1625 2700
Wire Wire Line
	1625 3050 1625 3200
Wire Wire Line
	1625 3200 1225 3200
Wire Wire Line
	1225 3200 1225 2900
$Comp
L power:GND #PWR020
U 1 1 5EA5D51E
P 1625 3200
F 0 "#PWR020" H 1625 2950 50  0001 C CNN
F 1 "GND" H 1630 3027 50  0000 C CNN
F 2 "" H 1625 3200 50  0001 C CNN
F 3 "" H 1625 3200 50  0001 C CNN
	1    1625 3200
	-1   0    0    -1  
$EndComp
Connection ~ 1625 3200
Text HLabel 875  2425 0    50   Output ~ 0
LED
Wire Wire Line
	5150 4400 5000 4400
Wire Wire Line
	5150 4550 5000 4550
Wire Wire Line
	5150 4700 5000 4700
Text HLabel 5000 4400 0    50   Input ~ 0
CT1
Text HLabel 5000 4550 0    50   Input ~ 0
CT2
Text HLabel 5000 4700 0    50   Input ~ 0
CT3
Text Label 5475 5950 2    50   ~ 0
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
	6350 4250 8000 4250
Wire Wire Line
	6350 3500 6900 3500
Wire Wire Line
	6350 3650 6900 3650
Text Label 6900 4400 2    50   ~ 0
AVR_RST
Text Label 6900 3500 2    50   ~ 0
AVR_RX0
Text Label 6900 3650 2    50   ~ 0
AVR_TX0
Wire Wire Line
	9650 925  9400 925 
$Comp
L power:GND #PWR033
U 1 1 5EC2FAAE
P 9650 925
F 0 "#PWR033" H 9650 675 50  0001 C CNN
F 1 "GND" V 9655 797 50  0000 R CNN
F 2 "" H 9650 925 50  0001 C CNN
F 3 "" H 9650 925 50  0001 C CNN
	1    9650 925 
	0    -1   -1   0   
$EndComp
Text Label 5625 925  0    50   ~ 0
AVR_TX0
Text Label 5625 1125 0    50   ~ 0
AVR_RX0
Text Label 5625 1325 0    50   ~ 0
AVR_RST
Wire Wire Line
	6775 1125 7375 1125
Wire Wire Line
	7375 1325 6775 1325
Text Label 7025 1325 0    50   ~ 0
ESP_IO12
Text Label 7025 1125 0    50   ~ 0
ESP_TX2
Text Label 7025 925  0    50   ~ 0
ESP_RX2
Text Label 8575 925  0    50   ~ 0
AVR_UDPI
$Comp
L power:+3.3V #PWR034
U 1 1 5ECB15D8
P 10000 925
F 0 "#PWR034" H 10000 775 50  0001 C CNN
F 1 "+3.3V" H 10015 1098 50  0000 C CNN
F 2 "" H 10000 925 50  0001 C CNN
F 3 "" H 10000 925 50  0001 C CNN
	1    10000 925 
	1    0    0    -1  
$EndComp
Text Notes 5625 700  0    50   ~ 0
ATmega/ESP Serial Link\n(incl. Programming Atmega via J4)\n
Text Label 9400 1225 0    50   ~ 0
ESP_EN
$Comp
L power:GND #PWR038
U 1 1 5EC27338
P 10750 4600
F 0 "#PWR038" H 10750 4350 50  0001 C CNN
F 1 "GND" H 10755 4427 50  0000 C CNN
F 2 "" H 10750 4600 50  0001 C CNN
F 3 "" H 10750 4600 50  0001 C CNN
	1    10750 4600
	1    0    0    -1  
$EndComp
Text Label 5050 3950 2    50   ~ 0
Lock_State
Wire Wire Line
	6350 4550 6900 4550
Text Label 6500 4550 0    50   ~ 0
AVR_RX2
Text Label 6500 4700 0    50   ~ 0
AVR_TX2
Wire Wire Line
	4875 925  4700 925 
Wire Wire Line
	4700 1225 4875 1225
$Comp
L power:+12V #PWR021
U 1 1 5ECA8BCC
P 4700 925
F 0 "#PWR021" H 4700 775 50  0001 C CNN
F 1 "+12V" H 4715 1098 50  0000 C CNN
F 2 "" H 4700 925 50  0001 C CNN
F 3 "" H 4700 925 50  0001 C CNN
	1    4700 925 
	1    0    0    -1  
$EndComp
Text Label 4425 1025 0    50   ~ 0
AVR_RX2
Wire Wire Line
	4425 1025 4875 1025
Text Label 4425 1125 0    50   ~ 0
AVR_TX2
Wire Wire Line
	4425 1125 4875 1125
$Comp
L power:GND #PWR022
U 1 1 5ECB9625
P 4700 1225
F 0 "#PWR022" H 4700 975 50  0001 C CNN
F 1 "GND" H 4705 1052 50  0000 C CNN
F 2 "" H 4700 1225 50  0001 C CNN
F 3 "" H 4700 1225 50  0001 C CNN
	1    4700 1225
	1    0    0    -1  
$EndComp
Text Notes 4275 625  0    50   ~ 0
ATmega Extension Connector\n
Wire Wire Line
	10000 925  10000 1025
Wire Wire Line
	10000 1025 9400 1025
Wire Wire Line
	8900 925  8000 925 
Wire Wire Line
	8000 925  8000 4250
Wire Wire Line
	8900 1025 8300 1025
Wire Wire Line
	8900 1125 8200 1125
Wire Wire Line
	8900 1225 8400 1225
Wire Wire Line
	9400 1225 10250 1225
Text Label 9400 1125 0    50   ~ 0
ESP_IO15
Wire Wire Line
	6350 4400 6900 4400
Wire Wire Line
	1250 7100 1250 6650
Wire Wire Line
	1000 5625 1250 5625
Text HLabel 6900 4850 2    50   Input ~ 0
BOT
Wire Wire Line
	6350 4700 6500 4700
Wire Wire Line
	6600 4850 6500 4850
Connection ~ 6500 4700
Wire Wire Line
	6500 4700 6900 4700
Wire Wire Line
	6500 4850 6500 4700
Wire Wire Line
	6900 4850 6800 4850
$Comp
L Device:R_Small R41
U 1 1 606A2C31
P 6700 4850
F 0 "R41" V 6650 5000 50  0000 C CNN
F 1 "1k" V 6650 4700 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 6700 4850 50  0001 C CNN
F 3 "~" H 6700 4850 50  0001 C CNN
F 4 "594-MCT06030C1001FP5" H 6700 4850 50  0001 C CNN "PartNo"
F 5 "Mouser" H 6700 4850 50  0001 C CNN "Supplier"
	1    6700 4850
	0    1    1    0   
$EndComp
$Comp
L Device:R_Small R38
U 1 1 5F47BE53
P 6675 1325
F 0 "R38" V 6600 1325 50  0000 C CNN
F 1 "1k" V 6750 1325 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 6675 1325 50  0001 C CNN
F 3 "~" H 6675 1325 50  0001 C CNN
F 4 "594-MCT06030C1001FP5" H 6675 1325 50  0001 C CNN "PartNo"
F 5 "Mouser" H 6675 1325 50  0001 C CNN "Supplier"
	1    6675 1325
	0    1    1    0   
$EndComp
$Comp
L Device:R_Small R37
U 1 1 5F47BC59
P 6675 1125
F 0 "R37" V 6625 975 50  0000 C CNN
F 1 "1k" V 6625 1250 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 6675 1125 50  0001 C CNN
F 3 "~" H 6675 1125 50  0001 C CNN
F 4 "594-MCT06030C1001FP5" H 6675 1125 50  0001 C CNN "PartNo"
F 5 "Mouser" H 6675 1125 50  0001 C CNN "Supplier"
	1    6675 1125
	0    1    1    0   
$EndComp
$Comp
L Device:R_Small R34
U 1 1 5F47B25C
P 6125 925
F 0 "R34" V 6050 925 50  0000 C CNN
F 1 "1k" V 6200 925 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 6125 925 50  0001 C CNN
F 3 "~" H 6125 925 50  0001 C CNN
F 4 "594-MCT06030C1001FP5" H 6125 925 50  0001 C CNN "PartNo"
F 5 "Mouser" H 6125 925 50  0001 C CNN "Supplier"
	1    6125 925 
	0    1    1    0   
$EndComp
$Comp
L Connector_Generic:Conn_01x04 J3
U 1 1 5EC87485
P 5075 1125
F 0 "J3" H 5025 1350 50  0000 L CNN
F 1 "Conn_01x04" H 5155 1026 50  0001 L CNN
F 2 "atmevse-footprints:UART2-Header" H 5075 1125 50  0001 C CNN
F 3 "~" H 5075 1125 50  0001 C CNN
F 4 "" H 5075 1125 50  0001 C CNN "Part"
F 5 "710-61304011121" H 5075 1125 50  0001 C CNN "PartNo"
F 6 "Mouser" H 5075 1125 50  0001 C CNN "Supplier"
	1    5075 1125
	1    0    0    1   
$EndComp
$Comp
L Connector_Generic:Conn_02x04_Odd_Even J5
U 1 1 5EC1F0B2
P 9100 1025
F 0 "J5" H 9150 725 50  0000 C CNN
F 1 "Conn_02x04" H 9150 1251 50  0001 C CNN
F 2 "atmevse-footprints:PROGRAM-Header" H 9100 1025 50  0001 C CNN
F 3 "~" H 9100 1025 50  0001 C CNN
F 4 "" H 9100 1025 50  0001 C CNN "Part"
F 5 "710-61200821621 " H 9100 1025 50  0001 C CNN "PartNo"
F 6 "Mouser" H 9100 1025 50  0001 C CNN "Supplier"
	1    9100 1025
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Small R7
U 1 1 5EA5635B
P 1625 2950
F 0 "R7" H 1684 2996 50  0000 L CNN
F 1 "10k" H 1684 2905 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 1625 2950 50  0001 C CNN
F 3 "~" H 1625 2950 50  0001 C CNN
F 4 "" H 1625 2950 50  0001 C CNN "Part"
F 5 "594-MCT06030C1002FP5 " H 1625 2950 50  0001 C CNN "PartNo"
F 6 "Mouser" H 1625 2950 50  0001 C CNN "Supplier"
	1    1625 2950
	-1   0    0    -1  
$EndComp
$Comp
L Transistor_FET:BSS138 Q1
U 1 1 5EA4C2D0
P 1325 2700
F 0 "Q1" H 1529 2746 50  0000 L CNN
F 1 "BSS138" H 1529 2655 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23_Handsoldering" H 1525 2625 50  0001 L CIN
F 3 "https://www.fairchildsemi.com/datasheets/BS/BSS138.pdf" H 1325 2700 50  0001 L CNN
F 4 "" H 1325 2700 50  0001 C CNN "Part"
F 5 "512-BSS138 " H 1325 2700 50  0001 C CNN "PartNo"
F 6 "Mouser" H 1325 2700 50  0001 C CNN "Supplier"
	1    1325 2700
	-1   0    0    -1  
$EndComp
$Comp
L Device:R_Small R12
U 1 1 5EA3C491
P 4725 5950
F 0 "R12" V 4529 5950 50  0000 C CNN
F 1 "5.6k" V 4620 5950 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 4725 5950 50  0001 C CNN
F 3 "~" H 4725 5950 50  0001 C CNN
F 4 "" H 4725 5950 50  0001 C CNN "Part"
F 5 "594-MCT06030C5601FP5 " H 4725 5950 50  0001 C CNN "PartNo"
F 6 "Mouser" H 4725 5950 50  0001 C CNN "Supplier"
	1    4725 5950
	0    -1   1    0   
$EndComp
$Comp
L Device:R_Small R11
U 1 1 5EA33196
P 4975 6700
F 0 "R11" H 5034 6746 50  0000 L CNN
F 1 "2k" H 5034 6655 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 4975 6700 50  0001 C CNN
F 3 "~" H 4975 6700 50  0001 C CNN
F 4 "" H 4975 6700 50  0001 C CNN "Part"
F 5 "594-MCT06030C2001FP5 " H 4975 6700 50  0001 C CNN "PartNo"
F 6 "Mouser" H 4975 6700 50  0001 C CNN "Supplier"
	1    4975 6700
	-1   0    0    -1  
$EndComp
$Comp
L Diode:BAT54S D3
U 1 1 5E9FC0D6
P 3450 6400
F 0 "D3" V 3496 6488 50  0000 L CNN
F 1 "BAT54S" V 3405 6488 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23_Handsoldering" H 3525 6525 50  0001 L CNN
F 3 "https://www.diodes.com/assets/Datasheets/ds11005.pdf" H 3330 6400 50  0001 C CNN
F 4 "" H 3450 6400 50  0001 C CNN "Part"
F 5 "757-TBAT54SLM " H 3450 6400 50  0001 C CNN "PartNo"
F 6 "Mouser" H 3450 6400 50  0001 C CNN "Supplier"
	1    3450 6400
	0    1    -1   0   
$EndComp
$Comp
L Diode:BAT54S D2
U 1 1 5E9F9C87
P 2950 6400
F 0 "D2" V 2996 6488 50  0000 L CNN
F 1 "BAT54S" V 2905 6488 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23_Handsoldering" H 3025 6525 50  0001 L CNN
F 3 "https://www.diodes.com/assets/Datasheets/ds11005.pdf" H 2830 6400 50  0001 C CNN
F 4 "" H 2950 6400 50  0001 C CNN "Part"
F 5 "757-TBAT54SLM " H 2950 6400 50  0001 C CNN "PartNo"
F 6 "Mouser" H 2950 6400 50  0001 C CNN "Supplier"
	1    2950 6400
	0    1    -1   0   
$EndComp
$Comp
L Device:C_Small C13
U 1 1 5E9E7944
P 1000 5725
F 0 "C13" H 1092 5771 50  0000 L CNN
F 1 "100n" V 1100 5525 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.18x1.45mm_HandSolder" H 1000 5725 50  0001 C CNN
F 3 "~" H 1000 5725 50  0001 C CNN
F 4 "" H 1000 5725 50  0001 C CNN "Part"
F 5 "963-EMF212B7104KGHT " H 1000 5725 50  0001 C CNN "PartNo"
F 6 "Mouser" H 1000 5725 50  0001 C CNN "Supplier"
	1    1000 5725
	1    0    0    -1  
$EndComp
$Comp
L smartevse:FAN3214TMX U8
U 1 1 5E9D4E3E
P 1950 6150
F 0 "U8" H 1950 6858 69  0000 C CNN
F 1 "FAN3214TMX" H 1950 6738 69  0000 C CNN
F 2 "Package_SO:SOIC-8_3.9x4.9mm_P1.27mm" H 1950 6150 50  0001 C CNN
F 3 "https://www.mouser.de/datasheet/2/308/FAN3214T_F085-D-1805909.pdf" H 1950 6150 50  0001 C CNN
F 4 "Mouser" H 1950 6150 50  0001 C CNN "Supplier"
F 5 "512-FAN3214TMX" H 1950 6150 50  0001 C CNN "PartNo"
	1    1950 6150
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C14
U 1 1 5E9CAC62
P 2725 1425
F 0 "C14" V 2954 1425 50  0000 C CNN
F 1 "100n" V 2863 1425 50  0000 C CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.18x1.45mm_HandSolder" H 2725 1425 50  0001 C CNN
F 3 "~" H 2725 1425 50  0001 C CNN
F 4 "" H 2725 1425 50  0001 C CNN "Part"
F 5 "963-EMF212B7104KGHT " H 2725 1425 50  0001 C CNN "PartNo"
F 6 "Mouser" H 2725 1425 50  0001 C CNN "Supplier"
	1    2725 1425
	0    -1   -1   0   
$EndComp
$Comp
L smartevse:SN65HVD72C U7
U 1 1 5E9C58C2
P 1825 1625
F 0 "U7" H 1825 2233 69  0000 C CNN
F 1 "SN65HVD72C" H 1825 2113 69  0000 C CNN
F 2 "Package_SO:SOIC-8_3.9x4.9mm_P1.27mm" H 1825 1625 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/sn65hvd72.pdf?HQS=TI-null-null-mousermode-df-pf-null-wwe&ts=1590747515592" H 1825 1625 50  0001 C CNN
F 4 "Mouser" H 1825 1625 50  0001 C CNN "Supplier"
F 5 "595-SN65HVD72D" H 1825 1625 50  0001 C CNN "PartNo"
	1    1825 1625
	-1   0    0    -1  
$EndComp
$Comp
L Device:R_Small R10
U 1 1 5E9C5025
P 6700 3050
F 0 "R10" V 6750 2900 50  0000 C CNN
F 1 "100" V 6750 3200 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 6700 3050 50  0001 C CNN
F 3 "~" H 6700 3050 50  0001 C CNN
F 4 "" H 6700 3050 50  0001 C CNN "Part"
F 5 "490-TB005-762-10BE " H 6700 3050 50  0001 C CNN "PartNo"
F 6 "Mouser" H 6700 3050 50  0001 C CNN "Supplier"
	1    6700 3050
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
F 5 "490-TB005-762-10BE " H 6700 2900 50  0001 C CNN "PartNo"
F 6 "Mouser" H 6700 2900 50  0001 C CNN "Supplier"
	1    6700 2900
	0    -1   -1   0   
$EndComp
$Comp
L Device:R_Small R8
U 1 1 5E9C3471
P 6700 2750
F 0 "R8" V 6750 2600 50  0000 C CNN
F 1 "100" V 6750 2900 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 6700 2750 50  0001 C CNN
F 3 "~" H 6700 2750 50  0001 C CNN
F 4 "" H 6700 2750 50  0001 C CNN "Part"
F 5 "490-TB005-762-10BE " H 6700 2750 50  0001 C CNN "PartNo"
F 6 "Mouser" H 6700 2750 50  0001 C CNN "Supplier"
	1    6700 2750
	0    -1   -1   0   
$EndComp
$Comp
L Device:C_Small C15
U 1 1 5E9BECE4
P 5750 5000
F 0 "C15" V 5521 5000 50  0000 C CNN
F 1 "100n" V 5612 5000 50  0000 C CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.18x1.45mm_HandSolder" H 5750 5000 50  0001 C CNN
F 3 "~" H 5750 5000 50  0001 C CNN
F 4 "" H 5750 5000 50  0001 C CNN "Part"
F 5 "963-EMF212B7104KGHT " H 5750 5000 50  0001 C CNN "PartNo"
F 6 "Mouser" H 5750 5000 50  0001 C CNN "Supplier"
	1    5750 5000
	0    1    1    0   
$EndComp
$Comp
L Device:C_Small C17
U 1 1 5E9B9A4C
P 7300 4000
F 0 "C17" H 7392 4046 50  0000 L CNN
F 1 "100n" H 7392 3955 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.18x1.45mm_HandSolder" H 7300 4000 50  0001 C CNN
F 3 "~" H 7300 4000 50  0001 C CNN
F 4 "" H 7300 4000 50  0001 C CNN "Part"
F 5 "963-EMF212B7104KGHT " H 7300 4000 50  0001 C CNN "PartNo"
F 6 "Mouser" H 7300 4000 50  0001 C CNN "Supplier"
	1    7300 4000
	1    0    0    -1  
$EndComp
$Comp
L smartevse:ATmega4808-XFR U10
U 1 1 5E9B0383
P 5750 3800
F 0 "U10" H 5750 5075 50  0000 C CNN
F 1 "ATmega4808-X" H 5750 4984 50  0000 C CNN
F 2 "Package_SO:SSOP-28_5.3x10.2mm_P0.65mm" H 5750 3800 50  0001 C CIN
F 3 "http://ww1.microchip.com/downloads/en/DeviceDoc/40002018A.pdf" H 5750 3800 50  0001 C CNN
F 4 "Mouser" H 5750 3800 50  0001 C CNN "Supplier"
F 5 "556-ATMEGA4808-XF" H 5750 3800 50  0001 C CNN "PartNo"
	1    5750 3800
	1    0    0    -1  
$EndComp
Text HLabel 6900 2750 2    50   Output ~ 0
SSR_L1
Wire Wire Line
	1000 5300 1000 5350
Wire Wire Line
	1000 5350 1250 5350
Connection ~ 1250 5350
Wire Notes Line
	4225 525  4225 1550
$Comp
L Connector_Generic:Conn_01x04 J4
U 1 1 5F466074
P 7100 1700
F 0 "J4" H 7100 1925 50  0000 C CNN
F 1 "Conn_01x04" H 7400 1600 50  0001 C CNN
F 2 "atmevse-footprints:UART0-Header" H 7100 1700 50  0001 C CNN
F 3 "~" H 7100 1700 50  0001 C CNN
F 4 "na" H 7100 1700 50  0001 C CNN "PartNo"
F 5 "Mouser" H 7100 1700 50  0001 C CNN "Supplier"
	1    7100 1700
	1    0    0    1   
$EndComp
$Comp
L power:GND #PWR032
U 1 1 5EC306B7
P 6300 1800
F 0 "#PWR032" H 6300 1550 50  0001 C CNN
F 1 "GND" H 6375 1650 50  0000 R CNN
F 2 "" H 6300 1800 50  0001 C CNN
F 3 "" H 6300 1800 50  0001 C CNN
	1    6300 1800
	1    0    0    -1  
$EndComp
Wire Wire Line
	6900 1800 6300 1800
Wire Wire Line
	6900 1500 6500 1500
Wire Wire Line
	6900 1700 6300 1700
Wire Wire Line
	6400 1600 6900 1600
Wire Notes Line
	4225 525  5425 525 
Wire Notes Line
	5425 525  5425 1550
Wire Notes Line
	5425 1550 4225 1550
Wire Notes Line
	5575 525  5575 2050
Wire Notes Line
	5575 2050 7475 2050
Wire Notes Line
	7475 2050 7475 525 
Wire Notes Line
	5575 525  7475 525 
Wire Notes Line
	550  550  3125 550 
Wire Notes Line
	3125 550  3125 2300
Wire Notes Line
	3125 2300 550  2300
Wire Notes Line
	550  2300 550  550 
Text Notes 4425 5050 2    50   ~ 0
Lock Driver\n
Wire Wire Line
	8100 1400 9850 1400
Wire Wire Line
	9850 1400 9850 1125
Wire Wire Line
	9850 1125 9400 1125
Text Label 8575 1225 0    50   ~ 0
ESP_IO0
Text Label 8575 1025 0    50   ~ 0
ESP_TX0
Text Label 8575 1125 0    50   ~ 0
ESP_RX0
Wire Notes Line
	10325 1600 10325 525 
Wire Notes Line
	10325 525  7600 525 
Wire Notes Line
	7600 525  7600 1600
Wire Notes Line
	7600 1600 10325 1600
Text Notes 7625 625  0    50   ~ 0
ATmega/ESP Programming Header (Custom Pinout)\n
NoConn ~ 8950 4900
NoConn ~ 8950 4800
NoConn ~ 8950 5100
NoConn ~ 8950 5000
NoConn ~ 8950 4700
NoConn ~ 8950 4600
$Comp
L power:GND #PWR031
U 1 1 5ECB99BA
P 9550 5400
F 0 "#PWR031" H 9550 5150 50  0001 C CNN
F 1 "GND" H 9555 5227 50  0000 C CNN
F 2 "" H 9550 5400 50  0001 C CNN
F 3 "" H 9550 5400 50  0001 C CNN
	1    9550 5400
	1    0    0    -1  
$EndComp
Wire Wire Line
	8100 3700 8950 3700
Wire Wire Line
	8100 3700 8100 1400
$Comp
L RF_Module:ESP32-WROOM-32D U9
U 1 1 5EC08E25
P 9550 4000
F 0 "U9" H 9550 5700 50  0000 C CNN
F 1 "ESP32-WROOM-32" H 9500 5600 50  0000 C CNN
F 2 "RF_Module:ESP32-WROOM-32" H 9550 2500 50  0001 C CNN
F 3 "https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32_datasheet_en.pdf" H 9250 4050 50  0001 C CNN
F 4 "Mouser" H 9550 4000 50  0001 C CNN "Supplier"
F 5 "356-ESP32WRM32E128PH " H 9550 4000 50  0001 C CNN "PartNo"
	1    9550 4000
	-1   0    0    -1  
$EndComp
$Comp
L Device:R R25
U 1 1 5EBDA7D4
P 10750 2550
F 0 "R25" H 10820 2596 50  0000 L CNN
F 1 "12k" H 10820 2505 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" V 10680 2550 50  0001 C CNN
F 3 "~" H 10750 2550 50  0001 C CNN
F 4 "" H 10750 2550 50  0001 C CNN "Part"
F 5 "594-MCT06030C1202FP5 " H 10750 2550 50  0001 C CNN "PartNo"
F 6 "Mouser" H 10750 2550 50  0001 C CNN "Supplier"
	1    10750 2550
	1    0    0    -1  
$EndComp
$Comp
L Device:C C21
U 1 1 5EBDAC66
P 10750 3050
F 0 "C21" H 10865 3096 50  0000 L CNN
F 1 "1n50V" H 10865 3005 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 10788 2900 50  0001 C CNN
F 3 "~" H 10750 3050 50  0001 C CNN
F 4 "" H 10750 3050 50  0001 C CNN "Part"
F 5 "77-VJ0805A102KXAAC" H 10750 3050 50  0001 C CNN "PartNo"
F 6 "Mouser" H 10750 3050 50  0001 C CNN "Supplier"
	1    10750 3050
	1    0    0    -1  
$EndComp
$Comp
L Device:CP_Small C16
U 1 1 5EC15E77
P 10600 4200
F 0 "C16" V 10500 4300 50  0000 L CNN
F 1 "100u6.3V" V 10700 3950 50  0000 L CNN
F 2 "Capacitor_Tantalum_SMD:CP_EIA-2012-15_AVX-P_Pad1.30x1.05mm_HandSolder" H 10600 4200 50  0001 C CNN
F 3 "~" H 10600 4200 50  0001 C CNN
F 4 "" H 10600 4200 50  0001 C CNN "Part"
F 5 "581-TLCR107M006RTA " H 10600 4200 50  0001 C CNN "PartNo"
F 6 "Mouser" H 10600 4200 50  0001 C CNN "Supplier"
	1    10600 4200
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C22
U 1 1 5EC16D76
P 10900 4200
F 0 "C22" V 10800 4300 50  0000 L CNN
F 1 "1u10V" V 11000 4050 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 10900 4200 50  0001 C CNN
F 3 "~" H 10900 4200 50  0001 C CNN
F 4 "" H 10900 4200 50  0001 C CNN "Part"
F 5 "77-VJ805Y105KXARW1BC " H 10900 4200 50  0001 C CNN "PartNo"
F 6 "Mouser" H 10900 4200 50  0001 C CNN "Supplier"
	1    10900 4200
	1    0    0    -1  
$EndComp
Wire Wire Line
	8400 1225 8400 2800
Wire Wire Line
	8200 1125 8200 3100
Wire Wire Line
	8300 1025 8300 2900
Text Label 8525 3700 0    50   ~ 0
ESP_IO15
NoConn ~ 8950 3000
NoConn ~ 10150 3100
NoConn ~ 10150 3000
NoConn ~ 10150 4000
NoConn ~ 10150 4100
NoConn ~ 10150 4200
NoConn ~ 10150 4300
NoConn ~ 10150 4400
NoConn ~ 10150 4500
NoConn ~ 8950 4500
NoConn ~ 8950 4400
NoConn ~ 8950 4300
NoConn ~ 8950 4200
NoConn ~ 8950 4100
NoConn ~ 8950 4000
NoConn ~ 8950 3600
NoConn ~ 8950 3500
NoConn ~ 8950 3300
NoConn ~ 8950 3200
Text Label 10750 3850 2    50   ~ 0
ESP_VDD
$Comp
L power:+3.3V #PWR037
U 1 1 5EC38273
P 10750 3800
F 0 "#PWR037" H 10750 3650 50  0001 C CNN
F 1 "+3.3V" H 10765 3973 50  0000 C CNN
F 2 "" H 10750 3800 50  0001 C CNN
F 3 "" H 10750 3800 50  0001 C CNN
	1    10750 3800
	1    0    0    -1  
$EndComp
Wire Wire Line
	10750 3900 10900 3900
Connection ~ 10750 3900
Wire Wire Line
	10750 3900 10750 3800
Wire Wire Line
	10900 3900 10900 4100
Wire Wire Line
	10600 3900 10750 3900
Wire Wire Line
	10600 4100 10600 3900
Wire Wire Line
	10750 4500 10900 4500
Connection ~ 10750 4500
Wire Wire Line
	10750 4500 10750 4600
Wire Wire Line
	10900 4500 10900 4300
Wire Wire Line
	10600 4500 10750 4500
Wire Wire Line
	10600 4300 10600 4500
Connection ~ 10250 2800
Wire Wire Line
	10750 2800 10750 2900
Connection ~ 10750 2800
Wire Wire Line
	10750 2800 10250 2800
Wire Wire Line
	10750 2700 10750 2800
$Comp
L power:+3.3V #PWR035
U 1 1 5EBDC16A
P 10750 2400
F 0 "#PWR035" H 10750 2250 50  0001 C CNN
F 1 "+3.3V" H 10765 2573 50  0000 C CNN
F 2 "" H 10750 2400 50  0001 C CNN
F 3 "" H 10750 2400 50  0001 C CNN
	1    10750 2400
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR036
U 1 1 5EBDB705
P 10750 3200
F 0 "#PWR036" H 10750 2950 50  0001 C CNN
F 1 "GND" H 10755 3027 50  0000 C CNN
F 2 "" H 10750 3200 50  0001 C CNN
F 3 "" H 10750 3200 50  0001 C CNN
	1    10750 3200
	1    0    0    -1  
$EndComp
Wire Wire Line
	9800 2550 9550 2550
Wire Wire Line
	9550 2600 9550 2550
Text Label 9800 2550 0    50   ~ 0
ESP_VDD
Text Label 8525 2800 0    50   ~ 0
ESP_IO0
Text Label 8525 3900 0    50   ~ 0
ESP_TX2
Text Label 8525 3800 0    50   ~ 0
ESP_RX2
Wire Wire Line
	8950 3900 8425 3900
Wire Wire Line
	8950 3800 8425 3800
Text Label 8525 3400 0    50   ~ 0
ESP_IO12
Wire Wire Line
	8950 3400 8425 3400
Wire Wire Line
	8200 3100 8950 3100
Wire Wire Line
	8300 2900 8950 2900
Text Label 8525 3100 0    50   ~ 0
ESP_RX0
Text Label 8525 2900 0    50   ~ 0
ESP_TX0
Wire Wire Line
	8950 2800 8400 2800
Wire Wire Line
	10250 2800 10250 1225
Wire Wire Line
	10150 2800 10250 2800
Wire Wire Line
	1025 1425 1025 900 
Wire Wire Line
	4050 3050 5150 3050
Wire Wire Line
	1025 1425 1125 1425
Wire Wire Line
	4500 4750 4500 2750
Wire Wire Line
	4350 3200 4350 4825
Wire Wire Line
	4350 3200 5150 3200
Wire Wire Line
	700  4825 700  6150
Wire Wire Line
	700  4825 4350 4825
Wire Wire Line
	800  4750 800  6050
Wire Wire Line
	800  4750 4500 4750
Wire Notes Line
	575  7375 575  4950
$Comp
L power:+3.3V #PWR025
U 1 1 5EA2A4B6
P 5175 6200
F 0 "#PWR025" H 5175 6050 50  0001 C CNN
F 1 "+3.3V" H 5190 6373 50  0000 C CNN
F 2 "" H 5175 6200 50  0001 C CNN
F 3 "" H 5175 6200 50  0001 C CNN
	1    5175 6200
	-1   0    0    -1  
$EndComp
$Comp
L Diode:BAT54S D4
U 1 1 5EA24B11
P 5175 6500
F 0 "D4" V 5200 6550 50  0000 L CNN
F 1 "BAT54S" V 5125 6550 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23_Handsoldering" H 5250 6625 50  0001 L CNN
F 3 "https://www.diodes.com/assets/Datasheets/ds11005.pdf" H 5055 6500 50  0001 C CNN
F 4 "" H 5175 6500 50  0001 C CNN "Part"
F 5 "757-TBAT54SLM " H 5175 6500 50  0001 C CNN "PartNo"
F 6 "Mouser" H 5175 6500 50  0001 C CNN "Supplier"
	1    5175 6500
	0    1    -1   0   
$EndComp
Wire Wire Line
	5175 6800 5175 7100
Wire Wire Line
	5175 7100 4975 7100
Connection ~ 3450 7100
Wire Notes Line
	4475 4950 4475 5650
Wire Notes Line
	4475 5650 5525 5650
Wire Notes Line
	5525 5650 5525 7375
Wire Notes Line
	575  4950 4475 4950
Wire Notes Line
	575  7375 5525 7375
Wire Wire Line
	4975 6800 4975 7100
Connection ~ 4975 7100
Wire Wire Line
	4975 7100 3450 7100
Wire Wire Line
	5700 5950 5700 5525
Wire Wire Line
	5700 5525 4650 5525
Wire Wire Line
	4650 5525 4650 3950
Wire Wire Line
	4975 5950 5700 5950
Wire Wire Line
	4650 3950 5150 3950
$Comp
L Diode:BAT54S D?
U 1 1 611CEBA9
P 2025 4100
AR Path="/5E903BF9/611CEBA9" Ref="D?"  Part="1" 
AR Path="/5E9B0066/611CEBA9" Ref="D7"  Part="1" 
F 0 "D7" V 2075 4175 50  0000 L CNN
F 1 "BAT54S" V 1975 4175 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23_Handsoldering" H 2100 4225 50  0001 L CNN
F 3 "https://www.diodes.com/assets/Datasheets/ds11005.pdf" H 1905 4100 50  0001 C CNN
F 4 "" H 2025 4100 50  0001 C CNN "Part"
F 5 "757-TBAT54SLM " H 2025 4100 50  0001 C CNN "PartNo"
F 6 "Mouser" H 2025 4100 50  0001 C CNN "Supplier"
	1    2025 4100
	0    -1   -1   0   
$EndComp
$Comp
L Device:R_Small R?
U 1 1 611CEBB2
P 1375 3500
AR Path="/5E903BF9/611CEBB2" Ref="R?"  Part="1" 
AR Path="/5E9B0066/611CEBB2" Ref="R24"  Part="1" 
F 0 "R24" V 1450 3500 50  0000 C CNN
F 1 "5.6k" V 1300 3500 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 1375 3500 50  0001 C CNN
F 3 "~" H 1375 3500 50  0001 C CNN
F 4 "" H 1375 3500 50  0001 C CNN "Part"
F 5 "594-MCT06030C5601FP5 " H 1375 3500 50  0001 C CNN "PartNo"
F 6 "Mouser" H 1375 3500 50  0001 C CNN "Supplier"
	1    1375 3500
	0    -1   1    0   
$EndComp
Wire Wire Line
	2225 4100 2225 3500
Wire Wire Line
	2225 3500 1625 3500
Wire Wire Line
	2025 4400 2025 4450
Wire Wire Line
	1475 3500 1625 3500
Connection ~ 1625 3500
Wire Wire Line
	1625 3950 1625 4450
Wire Wire Line
	1625 4450 2025 4450
Wire Wire Line
	1625 3500 1625 3750
$Comp
L Device:R_Small R?
U 1 1 611CEBC8
P 1625 3850
AR Path="/5E903BF9/611CEBC8" Ref="R?"  Part="1" 
AR Path="/5E9B0066/611CEBC8" Ref="R39"  Part="1" 
F 0 "R39" H 1500 3800 50  0000 C CNN
F 1 "2k" H 1525 3875 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 1625 3850 50  0001 C CNN
F 3 "~" H 1625 3850 50  0001 C CNN
F 4 "Mouser" H 1625 3850 50  0001 C CNN "Supplier"
F 5 "594-MCT06030C2001FP5 " H 1625 3850 50  0001 C CNN "PartNo"
	1    1625 3850
	1    0    0    1   
$EndComp
Wire Wire Line
	3900 1725 2675 1725
Wire Wire Line
	3900 1725 3900 3350
Connection ~ 2675 1725
Wire Wire Line
	4050 1625 3250 1625
Wire Wire Line
	3250 1625 3250 900 
Wire Wire Line
	3250 900  1025 900 
Wire Wire Line
	4050 1625 4050 3050
Wire Wire Line
	5150 2900 3750 2900
Wire Wire Line
	3750 2900 3750 1825
Wire Wire Line
	3750 1825 2525 1825
Wire Wire Line
	875  2425 1225 2425
Wire Wire Line
	1225 2425 1225 2500
$Comp
L power:+3.3V #PWR0114
U 1 1 612DAB53
P 2025 3800
F 0 "#PWR0114" H 2025 3650 50  0001 C CNN
F 1 "+3.3V" H 2040 3973 50  0000 C CNN
F 2 "" H 2025 3800 50  0001 C CNN
F 3 "" H 2025 3800 50  0001 C CNN
	1    2025 3800
	-1   0    0    -1  
$EndComp
$Comp
L power:GND #PWR0115
U 1 1 613069D3
P 2025 4450
F 0 "#PWR0115" H 2025 4200 50  0001 C CNN
F 1 "GND" H 2030 4277 50  0000 C CNN
F 2 "" H 2025 4450 50  0001 C CNN
F 3 "" H 2025 4450 50  0001 C CNN
	1    2025 4450
	-1   0    0    -1  
$EndComp
Connection ~ 2025 4450
Wire Wire Line
	3475 2700 3475 4100
Wire Wire Line
	3475 4100 5150 4100
Wire Wire Line
	1525 2700 1625 2700
Connection ~ 1625 2700
Wire Wire Line
	1625 2700 3475 2700
Wire Wire Line
	2225 3500 3325 3500
Wire Wire Line
	3325 3500 3325 4250
Wire Wire Line
	3325 4250 5150 4250
Connection ~ 2225 3500
Text HLabel 975  3500 0    50   Input ~ 0
Button
Wire Wire Line
	975  3500 1275 3500
Wire Notes Line
	550  2350 550  4675
Wire Notes Line
	550  4675 2750 4675
Wire Notes Line
	2750 4675 2750 2350
Wire Notes Line
	2750 2350 550  2350
Text Notes 2725 2450 2    50   ~ 0
Operator Controls\n
$Comp
L agccs-ctrl22:12V_buffered #PWR0120
U 1 1 60462D21
P 1000 5300
F 0 "#PWR0120" H 1000 5150 50  0001 C CNN
F 1 "12V_buffered" H 1075 5450 50  0000 C CNN
F 2 "" H 1000 5300 50  0001 C CNN
F 3 "" H 1000 5300 50  0001 C CNN
	1    1000 5300
	1    0    0    -1  
$EndComp
Wire Wire Line
	5625 925  6025 925 
Wire Wire Line
	5625 1125 6400 1125
Wire Wire Line
	5625 1325 6500 1325
Wire Wire Line
	6500 1500 6500 1325
Connection ~ 6500 1325
Wire Wire Line
	6500 1325 6575 1325
Wire Wire Line
	6300 1700 6300 925 
Wire Wire Line
	6225 925  6300 925 
Connection ~ 6300 925 
Wire Wire Line
	6300 925  7375 925 
Wire Wire Line
	6400 1600 6400 1125
Connection ~ 6400 1125
Wire Wire Line
	6400 1125 6575 1125
$EndSCHEMATC
