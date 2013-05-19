plipbox
=======

plipbox is an Arduino-based device that allows to connect low-end classic
Amigas via Ethernet to your local network. It bridges IP traffic received
via PLIP on the parallel port of the Amiga to the Ethernet port attached
to the Arduino.

Copyright (C) 2012,2013 Christian Vogelgsang <chris@vogelgsang.org>

Released under the GNU Public License V2 (see COPYING for details)

Introduction
------------

With the [plip2slip][1] project I already presented a device that uses a cheap
AVR 8 bit microcontroller (as found on the popular Arduino boards) to bridge
network traffic from the Amiga's parallel port (with the [MagPLIP][2] protocoll)
to another machine via a fast serial link.

plipbox extends the plip2slip project and replaces the serial link for IP traffic
with an on-board Ethernet port. This allows you to connect your Amiga directly
to your local network without any other machine assisting.

With the on-board Ethernet port the plipbox HW is more complex than the plip2slip
HW, but I tried to use common and easy available HW modules to simplify the
recreation of this device. This allows even novice users to build their own
plipbox. (See the hardware document for details).

The firmware for plipbox is open-source and hosted on [GitHub][3].
Clone this repository if you want to build the firmware yourself or if you
want to play around with it. 

[1]: http://lallafa.de/blog/amiga-projects/plip2slip/
[2]: http://aminet.net/package/comm/net/magPLIP38.1
[3]: https://github.com/cnvogelg/plipbox

Download Releases
-----------------

See my [plipbox blog page][4] for downloads of the current release archives.
These archives contain pre-built firmware and Amiga driver binaries in addtion
to the source code here.

[4]: http://lallafa.de/blog/amiga-projects/plipbox/

Documentation
-------------

 - [Change Log](ChangeLog.md): Changes in the releases
 - [Introduction](doc/src/intro.md): Introduction on plipbox
 - [Hardware](doc/src/hardware.md): How to build the device hardware
 - [Firmware](doc/src/firmware.md): How to setup the firmware
 - [Amiga Setup](doc/src/amiga.md): How to setup plipbox.device on your Amiga
 - [Python Emulator](doc/src/python.md): A plipbox emulator if you run your Amiga in FS-UAE
