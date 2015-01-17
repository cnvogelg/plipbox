Benchmark Results
=================

0. Introduction
---------------

Here you will find performance measurements of the plipbox software with a specific board.

For more details  on how to perform the tests see the firmware documentation.

### Tests

#### PB Test

 - plipbox console:
    - Test Mode **4**
    - Activate Auto Mode **a**
    - Wait approx. 60s
    - Deactivate Auto Mode **a**
    - Show statistics **s**

#### PIO Test

  - plipbox console:
    - Test Mode **3**
  - PC
    - **pio_test -c 1000**

#### Bridge Test (Mode 0)

 - plipbox console:
    - Test Mode **2**
    - Parameter **tm == 0**
 - PC
    - **pio_test -c 1000**

#### Bridge Test (Mode 1)

 - plipbox console:
    - Test Mode **2**
    - Parameter **tm == 1**
 - PC
    - **pio_test -c 1000**

#### Bridge 

 - plipbox console:
    - Mode **1**
 - PC
    - **pio_test -c 1000 -a amiga_ip**


1. Version 0.6
--------------

### 1.1 Benchmark ACA500/030

 - Plipbox:
    - Sofware: 0.6
    - Hardware: crasbe's plipbox (ATmega328p)
 - Amiga:
    - dev_test 0.6, udp_test 0.6, Roadshow 1.11
    - A500 + ACA500 + ACA 1230/33

#### PB Test

    cnt  bytes    err  drop rate
    033F 00133296 0000 0000 0234.36 KB/s rx plipbox
    033F 00133296 0000 0000 0234.80 KB/s tx

### PIO Test

    ip= 192.168.2.222 tgt_port= 6800 src_port= 6800 data_size 1400 count= 1000 delay= 0 verbose= False
    @1000: d=  8.92 ms  v=314.05 KB/s

### Bridge Test (Mode 0)

    ip= 192.168.2.222 tgt_port= 6800 src_port= 6800 data_size 1400 count= 10 delay= 0 verbose= False
    @10: d= 22.00 ms  v=127.27 KB/s

### Bridge Test (Mode 1)

    ip= 192.168.2.222 tgt_port= 6800 src_port= 6800 data_size 1400 count= 100 delay= 0 verbose= False
    @100: d= 22.64 ms  v=123.68 KB/s

### Bridge 

    ip= 192.168.2.42 tgt_port= 6800 src_port= 6800 data_size 1400 count= 1000 delay= 0 verbose= False
    @1000: d= 24.48 ms  v=114.38 KB/s

### 1.2 Benchmark ACA500

 - Plipbox:
    - Sofware: 0.6
    - Hardware: crasbe's plipbox (ATmega328p)
 - Amiga:
    - dev_test 0.6, udp_test 0.6, Roadshow 1.11
    - A500 + ACA500

#### PB Test

    cnt  bytes    err  drop rate
    056F 00202276 0000 0000 0175.39 KB/s rx plipbox
    056E 00201C8C 0000 0000 0201.11 KB/s tx

#### PIO Test

    ip= 192.168.2.222 tgt_port= 6800 src_port= 6800 data_size 1400 count= 1000 delay= 0 verbose= False
    @1000: d=  8.91 ms  v=314.27 KB/s

#### Bridge Test (Mode 0)

    ip= 192.168.2.222 tgt_port= 6800 src_port= 6800 data_size 1400 count= 100 delay= 0 verbose= False
    @100: d= 27.28 ms  v=102.65 KB/s

#### Bridge Test (Mode 1)

    ip= 192.168.2.222 tgt_port= 6800 src_port= 6800 data_size 1400 count= 100 delay= 0 verbose= False
    @100: d= 30.15 ms  v= 92.88 KB/s

#### Bridge

    ip= 192.168.2.42 tgt_port= 6800 src_port= 6800 data_size 1400 count= 1000 delay= 0 verbose= False
    @1000: d= 38.99 ms  v= 71.81 KB/s

EOF
