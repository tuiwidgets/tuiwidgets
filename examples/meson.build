# SPDX-License-Identifier: BSL-1.0

#ide:editable-filelist
examples_demo_sources = [
  'demo.cpp',
]

#ide:editable-filelist
examples_demo_headers = [
  'demo.h',
]

executable('tuiwidgets-demo', examples_demo_sources,
  qt.preprocess(moc_headers: examples_demo_headers),
  dependencies: [qt_dep, tuiwidgets_dep])

