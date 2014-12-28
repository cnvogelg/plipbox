from impacket.ImpactDecoder import EthDecoder
from impacket.ImpactPacket import *

class PacketDecoder:
  """decode a raw packet into a structure of transport layers
     using impacket lib
  """
  def __init__(self):
    self._decoder = EthDecoder()

  def decode_raw(self, raw_pkt):
    return self._decoder.decode(raw_pkt)


class PrettyPrinter:
  """convert packet contents into a one liner"""

  def __init__(self, my_mac):
    self.my_mac = my_mac
    self.my_ip = None
    self.my_mac_name = "plipbox_mac_addr"
    self.my_ip_name = "plipbox_ip"
    self.tcp_map = {}
    self.tcp_num = {}
    self.counter = 0

  def pkt_to_oneliner(self, eth_pkt):
    # get layer contained in ethernet packet
    pkt = eth_pkt.child()

    # IP packet
    if isinstance(pkt, IP):
      src_ip = self._map_ip(pkt.get_ip_src())
      tgt_ip = self._map_ip(pkt.get_ip_dst())
      sub_pkt = pkt.child()
      if isinstance(sub_pkt, TCP):
        src_port = sub_pkt.get_th_sport()
        tgt_port = sub_pkt.get_th_dport()
        src_key = "%s:%d" % (src_ip, src_port)
        tgt_key = "%s:%d" % (tgt_ip, tgt_port)
        key = (src_key, tgt_key)
        self._report_tcp(key, sub_pkt)
        t = "[TCP %s:%d -> %s:%d" % (src_ip, src_port, tgt_ip, tgt_port)
        tid = self._get_tcp_id(key)
        seq = self._map_tcp_seq(key, sub_pkt.get_th_seq())
        t += " (%s) seq=%s" % (tid, seq)
        if sub_pkt.get_ACK():
          ack = self._map_tcp_ack(key, sub_pkt.get_th_ack())
          t += " ack=%s" % ack
        t += " {%d}" % sub_pkt.get_th_win()
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
        t = "[ICMP %s(%d) %s -> %s]" % (type_name, it, src_ip, tgt_ip)
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
      if pkt.get_ar_tha() == self.my_mac:
        self.my_ip = tpa

      sham = self._map_mac_addr(sha)
      tham = self._map_mac_addr(tha)
      spam = self._map_ip(spa)
      tpam = self._map_ip(tpa)
      t = "[ARP %s %s (%s) -> %s (%s)]" % (op_name, spam, sham, tpam, tham)

      return t
      
    # unknown packet
    return None

  def _report_tcp(self, tcp_key, tcp_pkt):
    if tcp_pkt.get_SYN():
      seq = tcp_pkt.get_th_seq()
      self.tcp_map[tcp_key] = seq
      if not tcp_pkt.get_ACK():
        self.counter += 1
        self.tcp_num[tcp_key] = self.counter

  def _map_tcp_seq(self, tcp_key, seq):
    if tcp_key in self.tcp_map:
      seq_base = self.tcp_map[tcp_key]
      return seq - seq_base
    else:
      return seq

  def _map_tcp_ack(self, tcp_key, ack):
    swap_key = (tcp_key[1], tcp_key[0])
    if swap_key in self.tcp_map:
      ack_base = self.tcp_map[swap_key]
      return ack - ack_base
    else:
      return ack

  def _get_tcp_id(self, tcp_key):
    if tcp_key in self.tcp_num:
      return "%di" % self.tcp_num[tcp_key]
    else:
      swap_key = (tcp_key[1], tcp_key[0])
      if swap_key in self.tcp_num:
        return "%do" % self.tcp_num[swap_key]
      else:
        return "??"

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
