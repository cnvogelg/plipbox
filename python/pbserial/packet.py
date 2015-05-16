import struct

class PacketDecodeError(Exception):
  """Error decoding packet"""
  pass

class Packet:
  """A plipbox serial packet has a small fixed header and a raw payload

      +0: u08 MAGIC (0x42)
      +1: u08 VERSION (6)
      +2: u16 size of payload
      ===
      +4: header size
  """

  MAGIC = 0x42
  VERSION = 6
  HDR_SIZE = 4

  def __init__(self, data=None):
    self.payload = data
    if data is not None:
      self.size = len(data)
    else:
      self.size = 0

  def set_payload(self, payload):
    self.payload = payload
    self.size = len(payload)

  def encode(self):
    hdr = struct.pack("BBH", self.MAGIC, self.VERSION, self.size)
    if self.size > 0:
      return hdr + self.payload
    else:
      return hdr

  def decode_header(self, raw_hdr):
    """try to decode header and return size of payload"""
    if raw_hdr is None or len(raw_hdr) < 4:
      raise PacketDecodeError("Wrong header size!")
    (magic, version, size) = struct.unpack("BBH", raw_hdr)
    if magic != self.MAGIC:
      for i in raw_hdr:
        print(ord(i),)
      raise PacketDecodeError("No magic")
    if version != self.VERSION:
      raise PacketDecodeError("Wrong version")
    return size

  def get_size(self):
    return self.size

# test
if __name__ == "__main__":
  # encode packet
  data = "bla"
  pkt = Packet(data)
  raw = pkt.encode()
  # decode header
  hdr = raw[0:Packet.HDR_SIZE]
  npkt = Packet()
  size = npkt.decode_header(hdr)
  if size != len(data):
    print("Wrong SIZE!")
  # get data
  ndata = raw[Packet.HDR_SIZE:]
  if ndata != data:
    print("Wrong DATA!")
  npkt.set_payload(ndata)
  nraw = pkt.encode()
  if nraw != raw:
    print("Wrong RAW!")


