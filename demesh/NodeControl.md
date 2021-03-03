# Node Control Protocol

Individual Nodes receive messages from the host via the root node and the mesh network. All such messages and the respective acknowledgements are JSON encoded ASCII strings, i.e., records of key-value pairs in a human readable format. Any request sent must include an entry `"cmd":"~CMD~"`, where ~CMD~ specifies the request. The task `node_read_task()` indefinitely waits for messages, dispatches the requests accordingly and issues ancknowledgement. The latter must include a `"src":"~ADDR~"`entry and an `"mtype":"~TYPE~"`entry to indicate the reporting node and the format of any further key-value pairs in the reply, respectively. Most commonly, ~TYPE~ is set to ~CMD~ as this is sufficient for the host to adequately interpret the message. 

**Example.** When node d8:a0:1d:55:a7:10 receives  the message  `{"cmd":"status"}`, the node replies with a record like `{"src":"d8:a0:1d:55:a7:10", "mtype":"status", "parent":"d8:a0:1d:55:37:cd", "rssi":-40, "layer":2,"nodes":3, "plat":57}`.  By identifying ``"mtype":"status"`the host knows how to read the further key-value pairs in the reply. In this example, node d8:a0:1d:55:a7:10 reports on the status of its connection status to the mesh network. 

Most relevant requests and their specific key-value pairs given below. During development and/or for first-installation, the listed requests can initiated by the command-line tool  [dmctrl.py](../utils/).

### Restart Request

A restart request is specified by `"cmd:"restart"` and has no further parameters. The effect is a soft-reset of the ESP32. E.g. to restart all nodes in the mesh, the host may send `{"dst"="*","cmd"="restart"}` via TCP to the root node. 

### Status Request

The status request command is specified by `"cmd:"status"` and has no further parameters. The acknowledgement indicates the connection status in the mesh network and includes the following keys.

| Key                 | Comment                                                      |
| ------------------- | ------------------------------------------------------------ |
| "parent":"~PARENT~" | the address ~PARENT~ of this nodes parent; the root node will fill this value with the access point MAC address |
| "rssi":~VAL~        | with ~VAL~ the RSSI to the parent; this indicates the signal quality to the uplink |
| "layer":~VAL~       | with ~VAL~ the layer within the mesh; i.e., the number of nodes actually emitting a message until it reaches the access point |
| "nodes":~VAL~       | with ~VAL~ the number of nodes in the entire mesh network    |
| "plat":~VAL~        | with ~VAL~ the overall time ms for a message to be once propagated forth and back to the parent; i.e., parent-roundtrip latency |

To gather all  relevant data to figure the mesh topology, the host may send `{"dst":"*","cmd":"status"}` via TCP to the root node. 



### System Report Request

The system report request command is specified by `"cmd:"system"` and has no further parameters. The acknowledgement reports the overal system status  and includes the following keys.

| Key                     | Comment                                                      |
| ----------------------- | ------------------------------------------------------------ |
| "time":~TIME~           | system time ~TIME~ in ms; all nodes have manage a synchronised system time with a role-ove at 3600000, i.e., one hour |
| "version":"~MAJ~.~MIN~" | version of the firmware as string with ~MAJ~ and ~MIN~ one decimal digit each |
| "board":"~BOARD~"       | hardware platform identigyer, e.g. `"board":"m5stick"` for the M5StickC or `"board"="agccs12"` for our charging station with a Rev-1-2 board |
| "avrver":~VER~          | version of the firmware of the attached AVR as an integer; our firmware `ctrl22.c` reports a two digit number with the first digit the major versiom and the secont the minor version; version 0 is recerved to indicate "no AVR attached" |
| "plat"=~LATENCY~        | the estimated latency in ms to snd a message to the root     |

To obtain an overview over all nodes and their respective firmware versions, the host may send `{"dst":"*","cmd":"system"}` via TCP to the root node. 



### Time Synchronisation Request

A time synchronisation request is specified by `"cmd:"tsync"` and has no further parameters. It triggers a synchronisation of the system time as seen by the respective node with its parent. The acknowledgement reports the the time stamps taken.

| Key           | Comment                                                      |
| ------------- | ------------------------------------------------------------ |
| "tsync1":~T1~ | system time ~T1~ of the respective node when it sent the "ping" to its parent |
| "tsync2":~T2~ | system time ~T2~ of the parent  when it received the "ping"  |
| "tsync3":~T3~ | system time ~T3~ of the respective node when it received the acknowledgement from the parent |



### Talking to the Attached AVR

The relevant state of the attached AVR is encoded in a set of parameters to assemble the process image. By the request `"cmd":"avrgetpar"` the host can read individual parameters as specified by the additional key `"avrpar"`. The node acknowledgs by returning the key `"avrpar"` augmented by the additional key `"avrval"` to provide the value of the requested parameter. Likewise, the node implements the request `"cmd":"avrsetpar"` to write to the process image. Here, the additional keys `"avrpar"` and `"avrval"` specify the parameter of interest and the intended value to be written, respectively. In summary, reading and writing from/to the process image involves the following key-value pairs.



| Key              | Comment                                                      |
| ---------------- | ------------------------------------------------------------ |
| "avrpar":"~PAR~" | symbolic name ~PAR~ of the parameter to access; available parameters for our charging station are documented [here](../ctrl22/README.md#Serial-Line-Protocol). |
| "avrval":~VAL~   | value read from or to be written to the process image; in the acknowledgement on a write access, ~VAL~ will be set to `"ok"` on success. |

To have all charging stations flash their LED button twice at the beginning of every two-seconds period, the host may send `{"dst":"*", "cmd":"avrsetpar", "avrpar":"blinks", "avrval":2}` via TCP to the root node. 

**Note**. For sake o simplicity, `demesh.c` restricts the type of parameter values to  32bit signed integers, so there are no floats or strings;  the AVR firmware for our charging station further restricts this to a maximum of 16bit.



### Firmware Upgrade Request 

A firmware ugrade request is specified by `"cmd:"upgrade"` and refers to the ESP32 firmware `demesh.c`. There is no acknowledgement to this command. It should be followed up by a system report request. Parameters are provided as follows.

| Key                     | Comment                                                      |
| ----------------------- | ------------------------------------------------------------ |
| "board":"~BOARD~"       | hardware platform as configured via `make menuconfig` when specifying the board and as in indicated by system report request; e.g. `"board":"m5stick"` for the M5StickC or `"board"="agccs12"` for our charging station with a Rev-1-2 board |
| "version":"~MAJ~.~MIN~" | version to upgrade to in the same format as in a system report request reply |

The upgrade  process is organised by the root note and this is the only node to accept a `"cmd":"upgrade"`.  From the board and version data, the root node infers the firmware filename by convention, e.g., `demesh_m5stick_3_5` for a firmware operable on M5StickC hardware in version v3.5; the prefix `demesh` can be configured via `make menuconfig`, the remaining conventions are hardcoded in `demesh.c`. In particular, there must be one digit for the major version and one digit for the minor version. The root node then connects to an HTTP server to download the firmware file. The IP address defaults to the server to which the root node connects via TCP, the port defaults to 8071; again both configurable via  `make menuconfig`. Once the root has obtained the firmware, is offers it to all other nodes via "ESP-MDF magic". Indivual nodes aill accept the upgrade provided that board matches and the version is different to one it is currently running on.

On host side, the overall process has been implemented in the utility [dmctrl.py](../utils/) for inspection.

### Firmware Upgrade for the attached AVR

Also the attached AVR can be updated over the air (OTA), however, we are in charge to organise the process on our own, no "ESP-MDF magic" at this end. Our solution is not quite as sneak as Espressifs OTA, however, it is functional. In particular, the upgrade here is on a per node basis so the host needs ti track which nodes need an update. We use two commands to control the process.

First, the host issues a `"cmd":"avrota"`with the additional key  `"state":"recimg"` to alert the node to receive the firmware image we are going to send. The node confirms its AVR OTA state in its reply, so the is aware that it now may send the actual image.

Second, the host issues a number of `"cmd":"avrimg"` requests to transfer the firmware chunc by chunc. This command takes three more keys `avraddr`, `avrdata` and `avrcrc` , specifying the target address (counting from zero, regardless of any bootloader offset), the Base64 encoded slice of the firmware (typical 128 bytes) and a CRC double. The ESP32 buffers the image in a dedocated partition in flash memory. Reading back from flash, the CRC is checked and on success the acknowledgement to `"cmd":"avrimg"` will include a `"avrcrc":"ok"`entry.

Third, the host issues a `"cmd":"avrota"` with the additional key `"state":"flash"` to ask the ESP32 to forward the image to the attached AVR via the serial line using the Optiboot protocol and to finally have Optiboot to flash the new firmware. On success, the ESP acknowledges with the key-value pair `"state":"running"` , errors are indicated by  `"state":"halted"`.

On host side, the overall process has been implemented in the utility [dmctrl.py](../utils/) for further inspection.



## 