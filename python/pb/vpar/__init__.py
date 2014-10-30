from __future__ import print_function

import time

import pbproto
import sopty
import vpar
import threading
import Queue


class PBVPar:

    ACTION_RX_EVENT = 1
    ACTION_TX_EVENT = 2

    cmd_names = {
        0x11: "ami->pb",
        0x22: "ami<-pb"
    }

    def __init__(self, vpar_link, debug=0):
        self.vpar_link = vpar_link
        debug_vpar = ((debug & 1) == 1)
        debug_pb = ((debug & 2) == 2)
        self.debug = ((debug & 4) == 4)
        self.debug_io = ((debug & 8) == 8)
        self.sp = sopty.SoPTY(vpar_link)
        self.vpar = vpar.VPar(self.sp, debug=debug_vpar)
        self.pb = pbproto.PBProto(self.vpar, debug=debug_pb)
        self._do_sync = True
        self._is_open = False
        self._tx_pkt = None
        # threading
        self._action_cond = threading.Condition()
        self._action_cmd = 0
        self._send_queue = Queue.Queue()
        self._quit_event = threading.Event()
        self._wait_rx_event = threading.Event()
        self._wait_rx_thread = threading.Thread(target=self._run_rx_thread)

    def _add_action(self, cmd):
        self._action_cond.acquire()
        self._action_cmd |= cmd
        self._action_cond.notify_all()
        self._action_cond.release()

    def _get_actions(self, timeout):
        self._action_cond.acquire()
        # get current value
        cmd = self._action_cmd
        self._action_cmd = 0
        if cmd == 0:
            # wait for value
            self._action_cond.wait(timeout)
            cmd = self._action_cmd
            self._action_cmd = 0
        self._action_cond.release()
        return cmd

    def _run_rx_thread(self):
        self._log("run_rx: begin")
        while not self._quit_event.is_set():
            # wait for event
            self._wait_rx_event.wait()
            # now select and block
            self._log("run_rx: wait select")
            ok = self.vpar.can_read(None)
            if ok:
                self._log("run_rx: add rx event")
                self._add_action(self.ACTION_RX_EVENT)
                self._wait_rx_event.clear()
        self._log("run_rx: done")

    def open(self):
        self.sp.open()
        self._is_open = True
        self._wait_rx_event.set()
        self._wait_rx_thread.start()

    def close(self):
        self._is_open = False
        self.sp.close()
        self._quit_event.set()
        self._wait_rx_thread.join()

    def tx_pkt(self, buf):
        """tx triggered from ethernet reader thread"""
        if not self._is_open:
            return False
        # trigger action
        self._log("tx_pkt: add tx event (queue: %d)" %
                  self._send_queue.qsize())
        self._send_queue.put(buf)
        self._add_action(self.ACTION_TX_EVENT)

    def rx_pkt(self, timeout=None):
        # is open?
        if not self._is_open:
            return False
        # first check if the emu is attached
        if not self.vpar.can_write(timeout):
            return None
        # check state
        if not self._check_status(timeout):
            return None
        self._log("rx_pkt: got status")

        # block until actions arrive
        actions = self._get_actions(timeout)
        self._log("rx_pkt: got actions: %d " % actions)

        # need to handle command?
        if self.pb.must_handle() or (actions & self.ACTION_RX_EVENT) != 0:
            # handle command
            self._handle_cmd(timeout)
            # allow to select again
            self._wait_rx_event.set()

        # need to send?
        if not self._send_queue.empty():
            # can we send?
            if self.pb.can_send():
                buf = self._send_queue.get()
                self._log("rx_pkt: do tx (queue %d)" %
                          self._send_queue.qsize())
                self.pb.send(buf)
            else:
                self._log("rx_pkt: pending tx (queue %d)" %
                          self._send_queue.qsize())
                self.pb.request_send()

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
        cmd = -1
        try:
            # handle par command
            s = time.time()
            res = self.pb.handle(timeout)
            if res is None:
                return False
            cmd, n = res
            e = time.time()
            d = e - s

            name = "%02x" % cmd
            if cmd in self.cmd_names:
                name = self.cmd_names[cmd]
            self._log("%s : %s" %
                      (name, self._get_speed_bar(d, n)), io=True)
            return True
        except pbproto.PBProtoError as ex:
            e = time.time()
            d = e - s
            self._log("%12.6f %02x : ERROR %s" %
                      (d, cmd, ex), io=True)
            return False

    def _calc_speed(self, delta, size):
        if delta > 0:
            kibs = size / (delta * 1024)
        else:
            kibs = 0
        return "%6.3f KiB/s" % kibs

    def _get_speed_bar(self, delta, n):
        return "%s   %12.6f s [%d]" % (self._calc_speed(delta, n), delta, n)

    def _log(self, msg, io=False):
        if not io and not self.debug:
            return
        if io and not self.debug_io:
            return
        ts = time.time()
        sec = int(ts)
        usec = int(ts * 1000000) % 1000000
        print("%8d.%06d pv:" % (sec, usec), msg)
