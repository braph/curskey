#!/usr/bin/python3

import os, argparse
#argp = argparse.ArgumentParser('Foo')

tests = {}
blacklist = [
  'S-F10', # opens context menu (X11 wide)
]

class Test:
    def __init__(self, name, argv, blacklist, init):
        self.name = name
        self.argv = argv
        self.blacklist = blacklist if blacklist else []
        self.init = init

def t(name, argv, blacklist=None, init=None):
    tests[name] = Test(name, argv, blacklist, init)

#theterminal

t('terminator',
 ['terminator', '--no-dbus', '-g', 'terminator_config', '-x'],
 ['A-a', # key is not sent
  'A-g', # key is not sent
  'A-l', # open launch dialog
  'A-o', # key is not sent
  'F1',  # opens help in a browser
  'F11'  # maximize window
])

#t('xterm',
# ['xterm', '-e'],
# [],
# '''
# echo 'xterm*metaSendsEscape: true' > /tmp/xresources.temp;
# xrdb /tmp/xresources.temp;
# rm /tmp/xresources.temp;
# ''')

#t('st',           ['st', '-e']) TODO?!
#t('eterm',        ['Eterm', '-e'])
#t('konsole',      ['konsole', '-e'])
#t('aterm',        ['aterm', '-e'])
#t('terminology',  ['terminology', '-e'])
#t('rxvt',         ['rxvt', '-e'])
#t('urxvt',        ['urxvt', '-e'])
#t('literm',       ['literm', '-e'])

for name, test in tests.items():
    outfile = "%s.json" % name
    argv = test.argv
    argv.extend(['./terminal_test', '-o', outfile])
    for key in test.blacklist:    argv.extend(['-b', key])
    for key in blacklist:         argv.extend(['-b', key])
    print(name, argv)
    if test.init:
        os.system(test.init)
    os.spawnvp(os.P_WAIT, argv[0], argv)

