EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 2 4
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
L smartevse:Varistor RV3
U 1 1 5E9A965F
P 1825 3825
F 0 "RV3" H 1875 3875 50  0000 L CNN
F 1 "Varistor" H 1900 3775 50  0001 L CNN
F 2 "Varistor:RV_Disc_D15.5mm_W5mm_P7.5mm" V 1755 3825 50  0001 C CNN
F 3 "https://www.tdk-electronics.tdk.com/inf/70/db/var/SIOV_Leaded_StandarD.pdf" H 1825 3825 50  0001 C CNN
F 4 "" H 1825 3825 50  0001 C CNN "Part"
F 5 "871-B72214S0271K551" H 1825 3825 50  0001 C CNN "PartNo"
F 6 "Mouser" H 1825 3825 50  0001 C CNN "Supplier"
	1    1825 3825
	1    0    0    -1  
$EndComp
$Comp
L smartevse:AQH3223 U3
U 1 1 5E9A9676
P 4500 3300
F 0 "U3" H 4500 2790 70  0000 C CNN
F 1 "AQH3223" H 4500 2911 70  0000 C CNN
F 2 "smartevse-footprints:DIL07SMD" H 4500 3300 50  0001 C CNN
F 3 "" H 4500 3300 50  0001 C CNN
F 4 "" H 4500 3300 50  0001 C CNN "Part"
F 5 "769-AQH1213AX" H 4500 3300 50  0001 C CNN "PartNo"
F 6 "Mouser" H 4500 3300 50  0001 C CNN "Supplier"
	1    4500 3300
	-1   0    0    1   
$EndComp
$Comp
L smartevse:AQH3223 U2
U 1 1 5E9A967C
P 4500 2300
F 0 "U2" H 4500 1790 70  0000 C CNN
F 1 "AQH3223" H 4500 1911 70  0000 C CNN
F 2 "smartevse-footprints:DIL07SMD" H 4500 2300 50  0001 C CNN
F 3 "" H 4500 2300 50  0001 C CNN
F 4 "" H 4500 2300 50  0001 C CNN "Part"
F 5 "769-AQH1213AX" H 4500 2300 50  0001 C CNN "PartNo"
F 6 "Mouser" H 4500 2300 50  0001 C CNN "Supplier"
	1    4500 2300
	-1   0    0    1   
$EndComp
$Comp
L smartevse:AQH3223 U1
U 1 1 5E9A9682
P 4500 1300
F 0 "U1" H 4500 790 70  0000 C CNN
F 1 "AQH3223" H 4500 911 70  0000 C CNN
F 2 "smartevse-footprints:DIL07SMD" H 4500 1300 50  0001 C CNN
F 3 "" H 4500 1300 50  0001 C CNN
F 4 "" H 4500 1300 50  0001 C CNN "Part"
F 5 "769-AQH1213AX" H 4500 1300 50  0001 C CNN "PartNo"
F 6 "Mouser" H 4500 1300 50  0001 C CNN "Supplier"
	1    4500 1300
	-1   0    0    1   
$EndComp
Wire Wire Line
	5000 1100 5100 1100
Wire Wire Line
	5100 1100 5100 1200
Wire Wire Line
	5100 1300 5000 1300
Wire Wire Line
	5000 1200 5100 1200
Connection ~ 5100 1200
Wire Wire Line
	5100 1200 5100 1300
Wire Wire Line
	5200 1200 5100 1200
$Comp
L power:GND #PWR02
U 1 1 5E9A968F
P 5200 1200
F 0 "#PWR02" H 5200 950 50  0001 C CNN
F 1 "GND" H 5205 1027 50  0000 C CNN
F 2 "" H 5200 1200 50  0001 C CNN
F 3 "" H 5200 1200 50  0001 C CNN
	1    5200 1200
	1    0    0    -1  
$EndComp
Wire Wire Line
	5000 2100 5100 2100
Wire Wire Line
	5100 2100 5100 2200
Wire Wire Line
	5100 2300 5000 2300
Wire Wire Line
	5000 2200 5100 2200
Wire Wire Line
	5000 3100 5100 3100
Wire Wire Line
	5100 3100 5100 3200
Wire Wire Line
	5100 3300 5000 3300
Wire Wire Line
	5000 3200 5100 3200
$Comp
L power:GND #PWR03
U 1 1 5E9A969D
P 5200 2200
F 0 "#PWR03" H 5200 1950 50  0001 C CNN
F 1 "GND" H 5205 2027 50  0000 C CNN
F 2 "" H 5200 2200 50  0001 C CNN
F 3 "" H 5200 2200 50  0001 C CNN
	1    5200 2200
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR04
U 1 1 5E9A96A3
P 5200 3200
F 0 "#PWR04" H 5200 2950 50  0001 C CNN
F 1 "GND" H 5205 3027 50  0000 C CNN
F 2 "" H 5200 3200 50  0001 C CNN
F 3 "" H 5200 3200 50  0001 C CNN
	1    5200 3200
	1    0    0    -1  
$EndComp
Wire Wire Line
	5000 2500 5200 2500
Wire Wire Line
	5000 3500 5200 3500
Wire Wire Line
	5000 1500 5200 1500
NoConn ~ 4000 1100
NoConn ~ 4000 2100
NoConn ~ 4000 3100
Text Notes 3400 650  0    50   ~ 0
Solid State Relays
$Comp
L Converter_ACDC:IRM-10-12 U4
U 1 1 5E9A96BD
P 4125 5525
F 0 "U4" H 4125 5972 42  0000 C CNN
F 1 "IRM-10-12" H 4125 5893 42  0000 C CNN
F 2 "Converter_ACDC:Converter_ACDC_MeanWell_IRM-10-xx_THT" H 4125 5525 50  0001 C CNN
F 3 "https://www.meanwell.com/Upload/PDF/IRM-10/IRM-10-SPEC.PDF" H 4125 5525 50  0001 C CNN
F 4 "" H 4125 5525 50  0001 C CNN "Part"
F 5 "709-IRM10-12" H 4125 5525 50  0001 C CNN "PartNo"
F 6 "Mouser" H 4125 5525 50  0001 C CNN "Supplier"
	1    4125 5525
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR06
U 1 1 5E9A96CF
P 4825 6125
F 0 "#PWR06" H 4825 5875 50  0001 C CNN
F 1 "GND" H 4830 5952 50  0000 C CNN
F 2 "" H 4825 6125 50  0001 C CNN
F 3 "" H 4825 6125 50  0001 C CNN
	1    4825 6125
	1    0    0    -1  
$EndComp
$Comp
L power:+12V #PWR05
U 1 1 5E9A96D5
P 4825 4700
F 0 "#PWR05" H 4825 4550 50  0001 C CNN
F 1 "+12V" H 4840 4873 50  0000 C CNN
F 2 "" H 4825 4700 50  0001 C CNN
F 3 "" H 4825 4700 50  0001 C CNN
	1    4825 4700
	1    0    0    -1  
$EndComp
Text Notes 4400 4525 2    50   ~ 0
12V Power Supply
$Comp
L Device:C_Small C1
U 1 1 5E9A96DD
P 6525 1650
F 0 "C1" H 6575 1750 50  0000 L CNN
F 1 "10u16V" V 6625 1400 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 6563 1500 50  0001 C CNN
F 3 "~" H 6525 1650 50  0001 C CNN
F 4 "" H 6525 1650 50  0001 C CNN "Part"
F 5 "963-EMK212ABJ106MG-T" H 6525 1650 50  0001 C CNN "PartNo"
F 6 "Mouser" H 6525 1650 50  0001 C CNN "Supplier"
	1    6525 1650
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C3
U 1 1 5E9A96E3
P 6875 1650
F 0 "C3" H 6925 1750 50  0000 L CNN
F 1 "10u16V" V 6975 1400 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 6913 1500 50  0001 C CNN
F 3 "~" H 6875 1650 50  0001 C CNN
F 4 "" H 6875 1650 50  0001 C CNN "Part"
F 5 "963-EMK212ABJ106MG-T" H 6875 1650 50  0001 C CNN "PartNo"
F 6 "Mouser" H 6875 1650 50  0001 C CNN "Supplier"
	1    6875 1650
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C6
U 1 1 5E9A96EF
P 7575 1650
F 0 "C6" H 7625 1750 50  0000 L CNN
F 1 "10u16V" V 7675 1400 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 7613 1500 50  0001 C CNN
F 3 "~" H 7575 1650 50  0001 C CNN
F 4 "" H 7575 1650 50  0001 C CNN "Part"
F 5 "963-EMK212ABJ106MG-T" H 7575 1650 50  0001 C CNN "PartNo"
F 6 "Mouser" H 7575 1650 50  0001 C CNN "Supplier"
	1    7575 1650
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C7
U 1 1 5E9A96F5
P 7925 1650
F 0 "C7" H 7800 1750 50  0000 L CNN
F 1 "100n" V 7825 1475 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 7963 1500 50  0001 C CNN
F 3 "~" H 7925 1650 50  0001 C CNN
F 4 "" H 7925 1650 50  0001 C CNN "Part"
F 5 "963-EMF212B7104KGHT " H 7925 1650 50  0001 C CNN "PartNo"
F 6 "Mouser" H 7925 1650 50  0001 C CNN "Supplier"
	1    7925 1650
	1    0    0    -1  
$EndComp
Wire Wire Line
	6525 1350 6875 1350
Connection ~ 7575 1350
Wire Wire Line
	7575 1350 7925 1350
Connection ~ 7225 1350
Wire Wire Line
	7225 1350 7575 1350
Connection ~ 6875 1350
Wire Wire Line
	6875 1350 7225 1350
Wire Wire Line
	6525 2050 6875 2050
Wire Wire Line
	6875 2050 7225 2050
Connection ~ 6875 2050
Wire Wire Line
	7225 2050 7575 2050
Connection ~ 7225 2050
Wire Wire Line
	7575 2050 7925 2050
Connection ~ 7575 2050
Connection ~ 7925 1350
Wire Wire Line
	8725 1750 8725 2050
Wire Wire Line
	8725 2050 7925 2050
Connection ~ 7925 2050
$Comp
L Device:C_Small C9
U 1 1 5E9A9718
P 9325 1350
F 0 "C9" V 9096 1350 50  0000 C CNN
F 1 "100n" V 9187 1350 50  0000 C CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 9325 1350 50  0001 C CNN
F 3 "~" H 9325 1350 50  0001 C CNN
F 4 "" H 9325 1350 50  0001 C CNN "Part"
F 5 "963-EMF212B7104KGHT " H 9325 1350 50  0001 C CNN "PartNo"
F 6 "Mouser" H 9325 1350 50  0001 C CNN "Supplier"
	1    9325 1350
	0    1    1    0   
$EndComp
Wire Wire Line
	9225 1350 9125 1350
Wire Wire Line
	9125 1450 9475 1450
Wire Wire Line
	9475 1450 9475 1350
Wire Wire Line
	9475 1350 9425 1350
Wire Wire Line
	9475 1450 9475 1700
Connection ~ 9475 1450
$Comp
L Diode:PMEG4050EP D1
U 1 1 5E9A9724
P 9475 1850
F 0 "D1" V 9429 1929 50  0000 L CNN
F 1 "PMEG4010BEA" H 9125 1750 50  0000 L CNN
F 2 "Diode_SMD:D_SOD-323_HandSoldering" H 9475 1675 50  0001 C CNN
F 3 "https://assets.nexperia.com/documents/data-sheet/PMEG4050EP.pdf" H 9475 1850 50  0001 C CNN
F 4 "" H 9475 1850 50  0001 C CNN "Part"
F 5 "771-PMEG4010BEA,135" H 9475 1850 50  0001 C CNN "PartNo"
F 6 "Mouser" H 9475 1850 50  0001 C CNN "Supplier"
	1    9475 1850
	0    1    1    0   
$EndComp
Wire Wire Line
	9475 2000 9475 2050
Wire Wire Line
	9475 2050 9125 2050
Connection ~ 8725 2050
Wire Wire Line
	9975 2050 9975 1950
$Comp
L Device:R_Small R2
U 1 1 5E9A972E
P 9975 1850
F 0 "R2" H 10034 1896 50  0000 L CNN
F 1 "10k" V 9875 1800 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 9975 1850 50  0001 C CNN
F 3 "~" H 9975 1850 50  0001 C CNN
F 4 "" H 9975 1850 50  0001 C CNN "Part"
F 5 "594-MCT06030C1002FP5 " H 9975 1850 50  0001 C CNN "PartNo"
F 6 "Mouser" H 9975 1850 50  0001 C CNN "Supplier"
	1    9975 1850
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Small R1
U 1 1 5E9A9734
P 9975 1500
F 0 "R1" H 10034 1546 50  0000 L CNN
F 1 "31.6k" V 9875 1400 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 9975 1500 50  0001 C CNN
F 3 "~" H 9975 1500 50  0001 C CNN
F 4 "" H 9975 1500 50  0001 C CNN "Part"
F 5 "594-MCT06030C3162FP5" H 9975 1500 50  0001 C CNN "PartNo"
F 6 "Mouser" H 9975 1500 50  0001 C CNN "Supplier"
	1    9975 1500
	1    0    0    -1  
$EndComp
Wire Wire Line
	9475 2050 9975 2050
Connection ~ 9475 2050
$Comp
L Device:L L1
U 1 1 5E9A973D
P 9725 1350
F 0 "L1" V 9915 1350 50  0000 C CNN
F 1 "3.3u" V 9824 1350 50  0000 C CNN
F 2 "Inductor_SMD:L_1210_3225Metric_Pad1.42x2.65mm_HandSolder" H 9725 1350 50  0001 C CNN
F 3 "https://product.tdk.com/info/en/catalog/datasheets/inductor_commercial_standard_mlf1608_en.pdf" H 9725 1350 50  0001 C CNN
F 4 "" H 9725 1350 50  0001 C CNN "Part"
F 5 "810-MLF1608A3R3J " H 9725 1350 50  0001 C CNN "PartNo"
F 6 "Mouser" H 9725 1350 50  0001 C CNN "Supplier"
	1    9725 1350
	0    -1   -1   0   
$EndComp
Wire Wire Line
	9575 1350 9475 1350
Connection ~ 9475 1350
Wire Wire Line
	9125 1550 9125 1650
Wire Wire Line
	9125 1650 9975 1650
Connection ~ 9975 1650
Wire Wire Line
	9975 1650 9975 1750
Wire Wire Line
	9875 1350 9975 1350
$Comp
L Device:C_Small C10
U 1 1 5E9A974E
P 10525 1650
F 0 "C10" H 10375 1750 50  0000 L CNN
F 1 "10u16V" V 10425 1350 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 10563 1500 50  0001 C CNN
F 3 "~" H 10525 1650 50  0001 C CNN
F 4 "" H 10525 1650 50  0001 C CNN "Part"
F 5 "963-EMK212ABJ106MG-T" H 10525 1650 50  0001 C CNN "PartNo"
F 6 "Mouser" H 10525 1650 50  0001 C CNN "Supplier"
	1    10525 1650
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C11
U 1 1 5E9A9754
P 10825 1650
F 0 "C11" H 10675 1750 50  0000 L CNN
F 1 "10u16V" V 10725 1350 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 10863 1500 50  0001 C CNN
F 3 "~" H 10825 1650 50  0001 C CNN
F 4 "" H 10825 1650 50  0001 C CNN "Part"
F 5 "963-EMK212ABJ106MG-T" H 10825 1650 50  0001 C CNN "PartNo"
F 6 "Mouser" H 10825 1650 50  0001 C CNN "Supplier"
	1    10825 1650
	1    0    0    -1  
$EndComp
Connection ~ 10525 2050
Wire Wire Line
	10525 2050 10825 2050
Connection ~ 10525 1350
Wire Wire Line
	10525 1350 10825 1350
Wire Wire Line
	10825 1350 10825 1250
Connection ~ 10825 1350
$Comp
L power:+3.3V #PWR012
U 1 1 5E9A9760
P 10825 1250
F 0 "#PWR012" H 10825 1100 50  0001 C CNN
F 1 "+3.3V" H 10840 1423 50  0000 C CNN
F 2 "" H 10825 1250 50  0001 C CNN
F 3 "" H 10825 1250 50  0001 C CNN
	1    10825 1250
	1    0    0    -1  
$EndComp
$Comp
L power:+12V #PWR08
U 1 1 5E9A9766
P 7225 1250
F 0 "#PWR08" H 7225 1100 50  0001 C CNN
F 1 "+12V" H 7240 1423 50  0000 C CNN
F 2 "" H 7225 1250 50  0001 C CNN
F 3 "" H 7225 1250 50  0001 C CNN
	1    7225 1250
	1    0    0    -1  
$EndComp
Wire Wire Line
	7225 1250 7225 1350
Text Notes 6850 650  2    50   ~ 0
3.3V Regulator
$Comp
L smartevse:ICL7660 U5
U 1 1 5E9A976E
P 9815 3730
AR Path="/5E9A976E" Ref="U5"  Part="1" 
AR Path="/5E98FA8A/5E9A976E" Ref="U5"  Part="1" 
F 0 "U5" H 9815 4316 59  0000 C CNN
F 1 "TC7660SEOA" H 9815 4211 59  0000 C CNN
F 2 "Package_SO:SOIC-8_3.9x4.9mm_P1.27mm" H 9815 3730 50  0001 C CNN
F 3 "https://www.mouser.de/datasheet/2/268/21467B-1180369.pdf" H 9815 3730 50  0001 C CNN
F 4 "" H 9815 3730 50  0001 C CNN "Part"
F 5 "579-TC7660SEOA" H 9815 3730 50  0001 C CNN "PartNo"
F 6 "Mouser" H 9815 3730 50  0001 C CNN "Supplier"
	1    9815 3730
	1    0    0    -1  
$EndComp
Wire Wire Line
	9315 3430 9015 3430
$Comp
L power:+12V #PWR09
U 1 1 5E9A9775
P 9015 3030
F 0 "#PWR09" H 9015 2880 50  0001 C CNN
F 1 "+12V" H 9030 3203 50  0000 C CNN
F 2 "" H 9015 3030 50  0001 C CNN
F 3 "" H 9015 3030 50  0001 C CNN
	1    9015 3030
	1    0    0    -1  
$EndComp
Wire Wire Line
	9015 3430 9015 3080
Wire Wire Line
	9015 3080 10415 3080
Wire Wire Line
	10415 3080 10415 3430
Wire Wire Line
	10415 3430 10315 3430
Connection ~ 9015 3080
Wire Wire Line
	9015 3080 9015 3030
$Comp
L Device:C_Small C4
U 1 1 5E9A9781
P 8765 3330
F 0 "C4" H 8880 3376 50  0000 L CNN
F 1 "100n" V 8915 3130 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 8803 3180 50  0001 C CNN
F 3 "~" H 8765 3330 50  0001 C CNN
F 4 "" H 8765 3330 50  0001 C CNN "Part"
F 5 "963-EMF212B7104KGHT " H 8765 3330 50  0001 C CNN "PartNo"
F 6 "Mouser" H 8765 3330 50  0001 C CNN "Supplier"
	1    8765 3330
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C2
U 1 1 5E9A9787
P 8415 3780
F 0 "C2" H 8305 3865 50  0000 L CNN
F 1 "10u16V" V 8525 3595 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 8453 3630 50  0001 C CNN
F 3 "~" H 8415 3780 50  0001 C CNN
F 4 "" H 8415 3780 50  0001 C CNN "Part"
F 5 "963-EMK212ABJ106MG-T" H 8415 3780 50  0001 C CNN "PartNo"
F 6 "Mouser" H 8415 3780 50  0001 C CNN "Supplier"
	1    8415 3780
	1    0    0    -1  
$EndComp
Wire Wire Line
	8415 3630 9415 3630
Wire Wire Line
	8765 3830 9415 3830
$Comp
L Device:C_Small C8
U 1 1 5E9A9790
P 10615 4130
F 0 "C8" H 10730 4176 50  0000 L CNN
F 1 "10u16V" V 10765 3830 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 10653 3980 50  0001 C CNN
F 3 "~" H 10615 4130 50  0001 C CNN
F 4 "" H 10615 4130 50  0001 C CNN "Part"
F 5 "963-EMK212ABJ106MG-T" H 10615 4130 50  0001 C CNN "PartNo"
F 6 "Mouser" H 10615 4130 50  0001 C CNN "Supplier"
	1    10615 4130
	1    0    0    -1  
$EndComp
Wire Wire Line
	10615 4030 10315 4030
Wire Wire Line
	10615 4230 8765 4230
Wire Wire Line
	8765 4230 8765 3830
Connection ~ 8765 3830
Wire Wire Line
	6525 1350 6525 1550
Wire Wire Line
	6525 1750 6525 2050
Wire Wire Line
	6875 1750 6875 2050
Wire Wire Line
	6875 1350 6875 1550
Wire Wire Line
	7225 1350 7225 1550
Wire Wire Line
	7225 1750 7225 2050
Wire Wire Line
	7575 1350 7575 1550
Wire Wire Line
	7575 1750 7575 2050
Wire Wire Line
	7925 1750 7925 2050
Wire Wire Line
	7925 1350 7925 1550
Wire Wire Line
	10525 1350 10525 1550
Wire Wire Line
	10825 1350 10825 1550
Wire Wire Line
	10825 1750 10825 2050
Wire Wire Line
	10525 1750 10525 2050
Wire Wire Line
	8765 3430 8765 3830
Wire Wire Line
	8415 3880 8415 4030
Wire Wire Line
	8415 3630 8415 3680
$Comp
L power:GND #PWR011
U 1 1 5E9A97AB
P 9125 2050
F 0 "#PWR011" H 9125 1800 50  0001 C CNN
F 1 "GND" H 9130 1877 50  0000 C CNN
F 2 "" H 9125 2050 50  0001 C CNN
F 3 "" H 9125 2050 50  0001 C CNN
	1    9125 2050
	1    0    0    -1  
$EndComp
Connection ~ 9125 2050
Wire Wire Line
	9125 2050 8725 2050
$Comp
L power:GND #PWR07
U 1 1 5E9A97B3
P 8765 4230
F 0 "#PWR07" H 8765 3980 50  0001 C CNN
F 1 "GND" H 8770 4057 50  0000 C CNN
F 2 "" H 8765 4230 50  0001 C CNN
F 3 "" H 8765 4230 50  0001 C CNN
	1    8765 4230
	1    0    0    -1  
$EndComp
Connection ~ 8765 4230
Connection ~ 10615 4030
$Comp
L power:-12V #PWR010
U 1 1 5E9A97BD
P 10965 3930
F 0 "#PWR010" H 10965 4030 50  0001 C CNN
F 1 "-12V" H 10980 4103 50  0000 C CNN
F 2 "" H 10965 3930 50  0001 C CNN
F 3 "" H 10965 3930 50  0001 C CNN
	1    10965 3930
	1    0    0    -1  
$EndComp
Wire Wire Line
	8415 4030 9415 4030
NoConn ~ 10315 3630
NoConn ~ 10315 3830
Text Notes 8785 2745 2    50   ~ 0
12V Inverter\n
Wire Notes Line
	3350 550  3350 4050
Wire Notes Line
	3350 4050 5600 4050
Wire Notes Line
	5600 4050 5600 550 
Wire Notes Line
	5600 550  3350 550 
Wire Notes Line
	8265 2630 8265 4830
Wire Notes Line
	8265 4830 11115 4830
Wire Notes Line
	11115 4830 11115 2630
Wire Notes Line
	11115 2630 8265 2630
Connection ~ 5100 2200
Wire Wire Line
	5100 2200 5100 2300
Wire Wire Line
	5100 2200 5200 2200
Connection ~ 5100 3200
Wire Wire Line
	5100 3200 5100 3300
Wire Wire Line
	5100 3200 5200 3200
Text HLabel 5200 1500 2    50   Input ~ 0
SSR_L1
Text HLabel 5200 2500 2    50   Input ~ 0
SSR_L2
Text HLabel 5200 3500 2    50   Input ~ 0
SSR_L3
Text Label 990  1400 0    50   ~ 0
PE
Text Label 1650 1600 0    50   ~ 0
C_L1
Text Label 1650 1800 0    50   ~ 0
C_L2
Text Label 1650 2050 0    50   ~ 0
C_L3
$Comp
L power:GND #PWR01
U 1 1 5E9A964B
P 1075 2650
F 0 "#PWR01" H 1075 2400 50  0001 C CNN
F 1 "GND" H 1080 2477 50  0000 C CNN
F 2 "" H 1075 2650 50  0001 C CNN
F 3 "" H 1075 2650 50  0001 C CNN
	1    1075 2650
	1    0    0    -1  
$EndComp
Wire Wire Line
	975  1400 1075 1400
Wire Wire Line
	3550 3300 4000 3300
Wire Wire Line
	3550 2300 4000 2300
Text Label 1175 2350 0    50   ~ 0
L
$Comp
L smartevse:FuseT1.25 F2
U 1 1 5ECD985B
P 1750 1300
F 0 "F2" V 1825 1300 50  0000 C CNN
F 1 "T1.25A" V 1675 1300 50  0000 C CNN
F 2 "smartevse-footprints:TE5" V 1680 1300 50  0001 C CNN
F 3 "https://www.mouser.de/datasheet/2/240/Littelfuse_Fuse_392_Datasheet.pdf-795005.pdf" H 1750 1300 50  0001 C CNN
F 4 "" H 1750 1300 50  0001 C CNN "Part"
F 5 "576-3921125000" H 1750 1300 50  0001 C CNN "PartNo"
F 6 "Mouser" H 1750 1300 50  0001 C CNN "Supplier"
	1    1750 1300
	0    1    1    0   
$EndComp
Wire Wire Line
	8765 3230 8765 3080
Wire Wire Line
	8765 3080 9015 3080
$Comp
L smartevse:AP5100 U6
U 1 1 5ED2A5D9
P 8725 1450
F 0 "U6" H 8725 1817 50  0000 C CNN
F 1 "AP5100" H 8725 1726 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23-6" H 9425 1200 50  0001 C CNN
F 3 "https://www.mouser.de/datasheet/2/115/AP5100-82364.pdf" H 8475 1200 50  0001 C CNN
F 4 "Mouser" H 8725 1450 50  0001 C CNN "Supplier"
F 5 "621-AP5100WG-7" H 8725 1450 50  0001 C CNN "PartNo"
	1    8725 1450
	1    0    0    -1  
$EndComp
Wire Wire Line
	9975 1600 9975 1650
Wire Wire Line
	9975 1400 9975 1350
Wire Wire Line
	7925 1350 8025 1350
$Comp
L Device:R_Small R14
U 1 1 5ED2FD41
P 8225 1550
F 0 "R14" V 8125 1550 50  0000 C CNN
F 1 "100k" V 8300 1550 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 8225 1550 50  0001 C CNN
F 3 "~" H 8225 1550 50  0001 C CNN
F 4 "Mouser" H 8225 1550 50  0001 C CNN "Supplier"
F 5 "594-MCT06030C1003FP5 " H 8225 1550 50  0001 C CNN "PartNo"
	1    8225 1550
	0    1    1    0   
$EndComp
Wire Wire Line
	8125 1550 8025 1550
Wire Wire Line
	8025 1550 8025 1350
Connection ~ 8025 1350
Wire Wire Line
	8025 1350 8325 1350
Wire Wire Line
	9975 1350 10175 1350
Connection ~ 9975 1350
Wire Wire Line
	9975 2050 10525 2050
Connection ~ 9975 2050
$Comp
L Device:C_Small C26
U 1 1 5ED43B27
P 10175 1500
F 0 "C26" H 10267 1546 50  0000 L CNN
F 1 "100p" V 10275 1300 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 10175 1500 50  0001 C CNN
F 3 "~" H 10175 1500 50  0001 C CNN
F 4 "Mouser" H 10175 1500 50  0001 C CNN "Supplier"
F 5 "77-VJ0805Y101KXACBC " H 10175 1500 50  0001 C CNN "PartNo"
	1    10175 1500
	1    0    0    -1  
$EndComp
Wire Wire Line
	10175 1400 10175 1350
Connection ~ 10175 1350
Wire Wire Line
	10175 1350 10525 1350
Wire Wire Line
	10175 1600 10175 1650
Wire Wire Line
	10175 1650 9975 1650
NoConn ~ 975  2250
$Comp
L Device:CP_Small C27
U 1 1 603D814C
P 4825 5525
F 0 "C27" H 4850 5625 50  0000 L CNN
F 1 "2200u16V" V 4705 5335 50  0000 L CNN
F 2 "Capacitor_THT:CP_Radial_D12.5mm_P7.50mm" H 4825 5525 50  0001 C CNN
F 3 "~" H 4825 5525 50  0001 C CNN
F 4 "871-B41888C4228M000 " H 4825 5525 50  0001 C CNN "PartNo"
F 5 "Mouser" H 4825 5525 50  0001 C CNN "Supplier"
	1    4825 5525
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C5
U 1 1 5E9A96E9
P 7225 1650
F 0 "C5" H 7325 1750 50  0000 L CNN
F 1 "10u16V" V 7375 1400 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 7263 1500 50  0001 C CNN
F 3 "~" H 7225 1650 50  0001 C CNN
F 4 "" H 7225 1650 50  0001 C CNN "Part"
F 5 "963-EMK212ABJ106MG-T" H 7225 1650 50  0001 C CNN "PartNo"
F 6 "Mouser" H 7225 1650 50  0001 C CNN "Supplier"
	1    7225 1650
	1    0    0    -1  
$EndComp
Wire Wire Line
	975  2050 1825 2050
Wire Wire Line
	3550 5425 3725 5425
Wire Notes Line
	6225 550  6225 2400
Wire Wire Line
	4525 5025 4825 5025
$Comp
L Device:D_Zener_Small_ALT D13
U 1 1 6060FDF4
P 5450 5275
F 0 "D13" V 5404 5345 50  0000 L CNN
F 1 "9.1" V 5495 5345 50  0000 L CNN
F 2 "Diode_SMD:D_SOD-323_HandSoldering" H 5450 5275 50  0001 C CNN
F 3 "~" H 5450 5275 50  0001 C CNN
F 4 " 621-UDZ9V1BQ-13 " H 5450 5275 50  0001 C CNN "PartNo"
F 5 "Mouser" H 5450 5275 50  0001 C CNN "Supplier"
	1    5450 5275
	0    1    1    0   
$EndComp
$Comp
L Device:R_Small R40
U 1 1 6061109D
P 5450 5750
F 0 "R40" H 5509 5796 50  0000 L CNN
F 1 "2k" H 5509 5705 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" V 5380 5750 50  0001 C CNN
F 3 "~" H 5450 5750 50  0001 C CNN
F 4 "594-MCT06030C2001FP5 " H 5450 5750 50  0001 C CNN "PartNo"
F 5 "Mouser" H 5450 5750 50  0001 C CNN "Supplier"
	1    5450 5750
	1    0    0    -1  
$EndComp
Connection ~ 4825 5025
Wire Wire Line
	4825 5025 4825 4700
Wire Wire Line
	4825 5025 4825 5425
Wire Wire Line
	5450 5025 5450 5175
Wire Wire Line
	10965 4030 10965 3930
Wire Wire Line
	10615 4030 10965 4030
$Comp
L power:PWR_FLAG #FLG?
U 1 1 60BB0926
P 2000 7175
AR Path="/60BB0926" Ref="#FLG?"  Part="1" 
AR Path="/5E98FA8A/60BB0926" Ref="#FLG0101"  Part="1" 
F 0 "#FLG0101" H 2000 7250 50  0001 C CNN
F 1 "PWR_FLAG" H 2000 7348 50  0000 C CNN
F 2 "" H 2000 7175 50  0001 C CNN
F 3 "~" H 2000 7175 50  0001 C CNN
	1    2000 7175
	-1   0    0    1   
$EndComp
$Comp
L power:-12V #PWR0110
U 1 1 60BC5A0D
P 2500 7025
F 0 "#PWR0110" H 2500 7125 50  0001 C CNN
F 1 "-12V" H 2515 7198 50  0000 C CNN
F 2 "" H 2500 7025 50  0001 C CNN
F 3 "" H 2500 7025 50  0001 C CNN
	1    2500 7025
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR0111
U 1 1 60BCCB52
P 2000 7025
F 0 "#PWR0111" H 2000 6875 50  0001 C CNN
F 1 "+3.3V" H 2015 7198 50  0000 C CNN
F 2 "" H 2000 7025 50  0001 C CNN
F 3 "" H 2000 7025 50  0001 C CNN
	1    2000 7025
	1    0    0    -1  
$EndComp
Wire Wire Line
	2000 7025 2000 7175
Wire Notes Line
	11125 550  11125 2400
Wire Notes Line
	6225 550  11125 550 
Wire Notes Line
	6225 2400 11125 2400
$Comp
L Device:CP_Small C?
U 1 1 60BEF32A
P 7300 5525
AR Path="/5E9B0066/60BEF32A" Ref="C?"  Part="1" 
AR Path="/5E98FA8A/60BEF32A" Ref="C12"  Part="1" 
F 0 "C12" H 7350 5625 50  0000 L CNN
F 1 "15000u16V" V 7180 5315 50  0000 L CNN
F 2 "Capacitor_THT:CP_Radial_D22.0mm_P10.00mm_SnapIn" H 7300 5525 50  0001 C CNN
F 3 "~" H 7300 5525 50  0001 C CNN
F 4 "" H 7300 5525 50  0001 C CNN "Part"
F 5 "871-B41231A4159M000" H 7300 5525 50  0001 C CNN "PartNo"
F 6 "Mouser" H 7300 5525 50  0001 C CNN "Supplier"
	1    7300 5525
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Small R?
U 1 1 60BEF345
P 6550 5225
AR Path="/5E9B0066/60BEF345" Ref="R?"  Part="1" 
AR Path="/5E98FA8A/60BEF345" Ref="R4"  Part="1" 
F 0 "R4" V 6470 5165 50  0000 C CNN
F 1 "100" V 6470 5305 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 6550 5225 50  0001 C CNN
F 3 "~" H 6550 5225 50  0001 C CNN
F 4 "" H 6550 5225 50  0001 C CNN "Part"
F 5 "594-MCT06030C1000FP5 " H 6550 5225 50  0001 C CNN "PartNo"
F 6 "Mouser" H 6550 5225 50  0001 C CNN "Supplier"
	1    6550 5225
	0    1    1    0   
$EndComp
$Comp
L Device:R_Small R?
U 1 1 60BEF33C
P 6550 5625
AR Path="/5E9B0066/60BEF33C" Ref="R?"  Part="1" 
AR Path="/5E98FA8A/60BEF33C" Ref="R6"  Part="1" 
F 0 "R6" V 6470 5565 50  0000 C CNN
F 1 "100" V 6470 5705 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 6550 5625 50  0001 C CNN
F 3 "~" H 6550 5625 50  0001 C CNN
F 4 "" H 6550 5625 50  0001 C CNN "Part"
F 5 "594-MCT06030C1000FP5 " H 6550 5625 50  0001 C CNN "PartNo"
F 6 "Mouser" H 6550 5625 50  0001 C CNN "Supplier"
	1    6550 5625
	0    1    1    0   
$EndComp
$Comp
L Device:R_Small R?
U 1 1 60BEF333
P 6550 5425
AR Path="/5E9B0066/60BEF333" Ref="R?"  Part="1" 
AR Path="/5E98FA8A/60BEF333" Ref="R5"  Part="1" 
F 0 "R5" V 6470 5365 50  0000 C CNN
F 1 "100" V 6470 5515 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 6550 5425 50  0001 C CNN
F 3 "~" H 6550 5425 50  0001 C CNN
F 4 "" H 6550 5425 50  0001 C CNN "Part"
F 5 "594-MCT06030C1000FP5 " H 6550 5425 50  0001 C CNN "PartNo"
F 6 "Mouser" H 6550 5425 50  0001 C CNN "Supplier"
	1    6550 5425
	0    1    1    0   
$EndComp
$Comp
L Diode:PMEG4050EP D12
U 1 1 60BEF321
P 5900 5025
AR Path="/5E98FA8A/60BEF321" Ref="D12"  Part="1" 
AR Path="/5E9B0066/60BEF321" Ref="D?"  Part="1" 
F 0 "D12" H 5825 4900 50  0000 L CNN
F 1 "PMEG4010BEA" H 5625 5150 50  0000 L CNN
F 2 "Diode_SMD:D_SOD-323_HandSoldering" H 5900 4850 50  0001 C CNN
F 3 "https://assets.nexperia.com/documents/data-sheet/PMEG4050EP.pdf" H 5900 5025 50  0001 C CNN
F 4 "" H 5900 5025 50  0001 C CNN "Part"
F 5 "771-PMEG4010BEA,135" H 5900 5025 50  0001 C CNN "PartNo"
F 6 "Mouser" H 5900 5025 50  0001 C CNN "Supplier"
	1    5900 5025
	-1   0    0    1   
$EndComp
Connection ~ 6300 5025
Wire Wire Line
	6050 5025 6300 5025
Wire Wire Line
	6300 5025 6450 5025
Connection ~ 6300 5425
Wire Wire Line
	6300 5625 6450 5625
Wire Wire Line
	6300 5425 6300 5625
Connection ~ 6300 5225
Wire Wire Line
	6300 5025 6300 5225
Wire Wire Line
	6300 5425 6450 5425
Wire Wire Line
	6300 5225 6300 5425
Wire Wire Line
	6450 5225 6300 5225
Text HLabel 5700 5525 2    50   Output ~ 0
BOT
Wire Wire Line
	5450 5025 5750 5025
Connection ~ 5450 5025
$Comp
L Device:R_Small R?
U 1 1 60BEF34E
P 6550 5025
AR Path="/5E9B0066/60BEF34E" Ref="R?"  Part="1" 
AR Path="/5E98FA8A/60BEF34E" Ref="R3"  Part="1" 
F 0 "R3" V 6470 4965 50  0000 C CNN
F 1 "100" V 6470 5105 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric_Pad1.05x0.95mm_HandSolder" H 6550 5025 50  0001 C CNN
F 3 "~" H 6550 5025 50  0001 C CNN
F 4 "" H 6550 5025 50  0001 C CNN "Part"
F 5 "594-MCT06030C1000FP5 " H 6550 5025 50  0001 C CNN "PartNo"
F 6 "Mouser" H 6550 5025 50  0001 C CNN "Supplier"
	1    6550 5025
	0    1    1    0   
$EndComp
Wire Wire Line
	5450 5375 5450 5525
Wire Wire Line
	5450 5850 5450 6000
Wire Wire Line
	5450 5525 5700 5525
Connection ~ 5450 5525
Wire Wire Line
	5450 5525 5450 5650
Wire Wire Line
	7300 5625 7300 6000
Wire Wire Line
	2500 7025 2500 7175
$Comp
L power:PWR_FLAG #FLG?
U 1 1 60BB780A
P 2500 7175
AR Path="/60BB780A" Ref="#FLG?"  Part="1" 
AR Path="/5E98FA8A/60BB780A" Ref="#FLG0102"  Part="1" 
F 0 "#FLG0102" H 2500 7250 50  0001 C CNN
F 1 "PWR_FLAG" H 2500 7348 50  0000 C CNN
F 2 "" H 2500 7175 50  0001 C CNN
F 3 "~" H 2500 7175 50  0001 C CNN
	1    2500 7175
	-1   0    0    1   
$EndComp
Wire Wire Line
	3550 2300 3550 3300
Text Notes 600  650  0    50   ~ 0
Mains Input & Contactor Output\n
Wire Notes Line
	550  550  3150 550 
$Comp
L Connector:Screw_Terminal_01x05 J1
U 1 1 5F37F01F
P 775 1600
F 0 "J1" H 775 1900 50  0000 C CNN
F 1 "Screw_Terminal_01x05" H 693 1926 50  0001 C CNN
F 2 "atmevse-footprints:hv_screw_5pin_1" H 775 1600 50  0001 C CNN
F 3 "~" H 775 1600 50  0001 C CNN
	1    775  1600
	-1   0    0    -1  
$EndComp
$Comp
L Connector:Screw_Terminal_01x05 J2
U 1 1 5F3809ED
P 775 2150
F 0 "J2" H 775 1850 50  0000 C CNN
F 1 "Screw_Terminal_01x05" H 725 1750 50  0001 C CNN
F 2 "atmevse-footprints:hv_screw_5pin_2" H 775 2150 50  0001 C CNN
F 3 "~" H 775 2150 50  0001 C CNN
	1    775  2150
	-1   0    0    -1  
$EndComp
Wire Wire Line
	1275 1950 1275 2150
Wire Wire Line
	1275 1950 1275 1700
Connection ~ 1275 1950
Wire Wire Line
	1275 1700 1275 1500
Connection ~ 1275 1700
Wire Wire Line
	975  1500 1275 1500
Wire Wire Line
	975  1700 1275 1700
Wire Wire Line
	975  1950 1275 1950
Wire Wire Line
	975  2150 1275 2150
Wire Wire Line
	1075 1400 1075 2650
Wire Wire Line
	1475 1300 1600 1300
$Comp
L smartevse:Varistor RV2
U 1 1 5E9A9659
P 2175 2825
F 0 "RV2" H 2225 2880 50  0000 L CNN
F 1 "Varistor" H 2250 2780 50  0001 L CNN
F 2 "Varistor:RV_Disc_D15.5mm_W5mm_P7.5mm" V 2105 2825 50  0001 C CNN
F 3 "https://www.tdk-electronics.tdk.com/inf/70/db/var/SIOV_Leaded_StandarD.pdf" H 2175 2825 50  0001 C CNN
F 4 "" H 2175 2825 50  0001 C CNN "Part"
F 5 "871-B72214S0271K551" H 2175 2825 50  0001 C CNN "PartNo"
F 6 "Mouser" H 2175 2825 50  0001 C CNN "Supplier"
	1    2175 2825
	1    0    0    -1  
$EndComp
$Comp
L smartevse:Varistor RV1
U 1 1 5E9A9653
P 2525 1825
F 0 "RV1" H 2575 1875 50  0000 L CNN
F 1 "Varistor" H 2628 1780 50  0001 L CNN
F 2 "Varistor:RV_Disc_D15.5mm_W5mm_P7.5mm" V 2455 1825 50  0001 C CNN
F 3 "https://www.tdk-electronics.tdk.com/inf/70/db/var/SIOV_Leaded_StandarD.pdf" H 2525 1825 50  0001 C CNN
F 4 "" H 2525 1825 50  0001 C CNN "Part"
F 5 "871-B72214S0271K551" H 2525 1825 50  0001 C CNN "PartNo"
F 6 "Mouser" H 2525 1825 50  0001 C CNN "Supplier"
	1    2525 1825
	1    0    0    -1  
$EndComp
Wire Wire Line
	975  2350 1475 2350
Wire Wire Line
	1475 1300 1475 2350
Wire Wire Line
	1900 1300 3550 1300
Wire Wire Line
	3550 2300 3550 1300
Connection ~ 3550 2300
Connection ~ 3550 1300
Wire Wire Line
	3550 1300 4000 1300
Wire Wire Line
	2175 2600 4000 2600
Wire Wire Line
	2175 1800 2175 2600
Wire Wire Line
	2175 1800 975  1800
Wire Wire Line
	975  1600 2525 1600
Wire Wire Line
	1825 2050 1825 3600
Wire Wire Line
	1825 3600 4000 3600
Wire Wire Line
	2175 2600 2175 2675
Connection ~ 2175 2600
Wire Wire Line
	1825 3600 1825 3675
Connection ~ 1825 3600
Wire Notes Line
	550  4050 3150 4050
Wire Notes Line
	550  550  550  4050
Wire Notes Line
	3150 550  3150 4050
Wire Wire Line
	2525 1675 2525 1600
Wire Wire Line
	2525 1600 4000 1600
Connection ~ 2525 1600
Wire Wire Line
	1275 5625 1825 5625
Connection ~ 2525 5625
Wire Wire Line
	2525 5625 3725 5625
Connection ~ 2175 5625
Wire Wire Line
	2175 5625 2525 5625
Connection ~ 1825 5625
Wire Wire Line
	1825 5625 2175 5625
Text Label 2750 1300 0    50   ~ 0
L_fused
Wire Wire Line
	4525 5025 4525 5425
Wire Wire Line
	6650 5025 6800 5025
Wire Wire Line
	6800 5025 6800 5225
Wire Wire Line
	6800 5625 6650 5625
Wire Wire Line
	6650 5425 6800 5425
Connection ~ 6800 5425
Wire Wire Line
	6800 5425 6800 5625
Wire Wire Line
	6650 5225 6800 5225
Connection ~ 6800 5225
Wire Wire Line
	6800 5225 6800 5425
Connection ~ 6800 5025
Wire Wire Line
	7300 5025 7300 5425
Wire Wire Line
	5450 6000 7300 6000
Wire Wire Line
	4525 6000 4525 5625
Connection ~ 5450 6000
Wire Wire Line
	4825 6125 4825 6000
Connection ~ 4825 6000
Wire Wire Line
	4825 6000 4525 6000
Wire Wire Line
	4825 5625 4825 6000
$Comp
L Device:Polyfuse_Small F?
U 1 1 6191EDA6
P 5450 4875
AR Path="/5E903BF9/6191EDA6" Ref="F?"  Part="1" 
AR Path="/5E98FA8A/6191EDA6" Ref="F1"  Part="1" 
F 0 "F1" H 5475 4925 50  0000 L CNN
F 1 "FSMD020 " H 5450 4850 50  0000 L CNN
F 2 "Fuse:Fuse_1812_4532Metric_Pad1.30x3.40mm_HandSolder" H 5500 4675 50  0001 L CNN
F 3 "https://www.mouser.de/datasheet/2/240/Littelfuse_PTC_1812L_Datasheet.pdf-693388.pdf" H 5450 4875 50  0001 C CNN
F 4 "" H 5450 4875 50  0001 C CNN "Part"
F 5 "576-1812L020PR " H 5450 4875 50  0001 C CNN "PartNo"
F 6 "Mouser" H 5450 4875 50  0001 C CNN "Supplier"
	1    5450 4875
	-1   0    0    1   
$EndComp
Wire Wire Line
	4825 5025 5450 5025
Wire Wire Line
	4825 6000 5450 6000
Wire Notes Line
	3650 4425 7675 4425
Wire Notes Line
	7675 4425 7675 6400
Wire Notes Line
	3650 4425 3650 6400
Wire Notes Line
	3650 6400 7675 6400
Wire Wire Line
	1825 3975 1825 5625
Wire Wire Line
	3550 3300 3550 5425
Connection ~ 3550 3300
Wire Wire Line
	2175 2975 2175 5625
Wire Wire Line
	2525 1975 2525 5625
Wire Wire Line
	1275 2150 1275 5625
Connection ~ 1275 2150
Text Label 1175 1950 0    50   ~ 0
N
Text Label 1175 2150 0    50   ~ 0
N
Text Label 1175 1700 0    50   ~ 0
N
Text Label 1175 1500 0    50   ~ 0
N
$Comp
L power:PWR_FLAG #FLG?
U 1 1 61BCC623
P 1500 7175
AR Path="/61BCC623" Ref="#FLG?"  Part="1" 
AR Path="/5E98FA8A/61BCC623" Ref="#FLG0103"  Part="1" 
F 0 "#FLG0103" H 1500 7250 50  0001 C CNN
F 1 "PWR_FLAG" H 1500 7348 50  0000 C CNN
F 2 "" H 1500 7175 50  0001 C CNN
F 3 "~" H 1500 7175 50  0001 C CNN
	1    1500 7175
	-1   0    0    1   
$EndComp
$Comp
L power:PWR_FLAG #FLG?
U 1 1 61BD6084
P 1000 7175
AR Path="/61BD6084" Ref="#FLG?"  Part="1" 
AR Path="/5E98FA8A/61BD6084" Ref="#FLG0105"  Part="1" 
F 0 "#FLG0105" H 1000 7250 50  0001 C CNN
F 1 "PWR_FLAG" H 1000 7348 50  0000 C CNN
F 2 "" H 1000 7175 50  0001 C CNN
F 3 "~" H 1000 7175 50  0001 C CNN
	1    1000 7175
	-1   0    0    1   
$EndComp
Wire Wire Line
	1000 6900 1000 7175
Wire Wire Line
	1500 6900 1500 7175
Text Label 1000 6900 2    50   ~ 0
L_fused
Text Label 1500 6900 2    50   ~ 0
N
Wire Wire Line
	1500 6900 1450 6900
Wire Wire Line
	1000 6900 950  6900
Wire Wire Line
	6800 5025 7300 5025
Connection ~ 7300 5025
Wire Wire Line
	7300 4700 7300 5025
Wire Wire Line
	5450 4700 5450 4775
$Comp
L agccs-ctrl22:12V_fused #PWR0112
U 1 1 60437F75
P 5450 4700
F 0 "#PWR0112" H 5450 4550 50  0001 C CNN
F 1 "12V_fused" H 5465 4873 50  0000 C CNN
F 2 "" H 5450 4700 50  0001 C CNN
F 3 "" H 5450 4700 50  0001 C CNN
	1    5450 4700
	1    0    0    -1  
$EndComp
Wire Wire Line
	5450 4975 5450 5025
$Comp
L agccs-ctrl22:12V_buffered #PWR?
U 1 1 60477EB6
P 7300 4700
F 0 "#PWR?" H 7300 4550 50  0001 C CNN
F 1 "12V_buffered" H 7315 4873 50  0000 C CNN
F 2 "" H 7300 4700 50  0001 C CNN
F 3 "" H 7300 4700 50  0001 C CNN
	1    7300 4700
	1    0    0    -1  
$EndComp
$EndSCHEMATC
