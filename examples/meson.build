#ide:editable-filelist
exmaples_demo_sources = [
  'demo.cpp',
]

#ide:editable-filelist
exmaples_demo_headers = [
  'demo.h',
]

executable('tuiwidgets-demo', exmaples_demo_sources,
  qt5.preprocess(moc_headers: exmaples_demo_headers),
  dependencies: [qt5_dep, tuiwidgets_dep])

