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
	test

# Alternative handling of subdirectories.
# This reduces the entering/leaving of subdirectories.
# For details see:
# https://www.gnu.org/software/automake/manual/html_node/Alternative.html
EXTRA_DIST += \
	.github/workflows/build-and-test.yml \
	.github/workflows/clang-tidy-check.yml \
	.github/workflows/distcheck.yml \
	.github/workflows/macos-build-and-test.yml \
	.github/workflows/windows-build-and-test.yml

EXTRA_DIST += \
	build/download_fug_dsk_files.sh \
	build/fug.ico \
	build/windows/change_version.sh \
	build/windows/create_fug_dsk_files_installer.sh \
	build/windows/create_installer.sh \
	build/windows/download_and_install_envar_plugin.sh \
	build/windows/download_and_rebuild_libs.sh \
	build/windows/rebuild_flexemu_all_and_copy_libs.sh \
	build/windows/setQtProperties.ps1 \
	build/windows/installer/Flexemu.nsi \
	build/windows/installer/GetParameters.nsh \
	build/windows/installer/GetParameterValue.nsh \
	build/windows/installer/HistoricFLEXFiles.nsi \
	build/windows/installer/SplitFirstStrPart.nsh \
	build/windows/installer/StrStr.nsh

EXTRA_DIST += \
	docker/almalinux/Dockerfile \
	docker/alpine/Dockerfile \
	docker/altlinux/Dockerfile \
	docker/amazonlinux/Dockerfile \
	docker/archlinux/Dockerfile \
	docker/biglinux/Dockerfile \
	docker/cachyos/Dockerfile \
	docker/centos/Dockerfile \
	docker/debian_be/Dockerfile \
	docker/debian/Dockerfile \
	docker/debian_i386/Dockerfile \
	docker/debian_qt6/Dockerfile \
	docker/debian_testing/Dockerfile \
	docker/debian_testing_qt6/Dockerfile \
	docker/debian_unstable/Dockerfile \
	docker/debian_unstable_qt6/Dockerfile \
	docker/endlessos/Dockerfile \
	docker/fedora/Dockerfile \
	docker/kalilinux/Dockerfile \
	docker/linuxmint/Dockerfile \
	docker/mageia/Dockerfile \
	docker/manjaro/Dockerfile \
	docker/opensuse/Dockerfile \
	docker/oraclelinux/Dockerfile \
	docker/photon/Dockerfile \
	docker/rockylinux/Dockerfile \
	docker/slackware/Dockerfile \
	docker/slackware/setup.sh \
	docker/ubuntu/Dockerfile \
	docker/voidlinux/Dockerfile \
	docker/dockerRunForwardX11.sh \
	docker/readme.md \
	docker/statistics.md

EXTRA_DIST += \
	python/flex.py \
	python/flexemu_common.py \
	python/s_dir_entry_Printer.py \
	python/s_link_table_Printer.py \
	python/s_new_file_Printer.py \
	python/s_sys_info_record_Printer.py \
	python/s_sys_info_sector_Printer.py \
	python/st_t_Printer.py

EXTRA_DIST += \
	test/data/cat.cmd \
	test/data/cat.hex \
	test/data/cat.s19 \
	test/data/testdisk.dsk \
	test/data/testdisk.flx

AUTOMAKE_OPTIONS = dist-bzip2 no-dist-gzip

AM_DISTCHECK_CONFIGURE_FLAGS = --disable-post-install

metadata.json: src/updatemd .clang-tidy
	@set +x; \
	cd $(srcdir) > /dev/null; \
	if $(AM_V_P); then set -x; else echo "  GENMD    $@"; fi; \
	$(abs_builddir)/src/updatemd -c .clang-tidy -f $@; \
	cd - > /dev/null

clang-tidy-check:
	cd src; $(MAKE) clang-tidy-check

gtest_top = googletest/googletest
gmock_top = googletest/googlemock

# To support 'make distcheck' the following googletest files have to be added
# to the distribution.
EXTRA_DIST += \
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

# Same for jsoncpp files.
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

