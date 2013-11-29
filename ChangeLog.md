plipbox: ChangeLog
==================

version 0.1 (22.7.2012)
-----------------------

- initial public release
- converted plip2slip code base and added ethernet support
- introduced a NAT device in firmware that connects a point2point link from
  the Amiga with the local Ethernet
- added DHCP on Ethernet link
- initial HW support: Arduino 2009

version 0.2 (2.9.2012)
----------------------

- added HW support for AVR-NET-IO board

version 0.3 (19.5.2013)
-----------------------

- major redesign: removed NAT device and introducded MAC bridge, i.e.
  plipbox behaves like an Amiga Ethernet adapter and bridges all packets
  from there without alteration
- dropped lots of network code found in plipbox that is not needed for the
  MAC bridge
- rewrote and renamed Amiga SANA-II plipbox.device to be an Ethernet driver
- added a Python-based software emulation for the plipbox that operates with
  a patched FS-UAE Amiga emulator
- new approach makes the device zero-conf. Just attach and it works.

version 0.4 (01.6.2013)
-----------------------

- added hardware description for plipbox nano hardware
- added schematic and board design for plipbox nano base board
- added new firmware -nano for new plipbox nano pinout
- Amiga driver unchanged and stays at 0.3

version 0.5 (30.11.2013)
------------------------

- plipbox device now automatically receives its MAC address from driver
- you can change your MAC address with SANA-II command in software
  (if your TCP/IP stack supports it, e.g. Miami DX or Roadshow)
- plipbox device maps online/offline state to Ethernet module and
  disables it if devie is offline
- parallel line protocol completely rewritten to use a client server model
- added optional Ethernet full duplex support
- added optional Ethernet flow control
- improved device statistics
- added error log

