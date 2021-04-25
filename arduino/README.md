# Arduino Firmware ESP32/AVR

In parallel to our main intent to organise a network of charging station, we plan to provide a simple solution for stand-alone configurations, utilising the widely appreciated Arduino programming environment. This repository is organised as follows:  

- [Ctrl22One](./ctrl22one) is an Arduino sketch to run the ESP32 as a remote control for a single charging station
- [Ctrl22Ino](./ctrl22ino/) will become an Arduino sketch for drop-in replacement of our Ctrl22C firmware 

**DISCLAIMER**. Obviously, this is a side-track from our original project and we run this at low priority.





## Firmware based on the ESP32 Arduino Core

Easy to use remote control and monitoring Web GUI, all implemented stand-alone by the ESP32. No external server required, no internet gateway either, i.e, this also works if you decided that you home automation wireless network shall not leak into the internet. Optionally, you can configure the ESP32 to forward monitoring to an external MQTT broker. Details are provided [here](./ctrl22one/)), this is how it looks:

![ctrl22one](../images/ctrl22one-a.png)

 



## Firmware based on the Arduino XMegaCoreX 

Nothing ready to share at this point of time --- not sure, how to go with this, in particular the timing of the RMS current measurement does not translate too easily --- for the time being, we will stick our the bare-bone firmware [Ctrl22C](../ctrl22c/).



