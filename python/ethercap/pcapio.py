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

import time

# parse options
if len(sys.argv) < 2:
  print("Usage:",sys.argv[0],"<interface>")
  sys.exit(1)
ifc = sys.argv[1]

# verbose?
verbose = True if len(sys.argv) > 2 else False

# create pcap instance
# Mac OSX needs a hack here: without timeout it would block for ever
# now packets are reported after the timeout
if sys.platform == 'darwin':
  p = pcap.pcap(ifc, timeout_ms=100)
else:
  p = pcap.pcap(ifc)

# get stdin/stdout streams
stdin = sys.stdin.fileno()
stdout = sys.stdout.fileno()

# callback for incoming pcap packet
def got_packet(ts, pkt):
  if verbose:
    print(time.time(),"GOT",len(pkt))
  else:
    hdr = struct.pack("H", len(pkt))
    os.write(stdout, hdr)
    os.write(stdout, pkt)

# read loop: get packets via pcap and write them to stdout
def read_loop():
  try:
    while True:
      # dispatch forever
      p.dispatch(0, got_packet)
  except Exception:
    pass

# write loop: get packets from stdin and send via pcap
def write_loop():
  try:
    while True:
      # read from stdin
      hdr = os.read(stdin, 2)
      # error reading packet from stdin
      if hdr is None or len(hdr) == 0:
        break
      # read packet from stdin
      size = struct.unpack("H", hdr)[0]
      data = os.read(stdin, size)
      p.inject(data, size)
  except Exception:
    pass

# run read loop in own thread
t = threading.Thread(target=read_loop)
t.run()

# write loop runs in main thread
write_loop()
