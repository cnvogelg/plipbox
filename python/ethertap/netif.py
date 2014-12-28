import sys
import string

class NetIf:
  """query information about network interfaces and control them.
     its more or less a wrapper around the 'ifconfig' tool.
  """
  def __init__(self, oshelper):
    self._osh = oshelper

  def get_interfaces(self):
    """return a dictionary of all network interfaces or None if query failed"""
    res, stdout = self._osh.ifconfig_output()
    if res != 0:
      return None
    ifs = {}
    current = None
    for line in stdout.split('\n'):
      if line != "":
        # is an interace?
        if line[0] in string.ascii_letters:
          begin_if = True
        else:
          begin_if = False          
        elements = line.split()
        n = len(elements)
        if n > 0:
          # begin of interface section
          if begin_if:
            ifname = elements[0]
            if ifname[-1] == ':':
              ifname = ifname[:-1]
            current = {}
            ifs[ifname] = current
            elements = elements[1:]
          # check parameters
          if elements[0] == 'ether':
            current['mac'] = elements[1]
          elif elements[0] == 'inet':
            current['inet'] = elements[1]
            if n >= 4:
              if elements[2] == 'netmask':
                current['netmask'] = elements[3]
              if n >= 6:
                if elements[4] == 'broadcast':
                  current['broadcast'] = elements[5]
          elif elements[0] == 'media:':
            current['media'] = ' '.join(elements[1:])
          elif elements[0] == 'status:':
            if elements[1] == 'active':
              current['active'] = True
            elif elements[1] == 'inactive':
              current['active'] = False

    return ifs

  def has_interface(self, ifname):
    """return true if a network interface given by name exists"""
    return ifname in self.get_interfaces()

  def get_tap_netif_prefix(self):
    """return the prefix for TAP network interfaces"""
    return 'tap'

  def get_bridge_netif_prefix(self):
    """return the prefix for bridge interaces"""
    return "bridge"

  def get_ethernet_netif_prefix(self):
    if sys.platform == 'darwin':
      return 'en'
    else:
      return 'eth'

  def find_unused_netif(self, prefix):
    ifnames = self.get_interfaces()
    for a in range(99):
      ifname = prefix + str(a)
      if ifname not in ifnames:
        return ifname
    raise ValueError("can't find unused interface for prefix: "+prefix)

  def find_unused_bridge_netif(self):
    """return the name of an unused bridge net interface"""
    return self.find_unused_netif(self.get_bridge_netif_prefix())

  def find_unused_tap_netif(self):
    """return the name of an unused tap net interface"""
    return self.find_unused_netif(self.get_tap_netif_prefix())

  def split_netif_name(self, ifname):
    """split interface name into prefix and number"""
    if len(ifname) < 2:
      raise ValueError("interface name too short")
    if ifname[-1] not in string.digits:
      return (ifname, 0)
    if ifname[-2] in string.digits:
      return (ifname[:-2], int(ifname[-2:]))
    else:
      return (ifname[:-1], int(ifname[-1:]))

  def get_all_ethernet_netifs(self, ifnames=None):
    """return a list of valid ethernet interfaces"""
    if ifnames is None:
      ifnames = self.get_interfaces()
    eth_prefix = self.get_ethernet_netif_prefix()
    result = []
    for ifname in ifnames:
      (prefix, num) = self.split_netif_name(ifname)
      if prefix == eth_prefix:
        result.append(ifname)
    return result

  def pick_ethernet_netifs(self, active=True, configured_ip=True, only_zero_ip=False):
    ifs = self.get_interfaces()
    ifnames = self.get_all_ethernet_netifs(ifs)
    result = []
    for ifname in ifnames:
      entry = ifs[ifname]
      if entry.has_key('active'):
        entry_active = entry['active']
        if entry_active == active:
          # active state matches
          entry_configured = entry.has_key('inet') and \
                             entry.has_key('netmask') and \
                             entry.has_key('broadcast')
          if entry_configured == configured_ip:
            # return only zero ips
            if entry_configured and only_zero_ip:
              if entry['inet'] == '0.0.0.0':
                result.append(ifname)
            else:
              result.append(ifname)
    return result

  def if_is_configured(self, entry):
    return entry.has_key('inet') and entry.has_key('netmask') and entry.has_key('broadcast')

  def if_up(self, name):
    """try to bring up interface. return exitcode of ifconfig call"""
    return self._osh.ifconfig(name,'up')

  def if_down(self, name):
    """try to bring down interface. return exitcode of ifconfig call"""
    return self._osh.ifconfig(name,'down')

  def if_configure(self, name, inet, netmask=None, broadcast=None):
    """configure ip"""
    args = [name, inet]
    if netmask is not None:
      args.extend(('netmask', netmask))
    if broadcast is not None:
      args.extend(('broadcast', broadcast))
    return self._osh.ifconfig(*args)


# ----- test -----
if __name__ == '__main__':
  import oshelper
  osh = oshelper.OSHelper()
  netif = NetIf(osh)
  ifs = netif.get_interfaces()
  for ifname in ifs:
    print ifname,"->",ifs[ifname]
  print "'en0' ->", netif.has_interface('en0')
  print "free tap ->", netif.find_unused_tap_netif()
  print "free bridge ->", netif.find_unused_bridge_netif()
  print "split: 'ex0' ->", netif.split_netif_name('ex0')
  print "split: 'eth29' ->", netif.split_netif_name('eth29')
  print "get all eths ->", netif.get_all_ethernet_netifs()
  print "pick eths ->", netif.pick_ethernet_netifs()
  print "pick eths (only zero) ->", netif.pick_ethernet_netifs(only_zero_ip=True)
