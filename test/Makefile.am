# Instructions to make gtest library.
#
gtest_top = googletest/googletest
gmock_top = googletest/googlemock

check_LTLIBRARIES = libgtest.la libgmock.la

libgtest_la_SOURCES = $(top_srcdir)/$(gtest_top)/src/gtest-all.cc
libgtest_la_CPPFLAGS = \
	-std=c++14 \
	-isystem$(top_srcdir)/$(gtest_top)/include \
	-isystem$(top_srcdir)/$(gtest_top)
libgtest_la_LDFLAGS = -pthread

libgmock_la_SOURCES = $(top_srcdir)/$(gmock_top)/src/gmock-all.cc
libgmock_la_CPPFLAGS = \
	-std=c++14 \
	-isystem$(top_srcdir)/$(gtest_top)/include \
	-isystem$(top_srcdir)/$(gmock_top)/include \
	-isystem$(top_srcdir)/$(gmock_top)
libgmock_la_LDFLAGS = -pthread

# Instructions to make project targets.
#
SUFFIXES = .h.gch

BUILT_SOURCES = $(PCH_H_GCH)

DIRFLAGS = \
	-DF_DATADIR=\"$(pkgdatadir)\" \
	-DF_SYSCONFDIR=\"$(sysconfdir)\"

ALL_TARGETS_CXXFLAGS = \
	--std=c++17 -pipe -D_REENTRANT -pthread -fPIC \
	$(OS_TYPE) $(DIRFLAGS) $(MACOSX_VERSION_MIN)

EXTRA_DIST = \
	pch.h \
	pch.cpp \
	options.txt

# Instructions to make test executable.
#
check_PROGRAMS = unittests
noinst_PROGRAMS = unittests test_gccasm

noinst_DATA = metadata.json

unittests_SOURCES = \
	test_benv.cpp \
	test_binifile.cpp \
	test_blinxsys.cpp \
	test_bobserv.cpp \
	test_brcfile.cpp \
	test_cistring.cpp \
	test_colors.cpp \
	test_da6809.cpp \
	test_main.cpp \
	test_mc6809lg.cpp \
	test_misc1.cpp \
	test_fcnffile.cpp \
	test_fcinfo.cpp \
	test_ffilebuf.cpp \
	test_ffilecnts.cpp \
	test_filecntb.cpp \
	test_filecnts.cpp \
	test_filecont.cpp \
	test_fileread.cpp \
	test_filesystem.cpp \
	test_filfschk.cpp \
	test_fdirent.cpp \
	test_bdate.cpp \
	test_bdir.cpp \
	test_bintervl.cpp \
	test_bmembuf.cpp \
	test_breltime.cpp \
	test_btime.cpp \
	test_rndcheck.cpp \
	../src/blinxsys.cpp \
	../src/colors.cpp \
	../src/da6809.cpp \
	../src/fdoptman.cpp \
	../src/flblfile.cpp \
	../src/fversion.cpp \
	../src/mc6809lg.cpp \
	../src/mc6809st.cpp \
	../src/ndircont.cpp \
	../src/rndcheck.cpp \
	../src/bdate.h \
	../src/bdir.h \
	../src/benv.h \
	../src/binifile.h \
	../src/blinxsys.h \
	../src/bintervl.h \
	../src/bobserv.h \
	../src/bobservd.h \
	../src/bobshelp.h \
	../src/breltime.h \
	../src/btime.h \
	../src/cistring.h \
	../src/colors.h \
	../src/da6809.h \
	../src/dircont.h \
	../src/fattrib.h \
	../src/fcinfo.h \
	../src/fcnffile.h \
	../src/fcopyman.h \
	../src/fdirent.h \
	../src/fdoptman.h \
	../src/ffilebuf.h \
	../src/ffilecnt.h \
	../src/filecnts.h \
	../src/fileread.h \
	../src/filfschk.h \
	fixt_debugout.h \
	fixt_filecont.h \
	../src/flblfile.h \
	../src/flexerr.h \
	../src/fversion.h \
	../src/idircnt.h \
	../src/iffilcnt.h \
	../src/ifilcnti.h \
	../src/ifilecnt.h \
	../src/mc6809lg.h \
	../src/mc6809st.h \
	../src/misc1.h \
	../src/ndircont.h \
	../src/rfilecnt.h \
	../src/rndcheck.h \
	../src/scpulog.h

unittests_LDADD = libgtest.la libgmock.la ../src/libflex.a ../src/libfmt.a
unittests_LDFLAGS = -pthread
unittests_CPPFLAGS = \
	$(ALL_TARGETS_CXXFLAGS) \
	-isystem$(top_srcdir)/$(gtest_top)/include \
	-isystem$(top_srcdir)/$(gmock_top)/include \
	-isystem$(top_srcdir)/fmt/include \
	-I$(top_srcdir)/test \
	$(INCLUDE_PCH) \
	-I$(top_srcdir)/src

TESTS = unittests

TESTS_ENVIRONMENT = export GTEST_OUTPUT="xml";

test_gccasm_SOURCES = \
	../src/clogfile.cpp  \
	../src/colors.cpp \
	../src/command.cpp \
	../src/da6809.cpp \
	../src/flblfile.cpp \
	../src/foptman.cpp \
	../src/fversion.cpp \
	../src/inout.cpp \
	../src/mc6809.cpp \
	../src/mc6809in.cpp \
	../src/mc6809lg.cpp \
	../src/mc6809st.cpp \
	../src/schedule.cpp \
	../src/soptions.cpp \
	test_gccasm.cpp \
	testmain.cpp \
	../src/bdate.h \
	../src/bobserv.h \
	../src/bobservd.h \
	../src/bobshelp.h \
	../src/breltime.h \
	../src/btime.h \
	../src/clogfile.h \
	../src/command.h \
	../src/config.h \
	../src/cpustate.h \
	../src/csetfreq.h \
	../src/da6809.h \
	../src/debug.h \
	../src/e2.h \
	../src/engine.h \
	../src/fcinfo.h \
	../src/flblfile.h \
	../src/flexerr.h \
	../src/foptman.h \
	../src/fversion.h \
	../src/inout.h \
	../src/mc6809.h \
	../src/mc6809lg.h \
	../src/mc6809st.h \
	../src/memory.h \
	../src/misc1.h \
	../src/schedcpu.h \
	../src/schedule.h \
	../src/soptions.h \
	test_gccasm.h \
	../src/typedefs.h \
	../src/warnoff.h \
	../src/warnon.h

test_gccasm_LDADD = $(PTHREAD_LD) ../src/libflex.a ../src/libfmt.a
test_gccasm_LDFLAGS = -pthread
test_gccasm_CXXFLAGS = \
	$(ALL_TARGETS_CXXFLAGS) \
	-isystem$(top_srcdir)/$(gtest_top)/include \
	-isystem$(top_srcdir)/$(gmock_top)/include \
	-isystem$(top_srcdir)/fmt/include \
	$(INCLUDE_PCH) \
	-I$(top_srcdir)/src

CLEANFILES = \
	pch.h.gch \
	test.log

# Rule for precompiled header:
# Build a pch.h.gch file from pch.h
pch_h = pch.h
pch_h_gch = $(pch_h:.h=.h.gch)

metadata.json: ../src/updatemd $(test_gccasm_SOURCES) $(unittests_SOURCES)
	@set +x; \
	cd $(srcdir) > /dev/null; \
	if $(AM_V_P); then set -x; else echo "  GENMD    $@"; fi; \
	$(abs_top_builddir)/src/updatemd -l -k test -f $@; \
	cd - > /dev/null

################################################################################
# Use traditional-style suffix rules for precompiled header support.
# Also define dependencies of precompiled header file.
.h.h.gch:
	@set +x; \
	if $(AM_V_P); then set -x; else echo "  CXXPCH   $@"; fi; \
	$(CXX) $(ALL_TARGETS_CXXFLAGS) $(CXXFLAGS) -I$(top_srcdir)/src \
	-o $@ -x c++-header -c $<

pch.h.gch: pch.h ../src/misc1.h

clean-local:
	rm -rf test_detail.xml

