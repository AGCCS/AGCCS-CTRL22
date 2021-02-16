EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 4
Title "AGCCS-Ctrl22"
Date "2021-02-15"
Rev "1.2"
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Sheet
S 2450 3100 1800 1350
U 5E98FA8A
F0 "Mains and Power Circuitry" 50
F1 "mains_and_power.sch" 50
F2 "SSR_L1" I R 4250 3600 50 
F3 "SSR_L2" I R 4250 3750 50 
F4 "SSR_L3" I R 4250 3900 50 
$EndSheet
Text Notes 2950 3950 0    50   ~ 10
Mains Input Connector \nContactor Relays\n12V Power Supply\n3.3V DC Converter\n-12V Voltage Inverter
$Sheet
S 5000 3100 1750 2200
U 5E9B0066
F0 "ATmega4808 and Peripherals" 50
F1 "atmega4808_and_peripherals.sch" 50
F2 "SSR_L3" O L 5000 3900 50 
F3 "SSR_L2" O L 5000 3750 50 
F4 "SSR_L1" O L 5000 3600 50 
F5 "RS485_A" B R 6750 3200 50 
F6 "RS485_B" B R 6750 3350 50 
F7 "Lock_W" O R 6750 3800 50 
F8 "Lock_R" O R 6750 3650 50 
F9 "Lock_B" I R 6750 3500 50 
F10 "LED" O R 6750 3950 50 
F11 "CT0" I R 6750 4850 50 
F12 "CT1" I R 6750 5000 50 
F13 "CT2" I R 6750 5150 50 
F14 "Button" I R 6750 4100 50 
F15 "PP" I R 6750 4250 50 
F16 "CP" I R 6750 4400 50 
F17 "PWM_Out" O R 6750 4550 50 
F18 "Signal_Relay" O R 6750 4700 50 
$EndSheet
Text Notes 5600 4250 0    50   Italic 10
ATmega4808\nESP32\nRS485 Transceiver\nLock Actuator
$Sheet
S 7350 3100 1750 2200
U 5E903BF9
F0 "Analog Section and Low Voltage Outputs" 50
F1 "analog_section_low_voltage.sch" 50
F2 "PWM_Out" I L 7350 4550 50 
F3 "CP" O L 7350 4400 50 
F4 "LED" I L 7350 3950 50 
F5 "RS485_A" B L 7350 3200 50 
F6 "RS485_B" B L 7350 3350 50 
F7 "Lock_B" O L 7350 3500 50 
F8 "Lock_R" I L 7350 3650 50 
F9 "Lock_W" I L 7350 3800 50 
F10 "PP" O L 7350 4250 50 
F11 "Button" O L 7350 4100 50 
F12 "CT0" O L 7350 4850 50 
F13 "CT1" O L 7350 5000 50 
F14 "CT2" O L 7350 5150 50 
F15 "Signal_Relay" I L 7350 4700 50 
$EndSheet
Wire Wire Line
	6750 3200 7350 3200
Wire Wire Line
	7350 3350 6750 3350
Wire Wire Line
	6750 3500 7350 3500
Wire Wire Line
	7350 3650 6750 3650
Wire Wire Line
	6750 3800 7350 3800
Wire Wire Line
	6750 3950 7350 3950
Wire Wire Line
	6750 4250 7350 4250
Wire Wire Line
	6750 4400 7350 4400
Wire Wire Line
	6750 4550 7350 4550
Wire Wire Line
	7350 4700 6750 4700
Wire Wire Line
	6750 4100 7350 4100
Text Notes 8850 4300 2    50   ~ 10
Analog Section\n-Opamp Buffer\n- Comparator\nLow Voltage I/O Connector\nSignal Relays
Wire Wire Line
	5000 3600 4250 3600
Wire Wire Line
	5000 3750 4250 3750
Wire Wire Line
	5000 3900 4250 3900
Wire Wire Line
	6750 4850 7350 4850
Wire Wire Line
	6750 5000 7350 5000
Wire Wire Line
	6750 5150 7350 5150
$Comp
L power:+3.3V #PWR0110
U 1 1 5ED97DE8
P 1500 6000
F 0 "#PWR0110" H 1500 5850 50  0001 C CNN
F 1 "+3.3V" H 1515 6173 50  0000 C CNN
F 2 "" H 1500 6000 50  0001 C CNN
F 3 "" H 1500 6000 50  0001 C CNN
	1    1500 6000
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0111
U 1 1 5ED983F4
P 1050 6150
F 0 "#PWR0111" H 1050 5900 50  0001 C CNN
F 1 "GND" H 1055 5977 50  0000 C CNN
F 2 "" H 1050 6150 50  0001 C CNN
F 3 "" H 1050 6150 50  0001 C CNN
	1    1050 6150
	1    0    0    -1  
$EndComp
$Comp
L power:+12V #PWR0112
U 1 1 5ED9882E
P 1900 6000
F 0 "#PWR0112" H 1900 5850 50  0001 C CNN
F 1 "+12V" H 1915 6173 50  0000 C CNN
F 2 "" H 1900 6000 50  0001 C CNN
F 3 "" H 1900 6000 50  0001 C CNN
	1    1900 6000
	1    0    0    -1  
$EndComp
$Comp
L power:-12V #PWR0113
U 1 1 5ED9957C
P 2300 6000
F 0 "#PWR0113" H 2300 6100 50  0001 C CNN
F 1 "-12V" H 2315 6173 50  0000 C CNN
F 2 "" H 2300 6000 50  0001 C CNN
F 3 "" H 2300 6000 50  0001 C CNN
	1    2300 6000
	1    0    0    -1  
$EndComp
$Comp
L power:PWR_FLAG #FLG0101
U 1 1 5ED996AE
P 1500 6150
F 0 "#FLG0101" H 1500 6225 50  0001 C CNN
F 1 "PWR_FLAG" H 1500 6323 50  0000 C CNN
F 2 "" H 1500 6150 50  0001 C CNN
F 3 "~" H 1500 6150 50  0001 C CNN
	1    1500 6150
	-1   0    0    1   
$EndComp
$Comp
L power:PWR_FLAG #FLG0102
U 1 1 5ED99E56
P 2300 6150
F 0 "#FLG0102" H 2300 6225 50  0001 C CNN
F 1 "PWR_FLAG" H 2300 6323 50  0000 C CNN
F 2 "" H 2300 6150 50  0001 C CNN
F 3 "~" H 2300 6150 50  0001 C CNN
	1    2300 6150
	-1   0    0    1   
$EndComp
$Comp
L power:PWR_FLAG #FLG0103
U 1 1 5ED9AF19
P 1900 6150
F 0 "#FLG0103" H 1900 6225 50  0001 C CNN
F 1 "PWR_FLAG" H 1900 6323 50  0000 C CNN
F 2 "" H 1900 6150 50  0001 C CNN
F 3 "~" H 1900 6150 50  0001 C CNN
	1    1900 6150
	-1   0    0    1   
$EndComp
$Comp
L power:PWR_FLAG #FLG0104
U 1 1 5ED9B7C1
P 1050 6000
F 0 "#FLG0104" H 1050 6075 50  0001 C CNN
F 1 "PWR_FLAG" H 1050 6173 50  0000 C CNN
F 2 "" H 1050 6000 50  0001 C CNN
F 3 "~" H 1050 6000 50  0001 C CNN
	1    1050 6000
	1    0    0    -1  
$EndComp
Wire Wire Line
	1050 6000 1050 6150
Wire Wire Line
	1500 6000 1500 6150
Wire Wire Line
	1900 6000 1900 6150
Wire Wire Line
	2300 6000 2300 6150
Text Notes 7800 1450 0    71   ~ 0
Änderungsliste Leiterplattenlayout:\n- Footprints für J3, J4 und J5 angepasst mit Beschriftung\n- Footprint für IRM-10-12-Netzteil korrigiert\n- ESP32 SoC auf Rückseite der Platine verlegt\n- Zusätzliche Bauelemente aus Schaltplan eingefügt
Text Notes 600  2500 0    71   ~ 0
Revision 1.1\nÄnderungsliste Schaltplan:\n- Aufteilung der Anschlussklemmen auf zwei Terminalblöcke je Platinenseite\n   aufgrund von Trennstegen im Hutschienengehäuse. \n   (J1 -> J1, J2; J2 -> J6, J7)\n- Zusätzliche Schottkydioden BAT54S an den Analogeingängen für CP und \n   CT0-CT2 zum Schutz der uC-Pins vor Spannungsspitzen\n   (D8 - D11)\n- Veränderte Pin-Belegung und Footprint für J4 und J5: \n   J5 mit zusätzlichem ESP GPIO 15 auf Pin 7 als Programmierschnittstelle\n   J4 jetzt einreihig als serielle Schnittstelle, nur im Bedarfsfall bestückt\n- Angepasste Pinbelegung auf J6 und J7: 12V von J2, Pin 3 -> J6, Pin 6\n- Angepasste Pinbelegung auf J1 und J2: Separate Klemme für N für jedes Schütz\n- Strombegrenzung an 12V-Schiene bei U8: 4x 100 Ohm paralell -> 25 Ohm statt zuvor 50 Ohm \n- C16 ist ein polarisierter Tantal-Kondensator: Symbol entsprechend angepasst\n- Taster-Eingangsbeschaltung angepasst: R24 100 Ohm -> 1k Ohm, redundante Zener-Diode und Spannungsteiler entfernt (D8, R34)
$EndSCHEMATC
