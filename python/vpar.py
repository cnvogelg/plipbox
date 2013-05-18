# vpar.py - access virtual parallel port of patched FS-UAE

import select
import time

# bit masks for ctl flags
BUSY_MASK = 1
POUT_MASK = 2
SEL_MASK = 4
ACK_MASK = 8

VPAR_STROBE  = 0x08
VPAR_REPLY   = 0x10
VPAR_INIT    = 0x40
VPAR_EXIT    = 0x80

class VPar:
    def __init__(self, par_file, verbose=False):
        self.verbose = verbose
        self.par_file = par_file
        self.ctl = 0
        self.dat = 0
        self.fd = par_file.get_fd()
        self.init_flag = False
        self.exit_flag = False
    
    def close(self):
        """close the associated vpar I/O channel"""
        self.par_file.close()

    def drain(self):
        """slurp all incoming data"""
        while self._has_data():
            self.par_file.read(1)
            
    def _has_data(self, timeout=0):
        ready = select.select([self.fd],[],[],timeout)[0]
        return len(ready) > 0

    def _read(self, timeout=0, what="RX"):
        # poll port - if something is here read it first
        if self._has_data(timeout):
            d = self.par_file.read(2)
            self.ctl = ord(d[0])
            self.dat = ord(d[1])
            if self.ctl & VPAR_INIT == VPAR_INIT:
                self.init_flag = True
            if self.ctl & VPAR_EXIT == VPAR_EXIT:
                self.exit_flag = True
            if self.verbose:
                self._log("%s: ctl=%02x dat=%02x [%02x %02x]" % (what, self.ctl, self.dat, self.ctl, self.dat))
            return True
        else:
          return False
    
    def _write(self, data, timeout=None):
        self.par_file.write(data)
        # always receive ack value
        return self._read(timeout, "AK")
    
    def poll_state(self, timeout=0):
        """check if a state update is available on I/O channel
           return True if state update was available
        """
        return self._read(timeout)
    
    def request_state(self, timeout=None):
        """request a state update from the emulator"""
        if self.verbose:
            self._log("tx: request")        
        return self._write(chr(0)+chr(0), timeout)
            
    def trigger_ack(self):
        """trigger ACK flag of emulator's parallel port"""
        cmd = ACK_MASK
        if self.verbose:
            self._log("tx: ack [%02x %02x]" % (cmd,0))        
        return self._write(chr(cmd) + chr(0))

    def set_control_mask(self, val, timeout=None):
        """set bits of control port"""
        cmd = 0x40 + val
        if self.verbose:
            self._log("tx: set=%02x [%02x %02x]" % (val,cmd,0))
        return self._write(chr(cmd)+chr(0), timeout)
        
    def clr_control_mask(self, val, timeout=None):
        """clear bits of control port"""
        cmd = 0x80 + val
        if self.verbose:
            self._log("tx: clr=%02x [%02x %02x]" % (val,cmd,0))
        return self._write(chr(cmd)+chr(0), timeout)
        
    def set_data(self, val, timeout=None):
        """set data port value (if configured as input)"""
        cmd = 0x10
        if self.verbose:
            self._log("tx: dat=%02x [%02x %02x]" % (val,cmd,val))
        return self._write(chr(cmd)+chr(val), timeout)
        
    def peek_control(self):
        """get last value of control bits"""
        return self.ctl
    
    def peek_data(self):
        """get last value of data bits"""
        return self.dat
    
    def check_init_flag(self):
        """check and clear init flag"""
        v = self.init_flag
        self.init_flag = False
        return v
    
    def check_exit_flag(self):
        """check and clear exit flag"""
        v = self.exit_flag
        self.exit_flag = False
        return v
    
    def _log(self, msg):
        ts = time.time()
        sec = int(ts)
        usec = int(ts * 1000000) % 1000000 
        print "%8d.%06d" % (sec,usec),msg
