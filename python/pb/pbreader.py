import traceback
import reader
from ethframe import EthFrame

class PlipboxReader(reader.Reader):

  def __init__(self, pb_io, args, log, ts_base, decoder):
    reader.Reader.__init__(self, "PAR", args, log, ts_base, decoder)
    # setup plipbox
    try:
      pb_io.open()
    except Exception as e:
      print("pb: error opening:",e)
      sys.exit(3)
    self.pb_io = pb_io

  def tx_pkt(self, raw_buf):
    self.pb_io.tx_pkt(raw_buf)

  def set_env(self, eth, leave_event, online_event):
    self.eth = eth
    self.leave_event = leave_event
    self.online_event = online_event

  def run(self):
    try:
      self.log("starting plipbox reader")
      timeout = 0.5
      while not self.leave_event.is_set():
        # was a buffer received?
        raw_buf = self.pb_io.rx_pkt(timeout)
        if raw_buf is not None:
          # decode frame
          eth_frame = EthFrame()
          eth_frame.decode(raw_buf)
          n = len(raw_buf)
          # check for magic packets
          if eth_frame.is_magic_online():
            my_mac = eth_frame.src_mac
            self.log("magic: online: ", my_mac, True)
            tag = "M:ON"
            self.decoder.my_mac = my_mac
            self.eth.set_mac(my_mac)
            self.online_event.set()
          elif eth_frame.is_magic_offline():
            self.log("magic: offline", True)
            tag = "M:OF"
            self.online_event.clear()
          elif not self.args.rx and self.online_event.is_set():
            try:
              tag = None
              self.eth.tx_pkt(raw_buf)
            except OSError as e:
              self.log(e)
              tag = "ERR!"
          else:
            tag = "OFF "
          # decode packet
          self.log_packet(eth_frame, raw_buf, tag)
    except KeyboardInterrupt:
      self.log("BREAK [pb]")
      self.leave_event.set()
    except Exception as e:
      self.log("Unexpected error:", e)
      traceback.print_exc()
    finally:
      self.log("closing plipbox")
      self.pb_io.close()
      self.log("plipbox done")
