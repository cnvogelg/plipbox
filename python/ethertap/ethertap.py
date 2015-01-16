from __future__ import print_function
from .bridge import Bridge
from .tap import Tap
from .netif import NetIf
from .oshelper import OSHelper

class EtherTapError(Exception):
  """Custom error class for all EtherTap related errors"""
  pass

class EtherTap:
  """create a bridge between an ethernet adapter and a tap device"""

  def __init__(self, eth_if, tap_if=None, bridge_if=None, oshelper=None, netif=None, autoconfig=False):
    """create an EtherTap for the given ethernet interface"""
    if oshelper is None:
      oshelper = OSHelper()
    if netif is None:
      netif = NetIf(oshelper)
    if tap_if is None:
      tap_if = netif.find_unused_tap_netif()
      if tap_if is None:
        raise EtherTapError("can't find unused tap interface!")
    if bridge_if is None:
      bridge_if = netif.find_unused_bridge_netif()
      if bridge_if is None:
        raise EtherTapError("can't find unused bridge interface!")

    self.eth_if = eth_if
    self.tap_if = tap_if
    self.bridge_if = bridge_if
    self._osh = oshelper
    self._netif = netif
    self._tap = None
    self._bridge = None
    self._eth_config = False
    self._bridge_up = False
    self._autoconfig = autoconfig

  def open(self):
    # check interfaces
    ifnames = self._netif.get_interfaces()
    if self.eth_if not in ifnames:
      raise EtherTapError("ethernet interface not found: "+self.eth_if)
    if self.tap_if in ifnames:
      raise EtherTapError("tap interface already exists: "+self.tap_if)
    if self.bridge_if in ifnames:
      raise EtherTapError("bridge interface already exists: "+self.bridge_if)

    # check if ethernet is configured
    entry = ifnames[self.eth_if]
    if self._autoconfig and not self._netif.if_is_configured(entry):
      ret = self._netif.if_configure(self.eth_if, '0.0.0.0')
      if ret != 0:
        raise EtherTapError("failed configuring ethernet")
      ret = self._netif.if_up(self.eth_if)
      if ret != 0:
        raise EtherTapError("failed configuring ethernet")     
      self._eth_config = True

    # create tap
    self._tap = Tap(self.tap_if, self._osh, self._netif)
    fd = self._tap.open()
    if fd < 0:
      raise EtherTapError("error creating tap: " + self.tap_if)

    # create bridge
    self._bridge = Bridge(self.bridge_if, self._osh, self._netif)
    ret = self._bridge.create()
    if ret != 0:
      raise EtherTapError("error creating bridge: " + self.bridge_if)

    # add interfaces to bridge
    ret1 = self._bridge.add_if(self.eth_if)
    ret2 = self._bridge.add_if(self.tap_if)
    if ret1 != 0 or ret2 != 0:
      raise EtherTapError("error adding interfaces to bridge")

    # bring up bridge
    ret = self._bridge.up()
    if ret != 0:
      raise EtherTapError("error enabling bridge")
    self._bridge_up = True

  def close(self):
    errors = 0

    if self._tap is not None:
      # close tap
      ret = self._tap.close()
      if ret != 0:
        errors += 1

    if self._bridge is not None:
      # bring down bridge
      if self._bridge_up:
        ret = self._bridge.down()
        if ret != 0:
          errors += 1

      # remove bridge
      ret = self._bridge.destroy()
      if ret != 0:
        errors += 1

    # ethernet down
    if self._eth_config:
      ret = self._netif.if_down(self.eth_if)
      if ret != 0:
        errors += 1

    return errors

  def read(self, size=2048, timeout=None):
    """read a packet with given max size and optional timeout"""
    return self._tap.read(size, timeout)

  def write(self, buf):
    """write a packet"""
    return self._tap.write(buf)

  def __enter__(self):
    """for use in 'with'"""
    self.open()
    return self

  def __exit__(self, type, value, traceback):
    """for use in 'with'"""
    self.close()


# ----- Test -----
if __name__ == '__main__':
  with EtherTap('en3') as et:
    # show interface config
    ifs = et._netif.get_interfaces()
    for ifname in ifs:
      if ifname in (et.eth_if, et.tap_if, et.bridge_if):
        print(ifname, ifs[ifname])
