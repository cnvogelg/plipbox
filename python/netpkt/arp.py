from impacket.ImpactPacket import *

ETHER_BROADCAST = (0xff,) * 6

def gen_arp_self_request(mac_addr, ip_addr):
  """generate a gracious (self-referencing) ARP request
     to tell all others that we have this ip"""

  # build ethernet frame
  eth = Ethernet()
  eth.set_ether_type(0x0806)          # this is an ARP packet
  eth.set_ether_dhost(ETHER_BROADCAST)# destination host (broadcast)
  eth.set_ether_shost(mac_addr)

  # build ARP packet
  arp = ARP()
  arp.set_ar_hrd(1)
  arp.set_ar_hln(6)                   # ethernet address length = 6
  arp.set_ar_pln(4)                   # ip address length = 4
  arp.set_ar_pro(0x800)               # protocol: ip
  arp.set_ar_op(1)                    # opcode: request
  arp.set_ar_tha(ETHER_BROADCAST)     # target hardware address (broadcast)
  arp.set_ar_tpa(ip_addr)             # gracious ARP
  arp.set_ar_sha(mac_addr)            # source hardware address
  arp.set_ar_spa(ip_addr)             # source protocol address
  eth.contains(arp)

  # return raw bytes
  return eth.get_packet()

def handle_arp(eth_pkt, mac_addr, ip_addr):
  """check if incoming pkt is an ARP_REQUEST for my mac
     if yes then generate a ARP_REPLY and return the raw packet.
     all other passed packets simply return None"""
  pkt = eth_pkt.child()
  # check packet type
  if not isinstance(pkt, ARP):
    return None
  # check ARP operation
  op = pkt.get_ar_op()
  if op != 1: # request
    return None
  # check for IP
  pro = pkt.get_ar_pro()
  if pro != 0x800:
    return None
  # check tpy
  tpa = pkt.get_ar_tpa()
  if tpa != ip_addr:
    return None

  # build ARP reply
  pkt.set_ar_op(2) # reply
  sha = pkt.get_ar_sha()
  spa = pkt.get_ar_spa()
  pkt.set_ar_tha(sha)
  pkt.set_ar_tpa(spa)
  pkt.set_ar_sha(mac_addr)
  pkt.set_ar_spa(ip_addr)

  # return raw bytes
  return eth.get_packet()
