# implementation of the plipbox protocol

import vpar
import time
import datetime

class PBProtoError(Exception):
    def __init__(self, value):
        self.value = value
    
    def __str__(self):
        return repr(self.value)

class PBProto:
    # commands
    CMD_SEND = 0x11
    CMD_RECV = 0x22
    
    # control lines
    SEL = vpar.SEL_MASK     # in
    ACK = vpar.BUSY_MASK    # out
    REQ = vpar.POUT_MASK    # in
  
    def __init__(self, v, debug=False):
        self.vpar = v
        self.debug = debug
        self.recv_buf = None
        self.send_buf = None
    
    def start(self):
        # clear ACK
        self._set_rak(0)
    
    def send(self, buf):
        """Ask Amiga to receive a frame from me"""
        # simply trigger ack
        self.vpar.trigger_ack()
        # is a collision?
        if self.recv_buf != None:
            self._log("collision! already a recv buf set.")
            return False
        # prepare buffer 
        self.recv_buf = buf
        return True
    
    def recv(self):
        """Did the Amiga send a buffer?"""
        s = self.send_buf
        self.send_buf = None
        return s
    
    def must_handle(self):
        """check if need to handle"""
        # we need to react if the Amiga has triggered SEL = 1
        return self.vpar.peek_control() & self.SEL == self.SEL
    
    def handle(self):
        """wait for an incoming command"""
        self._log("handle_cmd")
        # --- begin command
        # wait SEL == 1
        self._wait_select(1)
        # read <CMD>
        cmd = self.vpar.peek_data()
        # set RAK = 1
        self._set_rak(1)
        # --- dispatch command
        self._log("got cmd: %02x" % cmd)
        if cmd == self.CMD_SEND:
            size = self._cmd_send()
        elif cmd == self.CMD_RECV:
            size = self._cmd_recv()
        else:
            size = 0
            self._log("UNKNOWN COMMAND!")
        # --- end command
        # set RAK = 0
        self._set_rak(0)
        # wait SEL == 0
        self._wait_select(0)
        return cmd,size
    
    def _cmd_send(self):
        """Amiga sends a buffer"""
        self._log("+++ incoming send +++")
        # get size HI
        self._wait_req(1)
        hi = self.vpar.peek_data()
        self._set_rak(0)
        # get size LO
        self._wait_req(0)
        lo = self.vpar.peek_data()
        self._set_rak(1)
        size = hi * 256 + lo
        self._log("size: %d" % size)
        # get data loop
        toggle = False
        data = ""
        for i in xrange(size):
            self._log("rx #%04d/%04d" % (i,size))
            self._wait_req(not toggle)
            d = self.vpar.peek_data()
            data += chr(d)
            self._set_rak(toggle)
            toggle = not toggle
        self.send_buf = data
        self._log("--- incoming send ---")
        return size

    def _cmd_recv(self):
        """Amiga wants to receive a buffer"""
        self._log("+++ incoming recv +++")
        data = self.recv_buf
        if data == None:
            size = 0
        else:
            size = len(data)
        self._log("size: %d" % size)
        hi = size / 256
        lo = size % 256
        # send size HI
        self._wait_req(1)
        self.vpar.set_data(hi)
        self._set_rak(0)
        # send size LO
        self._wait_req(0)
        self.vpar.set_data(lo)
        self._set_rak(1)
        # send data
        toggle = False
        for i in xrange(size):
            self._wait_req(not toggle)
            self._log("tx #%04d/%04d" % (i,size))
            d = ord(data[i])
            self.vpar.set_data(d)
            self._set_rak(toggle)
            toggle = not toggle
        # clear buffer
        self.recv_buf = None
        self._log("--- incoming recv ---")
        return size

    def _wait_select(self, value, timeout=1, ctx="", start=0):
        """wait for SELECT signal"""
        t = time.time()
        end = t + timeout
        found = False
        self._log("wait_select: value=%d" % value)
        while t < end:
            s = self.vpar.peek_control() & self.SEL
            if s and value:
                found = True
                break
            elif not s and not value:
                found = True
                break
                
            rem = end - t
            self.vpar.poll_state(rem)
            t = time.time()
        self._log("wait_select: value=%d -> %s" % (value,found))
        if not found:
          delta = t - start
          raise PBProtoError("%s: no select. delta=%5.3f timeout=%d" % (ctx, delta, timeout))

    def _wait_req(self, expect, timeout=1, ctx="", start=0):
        """wait for toggle on POUT signal"""
        #print expect
        t = time.time()
        end = t + timeout
        found = False
        self._log("wait_line_toggle: POUT == %s" % expect)
        while t < end:
            s = self.vpar.peek_control()
            if expect and ((s & vpar.POUT_MASK) == vpar.POUT_MASK):
                found = True
                break
            elif (not expect) and ((s & vpar.POUT_MASK) == 0):
                found = True
                break
            
            # check for SELECT -> arbitration loss?
            if (s & vpar.SEL_MASK) != vpar.SEL_MASK:
                delta = t - start
                raise PBProtoError("%s: lost select in line toggle. delta=%5.3f timeout=%d" % (ctx, delta, timeout))
                
            rem = end - t
            self.vpar.poll_state(rem)
            t = time.time()
        self._log("wait_line_toggle: POUT == %s -> found: %s" % (expect, found))
        if not found:
          delta = t - start
          raise PBProtoError("%s: missing line toggle. delta=%5.3f timeout=%d" % (ctx, delta, timeout))
    
    def _set_rak(self, value):
        if value:
            self.vpar.set_control_mask(vpar.BUSY_MASK)
        else:
            self.vpar.clr_control_mask(vpar.BUSY_MASK)
            
    def _log(self, msg):
        if not self.debug:
          return
        ts = time.time()
        sec = int(ts)
        usec = int(ts * 1000000) % 1000000 
        print "%8d.%06d pp:" % (sec,usec),msg

