plipbox: Firmware Setup
=======================

1. Flash Firmware
-----------------

### 1.1 Find Arduino Serial Port

You need to connect your Arduino typically via the USB port to your Mac or PC 
and this will install a "virtual" serial port that allows to reach the Arduino
via a serial link. 

The device is called differently depending on your OS:

 * Macs: `/dev/cu.usbserial-<serial>`
 * Linux: `/dev/ttyUSB<num>`
 * Windows: `COM1 ... COMx`
  
You can find out the port associated with your Arduino by looking at the registered devices before and after attaching the device.

### 1.2 Flash on Arduino Hardware with "avrdude"

For flashing the firmware you need a flash tool on your Mac/PC that talks via
this serial port. I use [avrdude][ad] here. It is available on all platforms and
a command line tools.

[ad]: http://www.nongnu.org/avrdude/

Now open a shell/terminal/cmd.exe on your OS and call the firmware tool with:

        > avrdude -p m328p -P <your_serial_port> -b 57600 -c arduino -U flash:w:plipbox-0.1-57600-arduino-atmega328.hex

This assumes that you have the plipbox firmware file called 
`plipbox-0.1-57600-arduino-atmega328.hex` in your current directory.
In the release archive you can find it in the `firmware` directory.

Furthermore, replace `<your_serial_port>` with the device name of your
Ardiuno serial device found in section 1.1.

If everything works well then you will see the following output:

        avrdude: AVR device initialized and ready to accept instructions
        
        Reading | ################################################## | 100% 0.00s
        
        avrdude: Device signature = 0x1e950f
        avrdude: NOTE: FLASH memory has been specified, an erase cycle will be performed
                 To disable this feature, specify the -D option.
        avrdude: erasing chip
        avrdude: reading input file "plipbox-0.1-57600-arduino-atmega328.hex"
        avrdude: input file plipbox-0.1-57600-arduino-atmega328.hex auto detected as Intel Hex
        avrdude: writing flash (16596 bytes):
        
        Writing | ################################################## | 100% 4.67s
        
        avrdude: 16596 bytes of flash written
        avrdude: verifying flash memory against plipbox-0.1-57600-arduino-atmega328.hex:
        avrdude: load data flash data from input file plipbox-0.1-57600-arduino-atmega328.hex:
        avrdude: input file plipbox-0.1-57600-arduino-atmega328.hex auto detected as Intel Hex
        avrdude: input file plipbox-0.1-57600-arduino-atmega328.hex contains 16596 bytes
        avrdude: reading on-chip flash data:
        
        Reading | ################################################## | 100% 3.37s
        
        avrdude: verifying ...
        avrdude: 16596 bytes of flash verified
        
        avrdude: safemode: Fuses OK
        
        avrdude done.  Thank you.

Now your device is fully operational and we use the serial link to communicate 
with plipbox.

### 1.3 Flash on AVR-NET-IO Board

The AVR-NET-IO-Board from Pollin.de has a different AVR
chip, the ATmega32 and thus needs another firmware version:

        plipbox-0.1-57600-avrnetio-atmega32.hex

Again I use avrdude to flash the firmware. Although the board provides a 
bootloader via serial port I was not able to use this with avrdude. So
I switched over to direct ISP programming but that needs an AVR ISP
programming adapter. I use the [USBasp][1] thats a cheap AVR ISP that
is supported by avrdude and has a USB connector. You can build one
yourself or buy a [kit][2].

The command avrdude is as follows:

        > avrdude -p m32 -c usbasp -U flash:w:plipbox-0.1-57600-avrnetio-atmega32.hex

[1]: http://www.fischl.de/usbasp/
[2]: http://www.fundf.net/usbasp/

2. plipbox Configuration
------------------------

The plipbox firmware uses the serial link available on your Arduino to show you 
device information and allows you to configure the device in this terminal.

Starting with version 0.3 the plipbox device is essentially a zero-config device,
i.e. you don't have to alter any parameters for typical default operation. 

Nevertheless, the firmware offers options to fine tune its behaviour and provides
commands for statistics and diagnosis. 

### 2.1 Setup your terminal program

On your Mac/PC you will need a terminal program to talk via the serial link.
If you haven't got a favorite one then have a look at [CoolTerm][1] for Mac OSX
or [TeraTerm][2] for Windows. Linux users might try [minicom][3] or [picocom][4].

[1]: http://freeware.the-meiers.org/
[2]: http://www.ayera.com/teraterm/
[3]: http://alioth.debian.org/projects/minicom/
[4]: http://code.google.com/p/picocom/

Run your terminal program and select the serial port of your Arduino (see 
section 1.1). For the serial port select the following parameters:

  * 8 N 1 = 8 data bits, no parity, 1 stop bit
  * no hardware (RTS/CTS) hand shaking
  * no software (XON/XOFF) hand shaking

If everything went well you will see the startup message of the plipbox 
firmware:

        Welcome to plipbox 0.3 20130519
        by lallafa (http://www.lallafa.de/blog)

You see all important parameters of your device and their current values.
Below is the current status of the device starting with `eth rev` that
tells you that the firmware correctly detected your Ethernet chip.

### 2.2 Entering/Leaving Command Mode

If you press Return in your terminal program then the device will enter
*command mode*. This mode allows you to enter commands that the device
will execute. Its mainly used to set parameters and load/save them.

Command mode is shown with a command prompt "> " in the beginning of a line.

If you enter **q** and Return you give the *quit* command to leave the command
mode and the device returns to active mode. 

*Note* that during command mode the device does NOT operate and does NOT pass
any IP packets.

See the following section for a complete list of all commands available.

A command usually consists of a command name and optional arguments. All
a seperated by spaces on the command line.

A command is always finished by pressing Return.

### 2.3 plipbox Commands

#### Common Commands

  - **q**: Leave command mode
  - **v**: Show plipbox firmware version
  - **p**: Show parameters
  - **ps**: Save parameters to EEPROM
  - **pl**: Load parameters from EEPROM
  - **pr**: Reset parameters to factory defaults

#### Configuration

  - **m <mac>**: Set MAC Address of the Ethernet Adapter. You must ensure that
    the plipbox.device in your Amiga configuration uses exactly the same MAC.
    Otherwise plipbox will not work. By default a generic MAC is pre-defined.
    This is fine if you have a single plipbox on your local network. If you use
    multiple plipboxes then they must have different MACs.
  
  - **tr <n>**: Set the number of retries a packet received via Ethernet is
    resent to the Amiga if the arbitration on the PLIP was lost. The default
    is 0 i.e. packet is dropped.

#### Statistics

  - **sd**: Dump the statistics information. The plipbox records a number of 
    typical network statistics including sent packets, send bytes, transfer
    errors and so on for each direction. This command prints the currently
    accumulated values.
    
  - **sr**: Reset the statistics counters.

#### Diagnosis

plipbox offers a rich set of diagnosis (or debug) commands that let you watch
the packet traffic running through the box. You can see the packets and even
decode their contents (to some degree).

  - **dd <nn>**: Master switch to enable diagnosis output for the selected 
    channels. The channels are given as a single byte value (2 hex digits)
    with the following encoding:
    
          1: plip(rx)
          2: eth(rx)
          4: plip(tx)
          8: eth(tx)
    
    Just add the values if multiple channels are to be selected. Value `0f`
    enables all channels.
  
  - **de**: Toggle dumping contents of Ethernet packets. If enabled this option
    prints the Ethernet header including source MAC, target MAC, packet type,
    and size.

  - **di**: Decode IP packets. If enabled the IP packet header is decoded and
    the source IP, target IP, protocol type is printed.
  
  - **da**: Decode ARP packets. If enabled the contents of ARP packets including
    ARP source HW+IP addr and destination HW+IP addr is printed.
  
  - **dp**: Decode IP protocols: TCP and UDP. If enabled some information from
    TCP and UDP packets is also decoded. This includes source/destination port.
    
  - **dl**: Give details on the PLIP transfer
  
  - **dy**: Measure latency between incoming ethernet packets and outgoing 
    PLIP packets and vice versa. 

### 2.4 plipbox Key Commands

If you are in *active mode* (not command mode) then you can press some command
keys to trigger actions on the device:

  - **d**: Dump the current statistics. Similar to **sd** command.
  - **r**: Reset statistics counters. Similar to **sr** command.

EOF





