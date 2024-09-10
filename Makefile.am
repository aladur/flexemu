ACLOCAL_AMFLAGS = -I m4

noinst_DATA = metadata.json

EXTRA_DIST = \
	AUTHORS \
	ChangeLog \
	COPYING \
	flexemu.sln \
	INSTALL \
	INSTALL.Win32 \
	INSTALL.unix \
	LICENSE \
	NEWS \
	README \
	README.md \
	TODO \
	.appveyor.yml \
	.cirrus.yml \
	.gdbinit \
	.gdbinit-context \
	.gitignore \
	.clang-tidy \
	options.txt

SUBDIRS = \
	src \
	disks \
	doc \
	monitor \
	build \
	python \
	test \
	.circleci \
	.github

AUTOMAKE_OPTIONS = dist-bzip2 no-dist-gzip

AM_DISTCHECK_CONFIGURE_FLAGS = --disable-post-install

metadata.json: src/updatemd .clang-tidy
	@set +x; \
	if $(AM_V_P); then set -x; else echo "  GENMD    $@"; fi; \
	src/updatemd -c $(top_srcdir)/.clang-tidy -f $(top_srcdir)/metadata.json

clang-tidy-check:
	cd src; $(MAKE) clang-tidy-check

EXTRA_DIST += \
	$(top_srcdir)/jsoncpp/include/json/allocator.h \
	$(top_srcdir)/jsoncpp/include/json/assertions.h \
	$(top_srcdir)/jsoncpp/include/json/config.h \
	$(top_srcdir)/jsoncpp/include/json/forwards.h \
	$(top_srcdir)/jsoncpp/include/json/json_features.h \
	$(top_srcdir)/jsoncpp/include/json/json.h \
	$(top_srcdir)/jsoncpp/include/json/reader.h \
	$(top_srcdir)/jsoncpp/include/json/value.h \
	$(top_srcdir)/jsoncpp/include/json/version.h \
	$(top_srcdir)/jsoncpp/include/json/writer.h \
	$(top_srcdir)/jsoncpp/src/lib_json/json_reader.cpp \
	$(top_srcdir)/jsoncpp/src/lib_json/json_tool.h \
	$(top_srcdir)/jsoncpp/src/lib_json/json_value.cpp \
	$(top_srcdir)/jsoncpp/src/lib_json/json_valueiterator.inl \
	$(top_srcdir)/jsoncpp/src/lib_json/json_writer.cpp

