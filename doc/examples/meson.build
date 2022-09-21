project('tuiwidgets-examples', ['cpp'], default_options : ['buildtype=debugoptimized', 'cpp_std=c++17'])

add_project_arguments('-Wall', '-Wextra', language: 'cpp')
add_project_arguments('-Werror=return-type', language: 'cpp')


qt5 = import('qt5')
qt5_dep = dependency('qt5', modules: 'Core')

tuiwidgets_dep = dependency('TuiWidgets')

executable('getting-started-index', ['getting-started/getting-started-index.cpp'],
           dependencies : [qt5_dep, tuiwidgets_dep])

executable('getting-started', ['getting-started/getting-started.cpp'],
           qt5.preprocess(moc_headers: 'getting-started/getting-started.h'),
           dependencies : [qt5_dep, tuiwidgets_dep])


