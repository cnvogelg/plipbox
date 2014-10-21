import traceback
import reader
from ethframe import EthFrame

class EthernetReader(reader.Reader):

  def __init__(self, eth_io, args, log, ts_base, decoder):
    reader.Reader.__init__(self, "eth", args, log, ts_base, decoder)
    # setup virtual ethernet
    try:
      eth_io.open()
    except Exception as e:
      print("veth: error opening:",e)
      sys.exit(1)
    self.eth_io = eth_io

  def tx_pkt(self, raw_buf):
    self.eth_io.tx_pkt(raw_buf)

  def set_env(self, pb, leave_event, online_event, pb_mac):
    self.pb = pb
    self.leave_event = leave_event
    self.online_event = online_event
    self.pb_mac = pb_mac

  def set_mac(self, pb_mac):
    self.pb_mac = pb_mac

  def run(self):
    try:
      self.log("starting ethernet reader")
      timeout = 0.5
      while not self.leave_event.is_set():
        # read packet
        raw_buf = self.eth_io.rx_pkt(timeout=timeout)
        if raw_buf is not None:
          # decode frame
          eth_frame = EthFrame()
          eth_frame.decode(raw_buf)
          # send packet to plipbox
          if not self.args.rx and self.online_event.is_set():
            # check if packet is passed
            keep = self.args.eth_pass or eth_frame.is_for_me(self.pb_mac)
            if keep:
              # submit to par send
              self.pb.tx_pkt(raw_buf)
              tag = None
            else:
              tag = "FILT"
          else:
            tag = "OFF "
          # decode packet
          self.log_packet(eth_frame, raw_buf, tag)
    except KeyboardInterrupt:
      self.log("BREAK [eth]")
      self.leave_event.set()
    except Exception as e:
      self.log("Unexpected error:", e)
      traceback.print_exc()
    finally:
      self.log("closing ethernet reader")
      self.eth_io.close()
      self.log("ethernet done")
