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
import select
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

# get send method: older pcap modules used 'inject', newer ones 'sendpacket'
try:
  p.inject
  send_func = lambda pkt : p.inject(pkt, len(pkt))
except AttributeError:
  send_func = lambda pkt : p.sendpacket(pkt)

# get stdin/stdout streams
stdin = sys.stdin.fileno()
stdout = sys.stdout.fileno()

# callback for incoming pcap packet
def write_packet_stdout(ts, pkt):
  if verbose:
    print(time.time(),"GOT",len(pkt))
  else:
    hdr = struct.pack("H", len(pkt))
    os.write(stdout, hdr)
    os.write(stdout, pkt)

def read_packet_stdin():
  # read from stdin
  hdr = os.read(stdin, 2)
  # error reading packet from stdin
  if hdr is None or len(hdr) == 0:
    return False
  # read packet from stdin
  size = struct.unpack("H", hdr)[0]
  data = os.read(stdin, size)
  send_func(data)
  return True

# main loop
def main_loop():
  try:
    while True:
      # dispatch full buffer
      p.dispatch(-1, write_packet_stdout)
      # check for incoming packet
      rx,_,_ = select.select([stdin],[],[],0)
      if stdin in rx:
        if not read_packet_stdin():
          break
  except Exception:
    pass
  except KeyboardInterrupt:
    pass

main_loop()
