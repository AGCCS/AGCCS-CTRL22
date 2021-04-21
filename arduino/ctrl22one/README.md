# CTRL22ONE

Web-GIU fo the control of one AGCCS-CTRL22 board in standalone configuration. Hence, no wifi-mesh, no load balancing. The main idea is to have a convenient basis to fine-tune the AVR firmware [ctrl22.c](../../ciyrl22/) to cope with variety of different EVs with different sleep/wake-up behaviour and such. We do log to a configurable MQTT broker and accept dynamic power limitation. This could be interesting for charging at home from solar power.  



[ work in progress --- scratch pad ]



Libraries used for Arduino sketch (all dated 2021/04):

- ESP32Core 1.0.6 (lots of contributors, Espressif)
- WiFiWebServer 1.1.1 (Khoi Hoang)
- MQTT 2.5.0 (Joël Gähwiler)
- ArduinoOTA 1.0.6 (Juray Andrassy, Arduino)
- ArdiunoJson 6.17.3 (Benoit Banchon) 



Libraries used on browser-side  i.e. JavaScript (all dated 2021/04)

- jQuery 3.6.0

- Bootstrap 5.0.0-beta3
- Bootstrap-Slider 11.0.2