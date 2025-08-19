EXTRA_DIST=$(app_icons) $(dist_icons) $(mimetype_icons)

app_icons = \
	flexemu.svg \
	flexemu-flexplorer.svg \
	flexemu-fsetup.svg

appicondir = $(datadir)/icons/hicolor/scalable/apps
appicon_DATA = $(app_icons)

dist_icons = \
	flexemu-address.svg \
	flexemu-ascii.svg \
	flexemu-autoupdate-winsize.svg \
	flexemu-breakpoints.svg \
	flexemu-cpu-undocumented.svg \
	flexemu-cpu.svg \
	flexemu-flexplorer-about.svg \
	flexemu-flexplorer-attributes.svg \
	flexemu-flexplorer-background.svg \
	flexemu-flexplorer-copy.svg \
	flexemu-flexplorer-delete.svg \
	flexemu-flexplorer-deselectall.svg \
	flexemu-flexplorer-dir.svg \
	flexemu-flexplorer-diskimagefiles.svg \
	flexemu-flexplorer-exit.svg \
	flexemu-flexplorer-extract.svg \
	flexemu-flexplorer-find.svg \
	flexemu-flexplorer-info.svg \
	flexemu-flexplorer-inject.svg \
	flexemu-flexplorer-new.svg \
	flexemu-flexplorer-open.svg \
	flexemu-flexplorer-opendir.svg \
	flexemu-flexplorer-options.svg \
	flexemu-flexplorer-options-needs-restart.svg \
	flexemu-flexplorer-paste.svg \
	flexemu-flexplorer-selectall.svg \
	flexemu-flexplorer-view.svg \
	flexemu-flexplorer-window-close.svg \
	flexemu-flexplorer.svg \
	flexemu-floppy.svg \
	flexemu-irq-gray.svg \
	flexemu-irq-lightblue.svg \
	flexemu-irq-maganta.svg \
	flexemu-irq-red.svg \
	flexemu-irq-yellow.svg \
	flexemu-irqs.svg \
	flexemu-ledred-off.svg \
	flexemu-ledred-on.svg \
	flexemu-logging.svg \
	flexemu-memory.svg \
	flexemu-original-frequency.svg \
	flexemu-run.svg \
	flexemu-screen-full.svg \
	flexemu-screen1.svg \
	flexemu-screen2.svg \
	flexemu-screen3.svg \
	flexemu-screen4.svg \
	flexemu-screen5.svg \
	flexemu-space.svg \
	flexemu-stop.svg \
	flexemu-toggle.svg \
	flexemu-update-winsize.svg

mimetype_icons = \
	flexemu-flexplorer.svg

mimetypeicondir = $(datadir)/icons/hicolor/scalable/mimetypes
mimetypeicon_DATA = $(mimetype_icons)

install-data-hook:
	cd $(DESTDIR)$(datadir)/icons/hicolor/scalable/mimetypes; \
	mv flexemu-flexplorer.svg application-x-flex-diskimage.svg

uninstall-hook:
	cd $(DESTDIR)$(datadir)/icons/hicolor/scalable/mimetypes; \
	rm -f application-x-flex-diskimage.svg
