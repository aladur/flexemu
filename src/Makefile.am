pkgdata_DATA = boot flexemu.xpm  \
		flexemu1.xpm flexemu2.xpm flexemu3.xpm flexcpu.xpm

sysconf_DATA = flexemu.conf

desktop_DATA = flexemu.desktop \
	       flexemu-flexplorer.desktop \
	       flexemu-fsetup.desktop

SUFFIXES = .rc .o
.rc.o:
	@WINDRES_EXE@ -o $@ -O coff -i $<

EXTRA_DIST = fdtest.cpp mc6809ex.cpp engine.cpp confignt.h \
             fsetup.ui fsetup.qrc Flexemu.ad flexemu.astyle \
	     JoystickTable.gnumeric \
	     flexdisk.rc wwinres.rc winres.rc fsetup.rc \
	     flexemu.vcxproj flexplorer.vcxproj fsetup.vcxproj \
	     $(pkgdata_DATA) \
	     $(sysconf_DATA) \
	     $(desktop_DATA)


SUBDIRS = bitmaps tools

# Directory where .desktop files are located
desktopdir = $(datadir)/applications

bin_PROGRAMS   = flexemu @FLEXPLORER@ @FSETUP@ mdcrtool flex2hex dsktool
EXTRA_PROGRAMS = flexplorer fsetup mdcrtool flex2hex
noinst_PROGRAMS = test

AM_CXXFLAGS = --std=c++11 \
              -DF_DATADIR=\"$(pkgdatadir)\" -DF_SYSCONFDIR=\"$(sysconfdir)\"

flexemu_CXXFLAGS    = $(AM_CXXFLAGS) \
			@CXXSPEED@ @OS_TYPE@ @X_CFLAGS@ @FLEXEMU_CXXADD@
flexplorer_CXXFLAGS = $(AM_CXXFLAGS) @OS_TYPE@ @WX_CFLAGS@
fsetup_CXXFLAGS     = $(AM_CXXFLAGS) @OS_TYPE@ @QT_CXXFLAGS@
mdcrtool_CXXFLAGS   = $(AM_CXXFLAGS) @OS_TYPE@
flex2hex_CXXFLAGS   = $(AM_CXXFLAGS) @OS_TYPE@
dsktool_CXXFLAGS    = $(AM_CXXFLAGS) @OS_TYPE@
test_CXXFLAGS       = $(AM_CXXFLAGS) @OS_TYPE@ -DUNIT_TEST

flexplorer_LDADD = @WX_LIBS@ @PTHREAD_LD@
fsetup_LDADD     = @QT_LIBS@ @PTHREAD_LD@
mdcrtool_LDADD   = @PTHREAD_LD@
flex2hex_LDADD   = @PTHREAD_LD@
dsktool_LDADD    = @PTHREAD_LD@
flexemu_LDADD    = @FLEX_RC@ @FLEXEMU_LDADD@ @X_PRE_LIBS@ @X_LIBS@ \
                   @X_EXTRA_LIBS@ @WIN32_LIBS@ @PTHREAD_LD@
test_LDADD       = @PTHREAD_LD@
flexemu_DEPENDENCIES = @FLEX_RC@

BUILT_SOURCES = ui_fsetup.h.stamp

fsetup_SOURCES = \
fsetup.cpp fsetupui.cpp foptman.cpp brcfile.cpp benv.cpp misc1.cpp\
bfileptr.cpp bregistr.cpp colors.cpp \
moc_fsetupui.cpp qrc_fsetup.cpp \
fsetupui.h foptman.h brcfile.h benv.h misc1.h \
bfileptr.h bregistr.h sguiopts.h \
ui_fsetup.h

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
bintervl.h crc.h

flexplorer_SOURCES = \
bdate.cpp bdir.cpp contpdlg.cpp dircont.cpp fcinfo.cpp \
fcopyman.cpp fdcframe.cpp fddnd.cpp fdirent.cpp fdlist.cpp \
fdpframe.cpp ffilebuf.cpp ffilecnt.cpp flexdisk.cpp flexerr.cpp misc1.cpp \
rfilecnt.cpp optdlg.cpp brcfile.cpp benv.cpp cvtwchar.cpp \
bmembuf.cpp bfileptr.cpp \
ifilecnt.cpp idircnt.cpp iffilcnt.cpp filecnts.cpp fmenufac.cpp bregistr.cpp \
bprocess.cpp mdcrtape.cpp bobservd.cpp fdfdnd.cpp \
bdate.h bdir.h bobserv.h bobservd.h bobshelp.h contpdlg.h dircont.h fcinfo.h \
fcopyman.h fdcframe.h fddnd.h fdirent.h fdlist.h \
fdpframe.h ffilebuf.h ffilecnt.h flexdisk.h flexerr.h misc1.h \
rfilecnt.h brcfile.h benv.h fmenufac.h \
bmembuf.h \
bfileptr.h ifilecnt.h ifilcnti.h idircnt.h iffilcnt.h \
filecntb.h filecont.h optdlg.h bregistr.h bprocess.h cvtwchar.h \
sprinter.h flexemu.h mdcrtape.h fdfdnd.h bintervl.h

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
bobshelp.h bobservd.h bobserv.h

################################################################################
# Specific build steps needed for Qt support.
moc_fsetupui.cpp: fsetupui.h
	${QT_MOC} $< -o $@

ui_fsetup.h.stamp: fsetup.ui
	${QT_UIC} fsetup.ui -o ui_fsetup.h.tmp
	cmp ui_fsetup.h.tmp ui_fsetup.h || mv ui_fsetup.h.tmp ui_fsetup.h

qrc_fsetup.cpp: fsetup.qrc
	${QT_RCC} -name fsetup $< -o $@

clean-local: clean-local-qt

.PHONY: clean-local-qt

clean-local-qt:
	rm -f moc_fsetupui.cpp ui_fsetup.h qrc_fsetup.cpp

################################################################################
# Post installation hook to execute update-desktop-database if present.
# Post installation hook to execute gtk-update-icon-cache if present.
appicondir = $(datadir)/icons/hicolor

install-data-hook :
	-$(UPDATE_DESKTOP_DATABASE) -q $(desktopdir)
	-$(GTK_UPDATE_ICON_CACHE) -q $(appicondir)

uninstall-hook :
	-$(UPDATE_DESKTOP_DATABASE) -q $(desktopdir)
	-$(GTK_UPDATE_ICON_CACHE) -q $(appicondir)

################################################################################
@no_appsdir@install \
@no_appsdir@install-app \
@no_appsdir@install-full :
@no_appsdir@	@echo installing $(APPSDIR)/Flexemu
@no_appsdir@	@$(INSTALL_DATA) $(top_srcdir)/src/Flexemu.ad $(APPSDIR)/Flexemu
@no_appsdir@	@echo "... installed app-defaults"

@no_appsdir@uninstall \
@no_appsdir@uninstall-app \
@no_appsdir@uninstall-full :
@no_appsdir@	-$(RM) $(APPSDIR)/Flexemu
################################################################################

