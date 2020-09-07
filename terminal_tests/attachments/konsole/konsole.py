#!/usr/bin/python3
import os, sys
from lxml import etree

''' Wrapper script for launching Konsole with all keys unbound '''

# To export Konsole's keyboard scheme, do:
#   Konsole
#     -> Settings
#     -> Configure Keyboard Shortcuts
#     -> Manage Schemes
#     -> More Actions
#     -> Export Scheme

FILES = ['sessionui.rc', 'konsoleui.rc']
SCHEME_FILE = 'exported_keymap'
ACTION_NAMES = []
sys.argv[0] = 'konsole'

with open(SCHEME_FILE, 'r') as fh:
    for line in fh:
        if '=' in line:
            ACTION_NAMES.append(line.split('=')[0])

def update_configuration(infile, outfile):
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
    for name in ACTION_NAMES:
        Action = etree.Element('Action')
        Action.attrib['shortcut'] = ''
        Action.attrib['name'] = name
        ActionProperties.insert(0, Action)

    tree.write(outfile)

try:
    for file in FILES:
        conffile = os.path.expanduser('~/.local/share/kxmlgui5/konsole/%s' % file)
        backfile = conffile + '.back'

        if os.path.exists(conffile) and not os.path.exists(backfile):
            os.rename(conffile, backfile)

        update_configuration(file, conffile)

    os.spawnvp(os.P_WAIT, sys.argv[0], sys.argv)
finally:
    for file in FILES:
        conffile = os.path.expanduser('~/.local/share/kxmlgui5/konsole/%s' % file)
        backfile = conffile + '.back'

        if os.path.exists(backfile):
            os.rename(backfile, conffile)
