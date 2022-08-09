project('tuiwidgets', ['cpp'], default_options : ['buildtype=debugoptimized', 'cpp_std=c++17'])

posixsignalmanager_dep = subproject('posixsignalmanager').get_variable('posixsignalmanager_dep')

if meson.get_compiler('cpp').get_id() == 'gcc'
  add_project_arguments('-fvisibility=hidden', '-fvisibility-inlines-hidden', language: 'cpp')
endif

add_project_arguments('-Wall', '-Wextra', language: 'cpp')
add_project_arguments('-Werror=return-type', language: 'cpp')
add_project_arguments('-finput-charset=utf-8', '-fexec-charset=utf-8', language: 'cpp')

termpaint_dep = dependency('termpaint')
termpaint_image_dep = dependency('termpaint_image')

subdir('src')
