import time

import pbproto
import sopty
import vpar


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

    def open(self):
        self.sp.open()

    def close(self):
        self.sp.close()

    def check_status(self):
        """return True after reset, False for quit, None otherwise"""
        # need a vpar re-sync
        if self._do_sync:
            self._log("sync vpar with emu")
            self.vpar.request_state()
            self._log("start plipbox protocol")
            self.pb.start()
            self._do_sync = False
            return True

        # check emulator flags in vpar
        if self.vpar.check_init_flag():
            self._log("emu restarted")
            self._do_sync = True
        if self.vpar.check_exit_flag():
            self._log("emu exit")
            return False

    def tx_pkt(self, buf, timestamp=0):
        self.pb.send(buf)
        return self.pb.must_handle()

    def rx_pkt(self, timestamp=0):
        cmd = 0
        try:
            # handle par command
            s = time.time()
            cmd, n = self.pb.handle()
            e = time.time()
            d = e - s
            self._log("%12.6f *CMD%02x* :" %
                      (timestamp, cmd), self._get_speed_bar(d, n))
        except pbproto.PBProtoError as ex:
            e = time.time()
            d = e - s
            self._log("%12.6f *CMD%02x* : ERROR %s. delta=%5.3f" %
                      (timestamp, cmd, ex, d))
        # was a receive?
        return self.pb.recv()

    def _calc_speed(self, delta, size):
        if delta > 0:
            kibs = size / (delta * 1024)
        else:
            kibs = 0
        return "%6.3f KiB/s" % kibs

    def _get_speed_bar(self, delta, n):
        return "%s   %12.6f s" % (self._calc_speed(delta, n), delta)
