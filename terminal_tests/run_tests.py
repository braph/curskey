#!/usr/bin/python3
import sys, os, re, argparse
from lib.xrdb import *
from lib.which import *
from lib.filebackup import *

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

class ATest:
    def available(self):
        ''' Can this test be executed? '''
        return True

    def blacklist(self):
        ''' Which keys to blacklist '''
        return []

    def pre_run(self):
        pass

    def run(self, args):
        raise NotImplementedError()

    def cleanup(self):
        pass

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

TESTS = {}

for f in os.listdir('./tests'):
    if f == '__pycache__':
        continue

    elif f == 'README.md':
        continue ## TODO

    directory = None
    if f.endswith('.py'):
        f = f[0:-3]
        directory = os.path.abspath('.')
    else:
        directory = os.path.abspath('./tests/' + f)
        
    #TESTS[f] = __import__('tests.' + f)
    TESTS[f] = dict(
        module    = __import__('tests.' + f, fromlist=['tests']),
        directory = directory
    )

args.outdir = os.path.abspath(args.outdir)

try:
    os.mkdir(args.outdir)
except FileExistsError:
    pass

selected_tests = args.tests if args.tests else TESTS.keys()
for name in selected_tests:
    test = TESTS[name]
    os.chdir(test['directory'])

    test = test['module'].Test()

    if not test.available():
        print(name, 'is not available')
        continue

    outfile = os.path.join(args.outdir, "%s.json" % name)
    argv = [TEST_BIN, '-o', outfile]
    for key in args.blacklist:   argv.extend(['-b', key])
    for key in test.blacklist(): argv.extend(['-b', key])
    print(name, argv)

    try:
        test.pre_run()
        test.run(argv)
    except Exception as e:
        print(e)

    try:
        test.cleanup()
    except Exception as e:
        print(e)
