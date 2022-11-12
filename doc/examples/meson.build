project('tuiwidgets-examples', ['cpp'], default_options : ['buildtype=debugoptimized', 'cpp_std=c++17'])

add_project_arguments('-Wall', '-Wextra', language: 'cpp')
add_project_arguments('-Werror=return-type', '-Wno-unused-parameter', language: 'cpp')


qt5 = import('qt5')
qt5_dep = dependency('qt5', modules: 'Core')

tuiwidgets_dep = dependency('TuiWidgets')

executable('getting-started-index', ['getting-started/getting-started-index.cpp'],
           dependencies : [qt5_dep, tuiwidgets_dep])

executable('getting-started', ['getting-started/getting-started.cpp'],
           qt5.preprocess(moc_headers: 'getting-started/getting-started.h'),
           dependencies : [qt5_dep, tuiwidgets_dep])

executable('modeltrackby', ['modeltrackby/modeltrackby.cpp'],
           dependencies : [qt5_dep, tuiwidgets_dep])

widget_files = [
  'widgets/button.cpp',
  'widgets/checkbox.cpp',
  'widgets/dialog.cpp',
  'widgets/inputbox.cpp',
  'widgets/label.cpp',
  'widgets/listview.cpp',
  'widgets/menu.cpp',
  'widgets/menubar.cpp',
  'widgets/radiobuttons.cpp',
  'widgets/root.cpp',
  'widgets/textline.cpp',
  'widgets/window.cpp',
]

executable('widgets', widget_files,
           dependencies : [qt5_dep, tuiwidgets_dep])
