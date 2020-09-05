#!/usr/bin/python3

def keys():
    yield from [
        'Up', 'Down', 'Left', 'Right',
        'Insert', 'Delete', 'Home', 'End', 'Prior', 'Next',
        'Tab', 'Escape', 'BackSpace', 'space'
    ]

    for i in range(1, 13):
        yield 'F%d' % i

    for i in range(ord('a'), ord('z') + 1):
        yield chr(i)

def modifier_prefixes():
    for i in range(0, 7):
        m = ''
        if i & 1: m += 'Meta-'
        if i & 2: m += 'Ctrl-'
        if i & 4: m += 'Shift-'
        yield m

print('! Disable ISO14755 mode popup when pressing Shift+Ctrl')
print('URxvt.iso14755: false')
print('! Unbind all terminal defaults bindings')
for mod in modifier_prefixes():
    for key in keys():
        print('URxvt.keysym.%s%s:\tbuiltin-string:' % (mod, key))

