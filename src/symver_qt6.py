#!/usr/bin/env python3
# SPDX-License-Identifier: BSL-1.0

import sys

if len(sys.argv) != 3:
    print(f'{sys.argv[0]} qt5.symver qt6.symver')
    sys.exit(1)


with open(sys.argv[1], 'r') as f_in, open(sys.argv[2], 'w') as f_out:
    data = f_in.read()
    data = data.replace('QVector', 'QList').replace('QStringList', 'QList<QString>')
    f_out.write(data)

