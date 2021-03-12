EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "J5 Adaptor"
Date "2021-03-12"
Rev "1.2"
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Connector_Generic:Conn_02x04_Odd_Even J5
U 1 1 604B7404
P 6550 1650
F 0 "J5" H 6600 1875 50  0000 C CNN
F 1 "Conn_02x04_Odd_Even" H 6600 1876 50  0001 C CNN
F 2 "" H 6550 1650 50  0001 C CNN
F 3 "~" H 6550 1650 50  0001 C CNN
	1    6550 1650
	1    0    0    -1  
$EndComp
$Comp
L agccs-j5adaptor-symbols:USB-to-Serial J?
U 1 1 604D9A61
P 2150 1750
F 0 "J?" H 2475 1450 50  0001 L CNN
F 1 "USB-to-Serial" H 2250 2075 50  0000 L CNN
F 2 "" H 2150 1750 50  0001 C CNN
F 3 "~" H 2150 1750 50  0001 C CNN
	1    2150 1750
	-1   0    0    1   
$EndComp
Text Label 6875 1550 0    50   ~ 0
GND
$Comp
L Switch:SW_Push SW?
U 1 1 604DBBC7
P 9150 3625
F 0 "SW?" V 9150 3975 50  0001 R CNN
F 1 "ESP_BOOT" V 9225 4100 50  0000 R CNN
F 2 "" H 9150 3825 50  0001 C CNN
F 3 "~" H 9150 3825 50  0001 C CNN
	1    9150 3625
	0    1    -1   0   
$EndComp
$Comp
L Device:R R?
U 1 1 604DD454
P 9150 3175
F 0 "R?" H 9080 3221 50  0001 R CNN
F 1 "120" H 9080 3175 50  0000 R CNN
F 2 "" V 9080 3175 50  0001 C CNN
F 3 "~" H 9150 3175 50  0001 C CNN
	1    9150 3175
	-1   0    0    -1  
$EndComp
Text Label 6875 1650 0    50   ~ 0
3.3V
$Comp
L Device:R R?
U 1 1 604E0D90
P 9150 1925
F 0 "R?" H 9080 1971 50  0001 R CNN
F 1 "150" H 9075 1950 50  0000 R CNN
F 2 "" V 9080 1925 50  0001 C CNN
F 3 "~" H 9150 1925 50  0001 C CNN
	1    9150 1925
	-1   0    0    -1  
$EndComp
$Comp
L Device:LED D?
U 1 1 604E17C1
P 9150 2375
F 0 "D?" V 9189 2257 50  0001 R CNN
F 1 "LED" V 9143 2455 50  0000 L CNN
F 2 "" H 9150 2375 50  0001 C CNN
F 3 "~" H 9150 2375 50  0001 C CNN
	1    9150 2375
	0    1    -1   0   
$EndComp
$Comp
L Switch:SW_Push SW?
U 1 1 604E3705
P 8375 3625
F 0 "SW?" V 8375 3975 50  0001 R CNN
F 1 "ESP_EN" V 8450 4000 50  0000 R CNN
F 2 "" H 8375 3825 50  0001 C CNN
F 3 "~" H 8375 3825 50  0001 C CNN
	1    8375 3625
	0    1    -1   0   
$EndComp
$Comp
L Device:C C?
U 1 1 604E370B
P 8100 3650
F 0 "C?" H 8215 3696 50  0001 L CNN
F 1 "10n" H 8275 3550 50  0000 R CNN
F 2 "" H 8138 3500 50  0001 C CNN
F 3 "~" H 8100 3650 50  0001 C CNN
	1    8100 3650
	-1   0    0    -1  
$EndComp
$Comp
L Device:R R?
U 1 1 604E3711
P 8375 3175
F 0 "R?" H 8445 3221 50  0001 L CNN
F 1 "120" H 8305 3175 50  0000 R CNN
F 2 "" V 8305 3175 50  0001 C CNN
F 3 "~" H 8375 3175 50  0001 C CNN
	1    8375 3175
	-1   0    0    -1  
$EndComp
Wire Wire Line
	9150 2075 9150 2225
Wire Wire Line
	9150 3825 9150 4150
Wire Wire Line
	8375 3825 8375 4150
Connection ~ 8375 4150
Wire Wire Line
	8375 4150 8100 4150
Wire Wire Line
	8100 3800 8100 4150
Connection ~ 8100 4150
Wire Wire Line
	8375 3325 8375 3400
Wire Wire Line
	8100 3500 8100 3400
Wire Wire Line
	8100 3400 8375 3400
Connection ~ 8375 3400
Wire Wire Line
	8375 3400 8375 3425
$Comp
L Switch:SW_Push_SPDT SW?
U 1 1 604E9BAD
P 4400 1750
F 0 "SW?" H 4350 1450 50  0001 C CNN
F 1 "AVR/ESP programming" H 4425 2000 50  0000 C CNN
F 2 "" H 4400 1750 50  0001 C CNN
F 3 "~" H 4400 1750 50  0001 C CNN
	1    4400 1750
	1    0    0    1   
$EndComp
Text Label 6350 1550 2    50   ~ 0
UPDI
Text Label 6350 1650 2    50   ~ 0
ESP_TX0
Text Label 6350 1750 2    50   ~ 0
ESP_RX0
Wire Wire Line
	5075 1550 5075 1650
Wire Wire Line
	5075 1650 4600 1650
Wire Wire Line
	5075 1550 6350 1550
Wire Wire Line
	5200 1650 5200 1850
Wire Wire Line
	5200 1850 4600 1850
Wire Wire Line
	5200 1650 6350 1650
Wire Wire Line
	4175 1750 3700 1750
Wire Wire Line
	5350 1750 5350 1450
Wire Wire Line
	5350 1450 2700 1450
Wire Wire Line
	2700 1450 2700 1650
Wire Wire Line
	2700 1650 2350 1650
Wire Wire Line
	5350 1750 6350 1750
$Comp
L Device:R R?
U 1 1 6050594A
P 3250 1650
F 0 "R?" V 3043 1650 50  0001 C CNN
F 1 "4k7" V 3135 1650 50  0000 C CNN
F 2 "" V 3180 1650 50  0001 C CNN
F 3 "~" H 3250 1650 50  0001 C CNN
	1    3250 1650
	0    1    1    0   
$EndComp
Wire Wire Line
	3100 1650 2700 1650
Connection ~ 2700 1650
Wire Wire Line
	3400 1650 3700 1650
Wire Wire Line
	3700 1650 3700 1750
Connection ~ 3700 1750
Wire Wire Line
	3700 1750 2350 1750
Text Label 6875 1850 0    50   ~ 0
ESP_EN
Text Label 6350 1850 2    50   ~ 0
ESP_IO0
Wire Wire Line
	9850 1550 9850 4150
Wire Wire Line
	9150 2525 9150 2750
$Comp
L Device:Jumper_NO_Small JP?
U 1 1 60575057
P 7400 3625
F 0 "JP?" V 7446 3577 50  0001 R CNN
F 1 "DebugAVR" V 7400 3577 50  0000 R CNN
F 2 "" H 7400 3625 50  0001 C CNN
F 3 "~" H 7400 3625 50  0001 C CNN
	1    7400 3625
	0    -1   -1   0   
$EndComp
Wire Wire Line
	6850 1750 7400 1750
Wire Wire Line
	7400 1750 7400 3525
Wire Wire Line
	7400 3725 7400 4150
Connection ~ 7400 4150
Wire Wire Line
	7400 4150 8100 4150
Wire Wire Line
	9150 1650 9150 1775
Wire Wire Line
	6850 1650 9150 1650
Wire Wire Line
	6850 1550 9850 1550
Wire Wire Line
	8375 1850 8375 3025
Wire Wire Line
	6350 1850 6225 1850
Wire Wire Line
	6225 1850 6225 2750
Connection ~ 9150 2750
Wire Wire Line
	9150 2750 9150 3025
Wire Wire Line
	6850 1850 8375 1850
Wire Wire Line
	2475 4150 7400 4150
Wire Wire Line
	2475 1850 2350 1850
Wire Wire Line
	2475 1850 2475 4150
Wire Wire Line
	9150 3325 9150 3425
Wire Wire Line
	8375 4150 9150 4150
Wire Wire Line
	6225 2750 9150 2750
Connection ~ 9150 4150
Wire Wire Line
	9150 4150 9850 4150
Text Notes 3450 2650 0    59   ~ 0
*** keep unattained to pass on serial line to ESP \n(programming and monitoring)\n*** hold down for UPDI programming the AVR with \nPyupdi (the ESP will then lalso listen to the UPDI \nprogramming protocol, but this does not matter)\n*** on the AGCCS board, this is the RED switch
Text Notes 4675 3775 0    59   ~ 0
*** when pulled to ground, our firmeware “demesh.c”\nwill go in target-debug mode (i.e. accespoint with telnet\nforwarding of the AVRs Tx0 und Rx0)
Text Notes 6350 4425 0    59   ~ 0
*** ESP serial line programming: hold down ESP_BOOT while cycling ESP_EN;\non the AGCCS board, ESP_BOOT is BLACK and ESP_EN is GREY
$EndSCHEMATC
