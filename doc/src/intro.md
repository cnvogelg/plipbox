plipbox: Introduction
=====================

This document gives a technical introduction on how the plipbox device operates.

The plipbox device firmware implements a bridge for network packets arriving
on the parallel port of the Amiga (called PLIP) and the local Ethernet connected
to the plipbox hardware.

1. NAT device
-------------

Version 0.1 and 0.2 of plipbox implemented a NAT device between the parallel
PLIP interface and the Ethernet adapter.

A NAT device connects two distinct IP networks, i.e. both the PLIP and Ethernet
side of the plipbox have different IP networks and thus the plipbox has to 
have two addresses (one for each sub net):

                             +-----plipbox-----+
                             |          1a:11:a1:a0:47:11        
        +-------+   IP  +-----------+     +----------+ ARP/IP
        | Amiga | <---> | PLIP Port |-NAT-| Eth Port | <---> Local Network
        +-------+  p2p  +-----------+     +----------+  lan
       192.168.0.1      192.168.0.2      192.168.2.133        192.168.2.x
      magplip.device         |                 |        mask: 255.255.255.0
                             +-----plipbox-----+          gw: 192.168.2.1
                                                  

In this setup you have to configure quite a lot: You have to set your Amiga IP
and the PLIP port IP in an own sub net. Additionally, you have to set your
Ethernet port IP to an address suitable for your local network. If you have a
DHCP server in your local network then the plipbox firmware can use this
protocol to automatically acquire the Ethernet port IP address and sub net mask.

On the Amiga side you have to select a point-to-point networking setup in your
TCP/IP software stack. This defines one address for the magPLIP interface
(here 192.168.0.1) and one for the PLIP Port in plipbox (here 192.168.0.2).
All packets sent from the Amiga will arrive at the gateway on the PLIP Port.

Now NAT takes place and the sender addresses are replaced in all IP packets
before sending them to the Ethernet: A packet originating from the Amiga
(192.168.0.1) is sent with the Eth Port as sender (192.168.2.133) to the
local network.

If a reply IP packet arrives at the plipbox's Eth Port then NAT is reversed
and replaces the destination address of the Eth Port (192.168.2.133) with
the one of the Amiga (192.168.0.1). Then the packet is sent via PLIP to the
Amiga.

Note that in contrast to other NAT devices here no port remapping is done:
We NAT only a single machine (the Amiga) and we don't use our own ports locally.
This simplifies NATting a bit as not port maps need to be managed.

On the point-to-point link only IP packets are transported. All other types
especially ARP packets make no sense here as a HW address to IP mapping is not
required here. Unfortunately, some Amiga TCP/IP stacks do not support p2p links
very well and try to perform ARP here, too :/

However, ARP must be spoken on Eth Port to map local IP addresses to the
Ethernet MAC addresses. Whenever an IP packet (after NAT) is ready to be sent
out via Ethernet then we have to find out what Ethernet MAC address has the
destination. If the destination IP is on the local sub net (determined with the
netmask of the local network) then ARP is used to find the corresponding MAC.
Otherwise we ARP the gateway (gw) IP address on the local sub net and send
the packet there.

To make ARPing more performant plipbox keeps an ARP cache. This is a table
of IP to MAC address mappings that were already used on the local sub net.

The MAC address of the local ethernet adapter on the plipbox is set by the
firmware. The default firmware already ships with a pre-defined address. If
you use multiple plipboxes in one local sub net then you have to alter this
address for individual devices.

Regarding MTU size this approach actually can apply two sizes, one for each sub
net. While the Ethernet is typically set to 1500 the PLIP link is not limited
to this value and can use arbitrary ones. However, if the PLIP MTU is larger
than 1500 then the plipbox needs to fragment the packets but this is currently
not implemented.

In summary, the NAT approach of the firmware works well but needs quite a lot
of configuration. Furthermore, the NATing applied causes trouble to protocols
that use IP addresses inside the protocol. Most notably, the FTP needs to use
passive mode in this setup to work correctly.

Another cause for trouble in this setup is the NAT that is already applied in
most local home networks when reaching the Internet. E.g. a DSL router NATs all
outgoing traffic. So a double NAT is applied to all Amiga packets when they run
through plipbox, local network, and finally the Internet.

2. MAC bridge
-------------

Starting with version 0.3 plipbox the NAT approach is replaced by a MAC bridge.
This bridge is more "transparent" than NATting and should overcome the problems
associated with NAT.

The old Amiga network device (magplip.device) that offered a point-to-point
link is replaced with a new device driver (plipbox.device) that announces an
Ethernet compatible device. The TCP/IP stack on the Amiga now directly
generates Ethernet packets with a 1500 MTU and transfers them to the plipbox.
There the frame is directly (i.e. without any alteration) passed to the
Ethernet adapter and send to the local network.

Also any incoming frame on the local network is passed to the Amiga plipbox
device driver and deliverd as an Ethernet packet to the TCP/IP stack.

                               +-----plipbox-----+
        1a:11:a1:a0:47:11      |            1a:11:a1:a0:47:11        
          +-------+ ARP/IP +-----------+      +----------+ ARP/IP
          | Amiga | <----> | PLIP Port |-COPY-| Eth Port | <---> Local Network
          +-------+        +-----------+      +----------+  lan
         192.168.2.133         |                 |              192.168.2.x
        plipbox.device         |                 |        mask: 255.255.255.0
                               +-----plipbox-----+          gw: 192.168.2.1
                                                   

Now the plipbox firmware is most transparent and essentially looks like one big
ethernet device attached to the Amiga. In this setup no NAT is applied.

You now configure your Amiga's TCP/IP stack like it is directly attached to the
local network. Give yourself a static IP or use DHCP, set the local network's
net mask and the default gateway. Also the stack now needs to do ARPing to map
the IP addresses to Ethernet MAC addresses and it manages an own ARP cache.

The plipbox firmware is now very slim and almost zero-config. The only
parameter available is the MAC address of the Eth Port. This has a suitable
default value that only needs to be changed if multiple plipboxes are used in a
single local network.

Note that both the plipbox.device driver and the plipbox firmware need to know
the MAC address of the Eth port (here 1a:11:a1:a0:47:11). So if you change the
address then make sure that both parts use the same new address! The device
driver uses the address to generate the correct source address in each frame
while the plipbox uses the address to setup a HW Ethernet filter so that Eth
Port only reports packets suitable for the Amiga.

In summary, with the new approach the Amiga has a bit more to do (e.g. handling
ARP packets) but all problems that were observed with NAT are gone now.

EOF
 