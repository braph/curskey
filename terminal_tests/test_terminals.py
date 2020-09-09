#!/usr/bin/python3
import sys, os, argparse

''' Run the test binary in all terminals '''

os.chdir(os.path.dirname(sys.argv[0]))

TEST_BIN = os.path.abspath('./terminal_test')
TESTS = {}
BLACKLIST = [
  'S-F10',                            # Opens context menu (X11 wide)
  'S-Insert', 'A-S-Insert',           # Shift-Insert pastes X11 clipboard
  'C-u', 'C-S-u', 'C-A-u', 'C-S-A-u', # Ctrl-U kills text to beginning of the line
  'C-h', 'C-M-h',                     # Ctrl-H = Backspace
  'C-m', 'C-M-m',                     # Ctrl-M = Return
  'C-q', 'C-M-q'                      # Ctrl-Q = XON
]

class Test:
    def __init__(self, name, argv, blacklist, init):
        self.name = name
        self.argv = argv
        self.blacklist = blacklist if blacklist else []
        self.init = init

def t(name, argv, blacklist=None, init=None):
    TESTS[name] = Test(name, argv, blacklist, init)

t('terminator',
 ['terminator', '--no-dbus', '-g', 'attachments/terminator.ini', '-x'])

t('xterm',
 ['xterm', '-e'],
 [],
 ''' xrdb attachments/xterm.xresources ''')

t('urxvt',
 ['urxvt', '-e'],
 [],
 ''' xrdb attachments/urxvt.xresources ''')

t('konsole',
 ['./attachments/konsole/konsole.py', '-e'])

t('st',
 ['./attachments/st/st/st', '-e'])

t('kitty',
 ['kitty', '-o', 'clear_all_shortcuts=yes', '-e'])

t('eterm',        ['Eterm', '-e'])
t('aterm',        ['aterm', '-e'])
t('rxvt',         ['rxvt', '-e'])
t('literm',       ['literm', '-e'])
#t('terminology',  ['terminology', '-e'])
#t('theterminal',  ['theterminal', '-e'])

argp = argparse.ArgumentParser(description=__doc__)
argp.add_argument('-o', metavar='OUTPUT DIRECTORY', dest='outdir',
    default='results',
    help='Specify output directory for test results')
argp.add_argument('-b', metavar='KEY', action='append', dest='blacklist',
    default=BLACKLIST,
    help='Do not test KEY')
argp.add_argument('-t', metavar='TEST', action='append', dest='tests',
    help='Only run TEST - can be specified multiple times')
args = argp.parse_args();


try:
    os.mkdir(args.outdir)
except FileExistsError:
    pass

selected_tests = args.tests if args.tests else TESTS.keys()
for name in selected_tests:
    test = TESTS[name]
    outfile = os.path.join(args.outdir, "%s.json" % name)
    outfile = os.path.abspath(outfile) # TODO?
    argv = test.argv + [TEST_BIN, '-o', outfile]
    for key in test.blacklist: argv.extend(['-b', key])
    for key in args.blacklist: argv.extend(['-b', key])
    print(name, argv)
    if test.init:
        os.system(test.init)
    os.spawnvp(os.P_WAIT, argv[0], argv)

