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
F5 "BOT" O R 4250 4250 50 
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
F19 "BOT" I L 5000 4250 50 
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
Text Notes 750  1600 0    71   ~ 0
Revison 1.0, Mrz 2020\n- Entwurf P.T. auf Grundlage SmartEVSE\nRevision 1.1, Aug 2020\n- Diverse kleinere  Änderungen (Lock Widerstände, Pin-Belegungen, mehr\n   Überspannungsdioden, Taster Vorwiderstand, etc)\n- Layout/Bestückung (ESP32 auf der Unterseite, getrennte Schraubklemmen)\nRevision 1.2, Feb 2021\n- Tasterbeschaltung, Trenndiode für das Lock\n- Brownouterkennung  \n
Wire Wire Line
	4250 4250 5000 4250
$EndSCHEMATC
