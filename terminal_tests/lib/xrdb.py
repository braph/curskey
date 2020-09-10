#!/usr/bin/python3

import tempfile
import subprocess

class Xrdb():
    ''' Python API for xrdb(1) '''

    def __init__(self):
        self._save_file = None

    def save(self, prefix=None):
        ''' Save all properties to a NamedTemporaryFile(prefix=prefix) '''
        try:
            self._save_file = tempfile.NamedTemporaryFile(prefix=prefix)
            subprocess.check_call(['xrdb', '-query'], stdout=self._save_file.file)
            self._save_file.file.flush()
        except:
            self._save_file = None
            raise

    def restore(self):
        ''' Restore all properties that have been previously saved by `save()` '''
        if self._save_file:
            self.load(self._save_file.name)

    def load(self, filename):
        ''' Load all properties from `filename` '''
        subprocess.check_call(['xrdb', '-load', filename])
