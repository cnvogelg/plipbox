from impacket.ImpactDecoder import EthDecoder
from impacket.ImpactPacket import *

from ethframe import MacAddress

class PacketDecoder:

  def __init__(self, my_mac):
    self.decoder = EthDecoder()
    self.my_mac = my_mac
    self.my_ip = None
    self.my_mac_name = "plipbox_mac_addr"
    self.my_ip_name = "plipbox_ip"

  def decode_raw_pkt(self, raw_pkt):
    # decode ethernet packet
    eth_pkt = self.decoder.decode(raw_pkt)
    pkt = eth_pkt.child()

    # IP packet
    if isinstance(pkt, IP):
      src_ip = self._map_ip(pkt.get_ip_src())
      tgt_ip = self._map_ip(pkt.get_ip_dst())
      sub_pkt = pkt.child()
      if isinstance(sub_pkt, TCP):
        src_port = sub_pkt.get_th_sport()
        tgt_port = sub_pkt.get_th_dport()
        t = "[TCP %s:%d -> %s:%d" % (src_ip, src_port, tgt_ip, tgt_port)
        t += " win=%s seq=%s" % (sub_pkt.get_th_win(), sub_pkt.get_th_seq())
        if sub_pkt.get_ACK():
          t += " ack=%s " % sub_pkt.get_th_ack()
        t += self._get_tcp_flags(sub_pkt) + "]"
        return t
      elif isinstance(sub_pkt, UDP):
        src_port = sub_pkt.get_uh_sport()
        tgt_port = sub_pkt.get_uh_dport()
        t = "[UDP %s:%d -> %s:%d]" % (src_ip, src_port, tgt_ip, tgt_port)
        return t
      elif isinstance(sub_pkt, ICMP):
        it = sub_pkt.get_icmp_type()
        type_name = sub_pkt.get_type_name(it)
        t = "[ICMP %s %s -> %s]" % (type_name, src_ip, tgt_ip)
        return t
      else:
        return None

    # ARP packet
    elif isinstance(pkt, ARP):
      op = pkt.get_ar_op()
      op_name = pkt.get_op_name(op)
      sha = pkt.as_hrd(pkt.get_ar_sha())
      tha = pkt.as_hrd(pkt.get_ar_tha())
      spa = pkt.as_pro(pkt.get_ar_spa())
      tpa = pkt.as_pro(pkt.get_ar_tpa())

      # assign my ip
      if MacAddress(pkt.get_ar_tha()) == self.my_mac:
        self.my_ip = tpa

      sham = self._map_mac_addr(sha)
      tham = self._map_mac_addr(tha)
      spam = self._map_ip(spa)
      tpam = self._map_ip(tpa)
      t = "[ARP %s %s (%s) -> %s (%s)]" % (op_name, spam, sham, tpam, tham)

      return t
      
    # unknown packet
    return None

  def _map_mac_addr(self, mac_str):
    if mac_str == self.my_mac.__str__():
      return self.my_mac_name
    else:
      return mac_str

  def _map_ip(self, ip):
    if ip == self.my_ip:
      return self.my_ip_name
    else:
      return ip

  def _get_tcp_flags(self, pkt):
    t = ''
    if pkt.get_ECE():
      t += ' ece'
    if pkt.get_CWR():
      t += ' cwr'
    if pkt.get_ACK():
      t += ' ack'
    if pkt.get_FIN():
      t += ' fin'
    if pkt.get_PSH():
      t += ' push'
    if pkt.get_RST():
      t += ' rst'
    if pkt.get_SYN():
      t += ' syn'
    if pkt.get_URG():
      t += ' urg'
    return t
