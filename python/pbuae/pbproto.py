from __future__ import print_function
import logging
import time
import vpar


class PBProtoError(Exception):
  """Fatal errors in the plipbox protocol"""
  pass


class PBProto:
  """handle the plipbox protocol spoken via vpar"""

  # commands
  CMD_SEND = 0x11
  CMD_RECV = 0x22

  # control lines
  SEL = vpar.SEL_MASK     # in
  ACK = vpar.BUSY_MASK    # out
  REQ = vpar.POUT_MASK    # in

  def __init__(self, v):
    """create the protocol handler with the given vpar instance"""
    self._log = logging.getLogger(__name__)
    self._vpar = v
    self._send_pkt_func = None
    self._recv_pkt_func = None
    self._in_sync = False

  def set_packet_handler(self, recv_pkt_func, send_pkt_func):
    """set functions that handle the incoming/outgoing packets.
       they will be called according to the command received in handle().

       recv_pkt() -> data: Amiga wants to receive a packet
       send_pkt(data) -> Amiga wants to send a packet
    """
    self._send_pkt_func = send_pkt_func
    self._recv_pkt_func = recv_pkt_func

  def open(self):
    """open protocol handler and associated vpar link"""
    self._vpar.open()

  def close(self):
    """close protocol handler and associated vpar link"""
    self._vpar.close()

  def request_recv(self):
    """request the reception of a packet from the Amiga"""
    # simply trigger ack
    self._vpar.trigger_ack()

  def sync_with_emu(self, timeout=None):
    """sync with emu and return true if sync was done or false if not"""
    # get initial sync
    ok = self._vpar.request_state(timeout)
    if not ok:
      return False
    else:
      self._in_sync = True
      return True

  def handle(self):
    """main entry to handle the plipbox protocol on the vpar link.
       it will try to process one command and then returns the command.
       if the command is valid it will trigger the packet handler functions
       set above.

       call will block until something has happened or timeout occurred.

       returns: False - not connected
                None - timeout
                <cmd_value> - handled command
       raises: PBProtoError if sync was lost or fatal protocol error
    """
    # must be in sync
    if not self._in_sync:
      return False

    self._log.debug("enter handle() loop")
    while True:
      # emu init?
      if self._vpar.check_init_flag():
        self._log.info("emu init")
        # clear ACK
        self._set_rak(0)

      # emu exit?
      if self._vpar.check_exit_flag():
        self._log.info("emu exit")
        self._in_sync = False
        return False

      # we need to react if the Amiga has triggered SEL = 1
      if self._vpar.peek_control() & self.SEL == self.SEL:
        self._log.debug("got SEL")
        return self._handle_cmd()

      # request a current state (block until state updates)
      ok = self._vpar.poll_state()
      if not ok:
        # timeout occurred
        return None
      self._log.debug("got state: ctl=%02x dat=%02x" % (self._vpar.peek_control(), self._vpar.peek_data()))

  def _handle_cmd(self):
    """wait for an incoming command"""
    # --- begin command
    # wait SEL == 1
    ok = self._wait_select(1, throw=False)
    if not ok:
      return None

    # read <CMD> byte
    cmd = self._vpar.peek_data()
    self._log.info("got cmd: %02x" % cmd)

    # prepare data for packet if its a receive command
    if cmd == self.CMD_RECV:
      if self._recv_pkt_func is not None:
        data = self._recv_pkt_func()
      else:
        self._log.warning("no recv_pkt_func set!")
        data = ""

    # start timing
    ts = time.time()

    # set RAK = 1
    self._set_rak(1)
    if cmd == self.CMD_SEND:
      data = self._cmd_send(ts)
    elif cmd == self.CMD_RECV:
      self._cmd_recv(ts, data)
    else:
      self._log.error("UNKNOWN COMMAND: %02x" % cmd)
      data = ""

    # --- end command
    # wait SEL == 0
    self._wait_select(0)
    # set RAK = 0
    self._set_rak(0)

    # end timing
    te = time.time()
    self._log.info("cmd time: delta=%.4f" % (te - ts))

    # process sent data
    if cmd == self.CMD_SEND:
      if self._send_pkt_func is not None:
        self._send_pkt_func(data)
      else:
        self._log.warning("no send_pkt_func set!")

    return cmd, len(data)

  def _cmd_send(self, ts):
    """Amiga sends a buffer"""
    self._log.debug("+++ incoming send +++")
    # get size HI
    self._wait_req(1, ctx="get_size_hi", start=ts)
    hi = self._vpar.peek_data()
    self._set_rak(0)
    # get size LO
    self._wait_req(0, ctx="get_size_lo", start=ts)
    lo = self._vpar.peek_data()
    self._set_rak(1)
    size = hi * 256 + lo
    self._log.debug("send size: %d" % size)
    # get data loop
    toggle = False
    data = ""
    for i in xrange(size):
      self._log.debug("rx #%04d/%04d" % (i, size))
      self._wait_req(not toggle, ctx="get_data_#%d" % i, start=ts)
      d = self._vpar.peek_data()
      data += chr(d)
      self._set_rak(toggle)
      toggle = not toggle
    self._log.debug("--- incoming send ---")
    return data

  def _cmd_recv(self, ts, data):
    """Amiga wants to receive a buffer"""
    self._log.debug("+++ incoming recv +++")
    size = len(data)
    self._log.debug("recv size: %d" % size)
    hi = size / 256
    lo = size % 256
    # send size HI
    self._wait_req(1, ctx="put_size_hi", start=ts)
    self._vpar.set_data(hi)
    self._set_rak(0)
    # send size LO
    self._wait_req(0, ctx="put_size_lo", start=ts)
    self._vpar.set_data(lo)
    self._set_rak(1)
    # send data
    toggle = False
    for i in xrange(size):
      self._wait_req(not toggle, ctx="put_data_#%d" % i, start=ts)
      self._log.debug("tx #%04d/%04d" % (i, size))
      d = ord(data[i])
      self._vpar.set_data(d)
      self._set_rak(toggle)
      toggle = not toggle
    # clear buffer
    self.recv_buf = None
    self._log.debug("--- incoming recv ---")

  def _wait_select(self, value, timeout=5, ctx="", start=0, throw=True):
    """wait for SELECT signal"""
    t = time.time()
    begin = t
    end = t + timeout
    found = False
    if value:
      value = 1
    else:
      value = 0
    #self._log("wait_select: value=%d" % value)
    while t < end:
      s = self._vpar.peek_control() & self.SEL
      if s and value:
        found = True
        break
      elif not s and not value:
        found = True
        break

      rem = end - t
      self._vpar.poll_state(rem)
      t = time.time()

    self._log.debug("wait_sel: %d (%12.6f delay)"
                    % (value, t - begin))
    if not found and throw:
      delta = t - start
      raise PBProtoError("%s: no select. delta=%5.3f timeout=%d" %
                         (ctx, delta, timeout))
    return found

  def _wait_req(self, expect, timeout=5, ctx="", start=0):
    """wait for toggle on POUT signal"""
    #print expect
    t = time.time()
    begin = t
    end = t + timeout
    found = False
    #self._log("wait_line_toggle: POUT == %s" % expect)
    if expect:
      expect = 1
    else:
      expect = 0
    while t < end:
      s = self._vpar.peek_control()
      if expect and ((s & vpar.POUT_MASK) == vpar.POUT_MASK):
        found = True
        break
      elif (not expect) and ((s & vpar.POUT_MASK) == 0):
        found = True
        break

      # check for SELECT -> arbitration loss?
      if (s & vpar.SEL_MASK) != vpar.SEL_MASK:
        delta = t - start
        raise PBProtoError("%s: lost select in line toggle."
                           " delta=%5.3f timeout=%d"
                           % (ctx, delta, timeout))

      rem = end - t
      self._vpar.poll_state(rem)
      t = time.time()

    self._log.debug("wait_tog: %s (%12.6f delay)"
                    % (expect, t - begin))
    if not found:
      delta = t - start
      raise PBProtoError("%s: missing line toggle."
                         " delta=%5.3f timeout=%d"
                         % (ctx, delta, timeout))

  def _set_rak(self, value):
    if value:
      self._vpar.set_control_mask(vpar.BUSY_MASK)
    else:
      self._vpar.clr_control_mask(vpar.BUSY_MASK)


# ----- Test -----
if __name__ == '__main__':
  import util
  import vpar

  save_data = None
  count = 10
  logging.basicConfig()
  s = util.SoPTY('/tmp/vpar')
  v = vpar.VPar(s)
  p = PBProto(v)

  def recv():
    global save_data
    print("<-- want packet: size=%d" % len(save_data))
    return save_data

  def send(data):
    global count, save_data, p
    print("--> got packet: size=%d" % len(data))
    # ping pong
    if count > 0:
      count -= 1
      save_data = data
      p.request_recv()

  p.set_packet_handler(recv, send)
  p._log.setLevel(logging.INFO)
  p.open()
  print("syncing with emulator...")
  p.sync_with_emu()
  print("got sync!")
  while p.handle() is not False:
    print("loop")
  p.close()



