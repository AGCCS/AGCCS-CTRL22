# CTRL22ONE

Web-GIU fo the control of one AGCCS-CTRL22 board in standalone configuration, i.e., no wifi-mesh, no load balancing. The motivation here is to have a convenient basis to fine-tune the AVR firmware [ctrl22.c](../../ctrl22/) to cope with a variety of different EVs with different sleep/wake-up behaviour and such. We do log to a configurable MQTT broker and accept dynamic power limitation. This could be interesting for charging at home from solar power when available.

**DISCLAIMER.** Although in generally in a good shape, this is still work in progress.



## Key Features 

- simple Web GUI to start/stop charging, limit the maximal available power and to monitor the actual current drawn;

- maintain WLAN credentials is EEPROM; alternatively, provide an WLAN access-point and ask the operator to provide valid credentials via the Web GUI

- everything built-in, no RasPi required for basic operation;

- convenient over-the-air (OTA) upgrade via the Arduino IDE (at this stage the ESP32 only, not the attached AVR)

- optional forward status reports to an MQTT broker (configurable via the Web GIU)

  ![](

  )

## Implementation Outline

The ESP32 firmware ``CTRL22ONE`` implements a simple HTTP server to handle static GET requests. It serves "text files" which are encoded as strings in PROGMEM. There is one main page ``index.html`` which provides a reactive GUI based on [jQuery](https://jquery.com/)/[Bootstrap](https://getbootstrap.com/). Here, _reactive_ reads that individual GUI elements adapt their look and feel as expected and without further coding on our side; i.e., accordion stacks collapse and expand on user user interaction. Also the layout mechanism adapts to the screen size and the GUI should be usable both in desktop and on mobile devices. However for dynamic content elements we do have to code. The JavaScript embedded in  ``index.html`` connects to a websocket on the ESP32 and forwards any changes of control and.or configuration parameters to that socket. The ESP32 in turn is responsible to forward appropriate control to the target AVR and/or to adapt its configuration. 

![](/Users/tmoor/current/code/agccs-ctrl22/images/httpjscss.png)

## Build Process 

Load the sketch in the Arduino IDE, compile, download, done. Some considerations:

- install the below third party components; use the board manager for ESP32 Core and the library manager for the remaining libraries; they all come with GitHub documentation incl. installation; be aware that there typically exist multiple libraries with similar functionality but different API --- the name matters.

- you can conduct first tests with an M5StickC or an ESP32 dev board, but finally you will need a *J5-Programming-Adaptor* to program the AGCCS rev 1.2 board (see [circuit](../../circuit)); regarding the Arduino IDE, the AGCCS board can be configured as "ESP32 dev board";

- before flashing via the Arduino IDE be sure to choose "large APP, minimal SPIFS, with OTA" as partition table (_Tools_- menu)

- the sketch is rather verbose on the serial line, so the serial-monitor can be utilised to locate issues if any

- for simplicity, we opted to embed the files served via HTTP as PROGEM strings; the respective include files are located in the directory  `./webinc/*`  and they are built from the sources in `./websrc/`*; hence, if you plan for changes on this end, you will need to re-generate the respective include files; for OSX/Linux, we provide the shell script `mkheaders`  (let us know about more comfortable but yet lightweight solutions).

  



## Third Party Components 

Although we believe for reasons that to code directly on top of the ESP-MDF/IDF SDKs, we must admit the a functional prototype was obtained with the Arduino environment with very little effort. One aspect here is that the ESP32 SoC is considered sufficiently powerful to trade in some resources in favour for a modern C++ coding style. However, a good share of the experienced convenience is owed to the readily available libraries with their high-level intuitive APIs.  Specifically

- ESP32Core 1.0.6 (lots of contributors, Espressif)
- WiFiWebServer 1.1.1 (Khoi Hoang)
- MQTT 2.5.0 (Joël Gähwiler)
- ArduinoOTA 1.0.6 (Juray Andrassy, Arduino)
- ArduinoJson 6.17.3 (Benoit Banchon) 

Likewise, on the HTML/CSS/JavaSkript side we did not need to star from scratch. To understand the technical details of how hundreds of modern CSS properties interact can be really cumbersome. None of this was our concern --- Boostrap & friends wrap it all up with a nice top-level interface, no need to worry. 

- jQuery 3.6.0
- Bootstrap 5.0.0-beta3
- Bootstrap-Slider 11.0.2