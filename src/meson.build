# SPDX-License-Identifier: BSL-1.0
qt5 = import('qt5')
qt5_dep = dependency('qt5', modules: 'Core')
main_vscript = 'tuiwidgets.symver'
if host_machine.system() == 'linux'
  # for now, only do this on linux, expand supported platforms as needed
  main_ld_vscript = '-Wl,--version-script,@0@/@1@'.format(meson.current_source_dir(), main_vscript)
else
  main_ld_vscript = []
endif


#ide:editable-filelist
tuiwidgets_moc_headers = [
  'Tui/ZBasicDefaultWidgetManager.h',
  'Tui/ZBasicWindowFacet.h',
  'Tui/ZButton.h',
  'Tui/ZCheckBox.h',
  'Tui/ZClipboard.h',
  'Tui/ZCommandManager.h',
  'Tui/ZCommandNotifier.h',
  'Tui/ZDefaultWidgetManager.h',
  'Tui/ZDialog.h',
  'Tui/ZDocument.h',
  'Tui/ZHBoxLayout.h',
  'Tui/ZInputBox.h',
  'Tui/ZLabel.h',
  'Tui/ZLayout.h',
  'Tui/ZListView.h',
  'Tui/ZMenu.h',
  'Tui/ZMenu_p.h',
  'Tui/ZMenubar.h',
  'Tui/ZRadioButton.h',
  'Tui/ZRoot.h',
  'Tui/ZShortcut.h',
  'Tui/ZTerminal.h',
  'Tui/ZTerminalDiagnosticsDialog.h',
  'Tui/ZTextEdit.h',
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
  'Tui/ZClipboard.h',
  'Tui/ZColor.h',
  'Tui/ZCommandManager.h',
  'Tui/ZCommandNotifier.h',
  'Tui/ZCommon.h',
  'Tui/ZDefaultWidgetManager.h',
  'Tui/ZDialog.h',
  'Tui/ZDocument.h',
  'Tui/ZDocumentCursor.h',
  'Tui/ZDocumentLineMarker.h',
  'Tui/ZDocumentSnapshot.h',
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
  'Tui/ZTerminalDiagnosticsDialog.h',
  'Tui/ZTest.h',
  'Tui/ZTextEdit.h',
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
  'Tui/Layout_p.cpp',
  'Tui/ListNode.cpp',
  'Tui/MarkupParser.cpp',
  'Tui/Misc/AbstractTableModelTrackBy.h',
  'Tui/Misc/SurrogateEscape.cpp',
  'Tui/Utils.cpp',
  'Tui/ZBasicDefaultWidgetManager.cpp',
  'Tui/ZBasicWindowFacet.cpp',
  'Tui/ZButton.cpp',
  'Tui/ZCheckBox.cpp',
  'Tui/ZClipboard.cpp',
  'Tui/ZColor.cpp',
  'Tui/ZCommandManager.cpp',
  'Tui/ZCommandNotifier.cpp',
  'Tui/ZCommon.cpp',
  'Tui/ZDefaultWidgetManager.cpp',
  'Tui/ZDialog.cpp',
  'Tui/ZDocument.cpp',
  'Tui/ZDocumentCursor.cpp',
  'Tui/ZDocumentLineMarker.cpp',
  'Tui/ZDocumentSnapshot.cpp',
  'Tui/ZDocument_find.cpp',
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
  'Tui/ZTerminalDiagnosticsDialog.cpp',
  'Tui/ZTest.cpp',
  'Tui/ZTextEdit.cpp',
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
  link_args : main_ld_vscript,
  link_depends : main_vscript,
  soversion: '0a',
  darwin_versions: ['1', '1'],
  install: true
)

install_headers(tuiwidgets_public_headers, subdir:'Tui')
install_headers(tuiwidgets_misc_public_headers, subdir:'Tui/Misc')

import('pkgconfig').generate(
  description: 'Terminal User Interface Widget Library',
  filebase: 'TuiWidgets',
  name: 'TuiWidgets',
  url: 'https://tuiwidgets.namepad.de/',
  libraries: [tuiwidgets],
  requires: [qt5_dep],
  subdirs: ['.']
)

tuiwidgets_dep = declare_dependency(link_with: tuiwidgets)

tests_as_installed = false

if get_option('tests')
    subdir('tests')
endif
