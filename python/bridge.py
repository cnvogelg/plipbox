import sys
import subprocess

class Bridge:
  """setup a ethernet bridge device connecting two ethernet devices on your platform"""
  def __init__(self, ifa, ifb, name='bridge0'):
    self._ifa = ifa
    self._ifb = ifb
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
  
  def up(self):
    if sys.platform == 'darwin':
      self._run([self._sudo,self._ifconfig,self._name,'create'])
      self._run([self._sudo,self._ifconfig,self._name,'addm',self._ifa,'addm',self._ifb])
      self._run([self._sudo,self._ifconfig,self._name,'up'])
    elif sys.platform == 'linux2':
      self._run([self._sudo,self._brctl,'addbr',self._name])
      self._run([self._sudo,self._brctl,'addif',self._name,self._ifa])
      self._run([self._sudo,self._brctl,'addif',self._name,self._ifb])
      self._run([self._sudo,self._ifconfig,self._name,'up'])
    
  def down(self):
    if sys.platform == 'darwin':
      self._run([self._sudo,self._ifconfig,self._name,'destroy'])
    elif sys.platform == 'linux2':
      self._run([self._sudo,self._ifconfig,self._name,'down'])
      self._run([self._sudo,self._brctl,'delbr',self._name])
