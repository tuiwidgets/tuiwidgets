#! /usr/bin/python3
# SPDX-License-Identifier: BSL-1.0

# entities.json from https://www.w3.org/TR/html5/entities.json

import json
from collections import namedtuple

Entry = namedtuple('Entry', ['len', 'has_next', 'strpool_offset', 'data'])

def fnv1a32(data):
    h = 2166136261

    for b in data:
        h = h ^ b
        h = (h * 16777619) & 0xffffffff;

    return h


j = json.load(open('entities.json', 'r'))

table_size = len(j)

hashtbl = [[] for i in range(table_size)]

for name, v in j.items():
    name = name[1:-1]
    hashtbl[fnv1a32(name.encode()) % table_size].append((name, v))

stringpool = ""
entries = []

for i in range(table_size):
    items = hashtbl[i]
    if not items:
        hashtbl[i] = None
    else:
        for j, (name, item) in enumerate(items):
            strtbl_offset = len(stringpool)
            stringpool += name
            data = item['codepoints']
            if len(data) not in (1, 2):
                raise RuntimeError('too many codepoints for entity ' + name)
            if j == 0:
                hashtbl[i] = len(entries)
            entries.append(Entry(len=len(name),
                                 has_next= (j != len(items) - 1),
                                 strpool_offset=strtbl_offset,
                                 data=data))

if len(entries) > 0x10000:
    raise RuntimeError('too many entities')

out = open('entities.inc', 'w')

print('static const char entitiesStringpool[] =', file=out)
rest = stringpool
while rest:
    print('    "' + rest[:100] + '"', file=out)
    rest = rest[100:]
print('  ;', file=out)

print('', file=out)
print('static const EntitiesEntry entitiesEntries[] = {', file=out)
for entry in entries:
    data1 = entry.data[0]
    if len(entry.data) == 2:
        data2 = entry.data[1]
    else:
        data2 = 0
    name = stringpool[entry.strpool_offset:entry.strpool_offset+entry.len]
    print('    { %2d, %d, %5d, { %7s, %7s } }, // %s' % (entry.len, entry.has_next, entry.strpool_offset, hex(data1), hex(data2), name), file=out)

print('};', file=out)
print('', file=out)
print('static const uint16_t entitiesHashtbl[] = {', file=out)

for offset in hashtbl:
    if offset is not None:
        print('    %d,' % (offset,), file=out)
    else:
        print('    0, // unused', file=out)

print('};', file=out)
print('', file=out)
print('static const uint16_t entitiesSize = %d;' % (table_size,), file=out)

out.close()
