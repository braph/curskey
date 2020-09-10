#!/usr/bin/python3

from __main__ import *

class Test(ATest):
    def available(self):
        return which('rxvt')

    def run(self, args):
        subprocess.call(['rxvt', '-e'] + args)
