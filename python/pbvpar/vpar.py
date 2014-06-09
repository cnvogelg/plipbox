# vpar.py - access virtual parallel port of patched FS-UAE

import select
import time

# bit masks for ctl flags
BUSY_MASK = 1
POUT_MASK = 2
SEL_MASK = 4
ACK_MASK = 8

VPAR_STROBE = 0x08
VPAR_REPLY = 0x10
VPAR_INIT = 0x40
VPAR_EXIT = 0x80


class VPar:

    def __init__(self, par_file, debug=False):
        self.debug = debug
        self.par_file = par_file
        self.ctl = 0
        self.dat = 0
        self.init_flag = False
        self.exit_flag = False
        self.reply_flag = False
        self.strobe_flag = False

    def drain(self):
        """slurp all incoming data"""
        while self._has_data():
            self.par_file.read(1)

    def _has_data(self, timeout=0):
        fd = self.par_file._fd
        if fd is None:
            return False
        ready = select.select([fd], [], [], timeout)[0]
        return len(ready) > 0

    def can_write(self, timeout=0):
        fd = self.par_file._fd
        if fd is None:
            return False
        ready = select.select([], [fd], [], timeout)[1]
        return len(ready) > 0

    def _decode_ctl(self, ctl, only=False):
        is_busy = (ctl & 1) == 1
        is_pout = (ctl & 2) == 2
        is_select = (ctl & 4) == 4

        if is_busy:
            res = "BUSY "
        elif only:
            res = "---- "
        else:
            res = "busy "

        if is_pout:
            res += "POUT "
        elif only:
            res += "---- "
        else:
            res += "pout "

        if is_select:
            res += "SELECT "
        elif only:
            res += "------ "
        else:
            res += "select "

        if ctl & VPAR_STROBE == VPAR_STROBE:
            res += "STROBE "
        if ctl & VPAR_INIT == VPAR_INIT:
            res += "INIT "
        if ctl & VPAR_EXIT == VPAR_EXIT:
            res += "EXIT "
        if ctl & VPAR_REPLY == VPAR_REPLY:
            res += "REPLY "
        return res

    def _read(self, timeout=0, what="RX"):
        # poll port - if something is here read it first
        if self._has_data(timeout):
            d = self.par_file.read(2)
            ctl = ord(d[0])
            dat = ord(d[1])
            if ctl & VPAR_INIT == VPAR_INIT:
                self.init_flag = True
            if ctl & VPAR_EXIT == VPAR_EXIT:
                self.exit_flag = True
            if ctl & VPAR_REPLY == VPAR_REPLY:
                self.reply_flag = True
            if ctl & VPAR_STROBE == VPAR_STROBE:
                self.strobe_flag = True
            self.ctl = ctl & 0x7
            self.dat = dat
            if self.debug:
                self._log("%s: ctl=%02x dat=%02x [%02x %02x] %s" %
                          (what, self.ctl, self.dat, ctl, dat,
                           self._decode_ctl(ctl)))
            return True
        else:
            return False

    def _write(self, data, timeout=None):
        """return True=write+read ok, False=write or read failed"""
        # check if we can write
        if not self.can_write(timeout):
            return False
        self.par_file.write(data)
        # wait for reply from emu. skip updates
        num = 0
        while True:
            ok = self._read(timeout, "R%d" % num)
            if not ok:
                return False
            # make sure it has reply flag set
            if self.check_reply_flag():
                break
            num += 1
        return True

    def poll_state(self, timeout=0):
        """check if a state update is available on I/O channel
           return True if state update was available
        """
        return self._read(timeout)

    def request_state(self, timeout=None):
        """request a state update from the emulator"""
        if self.debug:
            self._log("tx: request")
        return self._write(chr(0)+chr(0), timeout)

    def trigger_ack(self):
        """trigger ACK flag of emulator's parallel port"""
        cmd = ACK_MASK
        if self.debug:
            self._log("tx: ACK           [%02x %02x]" % (cmd, 0))
        return self._write(chr(cmd) + chr(0))

    def set_control_mask(self, val, timeout=None):
        """set bits of control port"""
        cmd = 0x40 + val
        if self.debug:
            self._log("tx: SET=%02x        [%02x %02x] %s" %
                      (val, cmd, 0, self._decode_ctl(val, True)))
        return self._write(chr(cmd)+chr(0), timeout)

    def clr_control_mask(self, val, timeout=None):
        """clear bits of control port"""
        cmd = 0x80 + val
        if self.debug:
            self._log("tx: CLEAR=%02x      [%02x %02x] %s" %
                      (val, cmd, 0, self._decode_ctl(val, True)))
        return self._write(chr(cmd)+chr(0), timeout)

    def set_data(self, val, timeout=None):
        """set data port value (if configured as input)"""
        cmd = 0x10
        if self.debug:
            self._log("tx: DATA=%02x       [%02x %02x]" %
                      (val, cmd, val))
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

    def check_reply_flag(self):
        v = self.reply_flag
        self.reply_flag = False
        return v

    def check_strobe_flag(self):
        v = self.strobe_flag
        self.strobe_flag = False
        return v

    def _log(self, msg):
        ts = time.time()
        sec = int(ts)
        usec = int(ts * 1000000) % 1000000
        print "%8d.%06d" % (sec, usec), msg
