# screen-admin

## Explain

This is a screen recording tool running in GNOME desktop environment.
Two recording modes are available:

* Full Screen Recording

* Area Screen Recording

## Note

To better experience this screen recording tool, please install ```kstatusnotifieitem/appindicatorsupport```extend
[kstatusnotifieitem](https://github.com/ubuntu/gnome-shell-extension-appindicator)

## Compile

```
meson build -Dprefix=/usr
ninja -C build
sudo ninja -C build install

```
