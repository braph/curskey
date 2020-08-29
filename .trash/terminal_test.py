#!/usr/bin/python3

import sys, os, signal

TEMP_FILE = "/tmp/curses_lib.get_key.result.tmp"

if os.system('which xdotool >/dev/null'):
    print("xdotool binary not found")
    sys.exit(1)

class Tester:
    def spawn_terminal(self, binary, *args):
        self.terminal_pid = os.spawnlp(os.P_NOWAIT, binary, *[binary + *args])

    def kill_terminal(self):
        os.kill(self.terminal_pid, signal.SIGTERM)

def call_get_key(keydef):
    os.system("./gey_key %s > %s; cat %s" % (keydef, TEMP_FILE, TEMP_FILE))

call_get_key("DELETE")

