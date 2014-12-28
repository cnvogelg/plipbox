import sys
import os
import fcntl
import struct
from oshelper import OSHelper
from netif import NetIf

class Tap:
  """Code for handling a TAP device"""

  def __init__(self, ifname='tap0', oshelper=None, netif=None):
    if oshelper is None:
      oshelper = OSHelper()
    if netif is None:
      netif = NetIf(oshelper)
    self._osh = oshelper
    self._netif = netif
    self._ifname = ifname

  def open(self):
    if sys.platform == 'darwin':
      # Mac OS X needs TUN/TAP OSX and
      # user must be member of group 'wheel'
      self._name = "/dev/" + self._ifname
      if not os.path.exists(self._name):
        return -1
      self._fd = os.open(self._name, os.O_RDWR)
      return self._fd
    elif sys.platform == 'linux2':
      # Linux needs 'tunctl' tool and user needs sudo access
      ret = self._osh.tunctl('-t', self._ifname,'-u', str(os.getuid()))
      if ret != 0:
        return -1
      # up interface
      ret = self._netif.if_up(self._ifname)
      if ret != 0:
        return -1
      # now open tap
      TUNSETIFF = 0x400454ca
      IFF_TAP = 0x0002
      IFF_NO_PI = 0x1000
      self._fd = os.open('/dev/net/tun', os.O_RDWR)
      fcntl.ioctl(self._fd, TUNSETIFF,
                  struct.pack("16sH", self._ifname,
                              IFF_TAP | IFF_NO_PI))
      return self._fd
    else:
      raise NotImplementedError("unsupported platform!")

  def close(self):
    os.close(self._fd)
    if sys.platform == 'linux2':
      # use 'tunctl' to remove tap
      ret = self._osh.tunctl('-d', self._ifname)
      return ret
    else:
      return 0

  def read(self, size, timeout=None):
    if timeout is not None:
      ready = select.select([self._fd], [], [], timeout)[0]
      if len(ready) == 0:
        return None
    return os.read(self._fd, size)

  def write(self, buf):
    return os.write(self._fd, buf)

  def get_fd(self):
    return self._fd


# ----- test -----
if __name__ == '__main__':
  tap = Tap()
  fd = tap.open()
  print "open", fd
  ret = tap.close()
  print "close", ret
