plipbox: Firmware Documentation
===============================

1. Flash Firmware
-----------------

### 1.1 Find plipbox Serial Port

You can connect your plipbox typically via the USB port to your Mac or PC 
and this will install a "virtual" serial port. Other plipbox devices have the
serial port available on 2 pins with +5V signals. Then you need an external
USB-to-Serial device to convert the serial signals to a virtual serial device.

The virtual serial port is called differently depending on your OS:

 * Macs: `/dev/cu.usbserial-<serial>`
 * Linux: `/dev/ttyUSB<num>`
 * Windows: `COM1 ... COMx`
  
You can find out the port associated with your plipbox by looking at the
registered devices before and after attaching the device.

### 1.2 Flashing with Serial Link

Some AVR devices like the Arduino-based ones allow you to flash the firmware
via the (virtual) serial link.

First select correct firmware file for flashing. The firmware used for the
Arduino 2009 prototype is called `*-ardiuno-*.hex` and the firmware for the
plipbox nano production version is called `*-nano-*.hex`. Choose the version
matching your hardware. Both have the same features but use a slightly
different pinout for accessing the Amiga's parallel port.

For flashing the firmware you need a flash tool on your Mac/PC that talks via
this serial port. I use [avrdude][ad] here. It is available on all platforms and
a command line tools.

[ad]: http://www.nongnu.org/avrdude/

Now open a shell/terminal/cmd.exe on your OS and call the firmware tool with:

        > avrdude -p m328p -P <your_serial_port> -b 57600 -c arduino -U flash:w:plipbox-0.x-57600-arduino-atmega328.hex

This assumes that you have the plipbox firmware file called 
`plipbox-0.x-57600-arduino-atmega328.hex` in your current directory.
In the release archive you can find it in the `avr/firmware` directory.

Furthermore, replace `<your_serial_port>` with the device name of your
Ardiuno serial device found in section 1.1.

If everything works well then you will see the following output:

        avrdude: AVR device initialized and ready to accept instructions
        
        Reading | ################################################## | 100% 0.00s
        
        avrdude: Device signature = 0x1e950f
        avrdude: NOTE: FLASH memory has been specified, an erase cycle will be performed
                 To disable this feature, specify the -D option.
        avrdude: erasing chip
        avrdude: reading input file "plipbox-0.x-57600-arduino-atmega328.hex"
        avrdude: input file plipbox-0.x-57600-arduino-atmega328.hex auto detected as Intel Hex
        avrdude: writing flash (16596 bytes):
        
        Writing | ################################################## | 100% 4.67s
        
        avrdude: 16596 bytes of flash written
        avrdude: verifying flash memory against plipbox-0.x-57600-arduino-atmega328.hex:
        avrdude: load data flash data from input file plipbox-0.x-57600-arduino-atmega328.hex:
        avrdude: input file plipbox-0.x-57600-arduino-atmega328.hex auto detected as Intel Hex
        avrdude: input file plipbox-0.x-57600-arduino-atmega328.hex contains 16596 bytes
        avrdude: reading on-chip flash data:
        
        Reading | ################################################## | 100% 3.37s
        
        avrdude: verifying ...
        avrdude: 16596 bytes of flash verified
        
        avrdude: safemode: Fuses OK
        
        avrdude done.  Thank you.

Now your device is fully operational and we use the serial link to communicate 
with plipbox.

### 1.3 Flashing with ISP

All AVR boards can be flashed with an In-System-Programmer (ISP) device. This
device directly programs the flash ROM. The ISP is an external tool that
is also typically connected via USB to your PC. I use the [USBasp][1] thats
a cheap AVR ISP that is supported by avrdude and has a USB connector. You can
build one yourself or buy a [kit][2].

The AVR-NET-IO-Board from Pollin.de needs to be flashed via ISP and this 
firmware variant:

        plipbox-<version>-57600-avrnetio-atmega32.hex

Again you can use `avrdude` to flash the firmware. The command avrdude is as
follows:

        > avrdude -p m32 -c usbasp -U flash:w:plipbox-0.1-57600-avrnetio-atmega32.hex

Please note the different flash adapter `usbasp` here and that you do not need
a serial speed now.

[1]: http://www.fischl.de/usbasp/
[2]: http://www.fundf.net/usbasp/

2. plipbox Configuration
------------------------

The plipbox firmware uses the serial link available on your plipbox to show you 
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

Run your terminal program and select the serial port of your plipbox (see 
section 1.1). For the serial port select the following parameters:

  * 8 N 1 = 8 data bits, no parity, 1 stop bit
  * no hardware (RTS/CTS) hand shaking
  * no software (XON/XOFF) hand shaking

### 2.2 Entering/Leaving Command Mode

If everything went well you will see the startup message of the plipbox 
firmware after a reset of the device:

        Welcome to plipbox <version> <date>
        by lallafa (http://www.lallafa.de/blog)

You see all important parameters of your device and their current values.
Below is the current status of the device starting with `eth rev` that
tells you that the firmware correctly detected your Ethernet chip.

If you press **Return** in your terminal program then the device will enter
*command mode*. This mode allows you to enter commands that the device
will execute. Its mainly used to set parameters and load/save them.

While the device is in command mode it does not handle any transfers - its
completely inactive. You have to quit command mode first to return to normal
operation.

Command mode is shown with a command prompt "> " in the beginning of a line.

If you enter **q** and **Return** you give the *quit* command to leave the
command mode and the device returns to active mode. 

If you enter **?** and **Return** a help text will be displayed. It shows
you all the commands that are available.

While running in active mode you can also press keys to trigger commands.
See section 2.4 for details on those keyboard commands.

### 2.3 plipbox Commands

See this section for a complete list of all commands available.

A command usually consists of a command name and optional arguments. All
are seperated by spaces on the command line. The arguments are always numbers
and given in hexadecimal (only IP address is given in decimal). The numbers
have a fixed size of either 2 or 4 digits and are denoted as follows:

  - **nn**: byte (8 bit)
    - hex value
    - always give 2 digits.
    - Example: `af`, `00`
  - **nnnn**: word (16 bit)
    - hex value
    - always give 4 digits.
    - Example: `0010`, `ab12`
  - **nn:nn:nn:nn:nn:nn**
    - 6 bytes in hex values.
    - Example: `1a:11:af:a0:47:11`
  - **nnn.nnn.nnn.nnn**
    - 4 bytes given in decimal. always give 3 digits per byte.
    - Example: `192.168.0.42`

A command is always finished by pressing **Return**.

#### 2.3.1 Common Commands

  - **q**: Leave command mode
  - **r**: Soft reset device and restart it
  - **v**: Show plipbox firmware version

#### 2.3.2 Parameter Storage

  - **p**: Show parameters
  - **ps**: Save parameters to EEPROM
  - **pl**: Load parameters from EEPROM
  - **pr**: Reset parameters to factory defaults

#### 2.3.3 Configuration Commands

  - **m nn:nn:nn:nn:nn:nn:nn** (Mac Address)
    - Set the mac address of the plipbox device.
    - Always ensure that the firmware uses the same value as the 
      plipbox.device driver!
    - Normally you don't need to alter the mac address via this parameter.
      The driver will automatically push the mac address to the firmware on
      every change.

  - **fd [nn]** (Full Duplex)
    - Toggle between Ethernet full and half duplex mode. If fd is
      set to one then full duplex mode is enabled. Note: the duplex mode can
      only be switched after a reset of the device. So set your desired value
      with this command then save the parameters with **ps** and reset the
      device with **r** to test the new setup. 

  - **fc [nn]** (Flow Control)
    - Toggle the use of Ethernet flow control to limit the rate
      of incoming Ethernet packets. If the parameter is set to one then flow
      control is enabled.

#### 2.3.4 Statistics Commands

  - **sd** (Dump Statistics)
    - Dump the statistics information. The plipbox records a number of 
      typical network statistics including sent packets, send bytes, transfer
      errors and so on for each direction. This command prints the currently
      accumulated values.
    
  - **sr** (Reset Statistics)
    - Reset the statistics counters.

#### 2.3.5 Test Commands

plipbox offers a rich set of diagnosis (or test) modes. Some of them use extra
parameters that can be configured in command mode. See section 3 for details.

  - **tl nnnn** (Test Packet Length) (4 byte hex word)
    - The number of bytes that will be transferred per packet in test mode.

  - **tt nnnn** (Test Packet Ethernet Type) (4 byte hex word)
    - The ethernet type of the packets that will be sent in test mode.

  - **ti nnn.nnn.nnn.nnn** (Test Mode IP) (IP address)
    - In PIO test mode the plipbox device has its own IP address

  - **tp nnnn** (Test Mode UDP Port) (4 byte hex word)
    - In PIO test mode the plipbox answers UDP requests on this port

  - **tm [nn]** (Toggle test submode)
    - Some test modes have a sub mode. Use this command to toggle it.

### 2.4 plipbox Key Commands

If you are in *active mode* (not command mode) then you can press some command
keys to trigger actions on the device:

#### 2.4.1 Mode Selection

  - **1** (Enter Bridge Mode)
  - **2** (Enter Bridge Test Mode)
  - **3** (Enter PIO Test Mode)
  - **4** (Enter plipbox Protocol Test Mode)

#### 2.4.2 Statistics

  - **s** (Dump Statistics)
    - Dump the current statistics.
    - Similar to **sd** command.
  - **S** (Reset Statistics)
    - Reset statistics counters. 
    - Similar to **sr** command.

#### 2.4.3 Diagnosis

  - **v** (Toggle Verbose)
    - If verbose is enabled then detailed information on every transfer
      is printed
    - Please note that verbose significantly degrades the performance of
      the device and should only be used for analysis purposes.
  - **p** (Send a Packet)
  - **P** (Send a Packet silent)
    - Trigger sending a test packet
    - Works in plipbox test mode only
  - **a** (Toggle auto-send Packets)
    - If enabled it will automatically send packets continuously until
      you stop auto mode again.


## 3. plipbox Run Modes

### 3.1 Terms

The plipbox device consists of two sub parts:

  - The **parallel port** connects to the Amiga. There the plipbox
    protocol (short: pb) is talked to transfer packets from/to the Amiga
    on this interface.
  - The **PIO port** (PIO stands for packet I/O). Currently, an ethernet
    controller is the PIO device of the plipbox. The PIO port transfers
    the packets to your local network.

Overview:

        | Local Network        +------- plipbox -----+
        |                +------------+    +------------------+
        |<-------------->| PIO Port   |    | PB/Parallel Port |<------> Amiga
        |                +------------+    +------------------+
        |                      +---------------------+

### 3.2 Normal Operation: Bridge Mode

After startup plipbox enters **bridge mode**. This is the normal operation
of the device: Both PIO and parallel port are active and packets are received
on both ports. If a packet arrives then the packet is sent on the other port.

On your Amiga you use the plipbox.device and your TCP/IP stack to transfer
your internet traffic.

Incoming PIO Traffic:

        | Local Network
        |                 +-----------+
        |---- Packets --->|  plipbox  |---- Packets ---> Amiga
        |                 +-----------+                  TCP/IP Stack
        |                                                + plipbox.device
        
Incoming PB Traffic:

        | Local Network
        |                 +-----------+
        |<--- Packets ----|  plipbox  |<--- Packets ---- Amiga
        |                 +-----------+                  TCP/IP Stack
        |                                                + plipbox.device

Use command key **1** (see section 2.4.1) to enable this mode.

### 3.3 UDP Roundtrip Tests

These tests allow you to test sending traffic across plipbox with a
special test setup: A PC test program called **pio_test** (see `python`
directory of the software distribution) is used to generate special UDP
packets that are sent to the plipbox. The plipbox will bridge them to the
Amiga and a special test program there will return them. The returned
packets are bridged to the PIO port and sent back to the PC. The PC test
program will then send the next UDP packet on the round trip. After a given
number of packets the test program stops and gives you average transfer speeds.

#### 3.3.1 Loopback with TCP/IP Stack: Bridge Mode + udp_test

The basic operation in this test mode is to use the TCP/IP Stack on the 
Amiga to receive and reply the packets. The plipbox device is operated in
the normal Bridge Mode.

UDP Packet Round Trip of Test:

      +-----------+ 1. Send UDP Pkt +---------+ 2. Bridge       +--------------+
      | PC with   |---------------->|         |---------------->| Amiga        |
      | pio_test  | 4. Bridge       | plipbox | 3. Reply UDP    | TCP/IP Stack |
      | running   |<----------------|  Bridge |<----------------| +udp_test    |
      +-----------+                 +---------+                 +--------------+

Tested Components:
  - plipbox Bridge Mode
  - plipbox.device in normal operation
  - TCP/IP Stack on Amiga

Test Setup:
  - On plipbox:
    - Make sure mode is Bridge (key **1**)
  - On Amiga:
    - Configure your TCP/IP with plipbox.device for normal operation
    - Bring up the plipbox network interface and retrieve assigned IP: `AmigaIP`, e.g. 192.168.2.42
  - On PC:
    - Make sure you can reach the Amiga by pinging the AmigaIP

    > ping 192.168.2.42

    - Now launch the test program and give the Amiga's IP

    > python27 pio_test -a 192.168.2.42 -c 1000

    - Note: The `-c` option gives the number of test packets to be sent

#### 3.3.2 Loopback with SANA-II Device Interface: Bridge Test Mode + dev_test

UDP Packet Round Trip of Test:

      +-----------+ 1. Send UDP Pkt +---------+ 2. Bridge       +--------------+
      | PC with   |---------------->|         |---------------->| Amiga        |
      | pio_test  | 4. Bridge       | plipbox | 3. Reply UDP    | no stack     |
      | running   |<----------------|         |<----------------| +dev_test    |
      +-----------+                 +---------+                 +--------------+

Test Setup:

  - On Amiga:
    - Stop your TCP/IP Stack if its still running.
    - Bring up the plipbox network interface and retrieve assigned IP: `AmigaIP`, e.g. 192.168.2.42
    - Make sure test mode parameter (`tm`) is set to zero 0.

  - On PC:
    - Make sure you can reach the Amiga by pinging the AmigaIP

    > ping 192.168.2.42

    - Now launch the test program and give the Amiga's IP

    > python27 pio_test -a 192.168.2.42 -c 1000

    - Note: The `-c` option gives the number of test packets to be sent

### 3.4 PB Test Mode

### 3.5 PIO Test Mode

### 3.6 PC Test Tools

### 3.7 Amiga Test Tools


EOF





