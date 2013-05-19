Build your own plipbox device
=============================

1. Arduino (2009)
----------------

Currently, the firmware is implemented on an Arduino 2009 board with an
ATmega328 running at 16 MHz. You can also use one of the many Arduino clones if
the ROM size is sufficient (16K or more), the AVR chip matches and enough I/O
pins are available. A new Arduino Uno or works similarly.

Setting up the missing parts for your plipbox isn't that hard:
All you need is to by some extra components and wire them together.

### 1.1 Network with ENC28J60 Chip

The essential addition to your Arduino board is a network shield or network
expansion card. Please note, that you need a network card with a *ENC28J60* chip
from Microchip. Cards using a *WizNet* chip will *NOT* work! (e.g. The Arduino
Ethernet Shield)

I use the [ENC28J60 Ethernet Module][1] from iteadstudio.

*Please note*: If you use an Ethernet shield then make sure that the shield
does not require pins that are already used for the Amiga cable (see below).
E.g. the *ENC28J60 Ethernet Shield* from iteadstudio will not work as
it uses some `DIGITAL` pins for extra functions like an SD card.

The wiring of the Ethernet module is as follows:

        ENC28J60 Ethernet Module     Function           Arduino
        ------------------------     -----------------  ----------------- 
          GND                         GND               GND
          SS                          Slave Select      DIGITAL 10 (PB2)
          MOSI                        Slave In Data     DIGITAL 11 (PB3)
          MISO                        Slave Out Data    DIGITAL 12 (PB4)
          SCK                         SPI Clock         DIGITAL 13 (PB5)
          VCC                         +3.3V Supply      (see Note)
    
*Note*: The Ethernet module needs a 3.3V supply while your Arduino needs to
run on +5V. Although the Arduiono already provides a +3.3V power source
on a pin, it will be too weak to feed the Ethernet chip. I opted to use
a 5V to 3.3V regulator to supply the ethernet module. You can use a 
ready-made 3.3V regulator board if you don't want to build it yourself.
See for example at [Sparkfun][2]

[1]: www.iteadstudio.com
[2]: www.sparkfun.com

### 1.2 Amiga Parallel Port Cable

You need to build a cable from the Amiga Parallel Port (DB 25 male) to the
Arduino. Try to use only a short wire length otherwise signal quality will
suffer. Up to 10 cm are suitable lengths.

       Amiga Parallel Port         Function          Arduino
       -------------------         ----------------  ---------------------------
          17 ... 22                GND               GND Pin
          1                        /STROBE           DIGITAL 2 (PD2)
          13                       SELECT            DIGITAL 3 (PD3)
          11                       BUSY              DIGITAL 4 (PD4)
          12                       POUT              DIGITAL 5 (PD5)
          8                        DATA 6            DIGITAL 6 (PD6)
          9                        DATA 7            DIGITAL 7 (PD7)
          10                       /ACK              DIGITAL 8 (PB0)
          2 ... 7                  DATA 0 ... 5      ANALOG 0 ... 5 (PC0 ... PC5)
      

### 1.3 Serial Port for Terminal Access

The plipbox device can be configured and debugged via the serial console available
on all Arduino devices. Usually you don't need extra HW to access this serial 
port.

A virtual serial port is often available via the USB connection.

Alternatively, you can connect your plip box serial port directly to your Amiga 
and then use an Amiga terminal program to control the device. In this case you
need to connect the TxD, RxD and GND wires from your Arduino to a RS232 level
converter and then to your Amiga. The level converter is essential, as the Amiga
uses +/-12V signals and the Arduino 0/5V. _NEVER_ connect your Amiga directly!


2. Pollin.de AVR-NET-IO Board
-----------------------------

The german online shop [Pollin][1] offers with the [AVR Net IO][2a] a low cost
ATmega32 board (either as a kit for 20 EUR or [already assembled][2b] for
around 30 EUR) that is suitable for the plipbox project. It already contains
the ENC 28j60 ethernet controller chip and a DB 25 connector that is already
suitable to connect the Amiga. The only thing missing in this setup for plibox
is a small wire!

[1]: http://www.pollin.de
[2a]: http://www.pollin.de/shop/dt/MTQ5OTgxOTk-/Bausaetze_Module/Bausaetze/Bausatz_AVR_NET_IO.html
[2b]: http://www.pollin.de/shop/dt/NjI5OTgxOTk-/Bausaetze_Module/Bausaetze/AVR_NET_IO_Fertigmodul.html
    
### 2.1 Add Strobe Wire

You need to add a wire from Pin 1 of the DB 25 Connector (J5) to Pin 16 on the 
ATmega (IC5):

        DB25 / J5 Pin 1 <-------------> ATmega32 / Pin 16 (PD2) 

If its difficult for you to find the ATmega pin then you can also use Pin 1 of 
the EXT connector:

        DB25 / J5 Pin 1 <-------------> EXT Conenctor / Pin 1


### 2.2 Modify Power-Supply (optional!)

If you build the board yourself you might want to replace the 9V power supply
required with a 5V regulated power supply (e.g. USB charger). In this
[mikrokontroller.net Forum Thread][mod] you find a picture with the required
adjustments for this mod:

  - Remove D1, D4
  - Replace D2, D5 with a wire
  - Remove IC1 and connect pin 1 and 3 of IC1 with a wire

Then you can use a +5V regulated power supply on J1. 
Please watch the polarity now:

(-) Minus is next to the ethernet connector

[mod]: http://www.mikrocontroller.net/articles/AVR_Net-IO_Bausatz_von_Pollin


----
That's it! All you need to construct your own plipbox HW is ready!

