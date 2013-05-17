import subprocess
import sys
import os
import pty

class SoPTY:
  def __init__(self, file_name):
    self._file_name = file_name
    (master, slave) = pty.openpty()
    self._slave_name = os.ttyname(slave)
    if os.path.exists(self._file_name):
      os.unlink(self._file_name)
    os.symlink(self._slave_name, self._file_name)
    self._fd = master
    # set a clean pty
    self._cmd = ['/bin/stty','-f',self._slave_name,
      'cs8','raw','-echo','-onlcr','-echoctl','-echoke','-echoe','-iexten']
    subprocess.check_call(self._cmd)
    #subprocess.check_call(['/bin/stty','-f',self._slave_name])
  
  def get_fd(self):
    return self._fd
  
  def close(self):
    try:
      os.unlink(self._file_name)
    except OSError:
      pass
    
  def read(self, size):
    return os.read(self._fd, size)
  
  def write(self, buf):
    os.write(self._fd, buf)

  def flush(self):
    os.fsync(self._fd)
