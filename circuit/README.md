Circuit
=======

The schematics used in this project have been adopted from the
[SmatEVSE project](https://github.com/SmartEVSE) with a number of modifications to

- have an AVR uC from the ATMega series to be attart developers from the Arduino community
- have an ESP32 for networked wireless communivations
- enable/disable any of the three supply phase seperately
- integrate power monitoring
- layout with handsoldering in mind

The original adaption to our needs was developed and tested in course of a BA thesis and has been
published in [this project](https://github.com/dreadnomad/FGCCS-Ctrl22). The present repository
is strip down to the essentials, so you may want to inspect the original sources; in
particular
[the BA thesis](https://github.com/dreadnomad/FGCCS-Ctrl22/blob/master/doc/Bachelorarbeit_Pascal_Thurnherr.pdf)
includes a convenient summary of the electrical specications given in IEC 62196, as relevant for this
project.


# Installing Firmware

Once set, updates can be conveniently deployed wirelessly via the utility
`dmctrl.py`; see [../utils](../utils/). However, to get bootstrapped, we need to install an
initial version of firmware for both th AVR uC and the ESP32 SoC. For this, our
board is equipped with a custom 8-pin header J5

| Fnct.    | Pin | Pin | Fnct.     |
|:--------:|:---:|:---:|:---------:|
| AVR-UPDI |  1  |  5  | GND       |
| ESP-TX0  |  2  |  6  | 3.3V      |
| ESP-RX0  |  3  |  7  | ESP-IO15  |
| ESP-IO0  |  4  |  8  | ESP-EN    |



### AVR Firmware

The ATMega4808 used in our project is a modern incarnation of the ATMega series,
and in many aspects more closely related to the XMega series. In particular, it is
natively programmed via the so called  UPDI one-pin interface. The professional way
to programm via UPDI is to use the recent Atmel ICE ($100+). This  works fine with 
Atmel AVR Studio but is (at the time of writing) not well supported by 
mainstream `avrdude`. Unexpectedly good, and really low price is the
[`pyudpi` project](https://github.com/mraardvark/pyupdi),
which only needs an of the shelf USB-serial converter and a single 4.7K resistor. Note that
our circuit is 3.3V and such must be the USB-serial converter.
`pyupdi` will program fuses, applications, bootloaders --- that's all we shall need.

Short instructions:
- set up your wiring
  ```
  USB-Serial-TX>---[4.7K]--->+<>AVR-UPDI (aka connector J5 pin 1)
                             |
  USB-Serial-RX>-------------+
  USB-Serisl-GND<>------------<>GND  (aka connector J5 pin 5)
  ``` 
- get `pyudpi` from Github
- install python add-ons `intelhex` and `serial`, e.g.
  ```
  sudo python3 -m pip install intelhex
  sudo python3 -m pip install serial
  ```
- test
  ```
  # to get some device specs and check the connection
  pyupdi.py -d mega4808 -c /dev/{SOME_USB_SERIAL_DEV} -b 115400 -i
  # to program flash memory,e.g. the Optiboot bootloader
  pyupdi.py -d mega4808 -c /dev/{SOME_USB_SERIAL_DEV} -b 115400 -f {OPTIBOOT_FOR_ATM4808}.hex
  ```

_Missing Topic:_ how we set the fuses for our project  


### ESP32 Firmware

The ESP32 SoC has a built in two-level bootloader an is programmed via the serial
interface TX0/RX0. To enter ootloader mode, IO0 must be set low wt the time when
EN becomes high (EN acts as inverted RESET, and IO0 selects the bootmode).

Short instructions:
- set up your ESP32-MDF SDK and compile the firmware `demesh`; see [../demesh](../demesh/) for instructions
- set up your wiring
  ```
  USB-Serial-TX>------------->ESP-RX0  (aka connector J5 pin 2)
  USB-Serial-RX<-------------<ESP-TX0  (aka connector J5 pin 3)
  USB-Serisl-GND<>---+------<>GND      (aka connector J5 pin 5)
                     |
                     +------<>ESP-IO0  (aka connector J5 pin 4)
                     |
                     +-[/]--->ESP-EN   (aka connector J5 pin 8)
  ```		     
- to flash the `demesh` firmware
  -- set ESP-EN to low
  -- set ESP-EN to not-connected
  -- run `make flash`
  This is effectively the same procedure as with common ESP32 dev.boards such as NodeMCU
  
  ``` 


# Developing/Testing the AVR firmware


We provide AVR firmware for our needs in [../ctrl22](../ctrl22/). It controls the charging process
and reports to AVR-TX0/RX0 (available on the 4-pin header J4).
If you plan to develop/test specific AVR firmware for your variant project, you can configure `demesh` to
provide a debug server for AVR firmware development.
Rather than to set up a wireless mesh network, the ESP32 will then
act as an accesspoint and provide a transparent telnet passthrough of the AVR serial port and/or
to the Optibiit boorloader. Thus, you can develop/test the AVR firmware largely independantly from
the somewhat involved wifi mesh but still have the convenience of "no wires from the charging station 
into my computer".

After connecting with the access point, the debug server can be accessed via

```
gtelnet 192.168.4.1
```

The IP address, the SSID and the WPA2 password are configured within `demesh`; see (../demesh)[../demesh/])
To inspect available commands for the provided AVR firmware, type `? [CR]`. To exit the session, 
type `Ctrl-Q` or `Ctrl-D`.

To access the AVR bootloader, the alternative port 2323 is utilised.
On connections, it resets the AVR to enter Optiboot an can thus be programmed
via `avrdude' in Arduino-Style. Example 

```
avrdude  -patmega4808  -carduino -Pnet:192.168.4.1:2323 -U flash:w:{SOME_HEX_FILE}.hex
```




