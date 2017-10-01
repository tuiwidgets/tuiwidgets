qt5 = import('qt5')
qt5_dep = dependency('qt5', modules: 'Core')

#ide:editable-filelist
tuiwidgets_moc_headers = [
  'Tui/ZTerminal.h',
  'Tui/ZWidget.h',
]

# find Tui -name "*.h" | grep -v "_p" | sed -e "s/\\(.*\\)/  '\1',/" | sort
tuiwidgets_public_headers = [
  'Tui/ZColor.h',
  'Tui/ZEvent.h',
  'Tui/ZPainter.h',
  'Tui/ZTerminal.h',
  'Tui/ZWidget.h',
  'Tui/tuiwidgets_internal.h',
]

#ide:editable-filelist
tuiwidgets_sources = [
  'Tui/ZColor.cpp',
  'Tui/ZEvent.cpp',
  'Tui/ZWidget.cpp',
  'Tui/ZTerminal.cpp',
  'Tui/ZPainter.cpp'
]

tuiwidgets_sources += [
    'Tui/ZTerminal_linux.cpp'
]

moc_files = qt5.preprocess(moc_headers: tuiwidgets_moc_headers, include_directories: '../src', install: true)

tuiwidgets = library('tuiwidgets', tuiwidgets_sources, moc_files,
  dependencies : [qt5_dep, termpaint_dep],
  install: true)

install_headers(tuiwidgets_public_headers, subdir:'Tui')

import('pkgconfig').generate(
  version: '0.0',
  description: 'Terminal User Interface Widget Library',
  filebase: 'TuiWidgets',
  name: 'TuiWidgets',
  libraries: [tuiwidgets],
  subdirs: ['.']
)
