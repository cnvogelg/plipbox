import util
import packet

class Host:
  """A virtual serial port reading/writing pbserial packets"""

  def __init__(self, link_name):
    self.link_name = link_name
    self.sopty = util.SoPTY(link_name)
    self.pkt = packet.Packet()

  def open(self):
    self.sopty.open()

  def close(self):
    self.sopty.close()

  def recv(self):
    hdr = self.sopty.read(self.pkt.HDR_SIZE)
    if len(hdr) == 0:
      return None
    size = self.pkt.decode_header(hdr)
    data = self.sopty.read(size)
    return data

  def send(self, data):
    self.pkt.set_payload(data)
    self.sopty.write(self.pkt.encode())

  def __enter__(self):
    """for use in 'with'"""
    self.open()
    return self

  def __exit__(self, type, value, traceback):
    """for use in 'with'"""
    self.close()
