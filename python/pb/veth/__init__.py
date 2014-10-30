import bridge
import tap
import os
import subprocess


class VEth:
    """veth - virtual ethernet class
       create a virtual ethernet port by bridging a real ethernet interface
       with a TAP device"""

    def __init__(self, eth_if, tap_if, bridge_if):
        self.eth_if = eth_if
        self.tap_if = tap_if
        self.bridge_if = bridge_if
        self._sudo = '/usr/bin/sudo'
        self._ifconfig = '/sbin/ifconfig'
        self.tap = tap.Tap(tap_if)
        self.bridge = bridge.Bridge(bridge_if)

    def _call_ifconfig(self, name, opts=None, check=True):
        # call ifconfig with the interface name and check return value
        with open(os.devnull, "w") as f:
            cmd = [self._sudo, self._ifconfig, name]
            if opts != None:
                cmd += opts
            if check:
                ret = subprocess.check_call(cmd, stdout=f, stderr=f)
            else:
                ret = subprocess.call(cmd, stdout=f, stderr=f)
            return ret

    def _has_if(self, name):
        return self._call_ifconfig(name, check=False) == 0

    def _setup_eth_if(self):
        self._call_ifconfig(self.eth_if, ['0.0.0.0', 'up'])

    def _pre_check_ifs(self):
        """validate if interfaces are free or available"""
        has_eth = self._has_if(self.eth_if)
        has_tap = self._has_if(self.tap_if)
        return has_eth and not has_tap

    def open(self):
        # check interfaces
        if not self._pre_check_ifs():
            raise Error("Invalid interfaces")
        # setup tap
        self.tap.open()
        # setup eth
        #self._setup_eth_if()
        # setup bridge
        has_bridge = self._has_if(self.bridge_if)
        if not has_bridge:
            self.bridge.create()
            self.bridge.add_if(self.eth_if)
        self.bridge.add_if(self.tap_if)
        self.bridge.up()

    def close(self):
        self.bridge.down()
        self.tap.close()
        #self.bridge.destroy()

    def rx_pkt(self, size=2048, timeout=None):
        return self.tap.read(size, timeout)

    def tx_pkt(self, buf):
        self.tap.write(buf)
