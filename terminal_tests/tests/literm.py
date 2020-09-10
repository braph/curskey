#!/usr/bin/python3

from __main__ import *

class Test(ATest):
    def available(self):
        return which('literm')

    def run(self, args):
        subprocess.call(['literm', '-e'] + args)
