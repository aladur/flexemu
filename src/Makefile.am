pkgdata_DATA = boot flexemu.xpm  \
		flexemu1.xpm flexemu2.xpm flexemu3.xpm flexcpu.xpm

sysconf_DATA = flexemu.conf

desktop_DATA = flexemu.desktop \
	       flexemu-flexplorer.desktop \
	       flexemu-fsetup.desktop

mimetype_DATA = flexemu.xml

SUFFIXES = .rc .o .ui _ui.h .qrc _qrc.cpp .h _moc.cpp
.rc.o:
	@WINDRES_EXE@ -o $@ -O coff -i $<

EXTRA_DIST = fdtest.cpp mc6809ex.cpp engine.cpp confignt.h \
	     fsetup.ui fpopt.ui fpnew.ui \
	     fsetup.qrc Flexemu.ad flexemu.astyle \
	     fpmain.qrc \
	     JoystickTable.gnumeric \
	     wwinres.rc winres.rc fsetup.rc \
	     flexemu.vcxproj flexplorer.vcxproj fsetup.vcxproj \
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

AM_CXXFLAGS = --std=c++11 \
              -DF_DATADIR=\"$(pkgdatadir)\" -DF_SYSCONFDIR=\"$(sysconfdir)\"

flexemu_CXXFLAGS    = $(AM_CXXFLAGS) \
			@CXXSPEED@ @OS_TYPE@ @X_CFLAGS@ @FLEXEMU_CXXADD@
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
flexemu_LDADD    = @FLEX_RC@ @FLEXEMU_LDADD@ @X_PRE_LIBS@ @X_LIBS@ \
                   @X_EXTRA_LIBS@ @WIN32_LIBS@ @PTHREAD_LD@
test_LDADD       = @PTHREAD_LD@
flexemu_DEPENDENCIES = @FLEX_RC@

# Rule for Qt moc files:
# Define all fsetup header files from which to create moc cpp files.
fsetup_moc_hdr = fsetupui.h
fsetup_moc_src = $(fsetup_moc_hdr:.h=_moc.cpp)

nodist_fsetup_SOURCES = fsetup_ui.h fsetup_qrc.cpp $(fsetup_moc_src)

# Define all flexplorer header files from which to create moc cpp files.
flexplorer_moc_hdr = fpnewui.h fpoptui.h fpwin.h fpmdich.h fpmodel.h fpedit.h
flexplorer_moc_src = $(flexplorer_moc_hdr:.h=_moc.cpp)

nodist_flexplorer_SOURCES = fpnew_ui.h fpopt_ui.h fpmain_qrc.cpp \
			    $(flexplorer_moc_src)

BUILT_SOURCES = $(nodist_fsetup_SOURCES) $(nodist_flexplorer_SOURCES)

CLEANFILES = $(nodist_fsetup_SOURCES) $(nodist_flexplorer_SOURCES)

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
pia2.cpp rfilecnt.cpp wd1793.cpp win32gui.cpp xabsgui.cpp xres.cpp \
xtgui.cpp brcfile.cpp benv.cpp foptman.cpp bmembuf.cpp bregistr.cpp\
bcommand.cpp cacttrns.cpp csetfreq.cpp btime.cpp schedule.cpp \
btimer.cpp clogfile.cpp bfileptr.cpp ifilecnt.cpp iffilcnt.cpp fileread.cpp \
filecnts.cpp mc6809st.cpp cvtwchar.cpp joystick.cpp \
apprun.cpp keyboard.cpp bytereg.cpp drisel.cpp terminal.cpp \
binifile.cpp fcnffile.cpp iodevdbg.cpp pia2v5.cpp mdcrtape.cpp \
bobservd.cpp \
sprinter.h \
absgui.h acia1.h bdate.h bdir.h bjoystck.h command.h \
da6809.h debug.h e2.h e2floppy.h vico1.h vico2.h engine.h fcinfo.h \
fcopyman.h fdirent.h ffilebuf.h ffilecnt.h flexerr.h inout.h \
iodevice.h mc146818.h mc6809.h mc6821.h mc6850.h memory.h misc1.h ioaccess.h \
mmu.h ndircont.h pia1.h pia2.h rfilecnt.h typedefs.h wd1793.h win32gui.h \
xabsgui.h xtgui.h brcfile.h benv.h foptman.h bmembuf.h bregistr.h \
bcommand.h cacttrns.h csetfreq.h btime.h schedule.h cpustate.h btimer.h \
clogfile.h bfileptr.h ifilecnt.h ifilcnti.h iffilcnt.h \
resource.h filecntb.h filecnts.h mc6809st.h \
schedcpu.h absdisas.h cvtwchar.h flexemu.h scpulog.h sguiopts.h \
fileread.h memtgt.h joystick.h keyboard.h apprun.h winctxt.h terminal.h \
bytereg.h drisel.h binifile.h fcnffile.h iodevdbg.h pia2v5.h mdcrtape.h \
memsrc.h bobserv.h bobservd.h bobshelp.h \
tsl/robin_growth_policy.h tsl/robin_hash.h tsl/robin_map.h tsl/robin_set.h \
bintervl.h crc.h warnoff.h warnon.h

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
btimer.cpp clogfile.cpp  \
mc6809st.cpp binifile.cpp fcnffile.cpp bobservd.cpp \
test_gccasm.h bdate.h \
command.h bfileptr.h da6809.h debug.h e2.h engine.h fcinfo.h \
flexerr.h inout.h \
memory.h mc6809.h misc1.h ioaccess.h \
typedefs.h \
csetfreq.h btime.h schedule.h cpustate.h btimer.h \
clogfile.h \
mc6809st.h \
schedcpu.h sprinter.h \
bobshelp.h bobservd.h bobserv.h \
warnoff.h warnon.h

################################################################################
# Use traditional-style suffix rules for generic Qt support.

# Create a *_moc.cpp file from a *.h file
# To see which *.h files are used to create *_moc.cpp files see
# variables fsetup_moc_hdr and flexplorer_moc_hdr.
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

install-data-hook: update-icon-cache update-mime-database update-desktop-database install-app-defaults

uninstall-hook: update-desktop-database update-mime-database update-icon-cache uninstall-app-defaults

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

install-app-defaults:
@no_appsdir@	@-if test -z "$(DESTDIR)"; then \
@no_appsdir@	echo Installing $(APPSDIR)/Flexemu; \
@no_appsdir@	$(INSTALL_DATA) $(top_srcdir)/src/Flexemu.ad $(APPSDIR)/Flexemu; \
@no_appsdir@	fi

uninstall-app-defaults:
@no_appsdir@	@-if test -z "$(DESTDIR)"; then \
@no_appsdir@	$(RM) $(APPSDIR)/Flexemu; \
@no_appsdir@	fi

################################################################################

