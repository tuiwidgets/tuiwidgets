#! /usr/bin/python3
# SPDX-License-Identifier: BSL-1.0

import subprocess
import sys
import os.path
import ctypes
import ctypes.util

import elftools
from elftools.elf.elffile import ELFFile
from elftools.elf.sections import SymbolTableSection
from elftools.elf.dynamic import DynamicSection

from elftools.elf.gnuversions import GNUVerSymSection, GNUVerDefSection

proj_root = os.path.normpath(os.path.dirname(__file__) + '/..')


def symbol_version(versym, verdef, nsym):

    if nsym >= versym.num_symbols():
        raise RuntimeError('nsym out of range')

    index = versym.get_symbol(nsym).entry['ndx']

    if index in ('VER_NDX_LOCAL', 'VER_NDX_GLOBAL'):
        return None

    index = int(index) & ~0x8000

    if index <= verdef.num_versions():
        return next(verdef.get_version(index)[1]).name

    raise RuntimeError(f'version of {nsym} not in range')


def addr2line(filename, sym):
    cp = subprocess.run(['eu-addr2line', '-e', filename, sym], capture_output=True, check=False)
    if cp.stderr:
        raise RuntimeError('eu-addr2line failed with' + cp.stderr.decode())
    res = cp.stdout.decode().strip()
    if '/' in res:
        return os.path.normpath(res)

    return res

class CharP(ctypes.c_char_p):
    pass

def demangle(name):
    lib_c = ctypes.CDLL(ctypes.util.find_library('c'))
    lib_cxx = ctypes.CDLL(ctypes.util.find_library('stdc++'))
    free = lib_c.free
    free.argtypes = [ctypes.c_void_p]

    cxa_demangle = getattr(lib_cxx, '__cxa_demangle')
    cxa_demangle.restype = CharP

    status = ctypes.c_int()
    res = cxa_demangle(ctypes.c_char_p(name.encode()), None, None, ctypes.pointer(status))
    if status.value != 0:
        raise RuntimeError(f'Unexpected status {status.value} when demangling {name}')

    unmangled = res.value.decode()

    free(res)

    return unmangled


def analyze_symbols(filename, version_prefix):
    ok = True

    f = open(filename, 'rb')
    elffile = ELFFile(f)

    versym = None
    verdef = None

    for section in elffile.iter_sections():
        if isinstance(section, GNUVerSymSection):
            versym = section
        elif isinstance(section, GNUVerDefSection):
            verdef = section

    if not versym:
        raise RuntimeError('GNUVerSymSection section missing')

    if not verdef:
        raise RuntimeError('GNUVerDefSection section missing')

    symbol_tables = [s for s in elffile.iter_sections() if isinstance(s, SymbolTableSection) and s.name == '.dynsym']

    if len(symbol_tables) != 1:
        raise RuntimeError("TODO handle multiple symbol tables")
    symbol_table = symbol_tables[0]

    for nsym, symbol in enumerate(symbol_table.iter_symbols()):
        if symbol['st_info']['type'] in ('STT_FILE', 'STT_SECTION'):
            continue
        if symbol['st_info']['bind'] == 'STB_LOCAL':
            continue
        #print(symbol.name, symbol['st_other']['visibility'], symbol['st_info']['type'], symbol['st_shndx'], symbol_version(verinfo, nsym))
        if symbol['st_shndx'] != 'SHN_UNDEF':
            src = addr2line(filename, symbol.name)
            symver = symbol_version(versym, verdef, nsym)

            if src.startswith(proj_root + '/'):
                if symver is None:
                    symver = '<unversioned>'
                    ok = False

                if not symver.startswith(version_prefix):
                    print("Symbol from our source not versioned", symbol.name, symver)
                    print(f'   {src}')
                    ok = False
            elif src.startswith('/usr/include/'):
                if symbol.name.startswith(version_prefix) and symver and symver.startswith(version_prefix):
                    # TODO investigate this case, observed on arm64 qt6 build.
                    pass
                elif symbol['st_info']['bind'] != 'STB_WEAK':
                    print("Symbol from external header, but not weak", symbol.name, symver)
                    print(f'   {src}')
                    ok = False
            elif src == '??:0':
                if symver is None or not symver.startswith(version_prefix):
                    demangled_name = demangle(symbol.name)

                    external = False
                    external_names = [
                        ('std::', ['vtable for ', 'typeinfo for ', 'typeinfo name for ']),
                        ('QMetaType::registerMutableView<', ['typeinfo for ', 'typeinfo name for ']),
                        ('QMetaType::registerMutableViewImpl<', ['guard variable for ']),
                        ('QMetaType::registerConverter<', ['typeinfo for ', 'typeinfo name for ']),
                        ('QMetaType::registerConverterImpl<', ['guard variable for ']),
                        'typeinfo for QSharedData',
                        'typeinfo name for QSharedData',
                        'QtPrivate::QMetaTypeForType<',
                        'QtPrivate::QMetaTypeInterfaceWrapper<',
                        'QMetaSequence::MetaSequence<',
                        'QtPrivate::QSequentialIterableMutableViewFunctor<',
                        'QtPrivate::QSequentialIterableConvertFunctor<',
                    ]

                    for external_def in external_names:
                        if isinstance(external_def, tuple):
                            external_name = external_def[0]
                            prefixes = external_def[1]
                        else:
                            external_name = external_def
                            prefixes = []

                        if demangled_name.startswith(external_name):
                            external = True

                        for prefix in prefixes:
                            if demangled_name.startswith(prefix + external_name):
                                external = True

                    if external:
                        # STB_LOOS is used as STB_GNU_UNIQUE on linux
                        if symbol['st_info']['bind'] != 'STB_WEAK' and symbol['st_info']['bind'] != 'STB_LOOS':
                            print("Symbol from ??, but not weak", symbol.name, symver, symbol['st_info']['bind'])
                            print(f'   {src}')
                            ok = False
                    else:
                        print("Symbol from ?? not versioned", symbol.name, demangled_name, symver, symbol['st_info']['type'])
                        print(f'   {src}')
                        ok = False

                if symbol['st_info']['type'] != 'STT_OBJECT':
                    print("Symbol from ?? not of object type", symbol.name, symver, symbol['st_info']['type'])
                    print(f'   {src}')
                    ok = False

            else:
                print("Unexpected source location: ", src)
                ok = False

    return ok


def main():
    mainlib_ok = analyze_symbols(sys.argv[1], sys.argv[2])

    return 0 if mainlib_ok else 1

if __name__ == '__main__':
    sys.exit(main())
