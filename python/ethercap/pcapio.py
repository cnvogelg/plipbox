#!/usr/bin/env python
# pcapio.py
#
# a small helper (running as root) that
# does package capture and injection
# it sends/receives the packets via stdout/stdin
#
# Usage: pcapio.py <interface>

from __future__ import print_function

import pcap
import sys
import os
import struct
import threading

# parse options
if len(sys.argv) != 2:
  print("Usage:",sys.argv[0],"<interface>")
  sys.exit(1)
ifc = sys.argv[1]

# create pcap instance
p = pcap.pcap(ifc)

stdin = sys.stdin.fileno()
stdout = sys.stdout.fileno()

# read loop
def read_loop():
  try:
    for ts, pkt in p:
      hdr = struct.pack("H", len(pkt))
      os.write(stdout, hdr)
      os.write(stdout, pkt)
  except OSError:
    pass

t = threading.Thread(target=read_loop)
t.run()

# write loop
try:
  while True:
    # read from stdin
    hdr = os.read(stdin, 2)
    if hdr is None or len(hdr) == 0:
      break
    size = struct.unpack("H", hdr)[0]
    data = os.read(stdin, size)
    p.inject(data, size)
except OSError:
  pass
