pkgdata_DATA = Flexemu.ad boot flexemu.xpm  \
		flexemu1.xpm flexemu2.xpm flexemu3.xpm flexcpu.xpm

SUFFIXES = .rc .o
.rc.o:
	@WINDRES_EXE@ -o $@ -O coff -i $<

EXTRA_DIST = fdtest.cpp mc6809ex.cpp engine.cpp confignt.h \
             absdisas.h buint.h buint.cpp optdlg.h \
             Flexemu.ad boot flexemu.xpm \
	     flexemu1.xpm flexemu2.xpm flexemu3.xpm flexcpu.xpm \
	     JoystickTable.gnumeric \
	     flexdisk.rc wwinres.rc winres.rc fsetup.rc \
	     flexemu.dsp flexdisk.dsp fsetup.dsp \
	     remove.bat

SUBDIRS = bitmaps tools

bin_PROGRAMS   = flexemu @FLEXPLORER@ @FSETUP@
EXTRA_PROGRAMS = flexplorer fsetup

flexemu_CXXFLAGS    = -g -Wall -pedantic -Wno-long-long \
			-DF_DATADIR=\"$(pkgdatadir)\" @CXXSPEED@ \
			@OS_TYPE@ @X_CFLAGS@ @FLEXEMU_CXXADD@
flexplorer_CXXFLAGS = -g -Wall -pedantic -Wno-long-long \
                        -DF_DATADIR=\"$(pkgdatadir)\" \
			@OS_TYPE@ @WX_CFLAGS@
fsetup_CXXFLAGS     = -g -Wall -pedantic -Wno-long-long \
                        -DF_DATADIR=\"$(pkgdatadir)\" \
			@OS_TYPE@ @WX_CFLAGS@

flexplorer_LDADD = @WX_LIBS@ @PTHREAD_LD@
fsetup_LDADD     = @WX_LIBS@ @PTHREAD_LD@
flexemu_LDADD    = @FLEX_RC@ @FLEXEMU_LDADD@ @PTHREAD_LD@ @X_PRE_LIBS@ @X_LIBS@ @X_EXTRA_LIBS@ @WIN32_LIBS@
flexemu_DEPENDENCIES = @FLEX_RC@

fsetup_SOURCES = \
fsetup.cpp fsetpdlg.cpp foptman.cpp brcfile.cpp benv.cpp bstring.cpp misc1.cpp\
bfileptr.cpp bregistr.cpp \
fsetup.h fsetpdlg.h foptman.h brcfile.h benv.h bstring.h misc1.h \
bfileptr.h bregistr.h

flexemu_SOURCES = \
absgui.cpp acia1.cpp bdate.cpp bdir.cpp bjoystck.cpp bstring.cpp colors.cpp \
command.cpp da6809.cpp e2floppy.cpp e2video.cpp fcinfo.cpp \
fcopyman.cpp fdirent.cpp ffilebuf.cpp ffilecnt.cpp flexerr.cpp inout.cpp \
memory.cpp main.cpp mc146818.cpp mc6809.cpp mc6809in.cpp \
mc6821.cpp mc6850.cpp misc1.cpp mmu.cpp ndircont.cpp pia1.cpp \
pia2.cpp rfilecnt.cpp wd1793.cpp win32gui.cpp xabsgui.cpp xres.cpp \
xtgui.cpp brcfile.cpp benv.cpp foptman.cpp bmembuf.cpp bregistr.cpp\
bthread.cpp bthreadfactory.cpp bthreadimp.cpp bposixthreadimp.cpp \
bmutex.cpp bcommand.cpp cacttrns.cpp csetfreq.cpp btime.cpp schedule.cpp \
btimer.cpp clogfile.cpp bfileptr.cpp ifilecnt.cpp iffilcnt.cpp \
bwin32threadimp.cpp mc6809st.cpp \
absgui.h acia1.h bdate.h bdir.h bjoystck.h bstring.h command.h \
da6809.h debug.h e2.h e2floppy.h e2video.h engine.h fcinfo.h \
fcopyman.h fdirent.h ffilebuf.h ffilecnt.h flexerr.h inout.h \
intmem.h iodevice.h mc146818.h mc6809.h mc6821.h mc6850.h memory.h misc1.h \
mmu.h ndircont.h pia1.h pia2.h rfilecnt.h typedefs.h wd1793.h win32gui.h \
xabsgui.h xtgui.h brcfile.h benv.h foptman.h bmembuf.h bregistr.h da.h \
bthread.h bthreadfactory.h bthreadimp.h bposixthreadimp.h bmutex.h \
bcommand.h cacttrns.h csetfreq.h btime.h schedule.h cpustate.h btimer.h \
clogfile.h bfileptr.h ifilecnt.h ifilcnti.h iffilcnt.h bdelete.h bsingle.h \
memory.h bwin32threadimp.h resource.h filecntb.h filecnts.h mc6809st.h \
schedcpu.h flexemu.h

EXTRA_flexemu_SOURCES = wwinres.rc

flexplorer_SOURCES = \
bdate.cpp bdir.cpp bstring.cpp contpdlg.cpp dircont.cpp fcinfo.cpp \
fclipbrd.cpp fcopyman.cpp fdcframe.cpp fddnd.cpp fdirent.cpp fdlist.cpp \
fdpframe.cpp ffilebuf.cpp ffilecnt.cpp flexdisk.cpp flexerr.cpp misc1.cpp \
rfilecnt.cpp optdlg.cpp brcfile.cpp benv.cpp\
bintervl.cpp blist.cpp baddrrng.cpp bhashtbl.cpp bident.cpp bsortlst.cpp\
bmembuf.cpp bbintree.cpp da.cpp da6809.cpp disconf.cpp bfileptr.cpp \
ifilecnt.cpp idircnt.cpp iffilcnt.cpp fmenufac.cpp bregistr.cpp \
bprocess.cpp \
bdate.h bdir.h bobserv.h bstring.h contpdlg.h dircont.h fcinfo.h \
fclipbrd.h fcopyman.h fdcframe.h fddnd.h fdirent.h fdlist.h \
fdpframe.h ffilebuf.h ffilecnt.h flexdisk.h flexerr.h misc1.h \
rfilecnt.h brcfile.h benv.h bsingle.h bdelete.h fmenufac.h \
bintervl.h blist.h baddrrng.h bhashtbl.h bident.h bsortlst.h bmembuf.h\
bbintree.h disconf.h bfileptr.h ifilecnt.h ifilcnti.h idircnt.h iffilcnt.h \
filecntb.h filecont.h bregistr.h bprocess.h flexemu.h

