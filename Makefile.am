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
	.github/workflows/windows-build-and-test.yml \
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
	build/windows/installer/StrStr.nsh \
	python/flex.py \
	python/flexemu_common.py \
	python/s_dir_entry_Printer.py \
	python/s_link_table_Printer.py \
	python/s_new_file_Printer.py \
	python/s_sys_info_record_Printer.py \
	python/s_sys_info_sector_Printer.py \
	python/st_t_Printer.py \
	test/data/cat.cmd \
	test/data/cat.hex \
	test/data/cat.s19 \
	test/data/testdisk.dsk \
	test/data/testdisk.flx

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

