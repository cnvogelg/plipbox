import arp
import icmp
import udp
import decode

class SimHost:
  """simulate a host with an UDP port by answering ICMP and UDP requests.
     this is done on ethernet packet level. you pass in frames and you will
     eventually receive reply frames.
  """
  def __init__(self, host_mac, host_ip, udp_port, verbose=False):
    self.host_mac = host_mac
    self.host_ip = host_ip
    self.udp_port = udp_port
    self.pd = decode.PacketDecoder()
    if verbose:
      self.pp = decode.PrettyPrinter(host_mac)
    else:
      self.pp = None

  def get_init_packet(self):
    """before entering the handle loop get this init packet and send it"""
    arp_pkt = arp.gen_arp_self_request(self.host_mac, self.host_ip)
    return arp_pkt.get_packet()

  def handle_packet(self, raw_pkt):
    """process incoming packets. return either None or reply raw_pkt"""
    eth_pkt = self.pd.decode_raw(raw_pkt)

    # show incoming packet
    if self.pp is not None:
      one_line = self.pp.pkt_to_oneliner(eth_pkt)
      self._print_verbose(one_line)

    # handle arp request
    reply_pkt = arp.handle_arp(eth_pkt, self.host_mac, self.host_ip)
    if reply_pkt is None:
      # handle potential pings
      reply_pkt = icmp.handle_ping(eth_pkt, self.host_ip)
      if reply_pkt is None:
        # handle UDP
        reply_pkt = udp.handle_udp_echo(eth_pkt, self.host_ip, self.udp_port)

    if reply_pkt is not None:
      # show outgoing packet
      if self.pp is not None:
        one_line = self.pp.pkt_to_oneliner(reply_pkt)
        self._print_verbose(one_line)

      raw_reply_pkt = reply_pkt.get_packet()
      return raw_reply_pkt
    else:
      return None

  def _print_verbose(self, line):
    print(line)
