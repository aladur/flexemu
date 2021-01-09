pkgdata_DATA = boot

sysconf_DATA = flexemu.conf

desktop_DATA = \
	flexemu.desktop \
	flexemu-flexplorer.desktop \
	flexemu-fsetup.desktop

mimetype_DATA = flexemu.xml

SUFFIXES = .rc .o .ui _ui.h .qrc _qrc.cpp .h _moc.cpp

.rc.o:
	@WINDRES_EXE@ -o $@ -O coff -i $<

EXTRA_DIST = \
	$(desktop_DATA) \
	$(flexemu_qrc) \
	$(flexemu_ui) \
	$(flexplorer_qrc) \
	$(flexplorer_ui) \
	$(fsetup_qrc) \
	$(fsetup_ui) \
	$(mimetype_DATA) \
	$(pkgdata_DATA) \
	$(sysconf_DATA) \
	confignt.h \
	dsktool.vcxproj \
	engine.cpp \
	fdtest.cpp \
	flex2hex.vcxproj \
	flexdisk.rc \
	Flexemu.ad \
	flexemu.astyle \
	flexemu.rc \
	flexemu.vcxproj \
	flexplorer.vcxproj \
	fsetup.rc \
	fsetup.vcxproj \
	JoystickTable.gnumeric \
	mc6809ex.cpp \
	mdcrtool.vcxproj \
	msvcQtPath.props

SUBDIRS = bitmaps tools

# Directory where *.desktop files are located
desktopdir = $(datadir)/applications

# Directory where mimetype *.xml files are located
mimetypebasedir = $(datadir)/mime
mimetypedir = $(mimetypebasedir)/packages

bin_PROGRAMS = \
	@FLEXPLORER@ \
	@FSETUP@ \
	dsktool \
	flex2hex \
	flexemu \
	mdcrtool

EXTRA_PROGRAMS = \
	dsktool \
	flex2hex \
	flexplorer \
	fsetup \
	mdcrtool

noinst_PROGRAMS = test

AM_CXXFLAGS = \
	--std=c++14 \
	-DF_DATADIR=\"$(pkgdatadir)\" \
	-DF_SYSCONFDIR=\"$(sysconfdir)\"

flexemu_CXXFLAGS = \
	$(AM_CXXFLAGS) \
	@CXXSPEED@ @OS_TYPE@ @X_CFLAGS@ @FLEXEMU_CXXADD@ @QT_CXXFLAGS@

flexplorer_CXXFLAGS = $(AM_CXXFLAGS) @OS_TYPE@ @QT_CXXFLAGS@
fsetup_CXXFLAGS     = $(AM_CXXFLAGS) @OS_TYPE@ @QT_CXXFLAGS@
mdcrtool_CXXFLAGS   = $(AM_CXXFLAGS) @OS_TYPE@
flex2hex_CXXFLAGS   = $(AM_CXXFLAGS) @OS_TYPE@
dsktool_CXXFLAGS    = $(AM_CXXFLAGS) @OS_TYPE@
test_CXXFLAGS       = $(AM_CXXFLAGS) @OS_TYPE@ -DUNIT_TEST

flexplorer_LDADD = @QT_LIBS@ @PTHREAD_LD@
fsetup_LDADD     = @QT_LIBS@ @PTHREAD_LD@
mdcrtool_LDADD   = @PTHREAD_LD@
flex2hex_LDADD   = @PTHREAD_LD@
dsktool_LDADD    = @PTHREAD_LD@
flexemu_LDADD    = \
	@FLEX_RC@ @FLEXEMU_LDADD@ @X_PRE_LIBS@ @X_LIBS@ \
	@X_EXTRA_LIBS@ @WIN32_LIBS@ @PTHREAD_LD@ @QT_LIBS@
test_LDADD       = @PTHREAD_LD@
flexemu_DEPENDENCIES = @FLEX_RC@

# Rule for Qt qrc files:
# Define all *.qrc files from which to create *_qrc.cpp files.
flexemu_qrc = flexemu.qrc
flexemu_qrc_src = $(flexemu_qrc:.qrc=_qrc.cpp)
fsetup_qrc = fsetup.qrc
fsetup_qrc_src = $(fsetup_qrc:.qrc=_qrc.cpp)
flexplorer_qrc = fpmain.qrc
flexplorer_qrc_src = $(flexplorer_qrc:.qrc=_qrc.cpp)

# Rule for Qt ui files:
# Define all *.ui files from which to create *_ui.h files.
flexemu_ui = \
	brkpt.ui \
	cpustat.ui \
	logfil.ui
flexemu_ui_hdr = $(flexemu_ui:.ui=_ui.h)
fsetup_ui = fsetup.ui
fsetup_ui_hdr = $(fsetup_ui:.ui=_ui.h)
flexplorer_ui = \
	fpnew.ui \
	fpopt.ui
flexplorer_ui_hdr = $(flexplorer_ui:.ui=_ui.h)

# Rule for Qt moc files:
# Define all header files from which to create *_moc.cpp files.
flexemu_moc_hdr = \
	brkptui.h \
	drawnwid.h \
	e2screen.h \
	efslctle.h \
	logfilui.h \
	qtgui.h
flexemu_moc_src = $(flexemu_moc_hdr:.h=_moc.cpp)
fsetup_moc_hdr = fsetupui.h
fsetup_moc_src = $(fsetup_moc_hdr:.h=_moc.cpp)
flexplorer_moc_hdr = \
	fpedit.h \
	fpmdich.h \
	fpnewui.h \
	fpoptui.h \
	fpwin.h
flexplorer_moc_src = $(flexplorer_moc_hdr:.h=_moc.cpp)

nodist_flexemu_SOURCES = $(flexemu_ui_hdr) $(flexemu_qrc_src) $(flexemu_moc_src)
nodist_fsetup_SOURCES = $(fsetup_ui_hdr) $(fsetup_qrc_src) $(fsetup_moc_src)

nodist_flexplorer_SOURCES = \
	$(flexplorer_ui_hdr) $(flexplorer_qrc_src) $(flexplorer_moc_src)

BUILT_SOURCES = \
	$(nodist_flexemu_SOURCES) \
	$(nodist_flexplorer_SOURCE) \
	$(nodist_fsetup_SOURCES)

CLEANFILES = \
	$(nodist_flexemu_SOURCES) \
	$(nodist_flexplorer_SOURCES) \
	$(nodist_fsetup_SOURCES)

fsetup_SOURCES = \
	benv.cpp \
	bfileptr.cpp \
	brcfile.cpp \
	bregistr.cpp \
	colors.cpp \
	foptman.cpp \
	fsetup.cpp \
	fsetupui.cpp \
	misc1.cpp\
	benv.h \
	bfileptr.h \
	brcfile.h \
	bregistr.h \
	config.h \
	cvtwchar.h \
	e2.h \
	flexemu.h \
	flexerr.h \
	foptman.h \
	fsetupui.h \
	misc1.h \
	sguiopts.h \
	typedefs.h \
	warnoff.h \
	warnon.h \
	winmain.h

flexemu_SOURCES = \
	absgui.cpp \
	acia1.cpp \
	apprun.cpp \
	bcommand.cpp \
	bdate.cpp \
	bdir.cpp \
	benv.cpp \
	bfileptr.cpp \
	binifile.cpp \
	bjoystck.cpp \
	bmembuf.cpp \
	bobservd.cpp \
	brcfile.cpp \
	bregistr.cpp\
	brkptui.cpp \
	btime.cpp \
	bui.cpp \
	bytereg.cpp \
	cacttrns.cpp \
	clogfile.cpp \
	colors.cpp \
	command.cpp \
	csetfreq.cpp \
	cvtwchar.cpp \
	da6809.cpp \
	drawnwid.cpp \
	drisel.cpp \
	e2floppy.cpp \
	e2screen.cpp \
	efslctle.cpp \
	fcinfo.cpp \
	fcnffile.cpp \
	fcopyman.cpp \
	fdirent.cpp \
	ffilebuf.cpp \
	ffilecnt.cpp \
	filecnts.cpp \
	fileread.cpp \
	flexerr.cpp \
	foptman.cpp \
	iffilcnt.cpp \
	ifilecnt.cpp \
	inout.cpp \
	iodevdbg.cpp \
	joystick.cpp \
	keyboard.cpp \
	logfilui.cpp \
	main.cpp \
	mc146818.cpp \
	mc6809.cpp \
	mc6809in.cpp \
	mc6809st.cpp \
	mc6821.cpp \
	mc6850.cpp \
	mdcrtape.cpp \
	memory.cpp \
	misc1.cpp \
	mmu.cpp \
	ndircont.cpp \
	pia1.cpp \
	pia2.cpp \
	pia2v5.cpp \
	qtgui.cpp \
	rfilecnt.cpp \
	schedule.cpp \
	terminal.cpp \
	vico1.cpp \
	vico2.cpp \
	wd1793.cpp \
	winmain.cpp \
	absdisas.h \
	absgui.h \
	acia1.h \
	apprun.h \
	bcommand.h \
	bdate.h \
	bdir.h \
	benv.h \
	bfileptr.h \
	binifile.h \
	bintervl.h \
	bjoystck.h \
	bmembuf.h \
	bmp.h \
	bobserv.h \
	bobservd.h \
	bobshelp.h \
	brcfile.h \
	bregistr.h \
	brkptui.h \
	btime.h \
	bui.h \
	bytereg.h \
	cacttrns.h \
	clogfile.h \
	command.h \
	cpustate.h \
	crc.h \
	csetfreq.h \
	cvtwchar.h \
	da6809.h \
	debug.h \
	drawnwid.h \
	drisel.h \
	e2.h \
	e2floppy.h \
	e2screen.h \
	efslctle.h \
	engine.h \
	fcinfo.h \
	fcnffile.h \
	fcopyman.h \
	fdirent.h \
	ffilebuf.h \
	ffilecnt.h \
	filecntb.h \
	filecnts.h \
	fileread.h \
	flexemu.h \
	flexerr.h \
	foptman.h \
	iffilcnt.h \
	ifilcnti.h \
	ifilecnt.h \
	inout.h \
	ioaccess.h \
	iodevdbg.h \
	iodevice.h \
	joystick.h \
	keyboard.h \
	logfilui.h \
	mc146818.h \
	mc6809.h \
	mc6809st.h \
	mc6821.h \
	mc6850.h \
	mdcrtape.h \
	memory.h \
	memsrc.h \
	memtgt.h \
	misc1.h \
	mmu.h \
	ndircont.h \
	pia1.h \
	pia2.h \
	pia2v5.h \
	propsui.h \
	qtgui.h \
	rfilecnt.h \
	schedcpu.h \
	schedule.h \
	scpulog.h \
	sguiopts.h \
	sprinter.h \
	terminal.h \
	tsl/robin_growth_policy.h \
	tsl/robin_hash.h \
	tsl/robin_map.h \
	tsl/robin_set.h \
	typedefs.h \
	vico1.h \
	vico2.h \
	warnoff.h \
	warnon.h \
	wd1793.h \
	winctxt.h \
	winmain.h

flexplorer_SOURCES = \
	bdate.cpp \
	bdir.cpp \
	benv.cpp \
	bfileptr.cpp \
	bmembuf.cpp \
	bobservd.cpp \
	bprocess.cpp \
	brcfile.cpp \
	bregistr.cpp \
	cvtwchar.cpp \
	dircont.cpp \
	fcinfo.cpp \
	fcopyman.cpp \
	fdirent.cpp \
	ffilebuf.cpp \
	ffilecnt.cpp \
	filecnts.cpp \
	flexerr.cpp \
	fpdnd.cpp \
	fpedit.cpp \
	fpmain.cpp \
	fpmdich.cpp \
	fpmodel.cpp \
	fpnewui.cpp \
	fpoptui.cpp \
	fpwin.cpp \
	idircnt.cpp \
	iffilcnt.cpp \
	ifilecnt.cpp \
	mdcrtape.cpp \
	misc1.cpp \
	rfilecnt.cpp \
	bdate.h \
	bdir.h \
	benv.h \
	bfileptr.h \
	bintervl.h
	bmembuf.h \
	bobserv.h \
	bobservd.h \
	bobshelp.h \
	bprocess.h \
	brcfile.h \
	bregistr.h \
	config.h \
	cvtwchar.h \
	dircont.h \
	fcinfo.h \
	fcopyman.h \
	fdirent.h \
	ffilebuf.h \
	ffilecnt.h \
	filecntb.h \
	filecnts.h \
	filecont.h \
	flexemu.h \
	flexerr.h \
	fpattrui.h \
	fpdnd.h \
	fpedit.h \
	fpmdich.h \
	fpmodel.h \
	fpnewui.h \
	fpoptui.h \
	fpwin.h \
	idircnt.h \
	iffilcnt.h \
	ifilcnti.h \
	ifilecnt.h \
	mdcrtape.h \
	memsrc.h \
	memtgt.h \
	misc1.h \
	rfilecnt.h \
	sprinter.h \
	warnoff.h \
	warnon.h

mdcrtool_SOURCES = \
	bdir.cpp \
	bfileptr.cpp \
	bmembuf.cpp \
	fileread.cpp \
	flexerr.cpp \
	mdcrfs.cpp \
	mdcrtape.cpp \
	mdcrtool.cpp \
	misc1.cpp \
	bdir.h \
	bfileptr.h \
	bintervl.h \
	bmembuf.h \
	config.h \
	fileread.h \
	flexerr.h \
	mdcrfs.h \
	mdcrtape.h \
	memsrc.h \
	memtgt.h \
	misc1.h \
	typedefs.h

flex2hex_SOURCES = \
	bfileptr.cpp \
	bmembuf.cpp \
	fileread.cpp \
	flex2hex.cpp \
	flexerr.cpp \
	misc1.cpp \
	bfileptr.h \
	bintervl.h \
	bmembuf.h \
	config.h \
	cvtwchar.h \
	fileread.h \
	flexerr.h \
	memsrc.h \
	memtgt.h \
	misc1.h \
	typedefs.h

dsktool_SOURCES = \
	bdate.cpp \
	bdir.cpp \
	bfileptr.cpp \
	dircont.cpp \
	dsktool.cpp \
	fcinfo.cpp \
	fcopyman.cpp \
	fdirent.cpp \
	ffilebuf.cpp \
	ffilecnt.cpp \
	filecnts.cpp \
	filfschk.cpp \
	flexerr.cpp \
	idircnt.cpp \
	iffilcnt.cpp \
	ifilecnt.cpp \
	misc1.cpp \
	rfilecnt.cpp \
	bdate.h \
	bdir.h \
	bfileptr.h \
	cvtwchar.h \
	dircont.h \
	fcinfo.h \
	fcopyman.h \
	fdirent.h \
	ffilebuf.h \
	ffilecnt.h \
	filecntb.h \
	filecnts.h \
	filecont.h \
	filfschk.h \
	flexemu.h \
	flexerr.h \
	idircnt.h \
	iffilcnt.h \
	ifilcnti.h \
	ifilecnt.h \
	misc1.h \
	rfilecnt.h \
	typedefs.h

test_SOURCES = \
	bcommand.cpp \
	bdate.cpp \
	bfileptr.cpp \
	binifile.cpp \
	bobservd.cpp \
	btime.cpp \
	clogfile.cpp  \
	colors.cpp \
	command.cpp \
	da6809.cpp \
	fcinfo.cpp \
	fcnffile.cpp \
	flexerr.cpp \
	inout.cpp \
	mc6809.cpp \
	mc6809in.cpp \
	mc6809st.cpp \
	memory.cpp \
	misc1.cpp \
	schedule.cpp \
	test_gccasm.cpp \
	testmain.cpp \
	bdate.h \
	bfileptr.h \
	bobserv.h \
	bobservd.h \
	bobshelp.h \
	btime.h \
	clogfile.h \
	command.h \
	cpustate.h \
	csetfreq.h \
	da6809.h \
	debug.h \
	e2.h \
	engine.h \
	fcinfo.h \
	flexerr.h \
	inout.h \
	ioaccess.h \
	mc6809.h \
	mc6809st.h \
	memory.h \
	misc1.h \
	schedcpu.h \
	schedule.h \
	sprinter.h \
	test_gccasm.h \
	typedefs.h \
	warnoff.h \
	warnon.h

################################################################################
# Use traditional-style suffix rules for generic Qt support.

# Create a *_moc.cpp file from a *.h file
# To see which *.h files are used to create *_moc.cpp files see
# variables flexemu_moc_hdr, fsetup_moc_hdr and flexplorer_moc_hdr.
.h_moc.cpp:
	${QT_MOC} $< -o $@

# Create a *_qrc.cpp file from a *.qrc file
.qrc_qrc.cpp:
	${QT_RCC} $< --name $(@F) -o $@

# Create a *_ui.h file from a *.ui file
.ui_ui.h:
	${QT_UIC} $< -o $@

################################################################################
# Post (un)installation hook:
#   gtk-update-icon-cache
#   update-mime-database
#   update-desktop-database

appicondir = $(datadir)/icons/hicolor

install-data-hook: update-icon-cache update-mime-database update-desktop-database

uninstall-hook: update-desktop-database update-mime-database update-icon-cache

update-icon-cache:
@IGNORE@	@-if test -z "$(DESTDIR)"; then \
@IGNORE@	echo "Updating Gtk icon cache.";\
@IGNORE@	$(GTK_UPDATE_ICON_CACHE) -f -t $(appicondir); \
@IGNORE@	else \
@IGNORE@	echo "*** Icon cache not updated. Remember to run:";\
@IGNORE@	echo "*** sudo $(GTK_UPDATE_ICON_CACHE) -f -t $(appicondir)";\
@IGNORE@	fi

update-mime-database:
@IGNORE@	@-if test -z "$(DESTDIR)"; then \
@IGNORE@	$(UPDATE_MIME_DATABASE) $(mimetypebasedir); \
@IGNORE@	fi

update-desktop-database:
@IGNORE@	@-if test -z "$(DESTDIR)"; then \
@IGNORE@	$(UPDATE_DESKTOP_DATABASE) $(desktopdir); \
@IGNORE@	fi

################################################################################

