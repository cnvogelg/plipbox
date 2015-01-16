from __future__ import print_function
import reader
import ethertap
import logging

class EthernetReader(reader.Reader):

  def __init__(self, quit_event, tap_if, **kwargs):
    reader.Reader.__init__(self, "eth", quit_event, **kwargs)
    self.tap_if = tap_if
    self.et = ethertap.EtherTap(tap_if)
  
  def open(self):
    self._log.debug("+open ethernet")
    self.et.open()
    self._log.debug("-open ethernet")
  
  def close(self):
    self._log.debug("+close ethernet")
    self.et.close()
    self._log.debug("-close ethernet")
  
  def _get_pkt(self):
    self._log.debug("+get_pkt")
    pkt = self.et.read(timeout=self.timeout)
    self._log.debug("-get_pkt")
    if pkt is None:
      return None
    else:
      self._log.info("got pkt: {0}".format(len(pkt)))
      return pkt

  def send(self, pkt):
    self._log.debug("+send_pkt: {0}".format(len(pkt)))
    self.et.write(pkt)
    self._log.debug("-send_pkt")
