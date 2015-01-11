plipbox: Amiga Driver Documentation
===================================

0. Changes
----------

* 2011-08-05  _38.1a_  plip2slip support
* 2012-06-03  _38.1b_  plipbox support
* 2013-04-06  _0.3_    introduced own plipbox.device ethernet device
* 2013-05-19  _0.3_    converted to markdown. added more stacks
* 2015-01-11  _0.6_    adapted to new file names

1. Supported Network Stacks
---------------------------

The following network stacks have been successfully tested with plipbox:

* AmiTCP 3.0b2
* Genesis (OS3.9)
* MiamiDX (from [Classic Workbench][cwb])
* [Roadshow 1.11][rs] 

[rs]: http://roadshow.apc-tcp.de/index-de.php
[cwb]: http://classicwb.abime.net


2. Setup Hints
--------------

### 2.1 Common Setup

 - All network stacks expect the plipbox device driver in `devs:networks` on
   your system volume. So copy this file first before proceeding.
 
 - Copy `plipbox.device` from this release in the `amiga/bin` directory to your
   Amiga installation. Select the suitable m680x0 version for your hardware
   platform and copy the plipbox.device file without the `RELEASE_000` or
   `RELEASE_020` extension:

        copy plipbox.device_RELEASE_000 devs:networks/plipbox.device

### 2.2 AmiTCP
 
 - For a very simple setup you can use the [Network Boot Disk for Amiga][nwbd]
   and start with this one. You have to modify the vanilla disk as it does
   not contain the plipdox drivers, yet. 
   [nwbd]: http://roadshow.apc-tcp.de/index-de.php
 - Insert this disk into your favorite Amiga emulator and perform the following
   steps to prepare the disk.
 - Copy `plipbox.device` on the disk (see section 2.1)
 - Edit the file `AmiTCP/db/interfaces` and add:
 
        plipbox dev=devs:networks/plipbox.device
 - Adjust your `AmiTCP:bin/startnet` and rename the interface to `plipbox0`.
   On the network boot disk: Edit `df0:s/Prefs/Env-Archive/nbddriver` (EnvARC:)
   and set `plipbox0`
 - In `EnvARC:sana2` (Network Boot Disk: `df0:s/Prefs/Env-Archive`) you can
   place an optional configuration file called `plipbox.config`.
   For options see the plipbox documentation.
   A sample file is available in directory `amiga/src` of this release.
 - Correctly set your Amiga's IP to match your network environment.
   In the network boot disk use `EnvARC:nbdip` to set the Amiga's IP and
   use `EnvARC:nbdgw` to set the gateway of your network.
 - Do not forget to configure the correct DNS server otherwise you won't be
   able to resolve non-numeric IP addresses. Have a look at the file
   `AmiTCP:db/netdb-myhost` and adapt the `NAMESERVER` entry accordingly.
   Use the DNS server of your local network here. (in my setup 192.168.2.1).
   The network boot disk uses the IP address stored in the `EnvARC:nbdns` file.

### 2.3 Genesis

  - Genesis is very similar to the AmiTCP setup (its more or less a UI for
    AmiTCP and uses its core for the network stuff) 
  - Copy the plibpox.device to your system (see section 2.1)
  - Open the `GenesisPrefs`
  - In `Interfaces` select `New`
    - In `Interface` Tab enter:
      - Enter the interface `Name: plipbox0`
      - Enter `IP Address, Gateway, Netmask` to match your local network
    - In `SANA II` Tab enter:
      - `Specify SANA II device: devs:networks/plipbox.device`  
    - Confirm interface creation with `Okay`
    - In `Resolve` Tab do not forget to name your DNS servers and set your
      domain name(s) 
  - Note: `dynamic` setup does not work with plipbox. You must either use static 
    or an external [DHCP client][dhcp] suitable for AmiTCP/Genesis.
  - Now you can start `Genesis` and enable your new `plipbox0` interface

[dhcp]: http://aminet.net/package/comm/tcp/dhcp_amitcp

### 2.4 MiamiDX

  - Launch `MiamiDX`
  - Select `Hardware` Tab and click `New`:
    - In `Select Hardware Type` pick `Ethernet`
    - Enter a name for the hardware, e.g. `Name: plipbox`
    - Keep `Type: SANA II`
    - Pick plipbox.device: `Driver: devs:networks/plipbox.device`
    - Confirm with `Ok`
  - Select `Interfaces` Tab and click `New`:
    - Pick `Interface Type: Ethernet`
    - Pick `Interface connection: LAN`
    - Select your hardware: `plipbox`
    - You can either configure your Amiga statically or with DHCP: Select
    `static` or `dynamic` in `IP Type, Netmask Type, Gateway Type`. Enter
    your network parameters in static mode.
    - Note: multicast is not supported in plipbox. Therefore, keep 
    `Multicast: disabled`.
    - Note: Configure DHCP in `TCP/IP Settings...` to fetch DNS servers, too.
  - In `Databases` Tab select Table `DNS servers` and add your static DNS
  server IPs (if you don't use dynamic DNS via DHCP)
  - Do not forget to save your settings with Amiga+S or `Menu: Settings -> Save`
  - Now you can go online with your new interface `plipbox`
  - If you use multiple plipbox devices in a single network then you have to
  set a unique MAC address for each one. Select `Hardware Tab` and double
  click your `plipbox` entry. Now select `SANA-II Parameters` and enter a
  new MAC address in the `Hardware address` field.

### 2.5 Roadshow

  - Begin with copying the plipbox device driver (see section 2.1)
  - Then you need an interface configuration file for the plipbox device.
  Simply copy a template from `Storage/NetInterface` and modify this one:
  
        copy sys:Storage/NetInterfaces/cnet devs:NetInterfaces/plipbox

  - Now adjust the following values in the file (use either static or DHCP section!):

        device=plipbox.device
        # -- DHCP --
        configure=dhcp
        # -- static --
        address=<your ip>
        netmask=<your mask>

  - Have a look in `devs:internet/name_resolution` to set your DNS servers and
  your domain name.
  - After a reboot the interface will be brought up automatically in the
  `User-Startup` section of Roadshow
  - For testing you can activate/de-activate the interface with:
  
        > addnetinterface devs:netinterfaces/plipbox
        > ... use network ...
        > netshutdown

  - To make sure that the plipbox device goes offline if you shut down the net
  you have to add the following options to your interface configuration file:
  
        downgoesoffline=yes
        
  - If you use multiple plipbox devices in a single network then you have to 
  assign them unique MAC addresses. You can set the MAC address of your plipbox
  with the following option in your Roadshow device configuration file:
  
        hardwareaddress=1a:11:a1:a0:00:01

  - A complete device config looks like:

        device=plipbox.device
        configure=dhcp
        downgoesoffline=yes
        hardwareaddress=1a:11:a1:a0:00:01


3. plipbox.device Configuration
-------------------------------

While the plipbox.device is in general zero-config and needs no adjustement,
you can control some options via a configuration file.

The text file needs to be called:

        ENV:SANA2/plipbox.config

Store your file in the environment archive to have it available after the
next reboot, too:

        ENVARC:SANA2/plipbox.config

The following options are supported:

  - **NOBURST** (switch /S) (default: burst on)
    - Starting with version 0.6 a fast burst mode is used for parallel port
      data transfer. It is always recommended to use this mode. However, if
      you experience problems with fast transfers then you can use this
      option to fall back to the old transfer protocol. Its slower but
      more reliable.

  - **TIMEOUT** (numerical key /K/N) (default: 500 * 1000) (unit: microseconds)
    - The parallel transfer uses time outs to detect error conditions.
    - Use this value to adjust timing.
 
  - **NOSPECIALSTATS** (switch /S) (default: special stats on)
    - The SANA-II device tracks statistics information.
    - Use this switch to disable the extra statistics information that is
      recorded during normal operation of the device.

  - **PRIORITY** (numerical key /K/N) (default: 0) (unit: AmigaOS task prio)
    - A server task is used in the plipbox.device to handle the parallel port
      transfers.
    - Use this value to alter the scheduling priority of the send/receive
      task.

  - **BPS** (numerical key /K/N) (default: 60 * 1024 * 8) (unit: bits/second)
    - A SANA-II device reports a bitrate per second value as an indication
      for the achievable transfer speed of the network device.
    - Use this parameter to adjust the speed that is reported to the TCP/IP
      stack from the device.

  - **MTU** (numerical key /K/N) (default: 1500) (unit: bytes)
    - The maximum transfer unit is the maximum number of bytes a device can 
      transfer in a single packet. By default its 1500 and exactly matches
      the MTU that Ethernet uses.
    - For the plipbox the MTU size is also the size of the parallel transfers.
    - Use this parameter to set a smaller MTU and tune the transfer rates.
    - Note that a non 1500 MTU will cause fragmentation on upper levels.


4. Build plipbox.device from Source
-----------------------------------

 - Only for advanced users! All others can use the supplied binaries!
   You really need to recompile the binaries only if you want to modify
   or enhance them.
 - I cross-compile the binaries here with vamos running the SAS C V6.58 compiler
 - Install [vamos][v] on your Mac or PC
 - Copy the following Amiga Directories either from a real machine or an emulator
   to a directory on your Mac. The directory is named `$HOME/amiga/shared`
   here but you can use another directory as well but need to adjust `AMIGA_DIR`
   in `amiga/src/makefile`!

        ~/amiga/shared:
          wb310             system root of a Workbench 3.1 HD Installation
          sc                complete installation directory of SAS C 6.58
          AmiTCP-SDK-4.3    AmiTCP SDK

   The [AmiTCP-SDK-4.3][sdk] is available on Aminet
 - Enter directory `amiga/src` of this release
 - Build on your Mac or PC shell with:

        > make dist       build release files
        > make all        build without optimization
        > make opt        build with optimization
        > make clean      remove files of current build
        > make clean_dist remove all build files

 - The resulting files can then be found in `amiga/bin`

[v]: http://lallafa.de/blog/amiga-projects/amitools/vamos/
[sdk]: http://aminet.net/package/comm/tcp/AmiTCP-SDK-4.3

EOF