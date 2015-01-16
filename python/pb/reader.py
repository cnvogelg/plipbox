from __future__ import print_function
import time
import threading
import traceback
import logging

class Reader:

  def __init__(self, name, quit_event, **kwargs):
    self._log = logging.getLogger(name)
    if "level" in kwargs:
      self._log.setLevel(kwargs['level'])
    self.name = name
    self.quit_event = quit_event
    self.thread = threading.Thread(target=self.run, name=self.name)
    self.timeout = 1
  
  def set_send_func(self, send_func):
    self.send_func = send_func
  
  def start(self):
    self.thread.start()
    
  def stop(self):
    self.quit_event.set()
    self.thread.join()
  
  def open(self):
    raise
    
  def close(self):
    raise
    
  def _get_pkt(self):
    raise
    
  def run(self):
    try:
      self._log.debug("++ run")
      while not self.quit_event.is_set():
        pkt = self._get_pkt()
        if pkt is not None:
          self.send_func(pkt)
        elif pkt is False:
          # do quit...
          self.quit_event.set()
      self._log.debug("-- run")
    except KeyboardInterrupt:
      print("***Break [%s]" % self.name)
      self.quit_event.set()
    except Exception as e:
      print(self.name, "Unexpected error:", e)
      traceback.print_exc()
      self.quit_event.set()
    self._log.debug("thread done")
