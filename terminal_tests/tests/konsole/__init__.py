#!/usr/bin/python3

from __main__ import *
from lxml import etree

# To export Konsole's keyboard scheme, do:
#   Konsole
#     -> Settings
#     -> Configure Keyboard Shortcuts
#     -> Manage Schemes
#     -> More Actions
#     -> Export Scheme

# TODO: Better File Cleanup, Konsole uses more config files than described here!

class Test(ATest):

    def __init__(self):
        self.SCHEME_FILE = 'exported_keymap'
        self.ACTION_NAMES = []
        self.sessionui_rc_backup = FileBackup('~/.local/share/kxmlgui5/konsole/sessionui.rc', True)
        self.konsoleui_rc_backup = FileBackup('~/.local/share/kxmlgui5/konsole/konsoleui.rc', True)

        with open(self.SCHEME_FILE, 'r') as fh:
            for line in fh:
                if '=' in line:
                    self.ACTION_NAMES.append(line.split('=')[0])


    def update_configuration(self, infile, outfile):
        '''
        For each available action add a <Action> inside <ActionProperties>:

          <ActionProperties scheme="Default">
            <Action shortcut="" name="foo-action" />
            <Action shortcut="" name="bar-action" />
            ...
          </ActionProperties>
        '''

        tree = etree.parse(infile)
        root = tree.getroot()

        ActionProperties = None

        for e in tree.iter():
            if e.tag == 'Action':
                e.attrib['shortcut'] = ""
            elif e.tag == 'ActionProperties':
                ActionProperties = e

        # Create <ActionProperties> and insert it if needed
        if ActionProperties is None:
            ActionProperties = etree.Element('ActionProperties')
            ActionProperties.attrib['scheme'] = "Default"
            root.insert(0, ActionProperties)

        # Create <Action name="..." shortcut=""> and insert them into ActionProperties
        for name in self.ACTION_NAMES:
            Action = etree.Element('Action')
            Action.attrib['shortcut'] = ''
            Action.attrib['name'] = name
            ActionProperties.insert(0, Action)

        tree.write(outfile)

    def available(self):
        return which('konsole')

    def pre_run(self):
        self.sessionui_rc_backup.save()
        self.konsoleui_rc_backup.save()

        os.makedirs(os.path.expanduser('~/.local/share/kxmlgui5/konsole'), exist_ok=True)

        self.update_configuration('konsoleui.rc', self.konsoleui_rc_backup.filename())
        self.update_configuration('sessionui.rc', self.sessionui_rc_backup.filename())

    def run(self, args):
        subprocess.call(['konsole', '-e'] + args)

    def cleanup(self):
        self.sessionui_rc_backup.restore()
        self.konsoleui_rc_backup.restore()

