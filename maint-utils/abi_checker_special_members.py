#! /usr/bin/python3

# SPDX-License-Identifier: BSL-1.0

# This is quite a hack. But better than nothing.
# This uses the compiler and the symbols to detect if the compiler generated live cycle and assignment operations are
# defined out of line to ensure ABI stability properties are kept.

# It contains some hacks to work around special cases that will need adjusting for future types.
# Assumes itanium c++ name mangling and to be used with an amd64 compiler (other archs might work).

import sys
import subprocess
import os
import re

mydir = os.path.dirname(__file__) + '/'

result_exe = mydir + '/../_build/abi_checker_exe'

subprocess.run(['c++', '-fno-access-control', '-Wa,-amhlsd=' + result_exe + '.asmlist', '-I../src',
                '-I/usr/include/x86_64-linux-gnu/qt5/QtCore', '-I/usr/include/x86_64-linux-gnu/qt5',
                '-D_FILE_OFFSET_BITS=64', '-Wall', '-Winvalid-pch', '-Wnon-virtual-dtor', '-std=c++17',
                '-O0', '-g', '-fvisibility=hidden', '-fvisibility-inlines-hidden', '-Wall', '-Wextra',
                '-Werror=return-type', '-DQT_CORE_LIB', '-fPIC', '-o', result_exe, 'abi_checker_special_members.cpp',
                '-lQt5Core', '../_build/src/libtuiwidgets.so', '-Wl,-rpath,' + mydir + '/../_build/src'],
                cwd=mydir, check=True)

result_exit = subprocess.run([result_exe], capture_output=True, encoding='utf8', errors='surrogateescape')

print(result_exit.stderr, end='')

nm_exit = subprocess.run(['nm', '--dynamic', '--extern-only', '--undefined-only', '--format=posix', result_exe],
                         capture_output=True, check=True, encoding='utf8', errors='surrogateescape')

undef_symbols = set()

undef_symbol_re = re.compile('^([_A-Za-z0-9]+)@[A-Z_.0-9]+ U')

for line in nm_exit.stdout.split('\n'):
    match = undef_symbol_re.match(line)
    if match:
        undef_symbols.add(match.group(1))

current_class = None
current_kind = None

inline_classes = set()

for line in result_exit.stdout.split('\n'):
    if line.startswith('CLASS '):
        # CLASS %s kind=%s
        op, current_class, kind_prefixed = line.split(' ', maxsplit=2)
        current_kind = kind_prefixed[5:]
    elif line.startswith('EXPECT '):
        op, symbol, desc = line.split(' ', maxsplit=2)
        if symbol in undef_symbols:
            pass
        elif symbol.endswith('D1Ev') and (symbol[:-4] + 'D2Ev') in undef_symbols:
            # also satisfied by destructor symbol used in implementation of a deriving class' destructor
            pass
        # HACK: Lots of hacks to avoid false positives when the constructor that acts as default as defaulted parameter.
        # Hopefully this does not generate to many false negatives
        elif symbol.endswith('C1Ev') and (symbol[:-4] + 'C1EPNS0_7ZWidgetE') in undef_symbols:
            # assuming defaulted parent ptr constructor acts as default constructor
            pass
        elif symbol == '_ZN3Tui2v07ZWidgetC1Ev' and '_ZN3Tui2v07ZWidgetC1EPS1_' in undef_symbols:
            # assuming defaulted parent ptr constructor acts as default constructor (ZWidget with name compression)
            pass
        elif symbol.endswith('C1Ev') and (symbol[:-4] + 'C1EP7QObject') in undef_symbols:
            # assuming defaulted parent ptr constructor acts as default constructor
            pass
        else:
            if current_kind == 'Inline':
                #print(f'Inline: {symbol} for {desc}')
                inline_classes.add(current_class)
            else:
                print(f'Missing symbol: {symbol} for {desc}')
    elif line == '':
        pass
    elif line.startswith('ERROR '):
        print(line)
    elif line.startswith('CLASS-SIG '):
        pass
    else:
        print('Unexpected output line: ', line)

print("inline classes: " + ", ".join(inline_classes))
