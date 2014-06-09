import time

import pbproto
import sopty
import vpar
import threading


class PBVPar:

    def __init__(self, vpar_link, debug=0, log=None):
        self.vpar_link = vpar_link
        debug_vpar = ((debug & 1) == 1)
        debug_pb = ((debug & 2) == 2)
        self.sp = sopty.SoPTY(vpar_link)
        self.vpar = vpar.VPar(self.sp, debug=debug_vpar)
        self.pb = pbproto.PBProto(self.vpar, debug=debug_pb)

        def _dummy(*l):
            pass
        if log is None:
            log = _dummy
        self._log = log
        self._do_sync = True
        self._is_open = False
        self._tx_pkt = None
        self._lock = threading.Lock()

    def open(self):
        self.sp.open()
        self._is_open = True

    def close(self):
        self._is_open = False
        self.sp.close()

    def tx_pkt(self, buf):
        """tx triggered from ethernet reader thread"""
        if not self._is_open:
            return False
        # syncronize vpar access
        with self._lock:
            return self.pb.send(buf)

    def rx_pkt(self, timeout=None):
        # is open?
        if not self._is_open:
            return False
        # synchronize vpar access
        with self._lock:
            # first check if the emu is attached
            if not self.vpar.can_write(timeout):
                return None
            # check state
            if not self._check_status(timeout):
                return None
            # handle command
            if not self._handle_cmd(timeout):
                return None
            # was a receive?
            return self.pb.recv()

    def _check_status(self, timeout=None):
        """return False if no state update"""
        # need a vpar re-sync
        if self._do_sync:
            ok = self.vpar.request_state(timeout)
            if not ok:
                return False
            self._log("start plipbox protocol")
            self.pb.start()
            self._do_sync = False

        # check emulator flags in vpar
        if self.vpar.check_init_flag():
            self._log("emu restarted")
            self._do_sync = True
        if self.vpar.check_exit_flag():
            self._log("emu exited")
            self._do_sync = True

        return True

    def _handle_cmd(self, timeout=None):
        cmd = 0
        try:
            # handle par command
            s = time.time()
            res = self.pb.handle(timeout)
            if res is None:
                return False
            cmd, n = res
            e = time.time()
            d = e - s
            self._log("%12.6f *CMD%02x* :" %
                      (s, cmd), self._get_speed_bar(d, n))
            return True
        except pbproto.PBProtoError as ex:
            e = time.time()
            d = e - s
            self._log("%12.6f *CMD%02x* : ERROR %s. delta=%5.3f" %
                      (s, cmd, ex, d))
            return False

    def _calc_speed(self, delta, size):
        if delta > 0:
            kibs = size / (delta * 1024)
        else:
            kibs = 0
        return "%6.3f KiB/s" % kibs

    def _get_speed_bar(self, delta, n):
        return "%s   %12.6f s" % (self._calc_speed(delta, n), delta)
