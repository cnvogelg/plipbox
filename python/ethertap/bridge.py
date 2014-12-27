import sys
from oshelper import OSHelper
from netif import NetIf

class Bridge:
  """setup a ethernet bridge device connecting two ethernet devices
     on your platform"""
  def __init__(self, ifname='bridge1', oshelper=None, netif=None):
    if oshelper is None:
      oshelper = OSHelper()
    if netif is None:
      netif = NetIf(oshelper)
    self._ifname = ifname
    self._osh = oshelper
    self._netif = netif

  def create(self):
    """create a new and empty bridge. return error code"""
    if sys.platform == 'darwin':
      ret = self._osh.ifconfig(self._ifname, 'create')
    elif sys.platform == 'linux2':
      ret = self._osh.brctl('addbr', self._ifname)
    return ret

  def add_if(self, if_name):
    """add an interface to the bridge"""
    if sys.platform == 'darwin':
      ret = self._osh.ifconfig(self._ifname, 'addm', if_name)
    elif sys.platform == 'linux2':
      ret = self._osh.brctl('addif', self._ifname, if_name)
    return ret

  def delete_if(self, if_name):
    """remove an interface from the bridge"""
    if sys.platform == 'darwin':
      ret = self._osh.ifconfig(self._ifname, 'deletem', if_name)
    elif sys.platform == 'linux2':
      ret = self._osh.brctl('delif', self._ifname, if_name)
    return ret

  def up(self):
    """enable the bridge"""
    return self._netif.if_up(self._ifname)

  def down(self):
    """disable the bridge"""
    return self._netif.if_down(self._ifname)

  def destroy(self):
    """remove the bridge"""
    if sys.platform == 'darwin':
      ret = self._osh.ifconfig(self._ifname, 'destroy')
    elif sys.platform == 'linux2':
      ret = self._osh.brctl('delbr', self._ifname)
    return ret


# ----- test -----
if __name__ == '__main__':
  osh = OSHelper()
  netif = NetIf(osh)
  ifname = netif.find_unused_bridge_netif()
  eths = netif.pick_ethernet_netifs()
  print "creating bridge", ifname
  bridge = Bridge(ifname, osh, netif)
  ret = bridge.create()
  print "create", ret

  for eth in eths:
    ret = bridge.add_if(eth)
    print "add_if", eth, ret

  ret = bridge.up()
  print "up", ret

  ret = bridge.down()
  print "down", ret

  for eth in eths:
    ret = bridge.delete_if(eth)
    print "delete_if", ret

  ret = bridge.destroy()
  print "destroy", ret
