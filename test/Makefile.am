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

# Instructions to make test executable.
#
check_PROGRAMS = unittests
noinst_PROGRAMS = unittests test_gccasm

unittests_SOURCES = \
	test_benv.cpp \
	test_binifile.cpp \
	test_brcfile.cpp \
	test_cistring.cpp \
	test_da6809.cpp \
	test_main.cpp \
	test_mc6809lg.cpp \
	test_misc1.cpp \
	test_ffilebuf.cpp \
	test_fileread.cpp \
	test_fdirent.cpp \
	test_bdate.cpp \
	test_boption.cpp \
	test_bintervl.cpp \
	test_bmembuf.cpp \
	test_breltime.cpp \
	test_btime.cpp \
	../src/bdate.cpp \
	../src/binifile.cpp \
	../src/benv.cpp \
	../src/bmembuf.cpp \
	../src/brcfile.cpp \
	../src/breltime.cpp \
	../src/btime.cpp \
	../src/da6809.cpp \
	../src/fattrib.cpp \
	../src/fdirent.cpp \
	../src/ffilebuf.cpp \
	../src/fileread.cpp \
	../src/flexerr.cpp \
	../src/mc6809lg.cpp \
	../src/mc6809st.cpp \
	../src/misc1.cpp \
	../src/bdate.h \
	../src/benv.h \
	../src/binifile.h \
	../src/bintervl.h \
	../src/boption.h \
	../src/breltime.h \
	../src/btime.h \
	../src/cistring.h \
	../src/da6809.h \
	../src/fattrib.h \
	../src/fdirent.h \
	../src/ffilebuf.h \
	../src/fileread.h \
	../src/flexerr.h \
	../src/mc6809lg.h \
	../src/mc6809st.h \
	../src/misc1.h \
	../src/scpulog.h

unittests_LDADD = libgtest.la libgmock.la ../src/libfmt.a
unittests_LDFLAGS = -pthread
unittests_CPPFLAGS = \
	-std=c++17 \
	-DF_DATADIR=\"$(pkgdatadir)\" \
	-DF_SYSCONFDIR=\"$(sysconfdir)\" \
	$(OS_TYPE) \
	-isystem$(top_srcdir)/$(gtest_top)/include \
	-isystem$(top_srcdir)/$(gmock_top)/include \
	-isystem$(top_srcdir)/fmt/include \
	-I$(top_srcdir)/src

TESTS = unittests

TESTS_ENVIRONMENT = export GTEST_OUTPUT="xml";

SUBDIRS = \
	data

test_gccasm_SOURCES = \
	../src/clogfile.cpp  \
	../src/colors.cpp \
	../src/command.cpp \
	../src/da6809.cpp \
	../src/foptman.cpp \
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
	../src/flexerr.h \
	../src/foptman.h \
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
	-std=c++17 \
	-DF_DATADIR=\"$(pkgdatadir)\" \
	-DF_SYSCONFDIR=\"$(sysconfdir)\" \
	$(OS_TYPE) \
	-isystem$(top_srcdir)/$(gtest_top)/include \
	-isystem$(top_srcdir)/$(gmock_top)/include \
	-isystem$(top_srcdir)/fmt/include \
	-I$(top_srcdir)/src
	-include ../src/pch.h

# To support 'make distcheck' the following googletest files have to be added
# to the distribution.
EXTRA_DIST = \
	$(top_srcdir)/$(gtest_top)/src/gtest.cc \
	$(top_srcdir)/$(gtest_top)/src/gtest-all.cc \
	$(top_srcdir)/$(gtest_top)/src/gtest-assertion-result.cc \
	$(top_srcdir)/$(gtest_top)/src/gtest-death-test.cc \
	$(top_srcdir)/$(gtest_top)/src/gtest-filepath.cc \
	$(top_srcdir)/$(gtest_top)/src/gtest-internal-inl.h \
	$(top_srcdir)/$(gtest_top)/src/gtest_main.cc \
	$(top_srcdir)/$(gtest_top)/src/gtest-matchers.cc \
	$(top_srcdir)/$(gtest_top)/src/gtest-port.cc \
	$(top_srcdir)/$(gtest_top)/src/gtest-printers.cc \
	$(top_srcdir)/$(gtest_top)/src/gtest-test-part.cc \
	$(top_srcdir)/$(gtest_top)/src/gtest-typed-test.cc \
	$(top_srcdir)/$(gtest_top)/include/gtest/gtest.h \
	$(top_srcdir)/$(gtest_top)/include/gtest/gtest-assertion-result.h \
	$(top_srcdir)/$(gtest_top)/include/gtest/gtest-death-test.h \
	$(top_srcdir)/$(gtest_top)/include/gtest/gtest-matchers.h \
	$(top_srcdir)/$(gtest_top)/include/gtest/gtest-message.h \
	$(top_srcdir)/$(gtest_top)/include/gtest/gtest-param-test.h \
	$(top_srcdir)/$(gtest_top)/include/gtest/gtest_pred_impl.h \
	$(top_srcdir)/$(gtest_top)/include/gtest/gtest-printers.h \
	$(top_srcdir)/$(gtest_top)/include/gtest/gtest_prod.h \
	$(top_srcdir)/$(gtest_top)/include/gtest/gtest-spi.h \
	$(top_srcdir)/$(gtest_top)/include/gtest/gtest-test-part.h \
	$(top_srcdir)/$(gtest_top)/include/gtest/gtest-typed-test.h \
	$(top_srcdir)/$(gtest_top)/include/gtest/internal/gtest-death-test-internal.h \
	$(top_srcdir)/$(gtest_top)/include/gtest/internal/gtest-filepath.h \
	$(top_srcdir)/$(gtest_top)/include/gtest/internal/gtest-internal.h \
	$(top_srcdir)/$(gtest_top)/include/gtest/internal/gtest-param-util.h \
	$(top_srcdir)/$(gtest_top)/include/gtest/internal/gtest-port-arch.h \
	$(top_srcdir)/$(gtest_top)/include/gtest/internal/gtest-port.h \
	$(top_srcdir)/$(gtest_top)/include/gtest/internal/gtest-string.h \
	$(top_srcdir)/$(gtest_top)/include/gtest/internal/gtest-type-util.h \
	$(top_srcdir)/$(gtest_top)/include/gtest/internal/custom/gtest.h \
	$(top_srcdir)/$(gtest_top)/include/gtest/internal/custom/gtest-port.h \
	$(top_srcdir)/$(gtest_top)/include/gtest/internal/custom/gtest-printers.h
# Same for googlemock files.
EXTRA_DIST += \
	$(top_srcdir)/$(gmock_top)/src/gmock.cc \
	$(top_srcdir)/$(gmock_top)/src/gmock-all.cc \
	$(top_srcdir)/$(gmock_top)/src/gmock-cardinalities.cc \
	$(top_srcdir)/$(gmock_top)/src/gmock-internal-utils.cc \
	$(top_srcdir)/$(gmock_top)/src/gmock_main.cc \
	$(top_srcdir)/$(gmock_top)/src/gmock-matchers.cc \
	$(top_srcdir)/$(gmock_top)/src/gmock-spec-builders.cc \
	$(top_srcdir)/$(gmock_top)/include/gmock/gmock.h \
	$(top_srcdir)/$(gmock_top)/include/gmock/gmock-actions.h \
	$(top_srcdir)/$(gmock_top)/include/gmock/gmock-cardinalities.h \
	$(top_srcdir)/$(gmock_top)/include/gmock/gmock-function-mocker.h \
	$(top_srcdir)/$(gmock_top)/include/gmock/gmock-matchers.h \
	$(top_srcdir)/$(gmock_top)/include/gmock/gmock-more-actions.h \
	$(top_srcdir)/$(gmock_top)/include/gmock/gmock-more-matchers.h \
	$(top_srcdir)/$(gmock_top)/include/gmock/gmock-nice-strict.h \
	$(top_srcdir)/$(gmock_top)/include/gmock/gmock-spec-builders.h \
	$(top_srcdir)/$(gmock_top)/include/gmock/internal/gmock-internal-utils.h \
	$(top_srcdir)/$(gmock_top)/include/gmock/internal/gmock-port.h \
	$(top_srcdir)/$(gmock_top)/include/gmock/internal/gmock-pp.h \
	$(top_srcdir)/$(gmock_top)/include/gmock/internal/custom/gmock-generated-actions.h \
	$(top_srcdir)/$(gmock_top)/include/gmock/internal/custom/gmock-matchers.h \
	$(top_srcdir)/$(gmock_top)/include/gmock/internal/custom/gmock-port.h

# Same for fmt files.
EXTRA_DIST += \
	$(top_srcdir)/fmt/src/fmt.cc \
	$(top_srcdir)/fmt/src/format.cc \
	$(top_srcdir)/fmt/src/os.cc \
	$(top_srcdir)/fmt/include/fmt/args.h \
	$(top_srcdir)/fmt/include/fmt/base.h \
	$(top_srcdir)/fmt/include/fmt/chrono.h \
	$(top_srcdir)/fmt/include/fmt/color.h \
	$(top_srcdir)/fmt/include/fmt/compile.h \
	$(top_srcdir)/fmt/include/fmt/core.h \
	$(top_srcdir)/fmt/include/fmt/format.h \
	$(top_srcdir)/fmt/include/fmt/format-inl.h \
	$(top_srcdir)/fmt/include/fmt/os.h \
	$(top_srcdir)/fmt/include/fmt/ostream.h \
	$(top_srcdir)/fmt/include/fmt/printf.h \
	$(top_srcdir)/fmt/include/fmt/ranges.h \
	$(top_srcdir)/fmt/include/fmt/std.h \
	$(top_srcdir)/fmt/include/fmt/xchar.h

clean-local:
	rm -rf test_detail.xml

