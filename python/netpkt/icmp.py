from impacket.ImpactPacket import *

def handle_ping(eth_pkt, my_ip):
  """check if incoming pkt is a ICMP_ECHO_REQUEST(8) to my ip.
     if yes then generate a ICMP_ECHO_REPLY(0) raw packet and return it.
     all other passed packets simply return None"""
  ip_pkt = eth_pkt.child()
  if not isinstance(ip_pkt, IP):
    # no IP packet
    return None
  icmp_pkt = ip_pkt.child()
  if not isinstance(icmp_pkt, ICMP):
    # no ICMP packet
    return None
  # get ICMP TYPE
  it = icmp_pkt.get_icmp_type()
  if it != ICMP.ICMP_ECHO:
    # no ECHO
    return None
  # check target ip
  my_ip_str = ".".join(map(str,my_ip))
  tgt_ip = ip_pkt.get_ip_dst()
  if tgt_ip != my_ip_str:
    return None

  # ok - a ping for use -> convert to reply
  icmp_pkt.set_icmp_type(ICMP.ICMP_ECHOREPLY)
  # swap IP
  src_ip = ip_pkt.get_ip_src()
  ip_pkt.set_ip_dst(src_ip)
  ip_pkt.set_ip_src(tgt_ip)
  # swap eth
  src_mac = eth_pkt.get_ether_shost()
  tgt_mac = eth_pkt.get_ether_dhost()
  eth_pkt.set_ether_shost(tgt_mac)
  eth_pkt.set_ether_dhost(src_mac)

  # enforce checksum
  icmp_pkt.set_icmp_cksum(0)
  icmp_pkt.auto_checksum = 1

  # return new packet
  return eth_pkt
