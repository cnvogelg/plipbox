import sys
import subprocess


class Bridge:
    """setup a ethernet bridge device connecting two ethernet devices
       on your platform"""
    def __init__(self, name='bridge1'):
        self._name = name
        self._sudo = '/usr/bin/sudo'
        self._ifconfig = '/sbin/ifconfig'
        if sys.platform == 'darwin':
            pass
        elif sys.platform == 'linux2':
            self._brctl = '/sbin/brctl'
        else:
            raise NotImplementedError("unsupported platform!")

    def _run(self, cmd):
        subprocess.check_call(cmd)

    def create(self):
        if sys.platform == 'darwin':
            self._run([self._sudo, self._ifconfig, self._name, 'create'])
        elif sys.platform == 'linux2':
            self._run([self._sudo, self._brctl, 'addbr', self._name])

    def add_if(self, if_name):
        if sys.platform == 'darwin':
            self._run([self._sudo, self._ifconfig, self._name, 'addm',
                       if_name])
        elif sys.platform == 'linux2':
            self._run([self._sudo, self._brctl, 'addif', self._name,
                       if_name])

    def up(self):
        self._run([self._sudo, self._ifconfig, self._name, 'up'])

    def down(self):
        self._run([self._sudo, self._ifconfig, self._name, 'down'])

    def destroy(self):
        if sys.platform == 'darwin':
            self._run([self._sudo, self._ifconfig, self._name, 'destroy'])
        elif sys.platform == 'linux2':
            self._run([self._sudo, self._brctl, 'delbr', self._name])
