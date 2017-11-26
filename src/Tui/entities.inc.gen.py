#! /usr/bin/python3

# entities.json from https://www.w3.org/TR/html5/entities.json

import json

j = json.load(open('entities.json', 'r'))

out = open('entities.inc', 'w')
for k in sorted(j.keys()): print('\t{"'+k+'", {' + ", ".join((str(c) for c in j[k]['codepoints'])) + '}},', file=out)
out.close()

