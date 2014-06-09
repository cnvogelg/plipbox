import subprocess
import sys
import os
import pty


class SoPTY:

    def __init__(self, file_name):
        self._file_name = file_name
        self._slave_name = None
        self._fd = None
        self._fd_slave = None

    def open(self):
        (master, slave) = pty.openpty()
        self._slave_name = os.ttyname(slave)
        if os.path.exists(self._file_name):
            os.unlink(self._file_name)
        os.symlink(self._slave_name, self._file_name)
        self._fd = master
        self._fd_slave = slave
        self._clean_tty()

    def _clean_tty(self):
        """make given tty really transparent for 8 bit transfers"""
        # stty uses -f on Mac OS X but -F on Linux
        if sys.platform == 'darwin':
            flag = '-f'
        else:
            flag = '-F'
        self._cmd = ['/bin/stty', flag, self._slave_name,
                     'cs8', 'raw', '-echo', '-onlcr', '-echoctl',
                     '-echoke', '-echoe', '-iexten']
        subprocess.check_call(self._cmd)

    def get_fd(self):
        return self._fd

    def close(self):
        try:
            os.close(self._fd)
            os.close(self._fd_slave)
            os.unlink(self._file_name)
            self._fd = None
            self._fd_slave = None
        except OSError:
            pass

    def read(self, size):
        return os.read(self._fd, size)

    def write(self, buf):
        os.write(self._fd, buf)

    def flush(self):
        os.fsync(self._fd)
