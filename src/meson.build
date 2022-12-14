# SPDX-License-Identifier: BSL-1.0
qt5 = import('qt5')
qt5_dep = dependency('qt5', modules: 'Core')

#ide:editable-filelist
tuiwidgets_moc_headers = [
  'Tui/ZBasicDefaultWidgetManager.h',
  'Tui/ZBasicWindowFacet.h',
  'Tui/ZButton.h',
  'Tui/ZCheckBox.h',
  'Tui/ZCommandManager.h',
  'Tui/ZCommandNotifier.h',
  'Tui/ZDefaultWidgetManager.h',
  'Tui/ZDialog.h',
  'Tui/ZHBoxLayout.h',
  'Tui/ZInputBox.h',
  'Tui/ZLabel.h',
  'Tui/ZLayout.h',
  'Tui/ZListView.h',
  'Tui/ZMenu.h',
  'Tui/ZMenuItem.h',
  'Tui/ZMenu_p.h',
  'Tui/ZMenubar.h',
  'Tui/ZRadioButton.h',
  'Tui/ZRoot.h',
  'Tui/ZShortcut.h',
  'Tui/ZTerminal.h',
  'Tui/ZTextLine.h',
  'Tui/ZVBoxLayout.h',
  'Tui/ZWidget.h',
  'Tui/ZWindow.h',
  'Tui/ZWindowContainer.h',
  'Tui/ZWindowFacet.h',
  'Tui/ZWindowLayout.h',
]

# find Tui -name "*.h" | grep -E -v -e "_p" -e "^Tui/[A-Y]" | sed -e "s/\\(.*\\)/  '\1',/" | sort
tuiwidgets_public_headers = [
  'Tui/ZBasicDefaultWidgetManager.h',
  'Tui/ZBasicWindowFacet.h',
  'Tui/ZButton.h',
  'Tui/ZCheckBox.h',
  'Tui/ZColor.h',
  'Tui/ZCommandManager.h',
  'Tui/ZCommandNotifier.h',
  'Tui/ZCommon.h',
  'Tui/ZDefaultWidgetManager.h',
  'Tui/ZDialog.h',
  'Tui/ZEvent.h',
  'Tui/ZFormatRange.h',
  'Tui/ZHBoxLayout.h',
  'Tui/ZImage.h',
  'Tui/ZInputBox.h',
  'Tui/ZLabel.h',
  'Tui/ZLayout.h',
  'Tui/ZLayoutItem.h',
  'Tui/ZListView.h',
  'Tui/ZMenu.h',
  'Tui/ZMenuItem.h',
  'Tui/ZMenubar.h',
  'Tui/ZPainter.h',
  'Tui/ZPalette.h',
  'Tui/ZRadioButton.h',
  'Tui/ZRoot.h',
  'Tui/ZShortcut.h',
  'Tui/ZSimpleFileLogger.h',
  'Tui/ZSimpleStringLogger.h',
  'Tui/ZStyledTextLine.h',
  'Tui/ZSymbol.h',
  'Tui/ZTerminal.h',
  'Tui/ZTest.h',
  'Tui/ZTextLayout.h',
  'Tui/ZTextLine.h',
  'Tui/ZTextMetrics.h',
  'Tui/ZTextOption.h',
  'Tui/ZTextStyle.h',
  'Tui/ZVBoxLayout.h',
  'Tui/ZValuePtr.h',
  'Tui/ZWidget.h',
  'Tui/ZWindow.h',
  'Tui/ZWindowContainer.h',
  'Tui/ZWindowFacet.h',
  'Tui/ZWindowLayout.h',
  'Tui/tuiwidgets_internal.h',
  # not public, but needed until users are migrated from staging to toolkit proper
  'Tui/MarkupParser.h',
]

# private headers that are included by public headers (header only facilities)
tuiwidgets_public_headers += [
  'Tui/ZMoFunc_p.h',
]

tuiwidgets_misc_public_headers = [
  'Tui/Misc/AbstractTableModelTrackBy.h',
  'Tui/Misc/SurrogateEscape.h',
]

#ide:editable-filelist
tuiwidgets_sources = [
  'Tui/Misc/AbstractTableModelTrackBy.h',
  'Tui/Misc/SurrogateEscape.cpp',
  'Tui/ZBasicDefaultWidgetManager.cpp',
  'Tui/ZBasicWindowFacet.cpp',
  'Tui/ListNode.cpp',
  'Tui/MarkupParser.cpp',
  'Tui/Layout_p.cpp',
  'Tui/ZButton.cpp',
  'Tui/ZCheckBox.cpp',
  'Tui/ZColor.cpp',
  'Tui/ZCommandManager.cpp',
  'Tui/ZCommandNotifier.cpp',
  'Tui/ZCommon.cpp',
  'Tui/ZDefaultWidgetManager.cpp',
  'Tui/ZDialog.cpp',
  'Tui/ZEvent.cpp',
  'Tui/ZFormatRange.cpp',
  'Tui/ZHBoxLayout.cpp',
  'Tui/ZImage.cpp',
  'Tui/ZInputBox.cpp',
  'Tui/ZLabel.cpp',
  'Tui/ZLayout.cpp',
  'Tui/ZLayoutItem.cpp',
  'Tui/ZListView.cpp',
  'Tui/ZMenu.cpp',
  'Tui/ZMenuItem.cpp',
  'Tui/ZMenubar.cpp',
  'Tui/ZMoFunc_p.h',
  'Tui/ZPainter.cpp',
  'Tui/ZPalette.cpp',
  'Tui/ZRadioButton.cpp',
  'Tui/ZRoot.cpp',
  'Tui/ZShortcut.cpp',
  'Tui/ZShortcutManager.cpp',
  'Tui/ZSimpleFileLogger.cpp',
  'Tui/ZSimpleStringLogger.cpp',
  'Tui/ZStyledTextLine.cpp',
  'Tui/ZSymbol.cpp',
  'Tui/ZTerminal.cpp',
  'Tui/ZTest.cpp',
  'Tui/ZTextLayout.cpp',
  'Tui/ZTextLine.cpp',
  'Tui/ZTextMetrics.cpp',
  'Tui/ZTextOption.cpp',
  'Tui/ZTextStyle.cpp',
  'Tui/ZVBoxLayout.cpp',
  'Tui/ZValuePtr.cpp',
  'Tui/ZWidget.cpp',
  'Tui/ZWindow.cpp',
  'Tui/ZWindowContainer.cpp',
  'Tui/ZWindowFacet.cpp',
  'Tui/ZWindowLayout.cpp',
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

tuiwidgets_generated = qt5.preprocess(moc_headers: tuiwidgets_moc_headers,
  include_directories: include_directories('../src'))

tuiwidgets = library('tuiwidgets', tuiwidgets_sources, tuiwidgets_generated,
  dependencies : [qt5_dep, termpaint_dep, termpaint_image_dep, posixsignalmanager_dep],
  cpp_args: tui_cpp_args,
  install: true
)

install_headers(tuiwidgets_public_headers, subdir:'Tui')
install_headers(tuiwidgets_misc_public_headers, subdir:'Tui/Misc')

import('pkgconfig').generate(
  version: '0.0',
  description: 'Terminal User Interface Widget Library',
  filebase: 'TuiWidgets',
  name: 'TuiWidgets',
  libraries: [tuiwidgets],
  subdirs: ['.']
)

testlib = static_library('testlib', 'tests/catch_main.cpp', dependencies: [qt5_dep, catch2_dep])

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
  'Tui/ZShortcut.cpp',
  'Tui/ZShortcutManager.cpp',
  'Tui/ZTerminal.cpp',
  'Tui/ZTerminal_linux.cpp',
  'Tui/ZTextMetrics.cpp',
  'Tui/ZWidget.cpp',
]

test('testtoolkitinternal',
  executable('testtoolkitinternal', testinternal_files,
    link_with: [tuiwidgets, testlib],
    dependencies: [qt5_dep, termpaint_dep, termpaint_image_dep, posixsignalmanager_dep, catch2_dep],
    cpp_args: ['-fno-inline', silence_warnings]
  ),
  timeout: 240,
  env: test_env
)

#ide:editable-filelist
test_files = [
  'tests/Testhelper.cpp',
  'tests/abstracttablemodeltrackby.cpp',
  'tests/basicdefaultwidgetmanager.cpp',
  'tests/basicwindowfacet/basicwindowfacet.cpp',
  'tests/boxlayout.cpp',
  'tests/button/button.cpp',
  'tests/checkbox/checkbox.cpp',
  'tests/color/color.cpp',
  'tests/command.cpp',
  'tests/eventrecorder.cpp',
  'tests/events.cpp',
  'tests/image/image.cpp',
  'tests/inputbox/inputbox.cpp',
  'tests/label/label.cpp',
  'tests/layout/layout.cpp',
  'tests/layout/layoutitem.cpp',
  'tests/layoutinvalidation.cpp',
  'tests/listview/listview.cpp',
  'tests/loggers.cpp',
  'tests/menu/menu.cpp',
  'tests/painting/painter_terminal.cpp',
  'tests/radiobutton/radiobutton.cpp',
  'tests/root/root.cpp',
  'tests/styledtextline/styledtextline.cpp',
  'tests/shortcut.cpp',
  'tests/surrogateescape.cpp',
  'tests/symbol/symbol.cpp',
  'tests/terminal.cpp',
  'tests/textlayout/formatrange.cpp',
  'tests/textlayout/textlayout.cpp',
  'tests/textlayout/textoption.cpp',
  'tests/textline/textline.cpp',
  'tests/textstyle.cpp',
  'tests/viewport/moveviewport.cpp',
  'tests/widget/widget-focus-order.cpp',
  'tests/widget/widget.cpp',
  'tests/window/dialog.cpp',
  'tests/window/window.cpp',
  'tests/windowcontainer.cpp',
  'tests/windowfacet.cpp',
  'tests/windowlayout.cpp',
]

test('testtoolkit',
  executable('testtoolkit', test_files,
    link_with: [tuiwidgets, testlib],
    dependencies: [qt5_dep, termpaint_dep, catch2_dep],
    cpp_args: ['-fno-inline', silence_warnings]
  ),
  timeout: 240,
  env: test_env
)
