subdir('counter')
subdir('icon')
subdir('hicolor')

i18n = import('i18n')

i18n.merge_file(
  'screen-admin-desktop',
  input: 'screen-admin.desktop.in',
  output: 'screen-admin.desktop',
  install: true,
  po_dir: join_paths(meson.source_root(), 'po'),
  install_dir : join_paths(get_option('prefix'), 'share/applications'),
  type: 'desktop',
)

appdata = 'screen-admin.appdata.xml'
i18n.merge_file(
  input: appdata + '.in',
  output: appdata,
  po_dir: join_paths(meson.source_root(), 'po'),
  install: true,
  install_dir: join_paths(get_option('prefix'), 'share/metainfo')
)  
