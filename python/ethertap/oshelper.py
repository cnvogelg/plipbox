#!/usr/bin/env python2.7
from __future__ import print_function
import os
import sys
import subprocess

class OSHelperError(Exception):
  """OSHelper throws these errors if something goes wrong"""
  pass

class OSHelper:
  """provider some OS specific network tools

     search the tools by path and ensure they are available.
     if necessary call them with sudo.
  """
  
  def __init__(self, use_sudo=True):
    # we'll need sudo for most commands
    self._ifconfig = '/sbin/ifconfig'
    self._tools = [ self._ifconfig ]
    self._use_sudo = use_sudo
    if use_sudo:
      self._sudo = '/usr/bin/sudo'
      self._tools.append(self._sudo)
    # check for platform specific tools
    if sys.platform == 'darwin':
      pass
    elif sys.platform == 'linux2':
      self._brctl = '/sbin/brctl'
      self._tunctl = '/usr/sbin/tunctl'
      self._tools.append(self._brctl)
      self._tools.append(self._tunctl)
    else:
      raise NotImplementedError("unsupported platform!")
    # check if all tools exist
    for tool in self._tools:
      if not os.path.exists(tool):
        raise OSHelperError("Tool not found: " + tool)

  def _get_cmd(self, cmd, args=None):
    full_cmd = []
    if self._use_sudo:
      full_cmd.append(self._sudo)
      # use sudo's non-interactive mode to avoid password
      # this will fail the command if a password is necessary...
      full_cmd.append('-n')
    full_cmd.append(cmd)
    if args is not None:
      full_cmd += args
    return full_cmd

  def _run(self, cmd, args):
    """run a command and return exit code"""
    full_cmd = self._get_cmd(cmd, args)
    # run and check result
    with open(os.devnull, "w") as f:
      ret = subprocess.call(full_cmd, stdout=f, stderr=f)
      return ret

  def _run_with_output(self, cmd, args):
    """run and return (exitcode, stdout, stderr)"""
    full_cmd = self._get_cmd(cmd, args)
    p = subprocess.Popen(full_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    (stdout, stderr) = p.communicate()
    return (p.returncode, stdout)
  
  def ifconfig(self, *args):
    """call ifconfig with the given set of parameters"""
    return self._run(self._ifconfig, args)

  def ifconfig_output(self, *args):
    return self._run_with_output(self._ifconfig, args)

  def brctl(self, *args):
    """call brctl with the given set of parameters"""
    if sys.platform != 'linux2':
      raise OSHelperError("'brctl' not supported on this platform")
    return self._run(self._brctl, args)

  def tunctl(self, *args):
    """call tunctl with the given set of parameters"""
    if sys.platform != 'linux2':
      raise OSHelperError("'tunctl' not supported on this platform")
    return self._run(self._tunctl, args)

# ----- test -----
if __name__ == '__main__':
  osh = OSHelper()
  res = osh.ifconfig_output()
  print(res)


