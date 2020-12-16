actions_icons = [
  ['16x16', 'screen-admin.png'],
  ['22x22', 'screen-admin.png'],
  ['24x24', 'screen-admin.png'],
  ['32x32', 'screen-admin.png'],
  ['48x48', 'screen-admin.png']
]

foreach icon: actions_icons
  install_data(
    join_paths(icon[0], 'apps', icon[1]),
    install_dir: join_paths(get_option('prefix'), 'share/icons', 'hicolor', icon[0], 'apps')
  )
endforeach
