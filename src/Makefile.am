pkgdata_DATA = boot flexemu.xpm  \
		flexemu1.xpm flexemu2.xpm flexemu3.xpm flexcpu.xpm

SUFFIXES = .rc .o
.rc.o:
	@WINDRES_EXE@ -o $@ -O coff -i $<

EXTRA_DIST = fdtest.cpp mc6809ex.cpp engine.cpp confignt.h \
             Flexemu.ad boot flexemu.astyle flexemu.xpm \
	     flexemu1.xpm flexemu2.xpm flexemu3.xpm flexcpu.xpm \
	     JoystickTable.gnumeric \
	     flexdisk.rc wwinres.rc winres.rc fsetup.rc \
	     flexemu.vcxproj flexplorer.vcxproj fsetup.vcxproj

SUBDIRS = bitmaps tools

bin_PROGRAMS   = flexemu @FLEXPLORER@ @FSETUP@
EXTRA_PROGRAMS = flexplorer fsetup
noinst_PROGRAMS = test

flexemu_CXXFLAGS    = --std=c++11 -Wall -Wextra -Wpedantic -Wno-long-long \
			-DF_DATADIR=\"$(pkgdatadir)\" @CXXSPEED@ \
			@OS_TYPE@ @X_CFLAGS@ @FLEXEMU_CXXADD@
flexplorer_CXXFLAGS = --std=c++11 -Wall -Wextra -Wpedantic -Wno-long-long \
                        -DF_DATADIR=\"$(pkgdatadir)\" \
			@OS_TYPE@ @WX_CFLAGS@
fsetup_CXXFLAGS     = --std=c++11 -Wall -Wextra -Wpedantic -Wno-long-long \
                        -DF_DATADIR=\"$(pkgdatadir)\" \
			@OS_TYPE@ @WX_CFLAGS@
test_CXXFLAGS       = --std=c++11 -Wall -Wextra -Wpedantic -Wno-long-long \
		        @OS_TYPE@ -DUNIT_TEST

flexplorer_LDADD = @WX_LIBS@ @PTHREAD_LD@
fsetup_LDADD     = @WX_LIBS@ @PTHREAD_LD@
flexemu_LDADD    = @FLEX_RC@ @FLEXEMU_LDADD@ @X_PRE_LIBS@ @X_LIBS@ @X_EXTRA_LIBS@ @WIN32_LIBS@ @PTHREAD_LD@
test_LDADD = @PTHREAD_LD@
flexemu_DEPENDENCIES = @FLEX_RC@

fsetup_SOURCES = \
fsetup.cpp fsetpdlg.cpp foptman.cpp brcfile.cpp benv.cpp misc1.cpp\
bfileptr.cpp bregistr.cpp\
fsetup.h fsetpdlg.h foptman.h brcfile.h benv.h misc1.h \
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
mc6809st.cpp cvtwchar.cpp joystick.cpp \
apprun.cpp keyboard.cpp bytereg.cpp drisel.cpp terminal.cpp \
sprinter.h \
absgui.h acia1.h bdate.h bdir.h bjoystck.h command.h \
da6809.h debug.h e2.h e2floppy.h vico1.h vico2.h engine.h fcinfo.h \
fcopyman.h fdirent.h ffilebuf.h ffilecnt.h flexerr.h inout.h \
iodevice.h mc146818.h mc6809.h mc6821.h mc6850.h memory.h misc1.h \
mmu.h ndircont.h pia1.h pia2.h rfilecnt.h typedefs.h wd1793.h win32gui.h \
xabsgui.h xtgui.h brcfile.h benv.h foptman.h bmembuf.h bregistr.h \
bcommand.h cacttrns.h csetfreq.h btime.h schedule.h cpustate.h btimer.h \
clogfile.h bfileptr.h ifilecnt.h ifilcnti.h iffilcnt.h bdelete.h bsingle.h \
memory.h resource.h filecntb.h filecnts.h mc6809st.h \
schedcpu.h absdisas.h cvtwchar.h flexemu.h scpulog.h sguiopts.h \
fileread.h memtgt.h joystick.h keyboard.h apprun.h winctxt.h terminal.h \
bytereg.h drisel.h

EXTRA_flexemu_SOURCES = wwinres.rc

flexplorer_SOURCES = \
bdate.cpp bdir.cpp contpdlg.cpp dircont.cpp fcinfo.cpp \
fclipbrd.cpp fcopyman.cpp fdcframe.cpp fddnd.cpp fdirent.cpp fdlist.cpp \
fdpframe.cpp ffilebuf.cpp ffilecnt.cpp flexdisk.cpp flexerr.cpp misc1.cpp \
rfilecnt.cpp optdlg.cpp brcfile.cpp benv.cpp cvtwchar.cpp \
bmembuf.cpp bfileptr.cpp \
ifilecnt.cpp idircnt.cpp iffilcnt.cpp fmenufac.cpp bregistr.cpp \
bprocess.cpp \
bdate.h bdir.h bobserv.h contpdlg.h dircont.h fcinfo.h \
fclipbrd.h fcopyman.h fdcframe.h fddnd.h fdirent.h fdlist.h \
fdpframe.h ffilebuf.h ffilecnt.h flexdisk.h flexerr.h misc1.h \
rfilecnt.h brcfile.h benv.h bsingle.h bdelete.h fmenufac.h \
bmembuf.h \
bfileptr.h ifilecnt.h ifilcnti.h idircnt.h iffilcnt.h \
filecntb.h filecont.h optdlg.h bregistr.h bprocess.h cvtwchar.h \
sprinter.h flexemu.h

test_SOURCES = \
testmain.cpp test_gccasm.cpp bdate.cpp colors.cpp \
command.cpp bfileptr.cpp da6809.cpp fcinfo.cpp \
flexerr.cpp inout.cpp \
memory.cpp mc6809.cpp mc6809in.cpp misc1.cpp \
bcommand.cpp btime.cpp schedule.cpp \
btimer.cpp clogfile.cpp  \
mc6809st.cpp \
test_gccasm.h bdate.h \
command.h bfileptr.h da6809.h debug.h e2.h engine.h fcinfo.h \
flexerr.h inout.h \
memory.h mc6809.h misc1.h \
typedefs.h \
csetfreq.h btime.h schedule.h cpustate.h btimer.h \
clogfile.h bdelete.h bsingle.h \
mc6809st.h \
schedcpu.h sprinter.h

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

