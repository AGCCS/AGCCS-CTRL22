# CTRL22ONE

Web-GIU fo the control of one single AGCCS-CTRL22 board in standalone configuration, i.e., no wifi-mesh, no load balancing. The motivation here is to have a convenient basis to fine-tune the AVR firmware [ctrl22.c](../../ctrl22/) to cope with a variety of different EVs with different sleep/wake-up behaviour and such. We do log to a configurable MQTT broker and accept dynamic power allocation. This could be interesting for charging at home from solar power when available.

**DISCLAIMER.** Although in generally in a good shape, this is still work in progress.



## Key Features 

- simple Web GUI to start/stop charging, limit the maximal available power and to monitor the actual current drawn;

- maintain WLAN credentials is EEPROM; alternatively, provide an WLAN access-point and ask the operator to provide valid credentials via the Web GUI

- everything built-in, no RasPi required for basic operation;

- convenient over-the-air (OTA) upgrade via the Arduino IDE (at this stage the ESP32 only, not the attached AVR)

- optional forward status reports to an MQTT broker (configurable via the Web GIU)

  

## Implementation Outline

The ESP32 firmware ``CTRL22ONE`` implements a simple HTTP server to handle GET requests. It serves "static text files" which are encoded as strings in PROGMEM. There is one main page ``index.html`` which provides a reactive GUI based on [jQuery](https://jquery.com/)/[Bootstrap](https://getbootstrap.com/). Here, _reactive_ reads that individual GUI elements adapt their look and feel as expected and without further coding on our side; i.e., accoridion stacks magically collapse and expand on user user interaction. Also the layout mechanism adapts to the screen size and the GUI should be usable both on desktops and on mobile devices. However, for dynamic content elements such as sliders and gauges we do have to code a little. The JavaScript embedded in  ``index.html`` connects to a websocket on the ESP32 and forwards any changes of control and or configuration parameters to that socket. The ESP32 in turn is responsible to forward appropriate control to the target AVR and/or to adapt its configuration. 

<img src="../../images/httpjscss.png" style="zoom:50%;" />

## Build Process 

Load the sketch in the Arduino IDE, compile, download, done. Some considerations:

- install the below third party components; use the board manager for the ESP32 Core and the library manager for the remaining libraries; they all come with GitHub documentation incl. installation instructions ; be aware that there typically exist multiple libraries for the same topic but with different API and different features --- the name matters.

- you can conduct first tests with an M5StickC or an ESP32 dev board, but finally you will need a *J5-Programming-Adaptor* to program the AGCCS rev 1.2 board (see [circuit](../../circuit)); regarding the Arduino IDE, the AGCCS board can be configured as "ESP32 dev board";

- before flashing via the Arduino IDE be sure to choose "large APP, minimal SPIFS, with OTA" as partition table (_Tools_- menu); on a common 4GByte ESP32 this gives un about 1.9GByte for our application and we should be fine with that; 

- the sketch is rather verbose on the serial line, so the serial-monitor can be utilised to locate issues if any;

- for simplicity, we opted to embed the files served via HTTP as PROGEM strings; the respective include files are located in the directory  `./webinc/*`  and they are built from the human editable sources in `./websrc/`*; hence, if you plan for changes on this end, you will need to re-generate the respective include files; for OSX/Linux, we provide the shell script `mkheaders`  (let us know about more comfortable but yet lightweight solutions; also  similar script for Windows would be appreciated).

  



## Third Party Components 

Although we prefer for good reasons to code directly on top of the ESP-MDF/IDF SDKs, we must admit that within the Arduino environment a functional prototype can be obtained with far less effort. One aspect here is that the ESP32 SoC is considered sufficiently powerful to trade in some resources in favour for a modern C++ coding style (think of string manipulation and/or JSON parsing) However, the major share of the experienced convenience is owed to the readily available libraries with their intuitive high-level APIs.  Specifically

- [ESP32Core](https://github.com/espressif/arduino-esp32) 1.0.6 (lots of contributors, Espressif)
- [WiFiWebServer](https://github.com/khoih-prog/WiFiWebServer) 1.1.1 (Khoi Hoang)
- [MQTT](https://github.com/256dpi/arduino-mqtt) 2.5.0 (Joël Gähwiler)
- [ArduinoOTA](https://github.com/jandrassy/ArduinoOTA) 1.0.6 (Juray Andrassy, Arduino)
- [ArduinoJson](https://arduinojson.org/) 6.17.3 (Benoit Banchon) 

Likewise, on the HTML/CSS/JavaSkript side we did not need to start from scratch. It can be be really cumbersome to try to learn the technical details on how hundreds of modern CSS properties and HTML elements interact. None of this should be of our concern: Boostrap & friends wrap it all up with a nice top-level interface, no need to worry --- thank you. 

- [jQuery](https://jquery.com/) 3.6.0 (OpenJS Foundation) 
- [Bootstrap](https://getbootstrap.com/) 5.0.0-beta3 (Bootstrap Team)
- [Bootstrap-Slider](https://github.com/seiyria/bootstrap-slider) 11.0.2 (lots of contributors, maintained by Kyle Kemp and Rohit Kalkur)