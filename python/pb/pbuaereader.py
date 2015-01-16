from __future__ import print_function
import reader
import pbuae
import Queue
import logging
import ethframe

class PBUAEReader(reader.Reader):

  def __init__(self, quit_event, pty_name, **kwargs):
    reader.Reader.__init__(self, "PAR", quit_event, **kwargs)
    self.pty_name = pty_name
    self.sopty = pbuae.SoPTY(pty_name)
    self.vpar = pbuae.VPar(self.sopty)
    self.pbproto = pbuae.PBProto(self.vpar)
    if 'level' in kwargs:
      self.pbproto._log.setLevel(kwargs['level'])
    self.pkt_queue = Queue.Queue()
    self.need_sync = True
    self.first_try = True
    self.online = False

  def open(self):
    self._log.debug("+open pbproto")
    self.pbproto.set_packet_handler(self._recv_cmd, self._send_cmd)
    self.pbproto.open()
    self._log.debug("-open pbproto")

  def close(self):
    self._log.debug("+close pbproto")
    self.pbproto.close()
    self._log.debug("-close pbproto")

  def _recv_cmd(self):
    """data will be received from Amiga"""
    pkt = self.pkt_queue.get()
    self._log.debug("AMIGA recv:")
    return pkt
    
  def _send_cmd(self, data):
    """data was sent from Amiga"""
    self._log.debug("AMIGA send: {}".format(len(data)))
    self._send_pkt = data

  def send(self, data):
    if not self.online:
      self._log.debug("ignore send - not online!")
    else:
      self.pkt_queue.put(data)
      self.pbproto.request_recv()

  def _get_pkt(self):
    try:
      # we need to sync with emulator first
      if self.need_sync:
        if self.first_try:
          print("syncing with emulator via vpar PTY on '%s'" % self.pty_name)
          self.first_try = False
        if self.pbproto.sync_with_emu(timeout=self.timeout):
          print("got sync")
          self.need_sync = False
        else:
          return None
      # try to handle pb
      self._send_pkt = None
      result = self.pbproto.handle()
      self._log.debug("handle: {}".format(result))
      if result is False:
        # end -> resync
        print("lost sync")
        self.need_sync = True
        self.first_try = True
        self.online = False
        return None
      elif self._send_pkt is not None:
        # got packet
        pkt = self._send_pkt
        ef = ethframe.EthFrame(pkt)
        if ef.is_magic_online():
          print("online")
          self.online = True
          return None
        elif ef.is_magic_offline():
          print("offline")
          self.online = False
          return None
        else:
          return pkt
      else:
        return None
    except pbuae.PBProtoError as e:
      print(e)
      self.need_sync = True
      self.first_try = True
      self.online = False
