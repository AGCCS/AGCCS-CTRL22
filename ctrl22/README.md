# Ctrl22 --- AVR Firmware

Here we provide our firmware for the AVR uC ATmega4808 to control the charging process according to the CCS standard. For details of the circuitry, see [../circuit](../circuit/).  The code is organised in a cyclic fashion, i.e., there is one indefinite while-loop which runs a number of callback functions which actually operate the attached hardware modules (e.g. generate control pilot CP, read back CP, read proximity PP, operate mains relays, measure current, etc.). The firmware is controlled via the TX0/RX0 serial line with a line-based human readable protocol. In our project, this control is exercised by an ESP32 SoC to allow for remote access. For development, we propose to set the ESP32 in target AVR development mode in order to forward the AVR's TX0/RX0 via telnet; see [../circuit](../circuit/) for details.



### Serial Line Protocol

The AVR organises its operation by a dedicated set of state variables. Relevant subsets of these variables can be read or written via the serial line. The protocol is as follows.

- to read the parameter `<PAR>`, send `<PAR>?\r\n` to the AVR; expect a reply in the form ` <PAR>=<VAL>\r\n`, or `fail` for parse error; here `<PAR>` the printable ASCII name of the parameter and `<VAL>` is  a signed 16bit integer in common decimal ASCII representation;
- to write the parameter `<PAR>`, send `<PAR>=<VAL>\r\n` to the AVR; expect a reply in the form ` <PAR>=<VAL>\r\n`, or `fail` for `parse error`; here `<PAR>` and  `<VAL>`  are ASCII encoded name and value of the respective parameter as indicated above;
- semantic suger: send `<PAR>!` to set the value to `1`, or  `<PAR>~` to set the value to `0`.

The protocol is strictly line based. That is, the host sends one line and expects one line as reply. This is to facilitate parsing on host side, which in our use case is done by the attached ESP32.

**Examples**

| Send to AVR       | Receive from AVR  | Comment                                                     |
| ----------------- | ----------------- | :---------------------------------------------------------- |
| `ver?/r/n`        | `ver=14/r/n`      | read firmware version v1.4                                  |
| `cmaxcur?/r/n`    | `cmaxcur=160/r/n` | read charging cable capacity 16.0A (-1 for no car detected) |
| `smaxcur=160/r/n` | `smaxcur=320/r/n` | set maximum available current to 32.0A                      |
| `amaxcur?/r/n`    | `amaxcur=160/r/n` | read maximum current actually allocated to vehicle          |
| `phases=12/r/n`   | `phases=12/r/n`   | enable mains phases L1 and L2                               |
| `cur1?/r/n`       | `cur1=65/r/n`     | read 6.5A as current drawn on phase 1                       |



Optional convenience commands are implemented on top of this strict scheme. The AVR replies on such commands are escaped with a beginning line `[[[\r\n` and trailing line `]]]\r\n`. Currently, there is only one such convenience command, namely `?\r\n` to request an overall system status.   

For the sake of simplicity, the AVR is passive, i.e., if we want a regular update on some aspects of its state, we need to poll. Exceptions are

- various debug switches in our current revision will generate informative lines of unsolicitated rumble that begin with the special character `%`
- for future revisions, we may send heartbeat data (e.g. drawn current) by single lines that begin with the special character `[` and that end with `]/r/n`.

Thus, our ESP32 firmware shall filter any lines that start with `[` or `%`.



### Compiling/Programming the AVR

We provide a `Makefile` that should be easily adaptable to Linux/MacOSX programming environments. The easiest way to get a recent AVR toolchain is to install the Arduino IDE and to figure the path of the relevant binaries. A simple `make` on the command line will then produce the binaries `ctrl22.bin` and `ctrl22.hex`. The fromer is required when updating firmware over the wireless mesh network (see [../demesh](../demesh/)), the latter when using `avrdude` via telnet or the J4 header (see [../circuit](../circuit/))