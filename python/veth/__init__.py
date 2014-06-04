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
        self._ifconfig = '/sbin/ifconfig'
        self.tap = tap.Tap(tap_if)
        self.bridge = bridge.Bridge(eth_if, tap_if, bridge_if)

    def _has_if(self, name):
        # call ifconfig with the interface name and check return value
        with open(os.devnull, "w") as f:
            cmd = [self._ifconfig, name]
            ret = subprocess.call(cmd, stdout=f, stderr=f)
            return ret == 0

    def pre_check_ifs(self):
        """validate if interfaces are free or available"""
        has_eth = self._has_if(self.eth_if)
        has_tap = self._has_if(self.tap_if)
        has_bridge = self._has_if(self.bridge_if)
        return has_eth and not has_tap and not has_bridge

    def open(self):
        self.tap.open()
        self.bridge.up()

    def close(self):
        self.bridge.down()
        self.tap.close()

    def rx_pkt(self, size=2048):
        return self.tap.read(size)

    def tx_pkt(self, buf):
        self.tap.write(buf)
