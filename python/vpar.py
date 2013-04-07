# vpar.py - access virtual parallel port of patched FS-UAE

import select

# bit masks for ctl flags
BUSY_MASK = 1
POUT_MASK = 2
SEL_MASK = 4
ACK_MASK = 8

class VPar:
    def __init__(self, par_name=None, verbose=False):
        self.par_name = par_name
        self.verbose = verbose
        self.par_file = open(self.par_name, "r+b", 0)
        self.ctl = 0
        self.dat = 0
    
    def close(self):
        self.par_file.close()

    def drain(self):
        """slurp all incoming data"""
        while self.has_data():
            self.par_file.read(1)
            
    def has_data(self, timeout=0):
        """poll if new data is available"""
        ready = select.select([self.par_file],[],[],timeout)[0]
        return len(ready)

    def read(self, timeout=0):
        """read status and data"""
        # poll port - if something is here read it first
        if self.has_data(timeout):
            d = self.par_file.read(2)
            self.ctl = ord(d[0])
            self.dat = ord(d[1])
            if self.verbose:
                print("RX: ctl=%02x dat=%02x [%02x %02x]" % (self.ctl, self.dat, self.ctl, self.dat))
    
    def write(self, data):
        self.par_file.write(data)
        self.par_file.flush()
    
    def request_state(self, timeout=10):
        self.write(chr(0)+chr(0))
        self.read(timeout)
            
    def trigger_ack(self):
        cmd = ACK_MASK
        self.write(chr(cmd) + chr(0))
        if self.verbose:
            print("tx: ack [%02x %02x]" % (cmd,0))        

    def set_status_mask(self, val):
        cmd = 0x40 + val
        self.write(chr(cmd)+chr(0))
        if self.verbose:
            print("tx: set=%02x [%02x %02x]" % (val,cmd,0))
        
    def clr_status_mask(self, val):
        cmd = 0x80 + val
        self.write(chr(cmd)+chr(0))
        if self.verbose:
            print("tx: clr=%02x [%02x %02x]" % (val,cmd,0))
        
    def set_data(self, val):
        cmd = 0x10
        self.write(chr(cmd)+chr(val))
        if self.verbose:
            print("tx: dat=%02x [%02x %02x]" % (val,cmd,val))
        
    def get_status(self, timeout=0):
        self.read(timeout)
        return self.ctl
    
    def get_data(self, timeout=0):
        self.read(timeout)
        return self.dat
        
    def get_status_data(self, timeout=0):
        self.read(timeout)
        return (self.ctl, self.dat)
        
    def peek_status(self):
        return self.ctl

