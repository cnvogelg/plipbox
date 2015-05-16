import serial
import packet

class Device:
  """A serial device that sends/receives packets in pbserial format"""

  def __init__(self, serial_dev):
    self.serial_dev = serial_dev
    self.serial = None
    self.pkt = packet.Packet()

  def open(self):
    self.serial = serial.Serial(self.serial_dev, timeout=2)

  def close(self):
    self.serial.close()
    self.serial = None

  def send(self, data):
    self.pkt.set_payload(data)
    raw = self.pkt.encode()
    self.serial.write(raw)

  def can_recv(self):
    return self.serial.inWaiting() > 0

  def recv(self):
    hdr = self.serial.read(self.pkt.HDR_SIZE)
    if len(hdr) == 0:
      return None
    size = self.pkt.decode_header(hdr)
    data = self.serial.read(size)
    return data

  def __enter__(self):
    """for use in 'with'"""
    self.open()
    return self

  def __exit__(self, type, value, traceback):
    """for use in 'with'"""
    self.close()
