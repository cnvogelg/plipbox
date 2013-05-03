import sys
import subprocess

class Bridge:
  """setup a ethernet bridge device connecting two ethernet devices on your platform"""
  def __init__(self, ifa, ifb, name='bridge0'):
    self._ifa = ifa
    self._ifb = ifb
    self._name = name
    self._sudo = '/usr/bin/sudo'
    if sys.platform == 'darwin':
      self._ifconfig = '/sbin/ifconfig'
    else:
      raise NotImplementedError("unsupported platform!")
  
  def _run(self, cmd):
    subprocess.check_call(cmd)
  
  def up(self):
    if sys.platform == 'darwin':
      self._run([self._sudo,self._ifconfig,self._name,'create'])
      self._run([self._sudo,self._ifconfig,self._name,'addm',self._ifa,'addm',self._ifb])
      self._run([self._sudo,self._ifconfig,self._name,'up'])
    
  def down(self):
    if sys.platform == 'darwin':
      self._run([self._sudo,self._ifconfig,self._name,'destroy'])