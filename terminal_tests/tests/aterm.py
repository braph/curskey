#!/usr/bin/python3

from __main__ import *

class Test(ATest):
    def available(self):
        return which('aterm')

    def run(self, args):
        subprocess.call(['aterm', '-e'] + args)
