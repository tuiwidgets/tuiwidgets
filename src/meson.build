qt5 = import('qt5')
qt5_dep = dependency('qt5', modules: 'Core')

#ide:editable-filelist
tuiwidgets_moc_headers = [
  'Tui/ZCommandManager.h',
  'Tui/ZCommandNotifier.h',
  'Tui/ZLayout.h',
  'Tui/ZRoot.h',
  'Tui/ZShortcut.h',
  'Tui/ZTerminal.h',
  'Tui/ZWidget.h',
]

# find Tui -name "*.h" | grep -E -v -e "_p" -e "^Tui/[A-Y]" | sed -e "s/\\(.*\\)/  '\1',/" | sort
tuiwidgets_public_headers = [
  'Tui/ZColor.h',
  'Tui/ZCommandManager.h',
  'Tui/ZCommandNotifier.h',
  'Tui/ZCommon.h',
  'Tui/ZEvent.h',
  'Tui/ZImage.h',
  'Tui/ZLayout.h',
  'Tui/ZLayoutItem.h',
  'Tui/ZPainter.h',
  'Tui/ZPalette.h',
  'Tui/ZRoot.h',
  'Tui/ZShortcut.h',
  'Tui/ZSimpleFileLogger.h',
  'Tui/ZSymbol.h',
  'Tui/ZTerminal.h',
  'Tui/ZTextMetrics.h',
  'Tui/ZValuePtr.h',
  'Tui/ZWidget.h',
  'Tui/tuiwidgets_internal.h',
  # not public, but needed until users are migrated from staging to toolkit proper
  'Tui/MarkupParser.h',
]

#ide:editable-filelist
tuiwidgets_sources = [
  'Tui/ListNode.cpp',
  'Tui/MarkupParser.cpp',
  'Tui/ZColor.cpp',
  'Tui/ZCommandManager.cpp',
  'Tui/ZCommandNotifier.cpp',
  'Tui/ZCommon.cpp',
  'Tui/ZEvent.cpp',
  'Tui/ZImage.cpp',
  'Tui/ZLayout.cpp',
  'Tui/ZLayoutItem.cpp',
  'Tui/ZPainter.cpp',
  'Tui/ZPalette.cpp',
  'Tui/ZRoot.cpp',
  'Tui/ZShortcut.cpp',
  'Tui/ZShortcutManager.cpp',
  'Tui/ZSimpleFileLogger.cpp',
  'Tui/ZSymbol.cpp',
  'Tui/ZTerminal.cpp',
  'Tui/ZTextMetrics.cpp',
  'Tui/ZValuePtr.cpp',
  'Tui/ZWidget.cpp',
]

tuiwidgets_sources += [
  'Tui/ZTerminal_linux.cpp'
]

tui_cpp_args = [
  '-DQT_DISABLE_DEPRECATED_BEFORE=0x050900',
  '-DQT_NO_CAST_FROM_ASCII',
  '-DQT_NO_CAST_TO_ASCII',
  '-DQT_NO_CAST_FROM_BYTEARRAY',
  '-DQT_NO_FOREACH',
  '-DQT_NO_KEYWORDS'
  #'-Werror=old-style-cast'
]

tuiwidgets_generated = qt5.preprocess(moc_headers: tuiwidgets_moc_headers, include_directories: '../src')

tuiwidgets = library('tuiwidgets', tuiwidgets_sources, tuiwidgets_generated,
  dependencies : [qt5_dep, termpaint_dep, termpaint_image_dep, posixsignalmanager_dep],
  cpp_args: tui_cpp_args,
  install: true
)

install_headers(tuiwidgets_public_headers, subdir:'Tui')

import('pkgconfig').generate(
  version: '0.0',
  description: 'Terminal User Interface Widget Library',
  filebase: 'TuiWidgets',
  name: 'TuiWidgets',
  libraries: [tuiwidgets],
  subdirs: ['.']
)

testlib = static_library('testlib', 'tests/catch_main.cpp')

#ide:editable-filelist
test_files = [
  'tests/markupparser.cpp',
  'Tui/MarkupParser.cpp',
]

silence_warnings = [
    '-Wno-padded'
]

test('testtoolkit',
  executable('testtoolkit', test_files,
    link_with: [tuiwidgets, testlib],
    dependencies: [qt5_dep],
    cpp_args: ['-fno-inline', silence_warnings]
  )
)
