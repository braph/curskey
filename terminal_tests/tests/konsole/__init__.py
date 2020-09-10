#!/usr/bin/python3

from __main__ import *

class Test(ATest):
    def available(self):
        return which('konsole')

    def pre_run(self):
        pass

    def run(self, args):
        pass
        #subprocess.call(['urxvt', '-e'] + args)

    def cleanup(self):
        pass

