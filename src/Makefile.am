pkgdata_DATA = boot

sysconf_DATA = flexemu.conf

desktop_DATA = flexemu.desktop \
	       flexemu-flexplorer.desktop \
	       flexemu-fsetup.desktop

mimetype_DATA = flexemu.xml

SUFFIXES = .rc .o .ui _ui.h .qrc _qrc.cpp .h _moc.cpp
.rc.o:
	@WINDRES_EXE@ -o $@ -O coff -i $<

EXTRA_DIST = fdtest.cpp mc6809ex.cpp engine.cpp confignt.h \
	     $(flexemu_ui) $(fsetup_ui) $(flexplorer_ui) \
	     $(flexemu_qrc) $(fsetup_qrc) $(flexplorer_qrc) \
	     Flexemu.ad flexemu.astyle \
	     JoystickTable.gnumeric \
	     flexemu.rc fsetup.rc flexdisk.rc \
	     flexemu.vcxproj flexplorer.vcxproj fsetup.vcxproj \
	     dsktool.vcxproj flex2hex.vcxproj mdcrtool.vcxproj \
	     msvcQtPath.props \
	     $(pkgdata_DATA) \
	     $(sysconf_DATA) \
	     $(desktop_DATA) \
	     $(mimetype_DATA)

SUBDIRS = bitmaps tools

# Directory where *.desktop files are located
desktopdir = $(datadir)/applications

# Directory where mimetype *.xml files are located
mimetypebasedir = $(datadir)/mime
mimetypedir = $(mimetypebasedir)/packages

bin_PROGRAMS   = flexemu @FLEXPLORER@ @FSETUP@ mdcrtool flex2hex dsktool
EXTRA_PROGRAMS = flexplorer fsetup mdcrtool flex2hex
noinst_PROGRAMS = test

AM_CXXFLAGS = --std=c++14 \
              -DF_DATADIR=\"$(pkgdatadir)\" -DF_SYSCONFDIR=\"$(sysconfdir)\"

flexemu_CXXFLAGS    = $(AM_CXXFLAGS) \
	@CXXSPEED@ @OS_TYPE@ @FLEXEMU_CXXADD@ @QT_CXXFLAGS@
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
flexemu_LDADD    = @FLEX_RC@ @FLEXEMU_LDADD@ \
	@WIN32_LIBS@ @PTHREAD_LD@ @QT_LIBS@
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
flexemu_ui = brkpt.ui logfil.ui cpustat.ui
flexemu_ui_hdr = $(flexemu_ui:.ui=_ui.h)
fsetup_ui = fsetup.ui
fsetup_ui_hdr = $(fsetup_ui:.ui=_ui.h)
flexplorer_ui = fpnew.ui fpopt.ui
flexplorer_ui_hdr = $(flexplorer_ui:.ui=_ui.h)

# Rule for Qt moc files:
# Define all header files from which to create *_moc.cpp files.
flexemu_moc_hdr = qtgui.h e2screen.h brkptui.h logfilui.h efslctle.h \
		  drawnwid.h
flexemu_moc_src = $(flexemu_moc_hdr:.h=_moc.cpp)
fsetup_moc_hdr = fsetupui.h
fsetup_moc_src = $(fsetup_moc_hdr:.h=_moc.cpp)
flexplorer_moc_hdr = fpnewui.h fpoptui.h fpwin.h fpmdich.h fpmodel.h fpedit.h
flexplorer_moc_src = $(flexplorer_moc_hdr:.h=_moc.cpp)

nodist_flexemu_SOURCES = $(flexemu_ui_hdr) $(flexemu_qrc_src) $(flexemu_moc_src)

nodist_fsetup_SOURCES = $(fsetup_ui_hdr) $(fsetup_qrc_src) $(fsetup_moc_src)

nodist_flexplorer_SOURCES = $(flexplorer_ui_hdr) $(flexplorer_qrc_src) \
			    $(flexplorer_moc_src)

BUILT_SOURCES = $(nodist_flexemu_SOURCES) \
		$(nodist_fsetup_SOURCES) \
		$(nodist_flexplorer_SOURCE)

CLEANFILES = $(nodist_flexemu_SOURCES) \
	     $(nodist_fsetup_SOURCES) \
	     $(nodist_flexplorer_SOURCES)

fsetup_SOURCES = \
fsetup.cpp fsetupui.cpp foptman.cpp brcfile.cpp benv.cpp misc1.cpp\
bfileptr.cpp bregistr.cpp colors.cpp \
fsetupui.h foptman.h brcfile.h benv.h misc1.h \
bfileptr.h bregistr.h sguiopts.h

flexemu_SOURCES = \
absgui.cpp acia1.cpp bdate.cpp bdir.cpp bjoystck.cpp colors.cpp \
command.cpp da6809.cpp e2floppy.cpp vico1.cpp vico2.cpp fcinfo.cpp \
fcopyman.cpp fdirent.cpp ffilebuf.cpp ffilecnt.cpp flexerr.cpp inout.cpp \
memory.cpp main.cpp mc146818.cpp mc6809.cpp mc6809in.cpp \
mc6821.cpp mc6850.cpp misc1.cpp mmu.cpp ndircont.cpp pia1.cpp \
pia2.cpp rfilecnt.cpp wd1793.cpp \
brcfile.cpp benv.cpp foptman.cpp bmembuf.cpp bregistr.cpp\
bcommand.cpp cacttrns.cpp csetfreq.cpp btime.cpp schedule.cpp \
clogfile.cpp bfileptr.cpp ifilecnt.cpp iffilcnt.cpp fileread.cpp \
filecnts.cpp mc6809st.cpp cvtwchar.cpp joystick.cpp \
apprun.cpp keyboard.cpp bytereg.cpp drisel.cpp terminal.cpp \
binifile.cpp fcnffile.cpp iodevdbg.cpp pia2v5.cpp mdcrtape.cpp \
bobservd.cpp \
qtgui.cpp e2screen.cpp brkptui.cpp logfilui.cpp efslctle.cpp bui.cpp \
drawnwid.cpp winmain.cpp \
sprinter.h \
absgui.h acia1.h bdate.h bdir.h bjoystck.h command.h \
da6809.h debug.h e2.h e2floppy.h vico1.h vico2.h engine.h fcinfo.h \
fcopyman.h fdirent.h ffilebuf.h ffilecnt.h flexerr.h inout.h \
iodevice.h mc146818.h mc6809.h mc6821.h mc6850.h memory.h misc1.h ioaccess.h \
mmu.h ndircont.h pia1.h pia2.h rfilecnt.h typedefs.h wd1793.h \
brcfile.h benv.h foptman.h bmembuf.h bregistr.h \
bcommand.h cacttrns.h csetfreq.h btime.h schedule.h cpustate.h \
clogfile.h bfileptr.h ifilecnt.h ifilcnti.h iffilcnt.h \
filecntb.h filecnts.h mc6809st.h \
schedcpu.h absdisas.h cvtwchar.h flexemu.h scpulog.h sguiopts.h \
fileread.h memtgt.h joystick.h keyboard.h apprun.h winctxt.h terminal.h \
bytereg.h drisel.h binifile.h fcnffile.h iodevdbg.h pia2v5.h mdcrtape.h \
memsrc.h bobserv.h bobservd.h bobshelp.h \
tsl/robin_growth_policy.h tsl/robin_hash.h tsl/robin_map.h tsl/robin_set.h \
bintervl.h crc.h warnoff.h warnon.h \
qtgui.h e2screen.h brkptui.h logfilui.h efslctle.h bui.h drawnwid.h \
propsui.h bmp.h winmain.h

flexplorer_SOURCES = \
fpmain.cpp fpwin.cpp fpmdich.cpp \
fpmodel.cpp fpedit.cpp \
fpnewui.cpp fpoptui.cpp \
fpdnd.cpp \
bdate.cpp bdir.cpp dircont.cpp fcinfo.cpp \
fcopyman.cpp fdirent.cpp \
ffilebuf.cpp ffilecnt.cpp flexerr.cpp misc1.cpp \
rfilecnt.cpp brcfile.cpp benv.cpp cvtwchar.cpp \
bmembuf.cpp bfileptr.cpp \
ifilecnt.cpp idircnt.cpp iffilcnt.cpp filecnts.cpp bregistr.cpp \
bprocess.cpp mdcrtape.cpp bobservd.cpp \
fpwin.h fpmdich.h \
fpmodel.h fpedit.h \
fpnewui.h fpoptui.h fpdnd.h fpattrui.h \
bdate.h bdir.h bobserv.h bobservd.h bobshelp.h dircont.h fcinfo.h \
fcopyman.h fdirent.h \
ffilebuf.h ffilecnt.h flexerr.h misc1.h \
rfilecnt.h brcfile.h benv.h \
bmembuf.h \
bfileptr.h ifilecnt.h ifilcnti.h idircnt.h iffilcnt.h \
filecntb.h filecont.h bregistr.h bprocess.h cvtwchar.h \
sprinter.h flexemu.h mdcrtape.h bintervl.h

mdcrtool_SOURCES = \
mdcrtool.cpp mdcrtape.cpp mdcrfs.cpp fileread.cpp bfileptr.cpp bmembuf.cpp \
bdir.cpp flexerr.cpp misc1.cpp \
mdcrtape.h mdcrfs.h fileread.h memsrc.h memtgt.h bfileptr.h bmembuf.h bdir.h \
typedefs.h misc1.h config.h flexerr.h bintervl.h

flex2hex_SOURCES = \
fileread.cpp bfileptr.cpp bmembuf.cpp \
flexerr.cpp misc1.cpp flex2hex.cpp \
fileread.h memsrc.h memtgt.h bfileptr.h bmembuf.h \
bintervl.h typedefs.h misc1.h config.h flexerr.h

dsktool_SOURCES = \
dsktool.cpp \
flexerr.cpp ffilebuf.cpp bfileptr.cpp bdir.cpp bdate.cpp \
ffilecnt.cpp rfilecnt.cpp dircont.cpp \
iffilcnt.cpp ifilecnt.cpp idircnt.cpp \
fcinfo.cpp fcopyman.cpp fdirent.cpp misc1.cpp \
filfschk.cpp filecnts.cpp \
flexerr.h ffilebuf.h bfileptr.h bdir.h bdate.h \
ffilecnt.h rfilecnt.h dircont.h \
iffilcnt.h ifilecnt.h idircnt.h \
fcinfo.h fcopyman.h fdirent.h filfschk.h misc1.h

test_SOURCES = \
testmain.cpp test_gccasm.cpp bdate.cpp colors.cpp \
command.cpp bfileptr.cpp da6809.cpp fcinfo.cpp \
flexerr.cpp inout.cpp \
memory.cpp mc6809.cpp mc6809in.cpp misc1.cpp \
bcommand.cpp btime.cpp schedule.cpp \
clogfile.cpp  \
mc6809st.cpp binifile.cpp fcnffile.cpp bobservd.cpp \
test_gccasm.h bdate.h \
command.h bfileptr.h da6809.h debug.h e2.h engine.h fcinfo.h \
flexerr.h inout.h \
memory.h mc6809.h misc1.h ioaccess.h \
typedefs.h \
csetfreq.h btime.h schedule.h cpustate.h \
clogfile.h \
mc6809st.h \
schedcpu.h sprinter.h \
bobshelp.h bobservd.h bobserv.h \
warnoff.h warnon.h

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
#   (Un)install application default file (X11)

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

