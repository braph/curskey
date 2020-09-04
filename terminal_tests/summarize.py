#!/usr/bin/python3

import sys, html, json, glob

KEY    = 0
KEYSEQ = 4
STATUS = 5

results = {}

def warn(*a, **kw):
    print(*a, **kw, file=sys.stderr)

def array_to_dictionary(array, key_position):
    """
    Transform
        [["KEY_FOO", ...], ["KEY_BAR", ...]]
    To
        {"KEY_FOO": ..., "KEY_BAR": ...}
    """
    dictionary = {}
    for row in array:
        key = row[key_position]
        dictionary[key] = row
    return dictionary

def all_keys(results):
    keys = set()
    for filename, json in results.items():
        keys.update(json.keys())
    return sorted(list(keys))

def print_row(tag, fields):
    print('<'+tag+'>')
    for field in fields:
        print('<td>' + html.escape(field) + '</td>')
    print('</'+tag+'>')

def print_test_summary(results):
    print('<table>')
    print_row('tr', ['terminal'] + list(results.keys()))
    for key in all_keys(results):
        row = [key]
        for filename, result in results.items():
            row.append(result[key][STATUS])
        print_row('tr', row)
    print('</table>')

def print_keyseq_table(results):
    print('<table>')
    print_row('tr', [''] + list(results.keys()))
    for key in all_keys(results):
        row = [key]
        for filename, result in results.items():
            row.append(result[key][KEYSEQ])
        print_row('tr', row)
    print('</table>')

for filename in glob.glob('*.json'):
    with open(filename, 'r') as fh:
        results[filename] = array_to_dictionary(json.load(fh), KEY)


print('<html>')
print('<head>')
print('<title>')
print('Test summary')
print('</title>')
print('</head>')
print('<body>')
print_test_summary(results)
print_keyseq_table(results)
print('</body>')
print('</html>')
