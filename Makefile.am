ACLOCAL_AMFLAGS = -I m4

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
	.gitignore

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

clang-tidy-check:
	cd src; $(MAKE) clang-tidy-check

