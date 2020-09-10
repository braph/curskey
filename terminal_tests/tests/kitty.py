#!/usr/bin/python3

from __main__ import *

class Test(ATest):
    def available(self):
        return which('kitty')

    def run(self, args):
        subprocess.call(['kitty', '-o', 'clear_all_shortcuts=yes', '-e'] + args)
