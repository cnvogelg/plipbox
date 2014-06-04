import os
import sys
import fcntl
import struct
import subprocess


class Tap:

    def __init__(self, base_name='tap0'):
        self.base_name = base_name
        self._sudo = '/usr/bin/sudo'
        self._ifconfig = '/sbin/ifconfig'

    def open(self):
        if sys.platform == 'darwin':
            # Mac OS X needs TUN/TAP OSX and
            # user must be member of group 'wheel'
            self._name = "/dev/" + self.base_name
            self._fd = os.open(self._name, os.O_RDWR)
        elif sys.platform == 'linux2':
            # Linux needs 'tunctl' tool and user needs sudo access
            self._tunctl = '/usr/sbin/tunctl'
            cmd = [self._sudo, self._tunctl, '-t', self.base_name,
                   '-u', str(os.getuid())]
            self._run(cmd)
            # up interface
            cmd = [self._sudo, self._ifconfig, self.base_name, 'up']
            self._run(cmd)
            # now open tap
            TUNSETIFF = 0x400454ca
            IFF_TAP = 0x0002
            IFF_NO_PI = 0x1000
            self._fd = os.open('/dev/net/tun', os.O_RDWR)
            fcntl.ioctl(self._fd, TUNSETIFF,
                        struct.pack("16sH", self.base_name,
                                    IFF_TAP | IFF_NO_PI))
        else:
            raise NotImplementedError("unsupported platform!")

    def _run(self, cmd):
        subprocess.check_call(cmd)

    def close(self):
        os.close(self._fd)
        if sys.platform == 'linux2':
            # use 'tunctl' to remove tap
            cmd = [self._sudo, self._tunctl, '-d', self.base_name]
            self._run(cmd)

    def read(self, size):
        return os.read(self._fd, size)

    def write(self, buf):
        os.write(self._fd, buf)

    def get_fd(self):
        return self._fd
