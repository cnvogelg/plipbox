from __future__ import print_function

import os
import sys
import signal
import struct

class EtherCap:
  def __init__(self, eth_if):
    pcapio = os.path.join(os.path.dirname(__file__),"pcapio.py")
    self.sudo = "/usr/bin/sudo"
    # use sudo's non-interactive -n switch
    self.args = (self.sudo, "-n", sys.executable, pcapio, eth_if)

  def open(self):
    # fork
    stdin  = sys.stdin.fileno()
    stdout = sys.stdout.fileno()
    pStdin, cStdout  = os.pipe()
    cStdin,  pStdout = os.pipe()
    self.pStdin = pStdin
    self.pStdout = pStdout
    self.pid = os.fork()
    if self.pid:
      # parent process
      os.close(cStdout)
      os.close(cStdin)
    else:
      # child process
      os.close(pStdin)
      os.close(pStdout)
      os.dup2(cStdin,  stdin)
      os.dup2(cStdout, stdout)
      # launch pcapio.py
      os.execv(self.sudo, self.args)

  def close(self):
    # close pipe
    os.close(self.pStdin)
    os.close(self.pStdout)

  def read(self):
    # read size header
    hdr = os.read(self.pStdin, 2)
    size = struct.unpack("H", hdr)[0]
    # read packet
    buf = os.read(self.pStdin, size)
    return buf

  def write(self, buf):
    # write header
    hdr = struct.pack("H", len(buf))
    os.write(self.pStdout, hdr)
    os.write(self.pStdout, buf)

  def __enter__(self):
    """for use in 'with'"""
    self.open()
    return self

  def __exit__(self, type, value, traceback):
    """for use in 'with'"""
    self.close()

# test
if __name__ == '__main__':
  import sys
  ifc = "eth0" if len(sys.argv) < 2 else sys.argv[1]
  ec = EtherCap(ifc)
  print("open",ifc)
  ec.open()
  print("read")
  buf = ec.read()
  print("got",len(buf))
  ec.write("hello, world!")
  print("close")
  ec.close()
  print("done")

