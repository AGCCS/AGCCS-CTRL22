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
S 2475 2800 1800 1350
U 5E98FA8A
F0 "Mains and Power Circuitry" 50
F1 "mains_and_power.sch" 50
F2 "SSR_L1" I R 4275 3300 50 
F3 "SSR_L2" I R 4275 3450 50 
F4 "SSR_L3" I R 4275 3600 50 
F5 "BOT" O R 4275 3950 50 
$EndSheet
Text Notes 2975 3650 0    50   ~ 10
Mains Input Connector \nContactor Relays\n12V Power Supply\n3.3V DC Converter\n-12V Voltage Inverter
$Sheet
S 4975 2800 1750 2200
U 5E9B0066
F0 "ATmega4808 and Peripherals" 50
F1 "atmega4808_and_peripherals.sch" 50
F2 "SSR_L3" O L 4975 3600 50 
F3 "SSR_L2" O L 4975 3450 50 
F4 "SSR_L1" O L 4975 3300 50 
F5 "RS485_A" B R 6725 2900 50 
F6 "RS485_B" B R 6725 3050 50 
F7 "Lock_W" O R 6725 3500 50 
F8 "Lock_R" O R 6725 3350 50 
F9 "Lock_B" I R 6725 3200 50 
F10 "LED" O R 6725 3650 50 
F11 "CT1" I R 6725 4550 50 
F12 "CT2" I R 6725 4700 50 
F13 "Button" I R 6725 3800 50 
F14 "PP" I R 6725 3950 50 
F15 "CP" I R 6725 4100 50 
F16 "PWM_Out" O R 6725 4250 50 
F17 "Signal_Relay" O R 6725 4400 50 
F18 "BOT" I L 4975 3950 50 
F19 "CT3" I R 6725 4850 50 
$EndSheet
Text Notes 5575 3950 0    50   Italic 10
ATmega4808\nESP32\nRS485 Transceiver\nLock Actuator
$Sheet
S 7550 2800 1750 2200
U 5E903BF9
F0 "Analog Section and Low Voltage Outputs" 50
F1 "analog_section_low_voltage.sch" 50
F2 "PWM_Out" I L 7550 4250 50 
F3 "CP" O L 7550 4100 50 
F4 "LED" I L 7550 3650 50 
F5 "RS485_A" B L 7550 2900 50 
F6 "RS485_B" B L 7550 3050 50 
F7 "Lock_B" O L 7550 3200 50 
F8 "Lock_R" I L 7550 3350 50 
F9 "Lock_W" I L 7550 3500 50 
F10 "PP" O L 7550 3950 50 
F11 "Button" O L 7550 3800 50 
F12 "CT1" O L 7550 4550 50 
F13 "CT2" O L 7550 4700 50 
F14 "Signal_Relay" I L 7550 4400 50 
F15 "CT3" O L 7550 4850 50 
$EndSheet
Wire Wire Line
	6725 2900 7550 2900
Wire Wire Line
	7550 3050 6725 3050
Wire Wire Line
	6725 3200 7550 3200
Wire Wire Line
	7550 3350 6725 3350
Wire Wire Line
	6725 3500 7550 3500
Wire Wire Line
	6725 3650 7550 3650
Wire Wire Line
	6725 3950 7550 3950
Wire Wire Line
	6725 4100 7550 4100
Wire Wire Line
	6725 4250 7550 4250
Wire Wire Line
	7550 4400 6725 4400
Wire Wire Line
	6725 3800 7550 3800
Text Notes 9050 4000 2    50   ~ 10
Analog Section\n-Opamp Buffer\n- Comparator\nLow Voltage I/O Connector\nSignal Relays
Wire Wire Line
	4975 3300 4275 3300
Wire Wire Line
	4975 3450 4275 3450
Wire Wire Line
	4975 3600 4275 3600
Wire Wire Line
	6725 4550 7550 4550
Wire Wire Line
	6725 4700 7550 4700
Wire Wire Line
	6725 4850 7550 4850
Text Notes 750  1600 0    71   ~ 0
Revison 1.0, Mrz 2020\n- Entwurf P.T. auf Grundlage SmartEVSE\nRevision 1.1, Aug 2020\n- Diverse kleinere  Änderungen (Lock Widerstände, Pin-Belegungen, mehr\n   Überspannungsdioden, Taster Vorwiderstand, etc)\n- Layout/Bestückung (ESP32 auf der Unterseite, getrennte Schraubklemmen)\nRevision 1.2, Feb 2021\n- Tasterbeschaltung, Trenndiode für das Lock\n- Brownouterkennung  \n
Wire Wire Line
	4275 3950 4975 3950
$EndSCHEMATC
