AGCCS-CTRL22
============

The Combined Charging System [CCS] provides the today's standard for low-cost
home-charging of electric vehicles with up to 22kW. The specification includes a quite simplistic
protocol by which the vehicle and the charging station negotiate available power. Of-the-shelf
wallboxes forward the common 3x230V supply at an adjustable current limit to
the vehicle via the CCS standard.

The aim of this project is to scale the simplicity of the low-cost single-user wallbox to
larger parking lots, commonly experienced with appartment buildings or shopping venues.
These configurations are characterised by a limited and possibly varying amount of available power.
Hence, a fixed allocation per parking slot would be inefficient. Instead, we seek for an environment
by which individual charging spots communicate and negotiate a power distribution dynamically by some
more appropriate scheme, e.g. first-come-first-serve, fairness-based energy distribution, or
ready-to-go by schedule. Our project consists of three main components

- a hardware platform to implement relevant sections of the CCS standard
- a per node software environment to extablish a communication network
- a centralised server for power allocation and monitoring purposes

All three components are open source and can be hence adapted to best fit a
variety of application scenarios.


**DISCLAIMER-1** We do not provide a turn-key solution and do not plan to do so in near future.
If you are looking for such, please consider the [SmatEVSE project](https://github.com/SmartEVSE).
Rather we invite the enthusiast to actively contribute or to passively make use of our
collection of resources and to share their experience.


**DISCLAIMER-2** All material provided in this repository comes 'as is' with no explicit or implied
waranty. In particular, the development of equipment that runs on mains power should only be considered
be individuals with relevant skills and with particular care.



# Hardware Platform

Our hardware platform is based on (1) relevant analog circuits to implement the specified electric
characteristic (2) an AVR series uC to operate the charging process and (3) an ESP32 SoC to provide
means of wireless communication. Regarding (1), our implementation is based on the SmatEVSE project; see
also above. Regarding (2) we opted for AVR as this implies the option to conveniently program within the
[Arduino environment](https://www.arduino.cc) --- although, at this stage the firmware we provide comes
'bare bone' for performance reasons. Regarding (3) we are fascinated by the powerful ESP32 SDKs
available from [Espressif](https://github.com/espressif). Schematics and a PCB Layout in editable
[KiKad](https://kicad.org) format are provided in the [folder ./circuit](./circuit/)


# Per Node Software

As indicated above, each charging spot comes with an AVR for time/safety critical low-level
behaviour which communicates via RS232 with an ESP32 for inter-node networking. While the AVR
is programed from scratch, the ESP32 firmware builds on the MDF SDK for wireless mesh-networking.
Thus, we expect a comperatively large area of coverage without additional inrastructure
like e.g. Wifi repeaters. The so called root node subscribes to and published from a
MQTT broker and is this interoperable with a wide range of possible server software.
Both, the AVR firmware and the ESP firmware support OTA updates, i.e., no manual crawling
along the parking lot. See also [./ctrl22](./ctrl22/) for the AVR firmware and [./demesh](./demsh/)
for the ESP32 firmware.


# Server Software

At the time of writing, we only provide an elementary monitoring server implemented in Python
for development/testing purposes; see [./utils](./utils/]. This will be dramatically updated in very neer future
--- stay tuned.



# How to Get Started

Well, no turn-key solution ... depending on interest an skills you may want to focus on hardware
and assemble your very own CCS implementation. It can operate independently to control a single
wallbox mounted in your garage and/or await for firmware updates to come. More the software person?
You can play along with our software infrastructure with low-cost M5Sticks to set up the mesh network.
Additional features are only limited by your imagination. However, for actual charging of electric
vehicles you will need to mate up with the hardware fraction. At the end its all open source ---
pick whatever you need for your completely different project --- perhaps drop an acknowledgement/link
when going public.



