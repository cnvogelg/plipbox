import os
import sys

class Tap:
  def __init__(self, base_name='tap0'):
    if sys.platform == 'darwin':
      self._name = "/dev/" + base_name
      self._fd = os.open(self._name, os.O_RDWR)
    else:
      raise NotImplementedError("unsupported platform!")
      
  def close(self):
    os.close(self._fd)
  
  def read(self, size):
    return os.read(self._fd, size)
    
  def write(self, buf):
    os.write(self._fd, buf)

  def get_fd(self):
    return self._fd