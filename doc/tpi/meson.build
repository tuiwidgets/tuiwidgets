project('tuiwidgets-tpi-image-builder', ['cpp'], default_options : ['buildtype=debugoptimized', 'cpp_std=c++17'])

add_project_arguments('-Wall', '-Wextra', language: 'cpp')
add_project_arguments('-Werror=return-type', language: 'cpp')


qt5 = import('qt5')
qt5_dep = dependency('qt5', modules: 'Core')

tuiwidgets_dep = dependency('TuiWidgets')

builder_files = [
  'tpi-image-builder.cpp',
  'boxlayout.cpp',
  'button.cpp',
  'checkbox.cpp',
  'dialog.cpp',
  'inputbox.cpp',
  'label.cpp',
  'listview.cpp',
  'menu.cpp',
  'radiobutton.cpp',
  'root.cpp',
  'textedit.cpp',
  'textline.cpp',
  'window.cpp',
  'demo_gettingstarted.cpp'
]

executable('tpi-image-builder', builder_files,
           dependencies : [qt5_dep, tuiwidgets_dep])

