# Utilities

We provide a number of utilities to support monitoring and development of our network of charging stations. Most conveniently `dmctrl.py` is a Python script that can be run on the designated host and that implements simple command line interface to inspect the ESP32 mesh network and to control the attached AVRs. There is also a script `upload.sh` to automate the upload from firmware to the host, including our file name conventions. The utilities are meant to facilitate first-installation and also the developlement process.  Those aspects of the functionality that are relevant for daily operation will be made available in a much more user friendly interface

 

## dmctrl.py

This Python script is meant for command line invocation on a per-task fashion. It is implemented rather pragmatically by listening to the TCP port 8070 and waiting for the root node of the mesh network to connect. It then forwards the specified message to the root node and awaits for a reply for the subsequent 10 second. Example:

```
./dmctrl '{"dst":"*","cmd":"status"}'
```

listens TCP port 8070 and sends the string `{"dst":"*","cmd":"status"}` to the next client to connect (observe the two shell escape characters `'` which are not part of the message). The client will be the role node of the mesh netword and, assuming that it runs the [demesh.c](../demesh/) firmware, will decode the JSON formatted message string. It hence notes the mesh broadcast address `"*"` and forwards the message to all nodes in the mesh. The individual nodes in turn decode the message, identify the command `"status"` and reply to the root by a JSON encoded status report regarding their connection to the mesh. Available commands are further explained in the [demesh.c documentation](../demesh/NodeControl.md). The root note in turn forwards the reply via the TCP connection to the host running `dmctrl.py`. Example incl. reply:

```
pi@lrt101:~ $ ./dmctrl.py '{"dst":"*","cmd":"status"}'
dmctrl: command {"dst":"*","cmd":"status"}
demesh: starting demesh server on 0.0.0.0:8070
192.168.5.185 connected, sending command
await for reply from 192.168.5.185
{"src":"d8:a0:1d:55:a7:10","mtype":"status","parent":"d8:a0:1d:55:37:cd","rssi":-40,"layer":2,"nodes":3,"plat":57}
{"src":"d8:a0:1d:54:e4:a4","mtype":"status","parent":"d8:a0:1d:55:37:cd","rssi":-52,"layer":2,"nodes":3,"plat":45}
{"src":"d8:a0:1d:55:37:cc","mtype":"status","parent":"dc:a6:32:6b:28:60","rssi":-26,"layer":1,"nodes":3,"plat":0}
shutting down demesh tcp server
```

This mesh consist of three nodes with d8:a0:1d:55:37:cc the root (i.e. layer 1) with two children d8:a0:1d:55:a7:10 and  d8:a0:1d:54:e4:a4 (i.e. layer 2). 

The actual protocol (JSON encoding, available commands, expected reply) is implemented by the ESP32 firmware [demesh](../demesh/) and, at this stage, transparent to `dmctrl.py`. However, we `dmctrl.py` provides a number of convenience add-ons as listed below.

```
pi@lrt101:~ $ ./dmctrl.py -?
usage:
 dmctrl                               // broadcast status request (see exmaple above)
 dmctrl <CMD>                         // broadcast <CMD>, i.e., send {"dst":"*","cmd":"<CMD>"}
 dmctrl <JSON>                        // specify verbatim JSON message to be sent
 dmctrl upgrade <VER> <BRD>           // disribute ESP firmware version/board as specified
 dmctrl avrflash <FILE> <NODE>         // flash avr image for target uC
 dmctrl avrgetpar <PAR> <NODE>         // get parameter <PAR> in target uC
 dmctrl avrsetpar <PAR> <VALUE> <NODE> // set parameter <PAR> in client uC 
 dmctrl monitor                        // monitor heartbeat of target uCs 
```

**Examples**

- to upgrade the ESP firmware on all nodes with an M5Stick board to version 3.5, issue
  ```
  ./dmctrl.py upgrade v3.5 m5stick
  ```
  this composes an appropriate JSON message, sends it to the root node and runs an HTTP server on TCP port 8071; the root will download the file `demesh_m5stick_3_5.bin` from the HTTP server, distribute it to those nodes that run on a  appropriate board with a version different to v3.5; this will take some time, check back with `dmctrl.py system`; in the case some nodes missed out, repeat the procedure.


- to upgrade the firmware AVR attached to the ESP32 node d8:a0:1d:55:a7:10 to version 1.2, run
  ```
  ./dmctrl.py avrflash ctrl22c_1_2.bin d8:a0:1d:55:a7:10
  ```
this will compose a number of adequate JSON encoded messages to be forwarded to node `d8:a0:1d:55:a7:10` and a final message to ask the ES32 to flash the AVR via the Optiboot protocol; you may check back by inquiring the value of the AVR parameter `"ver".`

- to get a parameter `"ver"` from (or to set the parameter `"blinks"` to `"5"` for) the target AVR on node d8:a0:1d:55:a7:10, run

  ```
  ./dmctrl.py avrgetpar ver
  ```
  or
  ```
  ./dmctrl.py avrsetpar blinks 5
  ```
respectively; this will compose an appropriate JSON encoded message to ask the ESP32 to set/get the respective parameter to/from the AVR attached via the serial line; available parameters depend on the AVR firmware, for `Ctrl22C` the serial line protocol is further explained [here](../ctrl22c/README.md#Serial-Line-Protocol).

 











