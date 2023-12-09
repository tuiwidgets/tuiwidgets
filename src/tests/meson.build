# NOTE: This file is also used for "as-installed" tests in distro packaging. So all used input variables
# are an semi-external interface.


uninstalled_headers = []

if not tests_as_installed
uninstalled_headers = include_directories('..')
endif

verbose_kwargs = {}
if meson.version().version_compare('>=0.62')
    verbose_kwargs += {'verbose': true}
endif

testlib = static_library('testlib', 'catch_main.cpp',
  include_directories: uninstalled_headers,
  dependencies: [qt5_dep, tuiwidgets_dep, catch2_dep]
)

test_env = environment()
test_env.set('TUIWIDGETS_TEST_DATA', meson.current_source_dir())

silence_warnings = [
    '-Wno-padded'
]

#ide:editable-filelist
testinternal_files = [
  'markupparser.cpp',
  'metrics/metrics.cpp',
  'painting/painting.cpp',
]

# parts of the main library that are needed for the internal tests
testinternal_files += [
  '../Tui/MarkupParser.cpp',
  '../Tui/ZImage.cpp',
  '../Tui/ZPainter.cpp',
  '../Tui/ZShortcut.cpp',
  '../Tui/ZShortcutManager.cpp',
  '../Tui/ZTerminal.cpp',
  '../Tui/ZTerminal_linux.cpp',
  '../Tui/ZTextMetrics.cpp',
  '../Tui/ZWidget.cpp',
]

if not tests_as_installed
  test('testtoolkitinternal',
    executable('testtoolkitinternal', testinternal_files,
      include_directories: uninstalled_headers,
      link_with: [testlib],
      dependencies: [qt5_dep, termpaint_dep, termpaint_image_dep, posixsignalmanager_dep, tuiwidgets_dep, catch2_dep],
      cpp_args: ['-fno-inline', silence_warnings]
    ),
    timeout: 240,
    env: test_env,
    kwargs: verbose_kwargs
  )
endif

#ide:editable-filelist
test_files = [
  'Testhelper.cpp',
  'abstracttablemodeltrackby.cpp',
  'basicdefaultwidgetmanager.cpp',
  'basicwindowfacet/basicwindowfacet.cpp',
  'boxlayout.cpp',
  'button/button.cpp',
  'checkbox/checkbox.cpp',
  'color/color.cpp',
  'clipboard.cpp',
  'command.cpp',
  'defaultwidgetmanager.cpp',
  'document/document.cpp',
  'document/document2.cpp',
  'document/document_find.cpp',
  'document/document_undo.cpp',
  'eventrecorder.cpp',
  'events.cpp',
  'image/image.cpp',
  'inputbox/inputbox.cpp',
  'label/label.cpp',
  'layout/layout.cpp',
  'layout/layoutitem.cpp',
  'layoutinvalidation.cpp',
  'listview/listview.cpp',
  'loggers.cpp',
  'menu/menu.cpp',
  'painting/painter_terminal.cpp',
  'radiobutton/radiobutton.cpp',
  'root/root.cpp',
  'shortcut.cpp',
  'styledtextline/styledtextline.cpp',
  'surrogateescape.cpp',
  'symbol/symbol.cpp',
  'terminal.cpp',
  'textedit/textedit.cpp',
  'textlayout/formatrange.cpp',
  'textlayout/textlayout.cpp',
  'textlayout/textoption.cpp',
  'textline/textline.cpp',
  'textstyle.cpp',
  'viewport/moveviewport.cpp',
  'widget/widget-focus-order.cpp',
  'widget/widget.cpp',
  'window/dialog.cpp',
  'window/window.cpp',
  'windowcontainer.cpp',
  'windowfacet.cpp',
  'windowlayout.cpp',
]

test('testtoolkit',
  executable('testtoolkit', test_files,
    include_directories: uninstalled_headers,
    link_with: [testlib],
    dependencies: [qt5_dep, termpaint_dep, tuiwidgets_dep, catch2_dep],
    cpp_args: ['-fno-inline', silence_warnings]
  ),
  timeout: 240,
  env: test_env,
  kwargs: verbose_kwargs
)
