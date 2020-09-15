#!/usr/bin/python3

from __main__ import *

class Test(ATest):
    def __init__(self):
        self._available = False
        self._terminfo_backup = FileBackup('~/.terminfo', True)

        # Already compiled
        if os.path.exists('./st/st'):
            self._available = True
            return

        # Checkout source
        if not os.path.exists('st'):
            subprocess.check_call(['git', 'clone', 'https://git.suckless.org/st'])

        with open('./st/config.def.h', 'r') as fh:
            config_h = fh.read()

        search  = r'static Shortcut shortcuts \[ \] = \{ .*? \} ;'
        replace = r'static Shortcut shortcuts[] = {};'
        search = search.replace(' ', '\\s*')
        config_h = re.sub(search, replace, config_h, 0, re.DOTALL)

        with open('./st/config.h', 'w') as fh:
            fh.write(config_h)

        subprocess.check_call(['make'], cwd='st')

        self._available = True

    def available(self):
        return self._available

    def pre_run(self):
        self._terminfo_backup.save()
        subprocess.check_call(['tic', '-sx', './st/st.info'])

    def run(self, args):
        subprocess.call(['./st/st', '-e'] + args)

    def cleanup(self):
        self._terminfo_backup.restore()

