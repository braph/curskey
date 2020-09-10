#!/usr/bin/python3

from __main__ import *

class Test(ATest):
    def available(self):
        return which('Eterm')

    def run(self, args):
        subprocess.call(['Eterm', '-e'] + args)
