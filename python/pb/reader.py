import time

class Reader:

  def __init__(self, name, args, log, ts_base, decoder=None):
    self.name = name
    self.args = args
    self.log = log
    self.decoder = decoder
    self.ts_base = ts_base

  def log_packet(self, eth_frame, raw_pkt, tag):
    # nothing to do
    if not self.args.verbose:
      return
    # filter all
    if not self.args.all_packets and tag is not None:
      return
    # tag == None -> ok
    if tag is None:
      tag = " ok "
    # get a timestamp
    t = time.time() - self.ts_base
    # try to decode packet
    if self.decoder is not None:
      info = self.decoder.decode_raw_pkt(raw_pkt)
    else:
      info = None
    # if decoding fails use eth frame
    if info is None:
      info = eth_frame.__str__()
    self.log("%12.6f %s: [%4d] [%s]" % (t, self.name, len(raw_pkt), tag), info)
