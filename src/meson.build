gnome = import('gnome')

sources = [
'main.c',
'screen-count.c',
'screen-save.c',
'screen-stop.c',
'screen-style.c',
'screen-area.c',
'screen-window.c',
]

executable('screen-admin',
  sources : sources,
  dependencies : [gtk_dep, indicator_dep, notify_dep, math_dep],
  include_directories: top_srcdir,
  install : true,
  install_dir : get_option('bindir')
  )
