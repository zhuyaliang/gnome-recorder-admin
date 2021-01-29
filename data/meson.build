subdir('counter')
subdir('icon')
subdir('hicolor')

i18n = import('i18n')

i18n.merge_file(
  'gnome-recorder-admin-desktop',
  input: 'gnome-recorder-admin.desktop.in',
  output: 'gnome-recorder-admin.desktop',
  install: true,
  po_dir: join_paths(meson.source_root(), 'po'),
  install_dir : join_paths(get_option('prefix'), 'share/applications'),
  type: 'desktop',
)

appdata = 'gnome-recorder-admin.appdata.xml'
i18n.merge_file(
  input: appdata + '.in',
  output: appdata,
  po_dir: join_paths(meson.source_root(), 'po'),
  install: true,
  install_dir: join_paths(get_option('prefix'), 'share/metainfo')
)  
