#!/usr/bin/python3

from __main__ import *

class Test(ATest):
    def __init__(self):
        self.xrdb = Xrdb()

    def available(self):
        return which('urxvt')

    def pre_run(self):
        self.xrdb.save('urxvt-')
        self.xrdb.load('Xresources')

    def run(self, args):
        subprocess.call(['urxvt', '-e'] + args)

    def cleanup(self):
        self.xrdb.restore()

