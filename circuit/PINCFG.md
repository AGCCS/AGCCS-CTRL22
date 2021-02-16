# ATmega4808 Pin Configuration

## Required Functionality

* Power Supply & GND                -> 4 Pins
* External Reset                    -> 1 Pin
* External Programming (UPDI)       -> 1 Pin
* 2x ADC Input (CP, PP)             -> 2 Pins
* 3x ADC Input (Load Balancing)     -> 3 Pins
* 3x UART TX & RX                   -> 6 Pins
* PWM Output                        -> 1 Pin
* Digital I/O
    - 3x SSR Toggle                 -> 3 Pins
    - Reed Relay Switch             -> 1 Pin 
    - 2x Lock Drive                 -> 2 Pins
    - LED, Button, Lock State       -> 3 Pins
    - RS485 Direction Toggle        -> 1 Pin
* Total: 28 Pins

## Pin Config

|No.    |Pin    |Function   |Direction  |Used For       |
|:-----:|:-----:|:---------:|:---------:|:-------------:|
|1      |PA7    |Digital I/O|Output     |Lock Driver A  |
|2      |PC0    |TX1        |Output     |RS485 Transmit |
|3      |PC1    |RX1        |Input      |RS485 Receive  |
|4      |PC2    |Digital I/O|Output     |Lock Driver B  |
|5      |PC3    |XDIR1      |Output     |RS485 Direction|
|6      |PD0    |ADC Input  |Input      |PP Signal Input|
|7      |PD1    |ADC Input  |Input      |CP Signal Input|
|8      |PD2    |Digital I/O|Input      |Lock State     |
|9      |PD3    |Digital I/O|Output     |LED Switch     |
|10     |PD4    |Digital I/O|Input      |Pushbutton     |
|11     |PD5    |ADC Input  |Input      |CT0            |
|12     |PD6    |ADC Input  |Input      |CT1            |
|13     |PD7    |ADC Input  |Input      |CT2            |
|14     |AVDD   |Supply     |Input      |VDD            |
|15; 21 |GND    |Supply     |Input      |GND            |
|16     |PF0    |TX2        |Output     |Serial Transmit|
|17     |PF1    |RX2        |Input      |Serial Receive |
|18     |PF6    |Ext Reset  |Input      |Reset          |
|19     |UPDI   |Debug/Progr|I/O        |UPDI Interface |
|20     |VDD    |Supply     |Input      |VDD            |
|22     |PA0    |TX0        |Output     |Serial Transmit|
|23     |PA1    |RX0        |Input      |Serial Receive |
|24     |PA2    |PWM        |Output     |PWM to CP      |
|25     |PA3    |Digital I/O|Output     |Reed Relay     |
|26     |PA4    |Digital I/O|Output     |SSR Switch L1  |
|27     |PA5    |Digital I/O|Output     |SSR Switch L2  |
|28     |PA6    |Digital I/O|Output     |SSR Switch L3  |
