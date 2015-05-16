import subprocess
import sys
import os
import pty

class SoPTY:
  """create a pseudo terminal so that vpar from FS-UAE can attach to it"""

  def __init__(self, link_name):
    """give a link_name that will point to the pty"""
    self._link_name = link_name
    self._slave_name = None
    self._fd = None

  def open(self):
    """try to open an new pty. publish the slave side as a link"""
    # create a new pty
    (master, slave) = pty.openpty()
    if master == -1 or slave == -1:
      raise IOError("can't create PTY pair!")
    # find tty name of slave
    self._slave_name = os.ttyname(slave)
    # try to symlink slave name
    if os.path.exists(self._link_name):
      os.unlink(self._link_name)
    os.symlink(self._slave_name, self._link_name)
    self._fd = master
    self._slave_fd = slave
    self._clean_tty()

  def _clean_tty(self):
    """make given tty really transparent for 8 bit transfers"""
    # stty uses -f on Mac OS X but -F on Linux
    if sys.platform == 'darwin':
      flag = '-f'
    else:
      flag = '-F'
    self._cmd = ['/bin/stty', flag, self._slave_name,
                 'cs8', 'raw', '-echo', '-onlcr', '-echoctl',
                 '-echoke', '-echoe', '-iexten']
    subprocess.check_call(self._cmd)

  def get_fd(self):
    """return the file descriptor if you want to select() it"""
    return self._fd

  def close(self):
    """close pty again"""
    try:
      os.unlink(self._link_name)
      os.close(self._fd)
      os.close(self._slave_fd)
    except OSError:
      pass

  def read(self, size):
    """perform a (blocking) read on the PTY"""
    data = ""
    n = size
    while len(data) < size:
       part = os.read(self._fd, n)
       np = len(part)
       if np == 0:
          # EOF
          break
       data = data + part
       n = n - np
    return data

  def write(self, buf):
    """perform a write on the PTY"""
    return os.write(self._fd, buf)

  def flush(self):
    """flush all data on the PTY"""
    os.fsync(self._fd)


# ----- Test -----
if __name__ == '__main__':
  p = SoPTY("/tmp/vpar")
  p.open()
  p.close()
