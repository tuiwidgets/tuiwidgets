qt5 = import('qt5')
qt5_dep = dependency('qt5', modules: 'Core')

#ide:editable-filelist
tuiwidgets_moc_headers = [
  'Tui/ZBasicWindowFacet.h',
  'Tui/ZCommandManager.h',
  'Tui/ZCommandNotifier.h',
  'Tui/ZLayout.h',
  'Tui/ZRoot.h',
  'Tui/ZShortcut.h',
  'Tui/ZTerminal.h',
  'Tui/ZWidget.h',
  'Tui/ZWindow.h',
  'Tui/ZWindowFacet.h',
]

# find Tui -name "*.h" | grep -E -v -e "_p" -e "^Tui/[A-Y]" | sed -e "s/\\(.*\\)/  '\1',/" | sort
tuiwidgets_public_headers = [
  'Tui/ZBasicWindowFacet.h',
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
  'Tui/ZSimpleStringLogger.h',
  'Tui/ZSymbol.h',
  'Tui/ZTerminal.h',
  'Tui/ZTest.h',
  'Tui/ZTextMetrics.h',
  'Tui/ZValuePtr.h',
  'Tui/ZWidget.h',
  'Tui/ZWindow.h',
  'Tui/ZWindowFacet.h',
  'Tui/tuiwidgets_internal.h',
  # not public, but needed until users are migrated from staging to toolkit proper
  'Tui/MarkupParser.h',
]

# private headers that are included by public headers (header only facilities)
tuiwidgets_public_headers += [
  'Tui/ZMoFunc_p.h',
]

#ide:editable-filelist
tuiwidgets_sources = [
  'Tui/ZBasicWindowFacet.cpp',
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
  'Tui/ZMoFunc_p.h',
  'Tui/ZPainter.cpp',
  'Tui/ZPalette.cpp',
  'Tui/ZRoot.cpp',
  'Tui/ZShortcut.cpp',
  'Tui/ZShortcutManager.cpp',
  'Tui/ZSimpleFileLogger.cpp',
  'Tui/ZSimpleStringLogger.cpp',
  'Tui/ZSymbol.cpp',
  'Tui/ZTerminal.cpp',
  'Tui/ZTest.cpp',
  'Tui/ZTextMetrics.cpp',
  'Tui/ZValuePtr.cpp',
  'Tui/ZWidget.cpp',
  'Tui/ZWindow.cpp',
  'Tui/ZWindowFacet.cpp',
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

testlib = static_library('testlib', 'tests/catch_main.cpp', dependencies: [qt5_dep])

test_env = environment()
test_env.set('TUIWIDGETS_TEST_DATA', meson.current_source_dir() / 'tests')

silence_warnings = [
    '-Wno-padded'
]

#ide:editable-filelist
testinternal_files = [
  'tests/markupparser.cpp',
  'tests/metrics/metrics.cpp',
  'tests/painting/painting.cpp',
]

# parts of the main library that are needed for the internal tests
testinternal_files += [
  'Tui/MarkupParser.cpp',
  'Tui/ZImage.cpp',
  'Tui/ZPainter.cpp',
  'Tui/ZShortcutManager.cpp',
  'Tui/ZTerminal.cpp',
  'Tui/ZTerminal_linux.cpp',
  'Tui/ZTextMetrics.cpp',
  'Tui/ZWidget.cpp',
]

test('testtoolkitinternal',
  executable('testtoolkitinternal', testinternal_files,
    link_with: [tuiwidgets, testlib],
    dependencies: [qt5_dep, termpaint_dep, termpaint_image_dep, posixsignalmanager_dep],
    cpp_args: ['-fno-inline', silence_warnings]
  ),
  env: test_env
)

#ide:editable-filelist
test_files = [
  'tests/basicwindowfacet/basicwindowfacet.cpp',
  'tests/color/color.cpp',
  'tests/Testhelper.cpp',
  'tests/viewport/moveviewport.cpp',
  'tests/window/window.cpp',
]

test('testtoolkit',
  executable('testtoolkit', test_files,
    link_with: [tuiwidgets, testlib],
    dependencies: [qt5_dep, termpaint_dep],
    cpp_args: ['-fno-inline', silence_warnings]
  ),
  env: test_env
)
