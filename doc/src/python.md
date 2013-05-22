plipbox: Python Emulator
========================

1. Introduction
---------------

Recently, I had some time to spend and wanted to work on the [plipbox][pb]
project's Amiga driver. Unfortunately, I only had my MacBook Pro with me and no
Amiga or plipbox hardware.

What does a SW developer do if the real hardware is not at hand?

Yes, you write an emulator that represents the missing HW in SW on a machine
you have access to...

### Adding a Virtual Parallel Port Protocol in FS-UAE

That said, I installed all required Amiga SW in a virtual environment in
[FS-UAE][fs] on my Mac and soon I had my **plipbox.device** SANA II driver
talking with the virtual parallel port in FS-UAE. What was missing now is a
protocol that communicates the complete I/O state from FS-UAE to another
process. Then I could write a standalone application that does emulate the
plipbox firmware and plipbox.device in FS-UAE would be operational.

I defined a simple two-byte bi-directional protocol called [**vpar**][vp] that
communicates any change in the emulation of FS-UAE's Amiga parallel port to an
external process and also receives external changes for input lines from the
process and realizes them in FS-UAE's emulated parallel port.

Having a POSIX compatible system with pseudo-terminals (ptys) I devised the
link between both processes as a simple file-open interface in FS-UAE and a pty
in the plipbox emulator (Note: any other interprocess mechanism works here also
but this one was very easy to add in FS-UAE).

The setup looks like this:

        +----------+                                  +---------+
        | plipbox  |                                  | patched |
        | emulator |--(opens)--> [PTY] <--(file I/O)--| FS-UAE  |
        +----------+                                  +---------+
                    <--------- vpar protocol -------->

All required changes to FS-UAE are available in my clone of the FS-UAE
repository on [GitHub][gh] in the `chris-devel` branch.

For the plipbox emulator application I chose my favorite script language
Python. With it setting up the pty for inter-process communication and
implementing the vpar protocol peer was done in a few hours.

[pb]: http://lallafa.de/blog/amiga-projects/plipbox
[fs]: http://fs-uae.net
[vp]: https://github.com/cnvogelg/fs-uae-gles/blob/chris-devel/vpar.md
[gh]: https://github.com/cnvogelg/fs-uae-gles/tree/chris-devel

### Setting up the Network

Ok, the plipbox emulator in Python can now talk PLIP via vpar's virtual
parallel port. The next step is to add access to an Ethernet device on the
lowest frame level for our plipbox emulator.

I had a look at the [pcap][pc] library that allows injecting packets into
an existing adapter, but the Python bindings often lacked the inject feature
of the library and so I dropped this idea.

Then I found a setup using a [TAP][tt] device to easily create and receive
ethernet packets from a user space application. Combine this with a real
ethernet adapter via an interface bridge and you have access to real ethernet
with a simple TAP file I/O interface (opening `/dev/tap0`) from your
application.

The setup looks like this:

        +----------+              +---------+                +----------+
        | Real Eth |              | TAP     |                | plipbox  |
        | Adapter  |<-- Bridge -->| Adapter |<-- File I/O -->| emulator |
        +----------+              +---------+                +----------+
            en1         bridge0       tap0
            +----- OS net interfaces ----+

For the real ethernet adapter I use a second USB ethernet adapter on my Mac
that is not configured (IP: 0.0.0.0) but active (up) in Mac OS X. This way OS X
does not use it and my emulator can use it exclusively.

Creating the TAP device and building the bridge with the real adapter is done
with some sudo'ed system command right in the startup of the emulator.

Note: the TAP driver is not shipped with Mac OS X itself, but you can add it by
installing the ones from the [TUN TAP OSX][to] project.

[pc]: http://www.tcpdump.org  
[tt]: http://en.wikipedia.org/wiki/TUN/TAP
[to]: http://tuntaposx.sourceforge.net

### Putting it all together

On top of the vpar Python module I implemented the magPLIP protocol layer in
an own Python class. Now all pieces to create the plipbox were available.

The plipbox main script sets up the vpar, magplip, and ethernet with TAP and
interface bridge. In the main loop it does I/O multi-plexing with select() on
the file descriptor for the vpar PTY and for the TAP file. 

A packet arriving from TAP is filtered (i.e. multicast and unknown broadcasts
are removed) similar to the real plipbox firmware and then delivered to the
emulated Amiga via the vpar connection.

A packet incoming via PLIP is directly delivered to the local network and thus
written to the TAP file.

That's it! A working plipbox running as a Python script on my Mac... 
Similar to the firmware I added some optional debugging output to see the
packet contents and to measure the timing and latency.

2. Setup
--------

If you want to setup your own plipbox emulation you'll need the following
prerequisites. Please note that the current approach relies on PTYs, TAP and
bridge network devices and thus requires an OS supporting these cool things.
I currently tested only Mac OS X and Linux. 

### Common Setup

- On all platforms you'll need a Python 2.7 without any special packages
  installed

### Mac OS X Installation

- First install the [TUN TAP OS X driver][tt] 

- The new TAP device files need special access rights to use them. I added my
  user to group `wheel` to gain access to these files:

        # dscl . -append /groups/wheel GroupMembership $LOGNAME

- plipbox sets up the interface bridge with some ifconfig commands that require
  root privileges. The script calls **sudo** to perform these actions. You can
  adjust your `/etc/sudoers` file with `visudo` to gain access without entering
  a password each time:

        %admin  ALL=(ALL) NOPASSWD: ALL

- Make sure that the ethernet interface you want to bridge is not configured
  for IP (IPv4 address is 0.0.0.0) but enbaled (i.e. up)

[tt]: http://en.wikipedia.org/wiki/TUN/TAP

### Linux Installation

- similar to OSX setup **sudo** to call commands without password

- the Linux kernel needs TUN/TAP support with the `tun` kernel module. Modern
  Linux systems ship this module already. If this module is not installed you
  can install it with `insmod tun`. Just make sure to find the device node
  `/dev/net/tun` in your file system.

- you need the `tunctl` tool. On Debian/Ubuntu systems it is available in the
  `uml-utilities` package.
  
- you need the `brctl` ethernet bridge tool. On Debian/Ubuntu systems it is
  available in the `bridge-utils` package.

3. Usage
--------

### Running plipbox

You typically run the plipbox emulator with:

      > cd python
      > ./plipbox -e <ethernet_if>

The `-e` option is required to set the ethernet address you want to use for
bridging. On Mac OS X this interface is typically named `en*`, on Linux it is
`eth*`.

You can quit plipbox by pressing `CTRL+C`. This will shut down all resources
including TAP and bridge interface and removes the vpar PTY.

### Running FS-UAE with vpar support

When the plipbox emulator is started it will wait for the connect from FS-UAE.
So start this tool next. Make sure to use my patched 
[FS-UAE with vpar support][gh]. In your FS-UAE configuration add the following 
line to enable vpar support:

      parallel_port = raw:/tmp/vpar

The path `/tmp/vpar` is the default link path that will be created by plipbox
and points to the PTY that was opened by plipbox for vpar communication. You
can change this path with the `-l` option.

You can set the environment variable `VPAR_DEBUG=1` before running FS-UAE to
see low-level vpar debug information similar to the `-d` debug switch in the
plipbox emulator.

If you quit FS-UAE then plipbox will be notified and shut down, too.

[gh]: https://github.com/cnvogelg/fs-uae-gles/tree/chris-devel

### Plipbox options

The following **plipbox options** are available:

- **-v**: enable verbose output. Shows incoming and outgoing packets.
- **-d**: enable debug output. Show the vpar and magplip diagnose messages.
  This allows you to diagnose the low level parallel protocol.
- **-e <ifname>**: Name of ethernet device to be used for bridging.
- **-t <ifname>**: Name of TAP device that will be created for packet generation
  from virtual PLIP. 
- **-l <path>**: The link that will point to the PTY that FS-UAE with vpar
  support can connect to. 
- **-E**: disable filtering of packets arriving from Ethernet
  
EOF
