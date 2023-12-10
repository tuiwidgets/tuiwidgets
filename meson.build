# SPDX-License-Identifier: BSL-1.0

project('tuiwidgets', ['cpp'], version: '0.2.1', default_options : ['buildtype=debugoptimized', 'cpp_std=c++17'])

posixsignalmanager_dep = dependency('PosixSignalManager',
  required: get_option('system-posixsignalmanager'),
  fallback:['posixsignalmanager', 'posixsignalmanager_dep'])

if meson.get_compiler('cpp').get_id() == 'gcc'
  add_project_arguments('-fvisibility=hidden', '-fvisibility-inlines-hidden', language: 'cpp')
endif

add_project_arguments('-Wall', '-Wextra', language: 'cpp')
add_project_arguments('-Werror=return-type', language: 'cpp')
add_project_arguments('-finput-charset=utf-8', '-fexec-charset=utf-8', language: 'cpp')

termpaint_dep = dependency('termpaint')
termpaint_image_dep = dependency('termpaint_image')

catch2_dep = dependency('catch2', required : get_option('system-catch2'))
if not catch2_dep.found()
  catch2_dep = declare_dependency(include_directories: include_directories('third-party'))
else
  if catch2_dep.version().version_compare('>=3.0')
    catch2_dep = [catch2_dep, declare_dependency(compile_args: ['-DCATCH3'])]
  endif
endif

subdir('src')

tuiwidgets_dep = declare_dependency(link_with: tuiwidgets, include_directories: ['src'])

subdir('examples')
