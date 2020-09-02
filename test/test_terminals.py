#!/usr/bin/python3

import os, argparse
#argp = argparse.ArgumentParser('Foo')

tests = {}

class Test:
    def __init__(self, name, argv, blacklist):
        self.name = name
        self.argv = argv
        self.blacklist = blacklist if blacklist else []

def t(name, argv, blacklist=None):
    tests[name] = Test(name, argv, blacklist)

t('st',           ['st', '-e'])
t('eterm',        ['Eterm', '-e'])
t('xterm',        ['xterm', '-e'])
t('konsole',      ['konsole', '-e'])
t('aterm',        ['aterm', '-e'])
t('terminology',  ['terminology', '-e'])
t('terminator',   ['terminator', '--no-dbus', '-x'], ['A-l', 'F8', 'F11'])
t('rxvt',         ['rxvt', '-e'])
t('urxvt',        ['urxvt', '-e'])
t('literm',       ['literm', '-e'])

for name, test in tests.items():
    print(name)
    outfile = "%s.json" % name
    argv = test.argv
    argv.extend(['./terminal_test', '-o', outfile])
    for key in test.blacklist:
        argv.extend(['-b', key])
    os.spawnvp(os.P_WAIT, argv[0], argv)

