EXTRA_DIST=$(app_icons) $(mimetype_icons)

app_icons = \
	flexemu.png \
	flexemu-flexplorer.png \
	flexemu-fsetup.png

appicondir = $(datadir)/icons/hicolor/48x48/apps
appicon_DATA = $(app_icons)

mimetype_icons = \
	flexemu-flexplorer.png

mimetypeicondir = $(datadir)/icons/hicolor/48x48/mimetypes
mimetypeicon_DATA = $(mimetype_icons)

install-data-hook:
	cd $(DESTDIR)$(datadir)/icons/hicolor/48x48/mimetypes; \
	mv flexemu-flexplorer.png application-x-flex-diskimage.png

uninstall-hook:
	cd $(DESTDIR)$(datadir)/icons/hicolor/48x48/mimetypes; \
	rm -f application-x-flex-diskimage.png
