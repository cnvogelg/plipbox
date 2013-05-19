plipbox: Python Emulator
========================

1. Introduction
---------------

Recently, I had some time to spend and wanted to work on the [plipbox][pb]
project's Amiga driver. Unfortunately, I only had my MacBook Pro with me and no
real hardware.

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
with a simple TAP file I/O interface (opening `/dev/tap0`) from your application.

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
