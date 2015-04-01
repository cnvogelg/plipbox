import serial
import struct

class USBSer:
  """allow to send and receive packets via the 'usbser' PIO"""

  def __init__(self, serial_dev):
    self.serial_dev = serial_dev
    self.serial = None

  def open(self):
    self.serial = serial.Serial(self.serial_dev, timeout=2)

  def close(self):
    self.serial.close()
    self.serial = None

  def send(self, data):
    hdr = struct.pack("BBH", 0x42, 0x06, len(data))
    self.serial.write(hdr)
    self.serial.write(data)

  def can_recv(self):
    return self.serial.inWaiting() > 0

  def recv(self, get_size):
    hdr = self.serial.read(4)
    if hdr is None or len(hdr) < 4:
      raise Exception("Read error")
    (magic, version, size) = struct.unpack("BBH", hdr)
    if magic != 0x42:
      raise Exception("No magic")
    if version != 0x06:
      raise Exception("Wrong version")
    data = self.serial.read(size)
    return data

