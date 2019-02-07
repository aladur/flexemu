pkgdata_DATA = boot flexemu.xpm  \
		flexemu1.xpm flexemu2.xpm flexemu3.xpm flexcpu.xpm
sysconf_DATA = flexemu.conf

SUFFIXES = .rc .o
.rc.o:
	@WINDRES_EXE@ -o $@ -O coff -i $<

EXTRA_DIST = fdtest.cpp mc6809ex.cpp engine.cpp confignt.h \
             Flexemu.ad boot flexemu.astyle flexemu.xpm \
	     flexemu1.xpm flexemu2.xpm flexemu3.xpm flexcpu.xpm \
	     JoystickTable.gnumeric \
	     flexdisk.rc wwinres.rc winres.rc fsetup.rc \
	     flexemu.vcxproj flexplorer.vcxproj fsetup.vcxproj \
	     flexemu.conf

SUBDIRS = bitmaps tools

bin_PROGRAMS   = flexemu @FLEXPLORER@ @FSETUP@ mdcrtool
EXTRA_PROGRAMS = flexplorer fsetup mdcrtool
noinst_PROGRAMS = test

AM_CXXFLAGS = -DF_DATADIR=\"$(pkgdatadir)\" -DF_SYSCONFDIR=\"$(sysconfdir)\"

flexemu_CXXFLAGS    = --std=c++11 -Wall -Wextra -Wpedantic -Wno-long-long \
		        $(AM_CXXFLAGS) \
			@CXXSPEED@ @OS_TYPE@ @X_CFLAGS@ @FLEXEMU_CXXADD@
flexplorer_CXXFLAGS = --std=c++11 -Wall -Wextra -Wpedantic -Wno-long-long \
		        $(AM_CXXFLAGS) \
			@OS_TYPE@ @WX_CFLAGS@
fsetup_CXXFLAGS     = --std=c++11 -Wall -Wextra -Wpedantic -Wno-long-long \
		        $(AM_CXXFLAGS) \
			@OS_TYPE@ @WX_CFLAGS@
mdcrtool_CXXFLAGS   = --std=c++11 -Wall -Wextra -Wpedantic -Wno-long-long \
		        $(AM_CXXFLAGS) \
			@OS_TYPE@ @WX_CFLAGS@
test_CXXFLAGS       = --std=c++11 -Wall -Wextra -Wpedantic -Wno-long-long \
		        $(AM_CXXFLAGS) \
		        @OS_TYPE@ -DUNIT_TEST

flexplorer_LDADD = @WX_LIBS@ @PTHREAD_LD@
fsetup_LDADD     = @WX_LIBS@ @PTHREAD_LD@
mdcrtool_LDADD   = @PTHREAD_LD@
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
tsl/robin_growth_policy.h tsl/robin_hash.h tsl/robin_map.h tsl/robin_set.h

flexplorer_SOURCES = \
bdate.cpp bdir.cpp contpdlg.cpp dircont.cpp fcinfo.cpp \
fcopyman.cpp fdcframe.cpp fddnd.cpp fdirent.cpp fdlist.cpp \
fdpframe.cpp ffilebuf.cpp ffilecnt.cpp flexdisk.cpp flexerr.cpp misc1.cpp \
rfilecnt.cpp optdlg.cpp brcfile.cpp benv.cpp cvtwchar.cpp \
bmembuf.cpp bfileptr.cpp \
ifilecnt.cpp idircnt.cpp iffilcnt.cpp fmenufac.cpp bregistr.cpp \
bprocess.cpp mdcrtape.cpp bobservd.cpp \
bdate.h bdir.h bobserv.h bobservd.h bobshelp.h contpdlg.h dircont.h fcinfo.h \
fcopyman.h fdcframe.h fddnd.h fdirent.h fdlist.h \
fdpframe.h ffilebuf.h ffilecnt.h flexdisk.h flexerr.h misc1.h \
rfilecnt.h brcfile.h benv.h fmenufac.h \
bmembuf.h \
bfileptr.h ifilecnt.h ifilcnti.h idircnt.h iffilcnt.h \
filecntb.h filecont.h optdlg.h bregistr.h bprocess.h cvtwchar.h \
sprinter.h flexemu.h mdcrtape.h

mdcrtool_SOURCES = \
mdcrtool.cpp mdcrtape.cpp mdcrfs.cpp fileread.cpp bfileptr.cpp bmembuf.cpp \
flexerr.cpp \
mdcrtape.h mdcrfs.h fileread.h memsrc.h memtgt.h bfileptr.h bmembuf.h \
typedefs.h misc1.h config.h flexerr.h

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

