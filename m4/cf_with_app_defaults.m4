dnl ---------------------------------------------------------------------------
dnl CF_WITH_APP_DEFAULTS version: 6 updated: 2015/01/02 09:05:50
dnl --------------------
dnl Handle configure option "--with-app-defaults", setting these shell
dnl variables:
dnl
dnl $APPSDIR is the option value, used for installing app-defaults files.
dnl $no_appsdir is a "#" (comment) if "--without-app-defaults" is given.
dnl
dnl Most Linux's use this:
dnl 	/usr/share/X11/app-defaults
dnl Debian uses this:
dnl 	/etc/X11/app-defaults
dnl DragonFlyBSD ports uses this:
dnl 	/usr/pkg/lib/X11/app-defaults
dnl FreeBSD ports use these:
dnl 	/usr/local/lib/X11/app-defaults
dnl 	/usr/local/share/X11/app-defaults
dnl Mandriva has these:
dnl 	/usr/lib/X11/app-defaults
dnl 	/usr/lib64/X11/app-defaults
dnl NetBSD has these
dnl 	/usr/X11R7/lib/X11/app-defaults
dnl OpenSolaris uses
dnl 	32-bit:
dnl 	/usr/X11/etc/X11/app-defaults
dnl 	/usr/X11/share/X11/app-defaults
dnl 	/usr/X11/lib/X11/app-defaults
dnl OSX uses
dnl		/opt/local/share/X11/app-defaults (MacPorts)
dnl		/opt/X11/share/X11/app-defaults (non-ports)
dnl	64-bit:
dnl 	/usr/X11/etc/X11/app-defaults
dnl 	/usr/X11/share/X11/app-defaults (I mkdir'd this)
dnl 	/usr/X11/lib/amd64/X11/app-defaults
dnl Solaris10 uses (in this order):
dnl 	/usr/openwin/lib/X11/app-defaults
dnl 	/usr/X11/lib/X11/app-defaults
AC_DEFUN([CF_WITH_APP_DEFAULTS],[
AC_MSG_CHECKING(for directory to install resource files)
AC_ARG_WITH(app-defaults,
	[  --with-app-defaults=DIR directory in which to install resource files (EPREFIX/lib/X11/app-defaults)],
	[APPSDIR=$withval],
	[APPSDIR='${exec_prefix}/lib/X11/app-defaults'])

if test "x[$]APPSDIR" = xauto
then
	APPSDIR='${exec_prefix}/lib/X11/app-defaults'
	for cf_path in \
		/opt/local/share/X11/app-defaults \
		/opt/X11/share/X11/app-defaults \
		/usr/share/X11/app-defaults \
		/usr/X11/share/X11/app-defaults \
		/usr/X11/lib/X11/app-defaults \
		/usr/lib/X11/app-defaults \
		/etc/X11/app-defaults \
		/usr/pkg/lib/X11/app-defaults \
		/usr/X11R7/lib/X11/app-defaults \
		/usr/X11R6/lib/X11/app-defaults \
		/usr/X11R5/lib/X11/app-defaults \
		/usr/X11R4/lib/X11/app-defaults \
		/usr/local/lib/X11/app-defaults \
		/usr/local/share/X11/app-defaults \
		/usr/lib64/X11/app-defaults
	do
		if test -d "$cf_path" ; then
			APPSDIR="$cf_path"
			break
		fi
	done
else
	cf_path=$APPSDIR
	CF_PATH_SYNTAX(cf_path)
fi

AC_MSG_RESULT($APPSDIR)
AC_SUBST(APPSDIR)

no_appsdir=
if test "$APPSDIR" = no
then
	no_appsdir="#"
else
	EXTRA_INSTALL_DIRS="$EXTRA_INSTALL_DIRS \$(APPSDIR)"
fi
AC_SUBST(no_appsdir)
])

