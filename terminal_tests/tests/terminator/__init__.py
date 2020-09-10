#!/usr/bin/python3

from __main__ import *

class Test(ATest):
    def available(self):
        return which('terminator')

    def run(self, args):
        subprocess.call(['terminator', '--no-dbus', '-g', 'config.ini', '-x'] + args)

