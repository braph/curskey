#!/usr/bin/python3

import os, shutil

class FileBackup():
    def __init__(self, filename, expand=True):
        if expand:
            filename = os.path.expanduser(filename)

        self._filename = os.path.abspath(filename)
        self._saved = False;

    def filename(self):
        return self._filename

    def save(self):
        self._saved = True
        self._file_existed = os.path.exists(self._filename)

        if self._file_existed:
            try:
                os.rename(self._filename, self._filename + '.backup')
            except:
                self._saved = False
                raise

    def restore(self):
        if not self._saved:
            return

        if self._file_existed:
            os.rename(self._filename + '.backup', self._filename)
        else:
            try:
                os.unlink(self._filename)
            except FileNotFoundError:
                pass
            except IsADirectoryError:
                shutil.rmtree(self._filename, ignore_errors=True)


class FileBackups():
    def __init__(self):
        self._file_backups = []

    def add(self, *a, **kw):
        self._file_backups.append(FileBackup(*a, **kw))

    def save(self):
        for backup in self._file_backups:
            try:    backup.save()
            except: pass

    def restore(self):
        for backup in self._file_backups:
            try:    backup.restore()
            except: pass

