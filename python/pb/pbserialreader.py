from __future__ import print_function
import reader
import pbserial
import logging

class PBSerialReader(reader.Reader):

  def __init__(self, quit_event, serial_port, **kwargs):
    reader.Reader.__init__(self, "ser", quit_event, **kwargs)
    self.serial_port = serial_port
    self.ser = pbserial.Device(serial_port)

  def open(self):
    self._log.debug("+open ethernet")
    self.ser.open()
    self._log.debug("-open ethernet")

  def close(self):
    self._log.debug("+close ethernet")
    self.ser.close()
    self._log.debug("-close ethernet")

  def _get_pkt(self):
    self._log.debug("+get_pkt")
    pkt = self.ser.recv()
    self._log.debug("-get_pkt")
    if pkt is None:
      return None
    else:
      self._log.info("got pkt: {0}".format(len(pkt)))
      return pkt

  def send(self, pkt):
    self._log.debug("+send_pkt: {0}".format(len(pkt)))
    self.ser.send(pkt)
    self._log.debug("-send_pkt")
