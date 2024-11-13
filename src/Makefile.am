pkgdata_DATA = boot

sysconf_DATA = \
	flexemu.conf \
	flexlabl.conf

desktop_DATA = \
	flexemu.desktop \
	flexemu-flexplorer.desktop

mimetype_DATA = flexemu.xml

libjsoncpp_top = jsoncpp/src/lib_json

SUFFIXES = .rc .o .ui _ui.h .qrc _qrc.cpp .h _moc.cpp .h.gch

.rc.o:
	@WINDRES_EXE@ -o $@ -O coff -i $<

EXTRA_DIST = \
	$(desktop_DATA) \
	$(flexemu_qrc) \
	$(flexemu_ui) \
	$(flexplorer_qrc) \
	$(flexplorer_ui) \
	$(mimetype_DATA) \
	$(pkgdata_DATA) \
	$(sysconf_DATA) \
	confignt.h \
	dsktool.vcxproj \
	engine.cpi \
	flex2hex.vcxproj \
	flexdisk.rc \
	flexemu.astyle \
	flexemu.rc \
	flexemu.vcxproj \
	flexplorer.vcxproj \
	JoystickTable.gnumeric \
	mc6809ex.cpi \
	mdcrtool.vcxproj \
	msvcQtPath.props.in \
	pch.h \
	pch.cpp \
	options.txt

SUBDIRS = bitmaps tools

# Directory where *.desktop files are located
desktopdir = $(datadir)/applications

# Directory where mimetype *.xml files are located
mimetypebasedir = $(datadir)/mime
mimetypedir = $(mimetypebasedir)/packages

noinst_LIBRARIES = libflex.a libfmt.a libjsoncpp.a

bin_PROGRAMS = \
	$(FLEXPLORER) \
	dsktool \
	flex2hex \
	flexemu \
	mdcrtool

noinst_PROGRAMS = updatemd

noinst_DATA = metadata.json

EXTRA_PROGRAMS = \
	dsktool \
	flex2hex \
	flexplorer \
	mdcrtool

DIRFLAGS = -DF_DATADIR=\"$(pkgdatadir)\" -DF_SYSCONFDIR=\"$(sysconfdir)\"

AM_CXXFLAGS = \
	--std=c++17 \
	$(DIRFLAGS) \
	-isystem $(top_srcdir)/fmt/include

# To use precompiled headers all targets have to use the same CXXFLAGS
# (especially preprocessor directives).
ALL_TARGETS_CXXFLAGS = \
	-pipe -D_REENTRANT -pthread -fPIC \
	$(CXXSPEED) $(OS_TYPE) $(X_CFLAGS) $(AM_CXXFLAGS) \
	$(CXXWARNINGS) $(MACOSX_VERSION_MIN) \
		-isystem $(top_srcdir)/jsoncpp/include

libjsoncpp_a_CXXFLAGS= --std=c++11 -isystem $(top_srcdir)/jsoncpp/include
libfmt_a_CXXFLAGS   = --std=c++11 -isystem $(top_srcdir)/fmt/include
libflex_a_CXXFLAGS  = $(ALL_TARGETS_CXXFLAGS) -include pch.h
flexemu_CXXFLAGS    = $(QT_CXXFLAGS) $(ALL_TARGETS_CXXFLAGS) -include pch.h
flexplorer_CXXFLAGS = $(QT_CXXFLAGS) $(ALL_TARGETS_CXXFLAGS) -include pch.h
mdcrtool_CXXFLAGS   = $(ALL_TARGETS_CXXFLAGS) -include pch.h
flex2hex_CXXFLAGS   = $(ALL_TARGETS_CXXFLAGS) -include pch.h
dsktool_CXXFLAGS    = $(ALL_TARGETS_CXXFLAGS) -include pch.h
updatemd_CXXFLAGS   = --std=c++17 $(OS_TYPE) \
			-isystem $(top_srcdir)/jsoncpp/include

libflex_a_LIBADD = $(LIBOBJS)

flexplorer_LDADD = $(QT_LIBS) $(PTHREAD_LD) libflex.a libfmt.a
mdcrtool_LDADD   = $(PTHREAD_LD) libflex.a libfmt.a
flex2hex_LDADD   = $(PTHREAD_LD) libflex.a libfmt.a
dsktool_LDADD    = $(PTHREAD_LD) libflex.a libfmt.a
flexemu_LDADD    = \
	$(FLEX_RC) $(FLEXEMU_LDADD) \
	$(WIN32_LIBS) $(PTHREAD_LD) $(QT_LIBS) $(X_LIBS) \
	libflex.a libfmt.a
updatemd_LDADD    = $(PTHREAD_LD) libflex.a libfmt.a libjsoncpp.a

libflex_a_DEPENDENCIES  = libfmt.a
flexemu_DEPENDENCIES    = $(FLEX_RC) libflex.a libfmt.a
flexplorer_DEPENDENCIES = libflex.a libfmt.a
flex2hex_DEPENDENCIES   = libflex.a libfmt.a
dsktool_DEPENDENCIES    = libflex.a libfmt.a
mdcrtool_DEPENDENCIES   = libflex.a libfmt.a
updatemd_DEPENDENCIES   = libflex.a libfmt.a libjsoncpp.a

# Rule for precompiled header:
# Build a pch.h.gch file from pch.h
pch_h = pch.h
pch_h_gch = $(pch_h:.h=.h.gch)

# Rule for Qt qrc files:
# Define all *.qrc files from which to create *_qrc.cpp files.
flexemu_qrc = flexemu.qrc
flexemu_qrc_src = $(flexemu_qrc:.qrc=_qrc.cpp)
flexplorer_qrc = fpmain.qrc
flexplorer_qrc_src = $(flexplorer_qrc:.qrc=_qrc.cpp)

# Rule for Qt ui files:
# Define all *.ui files from which to create *_ui.h files.
flexemu_ui = \
	brkpt.ui \
	cpustat.ui \
	logfil.ui \
	fsetup.ui \
	pprev.ui \
	about.ui
flexemu_ui_hdr = $(flexemu_ui:.ui=_ui.h)
flexplorer_ui = \
	fpnew.ui \
	fpopt.ui \
	fpcnv.ui
flexplorer_ui_hdr = $(flexplorer_ui:.ui=_ui.h)

# Rule for Qt moc files:
# Define all header files from which to create *_moc.cpp files.
flexemu_moc_hdr = \
	brkptui.h \
	drawnwid.h \
	e2screen.h \
	efslctle.h \
	fsetupui.h \
	logfilui.h \
	qtgui.h \
	poutwin.h
flexemu_moc_src = $(flexemu_moc_hdr:.h=_moc.cpp)
flexplorer_moc_hdr = \
	fpedit.h \
	fpmdich.h \
	fpnewui.h \
	fpoptui.h \
	fpcnvui.h \
	fpwin.h
flexplorer_moc_src = $(flexplorer_moc_hdr:.h=_moc.cpp)

nodist_flexemu_SOURCES = \
	$(flexemu_ui_hdr) $(flexemu_qrc_src) $(flexemu_moc_src) $(pch_h_gch)

nodist_flexplorer_SOURCES = \
	$(flexplorer_ui_hdr) $(flexplorer_qrc_src) $(flexplorer_moc_src) \
	$(pch_h_gch)

nodist_flex2hex_SOURCES = $(pch_h_gch)
nodist_dsktool_SOURCES = $(pch_h_gch)
nodist_mdcrtool_SOURCES = $(pch_h_gch)

BUILT_SOURCES = \
	$(nodist_flexemu_SOURCES) \
	$(nodist_flexplorer_SOURCES)

CLEANFILES = \
	$(nodist_flexemu_SOURCES) \
	$(nodist_flexplorer_SOURCES)

flexemu_SOURCES = \
	absgui.cpp \
	acia1.cpp \
	apprun.cpp \
	bjoystck.cpp \
	blinxsys.cpp \
	brkptui.cpp \
	bui.cpp \
	bytereg.cpp \
	cacttrns.cpp \
	clogfile.cpp \
	colors.cpp \
	command.cpp \
	csetfreq.cpp \
	da6809.cpp \
	drawnwid.cpp \
	drisel.cpp \
	e2floppy.cpp \
	e2screen.cpp \
	efslctle.cpp \
	fdoptman.cpp \
	flblfile.cpp \
	foptman.cpp \
	fsetupui.cpp \
	fversion.cpp \
	inout.cpp \
	iodevdbg.cpp \
	joystick.cpp \
	keyboard.cpp \
	logfilui.cpp \
	main.cpp \
	mc146818.cpp \
	mc6809.cpp \
	mc6809in.cpp \
	mc6809lg.cpp \
	mc6809st.cpp \
	mc6821.cpp \
	mc6850.cpp \
	mmu.cpp \
	ndircont.cpp \
	pagedet.cpp \
	pagedetd.cpp \
	pia1.cpp \
	pia2.cpp \
	pia2v5.cpp \
	poutwin.cpp \
	poverhlp.cpp \
	qtfree.cpp \
	qtgui.cpp \
	schedule.cpp \
	sodiff.cpp \
	soptions.cpp \
	terminal.cpp \
	vico1.cpp \
	vico2.cpp \
	wd1793.cpp \
	winmain.cpp \
	asciictl.h \
	absdisas.h \
	absgui.h \
	acia1.h \
	apprun.h \
	bcommand.h \
	bdate.h \
	bdir.h \
	benv.h \
	binifile.h \
	bintervl.h \
	blinxsys.h \
	bjoystck.h \
	bmembuf.h \
	bmp.h \
	bobserv.h \
	bobservd.h \
	bobshelp.h \
	brcfile.h \
	bregistr.h \
	brkptui.h \
	breltime.h \
	bscopeex.h \
	btime.h \
	bui.h \
	bytereg.h \
	cacttrns.h \
	cistring.h \
	clogfile.h \
	colors.h \
	command.h \
	config.h \
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
	efiletim.h \
	efslctle.h \
	engine.h \
	fattrib.h \
	fcinfo.h \
	fcnffile.h \
	fcopyman.h \
	fdirent.h \
	fdoptman.h \
	ffilebuf.h \
	ffilecnt.h \
	filecntb.h \
	filecnts.h \
	fileread.h \
	flblfile.h \
	flexemu.h \
	flexerr.h \
	foptman.h \
	fsetupui.h \
	fversion.h \
	iffilcnt.h \
	ifilcnti.h \
	ifilecnt.h \
	inout.h \
	iodevdbg.h \
	iodevice.h \
	joystick.h \
	keyboard.h \
	logfilui.h \
	mc146818.h \
	mc6809.h \
	mc6809lg.h \
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
	pagedet.h \
	pagedetd.h \
	pia1.h \
	pia2.h \
	pia2v5.h \
	poutwin.h \
	poverhlp.h \
	propsui.h \
	qtfree.h \
	qtgui.h \
	rfilecnt.h \
	rndcheck.h \
	schedcpu.h \
	schedule.h \
	scpulog.h \
	sodiff.h \
	soptions.h \
	terminal.h \
	typedefs.h \
	vico1.h \
	vico2.h \
	warnoff.h \
	warnon.h \
	wd1793.h \
	winctxt.h \
	winmain.h

flexplorer_SOURCES = \
	fpattrui.cpp \
	fpcnvui.cpp \
	fpdnd.cpp \
	fpedit.cpp \
	fpmain.cpp \
	fpmdich.cpp \
	fpmodel.cpp \
	fpnewui.cpp \
	fpoptman.cpp \
	fpoptui.cpp \
	fpwin.cpp \
	fversion.cpp \
	qtfree.cpp \
	bdate.h \
	bdir.h \
	benv.h \
	bintervl.h \
	bmembuf.h \
	bprocess.h \
	brcfile.h \
	bregistr.h \
	btime.h \
	cistring.h \
	config.h \
	cvtwchar.h \
	dircont.h \
	efilesiz.h \
	efiletim.h \
	fattrib.h \
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
	fpcnvui.h \
	fpdnd.h \
	fpedit.h \
	fpmdich.h \
	fpmodel.h \
	fpnewui.h \
	fpoptman.h \
	fpoptui.h \
	fpwin.h \
	fversion.h \
	idircnt.h \
	iffilcnt.h \
	ifilcnti.h \
	ifilecnt.h \
	mdcrtape.h \
	memsrc.h \
	memtgt.h \
	misc1.h \
	qtfree.h \
	rndcheck.h \
	rfilecnt.h \
	sfpopts.h \
	warnoff.h \
	warnon.h

mdcrtool_SOURCES = \
	mdcrfs.cpp \
	mdcrtool.cpp \
	bdir.h \
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
	flex2hex.cpp \
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
	dsktool.cpp \
	bdate.h \
	bdir.h \
	btime.h \
	cistring.h \
	config.h \
	cvtwchar.h \
	dircont.h \
	efiletim.h \
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

libflex_a_SOURCES = \
	bdate.cpp \
	bdir.cpp \
	benv.cpp \
	binifile.cpp \
	bmembuf.cpp \
	bobservd.cpp \
	bprocess.cpp \
	brcfile.cpp \
	bregistr.cpp \
	breltime.cpp \
	btime.cpp \
	cvtwchar.cpp \
	dircont.cpp \
	fattrib.cpp \
	fcinfo.cpp \
	fcnffile.cpp \
	fcopyman.cpp \
	fdirent.cpp \
	ffilebuf.cpp \
	ffilecnt.cpp \
	filecnts.cpp \
	fileread.cpp \
	filfschk.cpp \
	flexerr.cpp \
	idircnt.cpp \
	iffilcnt.cpp \
	ifilecnt.cpp \
	mdcrtape.cpp \
	memory.cpp \
	misc1.cpp \
	rfilecnt.cpp \
	rndcheck.cpp \
	bcommand.h \
	bdate.h \
	bdir.h \
	benv.h \
	binifile.h \
	bmembuf.h \
	bobserv.h \
	bobservd.h \
	bobshelp.h \
	bprocess.h \
	brcfile.h \
	bregistr.h \
	breltime.h \
	btime.h \
	cistring.h \
	config.h \
	cvtwchar.h \
	dircont.h \
	efiletim.h \
	fattrib.h \
	fcinfo.h \
	fcnffile.h \
	fcopyman.h \
	fdirent.h \
	ffilebuf.h \
	ffilecnt.h \
	filecntb.h \
	filecnts.h \
	fileread.h \
	filecont.h \
	filfschk.h \
	flexemu.h \
	flexerr.h \
	fversion.h \
	idircnt.h \
	iffilcnt.h \
	ifilcnti.h \
	ifilecnt.h \
	mdcrtape.h \
	memory.h \
	misc1.h \
	rfilecnt.h \
	rndcheck.h \
	typedefs.h

libfmt_a_SOURCES = \
	../fmt/src/format.cc \
	../fmt/src/os.cc

libjsoncpp_a_SOURCES = \
	$(top_srcdir)/$(libjsoncpp_top)/json_reader.cpp \
	$(top_srcdir)/$(libjsoncpp_top)/json_value.cpp \
	$(top_srcdir)/$(libjsoncpp_top)/json_writer.cpp

updatemd_SOURCES = \
	updatemd.cpp \
	benv.h \
	bprocess.h \
	misc1.h

# Execute updatemd to update file metadata.json.
# It has all c++ and c-header files as dependent to be able to update the
# lines of code for any source code change. Generated files are filtered
# out. See options.txt
metadata.json: updatemd $(libflex_a_SOURCES) \
	$(flexemu_SOURCES) $(flexplorer_SOURCES) $(dsktool_SOURCES) \
	$(flex2hex_SOURCES) $(mdcrtool_SOURCES) $(updatemd_SOURCES)
	@set +x; \
	cd $(srcdir) > /dev/null; \
	if $(AM_V_P); then set -x; else echo "  GENMD    $@"; fi; \
	$(abs_builddir)/updatemd -l -k src -f $@; \
	cd - > /dev/null

################################################################################
# Use traditional-style suffix rules for precompiled header support.
# Also define dependencies of precompiled header file.
.h.h.gch:
	@set +x; \
	if $(AM_V_P); then set -x; else echo "  CXXPCH   $@"; fi; \
	$(CXX) $(DEFS) $(ALL_TARGETS_CXXFLAGS) $(CXXFLAGS) \
	-o $@ -x c++-header -c $<

pch.h.gcp: pch.h misc1.h

################################################################################
# Use traditional-style suffix rules for generic Qt support.

# Create a *_moc.cpp file from a *.h file
# To see which *.h files are used to create *_moc.cpp files see
# variables flexemu_moc_hdr and flexplorer_moc_hdr.
.h_moc.cpp:
	@set +x; \
	if $(AM_V_P); then set -x; else echo "  QTMOC    $@"; fi; \
	${QT_MOC} $< -o $@

# Create a *_qrc.cpp file from a *.qrc file
.qrc_qrc.cpp:
	@set +x; \
	if $(AM_V_P); then set -x; else echo "  QTRCC    $@"; fi; \
	${QT_RCC} $< --name $(@F) -o $@

# Create a *_ui.h file from a *.ui file
.ui_ui.h:
	@set +x; \
	if $(AM_V_P); then set -x; else echo "  QTUIC    $@"; fi; \
	${QT_UIC} $< -o $@

# Static code analysis using clang-tidy. By default it reads the configuration
# from the file .clang-tidy in the current directory or nearest parent dir.
# Generated *_moc.cpp and *_qrc.cpp files are excluded from the check.
clang-tidy-check:
	$(CLANG_TIDY) `ls -1 *.cpp | sed "/.*_\(moc\|qrc\).cpp/d"` \
	-- @OS_TYPE@ $(DIRFLAGS) @CLANG_TIDY_OPTS@ \
	-isystem $(top_srcdir)/fmt/include \
	-isystem $(top_srcdir)/jsoncpp/include

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

