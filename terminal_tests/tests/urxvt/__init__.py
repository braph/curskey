#!/usr/bin/python3

from __main__ import *

class Test(ATest):
    def __init__(self):
        self.xrdb = Xrdb()

    def available(self):
        return which('xterm')

    def pre_run(self):
        self.xrdb.save('xterm-')
        self.xrdb.load('Xresources')

    def run(self, args):
        subprocess.call(['xterm', '-e'] + args)

    def cleanup(self):
        self.xrdb.restore()

