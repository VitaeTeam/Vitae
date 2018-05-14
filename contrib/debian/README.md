
Debian
====================
This directory contains files used to package vitaed/vitae-qt
for Debian-based Linux systems. If you compile vitaed/vitae-qt yourself, there are some useful files here.

## vitae: URI support ##


vitae-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install vitae-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your vitaeqt binary to `/usr/bin`
and the `../../share/pixmaps/vitae128.png` to `/usr/share/pixmaps`

vitae-qt.protocol (KDE)

