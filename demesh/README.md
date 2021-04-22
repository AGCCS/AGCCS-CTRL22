# Demesh --- ESP Firmware

An ESP wireless mesh network (MWifi) is organised as a tree of nodes which pass on messages to their direct parent or children. This is a completely different story than common Wifi, where each station communicates directly with the access point. In particular, there are neither TCP sockets,  UDP telegrams or IP addresses in a MWifi. Benefits include a larger coverage without dedicated repeaters and a lower power consumption. The tree will dynamically reconfigure itself depending on channel quality. This all sounds very involved and indeed is so. However, Espressif provides a ready-to-go SDK and all the magic happens under the hood; see [ESP-MDF on GitHub](https://github.com/espressif/esp-mdf), including a large number of getting-started examples, and/or the [reference documentation](https://docs.espressif.com/projects/esp-mdf/en/latest/?badge=latest).



## An MWifi to Control Attached Target uCs



For our specific setup, we want to address the following network architecture 



<img src="../images/meshtop.png" alt="Mesh Topology" style="zoom:75%;" />

Each node consist of an ESP32 SoC with an attached ACR uC, also referred to as the *target uC*. In the above sketch, the red arrows indicate communication via the MWifi. Each node is addressed by a unique part ID of the ESP32 which otherwise acts as the MAC address. Via this address each node can send a message to any other node, the MWifi will take care about the routing. One node allthough is special, namely the *root node*, given in grey colour. On the one hand it acts like all other nodes in that it can send messages via MWifi. But it also operates in station mode as part of an ordinary TCP/IP Wifi. In our application it utilises this secondary role to built up a TCP/IP connection to a predefined server port on the host and (a) takes commands from the host to be forwarded to specific nodes and (b) forwards messages from any node to the host. With this infrastructure the ESP32 firmware `demesh.c` implements the following basic functionality

- each ESP32 gathers on a regulat basis relevant aspects of the state of the respcetive target uC and composes a _heartbeat_ _message_ to be forwarded to the root node;
- the root note forwards heartbeat messages to the host;
- the host gathers the overall status of the network and, on a regular basis, issues appropriate commands to each individual node (e.g. power allocation for an attached EV charging station); in turn, the node forwards the command to the target uC.

Effectively we tunnel the serial line of the target uC to the host. To be of practical use we also provide some convenience features

- we synchronize systemtime such that our charging stations can flash their operator buttons nicely in sync;

- as an alternativ to plain TCP/IP sockets, the root not also publishes the hearbeat messages to an MQTT broker and subscribes to control messages;

- we allow to update the ESP32 firmware over the air (OTA);

- we allow to update the target uC firmware OTA; for this, the host first forwards the desired firmware image to the ESP32 which in turn runs an implementation of the Optiboot (subset of STK500) programmer emulation.

  



## Implementation Overview

The ESP32 is quite a powerfull SoC, providing two cores and 380kB RAM. The ESP-MDF SDK is built on top of the FreeRTOS operating system and we thus have taks, timers and socket IO. Indeed, programming the ESP32 feels much more like programming a POSIX compliment "System" than just a "Chip". We give a run through of the main building blocks of the provided firmware with a focus in message forwarding

**Upstream Link -- Receiving Messages from the Host.** The only node that can directly receive messages from the host is the root node. It connetcs via a TCP socket to the designated host, which is configurabe at compile time via `make menuconfig`.  The root node runs the task `upstream_read_task()` to read from this socket and expects *commands* aka *requests*. These are JSON encoded records of key-value pairs and must include a `"dst"="^ADDR^"` entry. The message is then forwarded by the mesh network to the node with the specified mesh address ^ADDR^. We implement two special purpose addresses, namely `"dst="*"` for a broadcast to all nodes in the mesh and `"dst="root"` for the root node in its role as an ordinary node.

**Upstream Link -- Sending Messages to the Host.** The only node that can directly send mesages to the host is again the root node. It does so via the same TCP socket on which it receives messages from the host, see above. On the root node runs the task  `root_read_task()` to receive messages from any other node and to do so in its specific role as root. The root node will foreward any message received in this role to the host via the TCP socket. Thus, any node can talk to the host by sending a mesh-network message explicitly to the root node. This is completely transparent, the root will not take any further actions.   

**Mesh-Network Messages.** Every node runs the task `node_read_task()` to receive messages. This includes the root note, however, in its secondary role as an ordinary node. Typically the messages originate from the host and have been propageted through the mesh. Such messages are referred to as _commands_ or _requests_  and are meant to control the individual nodes. Technically, commands are JSON encoded key-value pairs. The key `"cmd"` specifies the action to be taken and this impicitly refines the effective data type of the remainder of the message; i.e., which further keys must be present and how they affect the action to be taken. In turn, the node replys to any command with a JSON encoded acknowledgement, i.e., it sends a message to the root note to be forwarded to the host. Any acknowledgement must contain the reserved keys  `src` and `mtype` to specify the sending node and the type of the message. Although the mesh network does not provide TCP-like sockets, the host utilises the  `src` and `mtype` entiries to untangle any incomming message. Relevant commands are documented in more detail in [NodeControl.md](./NodeControl.md).

To actually send a command to a node, the host needs to listen on the designated TCP port and on connection write an appropriately encoded JSON message to the respective socket. This can be tested with general purpose tools like `netcat` aka `nc`; e.g., run `nc -l 8070` on the host to listen on the default port 8070 and on connection type `{"dst":"*","cmd"="status"}` to broadcast a status request -- and await the reply. For convenience, we provide the Python script [dmcrl.py](../utilities/)  which facilitates this process; e.g., run `./dmctrl.py status` to broadcast the same status request. 



## Node Control via MQTT

In addition to the elementary connection via a plaIn TCP socket, the root subscribes to a designated MQTT broker to receive commands and it published the acknowledgements to the same broker. The address of the broker defaults to the access-point IP at port 1884; this is configured at compile time via `make menuconfig`, see also below. In our specific set-up, we start the MQTT broker *mosquitto* on the RasPi as follows: 

```
pi@lrt101:~/ $ mosquitto -v -p 1884
1615160674: Opening ipv4 listen socket on port 1884.
1615160683: New client connected from 192.168.5.135 as ESP32_55A56C (c1, k120).
1615160683: Received SUBSCRIBE from ESP32_55A56C
1615160683: 	/DEMESH/+/control (QoS 0)
1615160686: Received PUBLISH from ESP32_55A56C '/DEMESH/d8a01d55a56c/heartbeat'
[...]
```

Here, we first observe the root node subscribing to the topic `/DEMESH/+/control`. The root node will forward any control message to the mesh network in the same way it does with messages received via the plain TCP socket. We will also observe  periodic heartbeat publications at  `/DEMESH/+/heartbeat`  from all nodes. Once this is functional on the broker side, we may run it in daemon mode, e.g.

```
pi@lrt101:~/ $ mosquitto -d -p 1884
```

Any message published to `/DEMESH/^ADDR^/control` will be forwarded as a node message to the node specified by `^ADDR^`. The address format for this purpose is "MAC address without the colons", e.g., `d8a01d54e4a4` for node  _d8:a0:1d:54:e4:a4_.  As with the TCP uplink, two special addresses are implemented:  use address `*` to initiate a broadcast over the entire mesh network, or `root` to send a control message to the root node. In response to any command the node will publish to `/DEMESH/^ADDR^/acknowledge`. The protocol (i.e. accepted messages and the format of the expected reply) is documented in [NodeControl.md](./NodeControl.md).



**Example.** To subscribe to any acknowledgement and/or heartbeat messages, run

```
$ mosquitto_sub -h lrt101 -p 1884 -t /DEMESH/+/heartbeat -t /DEMESH/+/acknowledge
```

on any machine in the local network. Here, `lrt101` is the DNS name of the host which runs the broker, i.e., the RasPi. Expect heartbeat messages at a default period of 5secs from each node, e.g,

```
{"dev":"d8a01d55a56c","mtype":"heartbeat","rssi":-72,"ccss":0,"smaxcur":0,"cmaxcur":100,"phases":123,"cur1":0,"cur2":0,"cur3":0}
```



**Example.** To broadcast a status request, run

```
$ mosquitto_pub -h lrt101 -p 1884 -t /DEMESH/*/control -m '{"cmd":"status"}'
```

If you are still subscribed to `/DEMESH/+/acknowledge`, you will be forwarded the replies, e.g.

```
{"dev":"d8a01d55a56c","mtype":"status","parent":"dc:a6:32:6b:28:60","rssi":-69,"layer":1,"nodes":1,"plat":0}
```



**Example**. For a charging cycle at a max of 32A on all three phases simulated on an M5Stick at address d8a01d55a56c, send the following control messages

```
$ mosquitto_pub -h lrt101 -p 1884 -t /DEMESH/d8a01d55a56c/control -m '{"cmd":"avrsetpar","avrpar":"smaxcur","avrval":320}'
$ mosquitto_pub -h lrt101 -p 1884 -t /DEMESH/d8a01d55a56c/control -m '{"cmd":"avrsetpar","avrpar":"phases","avrval":123}'
$ mosquitto_pub -h lrt101 -p 1884 -t /DEMESH/d8a01d55a56c/control -m '{"cmd":"avrsetpar","avrpar":"opbutton","avrval":1}'
```

The first target-AVR parameter `smaxcur` limits the mains supply current, the second `phases` enables all three phases. The third triggers the charging cycle by mimicking the operator button of the charging station; see [ctrl22c.c](../ctrl22c/) for detailed information on relevant target-AVR parameters.

## Compiling and Installing ESP32 Firmware

In the case you are familar with ESP-MDF, check that you have updated at lesat to version "v1.0_107", so no "beta". This is crucial because the underlying  ESP-IDF was upgraded from "v3.3.2" to "v4.2.0" and we have updated `demesh.c` accordingly. You're then set to configure and compile `demesh`

In the case you are not familiar with ESP-MDF, there are a number of concepts to become acquainted to, but we believe its worth the effort. Go to the repository and get a copy of ESP-MDF "v1.0". This is described in [ESP-MDF Getting Started](https://docs.espressif.com/projects/esp-mdf/en/latest/get-started/index.html#get-esp-mdf): 

```
mkdir esp
cd esp
git clone --recursive https://github.com/espressif/esp-mdf.git
```

However, when we last did so, we next needed to visit the included ESP-IDF directory and follow the ESP-IDF instructions to install the toolchain (compilers, python add-ons, etc.) as described in [EDP-IDF Getting started](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html#get-started-set-up-tools):

```
cd esp-mdf
cd esp-idf
. ./install.sh
cd ..
```

Now turn back and continue again to follow the  [ESP-MDF Getting Started](https://docs.espressif.com/projects/esp-mdf/en/latest/get-started/index.html#get-esp-mdf). There you are advised to take a copy of the "getting_started/hello_world" and run through the configure/build/install/monitor steps. Since `demesh` uses the very same development cycle, this is probably a good idea.

To actually install and monitor even the simple example, you'll need an ESP32 dev-board ($5 onwards). For setting up an MWifi it is advisable to have some direct visual feed-back, so you do not need to monitor every node over a dedicated serial line. Still budget but quite fancy is the M5Stick (10$) which has a built in mini TFT screen among other fancy features. For the fun of it, get at least five of them, preferably ten.

Once the "hello_world" compiles and installs flawlessly, you may give `demesh.c` a go. The basic workflow is as follows

```
cd ./demesh
make menuconfig
make -j 20
make flash
make monitor
```

Next to the common ESP32 configuration options, `make menuconfig` shows one page specifically for `demesh.c`:

- choose the board: `Nope` is the fallback for no attached IOs; `Gpio2` is a basic dev-board with an LED on IO2; `M5StickC` is for the M5Stick and includes support for the two pushbuttons and the TFT screen;`FGCCS 1.0` is the first revision of our EV charging controller; it is easy to add more boards as long as you are aware of how to access the specfic extra hardware and for which pupose you actually want to use it; check the `demesh.c` sources;
-  `Router SSID` and `Router password` specify the accesspoint the mesh shall connect to; you do not necessariliy need an dedicated RasPi; it's fine if the mesh connects to your home WLAN. **WARNING**: the mesh cannot reliably connect to the 192.168.4.* address space since it apparently uses this netmask internally (it was a nightmare to figure this one out);
- `Mesh ID` and `Mesh Password` are the login secrets for a node to join the mesh; 

- `Upstream Server IP/Port` is the server the root note will try to connect to; if you are using a RasPi as an access point this will be most likely it; otherwise use the IP address of any development machine in your WLAN; the port defaults to 8070;  
- `Firmware Server IP/Port` is the server to distribute ESP32 firmware updates via HTTP; in most cases, you'll use the same as the upstream server; the port defaults to 8071;
- `MQTT broker address` is where you are planning to run the MQTT broker; most likely the same as the upstream server; the address is specified as URL, e.g. `mqtt://192.168.5.1:1883"`
- The remainig options are only relevant for the target uC debugging mode; leave unchanged unless you plan to activate that mode.

There are only a view options regarding the ESP32 module itself that need our attention:

- in `menuconfig->Components->ESP32-specific->` choose  `No Core Dump`;  this is because we abuse this memory space to buffer firmware images for the target uC;

- the provided partitiontable `partition.cvs` has on extra entry `avrflash, data, 0xfe, , 64K`, and this is will become the firmware buffer for AVR target MCU; you need to run `make erase_flash` once to activate the non-standard partition table; 
- in the `menuconfig->Serial Flasher Config -> Port` choose the USB TTY device by which you connect your ESP32; if you have many ESP32s you my want to set this configuration parameter to a symbolic link such that you can programm multiple ESPs without recompiling; we use the link `usb-link` and provide the shell skript `lnport.sh` to set the link;
- for the M5Stick, set `menuconfig->Serial Flasher Config -> Baud Rate` to  `1500000` ... otherwise
  it will just not work.













