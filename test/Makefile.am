# Instructions to make gtest library.
#
gtest_top = googletest/googletest
check_LTLIBRARIES = libgtest.la
libgtest_la_SOURCES = $(top_srcdir)/$(gtest_top)/src/gtest-all.cc
libgtest_la_CPPFLAGS = \
	-std=c++14 \
	-I$(top_srcdir)/$(gtest_top)/include \
	-I$(top_srcdir)/$(gtest_top)

libgtest_la_LDFLAGS = -pthread

# Instructions to make test executable.
#
check_PROGRAMS = unittests
noinst_PROGRAMS = unittests

unittests_SOURCES = \
	test_misc1.cpp \
	../src/benv.cpp \
	../src/bfileptr.cpp \
	../src/flexerr.cpp \
	../src/misc1.cpp \
	../src/benv.h \
	../src/bfileptr.h \
	../src/flexerr.h
	../src/misc1.h

unittests_LDADD = libgtest.la
unittests_LDFLAGS = -pthread
unittests_CPPFLAGS = \
	-std=c++17 \
	-DF_DATADIR=\"$(pkgdatadir)\" \
	-DF_SYSCONFDIR=\"$(sysconfdir)\" \
	$(OS_TYPE) \
	-I$(top_srcdir)/$(gtest_top)/include \
	-I$(top_srcdir)/src

TESTS = unittests

TESTS_ENVIRONMENT = export GTEST_OUTPUT="xml";

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

clean-local:
	rm -rf test_detail.xml

