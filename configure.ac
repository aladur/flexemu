dnl Process this file with autoconf to produce a configure script.

AC_INIT([Flexemu],[3.12])
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR(m4)
AC_CANONICAL_TARGET
AC_ARG_PROGRAM dnl Perform program name transformation
AM_INIT_AUTOMAKE
LT_INIT
AM_MAINTAINER_MODE([disable])
AC_CONFIG_HEADERS([src/config.h:src/config.hin])

dnl Checks for programs.
case $host_os in
  *cygwin* ) CYGWIN=yes;;
         * ) CYGWIN=no;;
esac
case $host_os in
  *mingw32* ) MINGW32=yes;;
          * ) MINGW32=no;;
esac
AC_PROG_LN_S
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_MAKE_SET

dnl Checks for header files.
AC_PATH_XTRA
AC_HEADER_DIRENT
AC_PROG_EGREP

AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS_ONCE([sys/time.h])
AC_CHECK_HEADERS(stdarg.h)
AC_CHECK_HEADERS(unistd.h inttypes.h stdint.h)
AC_CHECK_HEADERS(fcntl.h sys/ioctl.h termios.h)
AC_CHECK_HEADERS(sys/statvfs.h sys/mount.h)
AC_CHECK_HEADER(windows.h, AC_DEFINE(WIN32,1,[1 if Windows headers are available]))
AC_CHECK_HEADER(linux/joystick.h, AC_DEFINE(LINUX_JOYSTICK_IS_PRESENT,1,[1 if Linux joystick headers are available]))
AC_CHECK_FUNCS(gettid)

dnl Checks for Win32 libraries.
FLX_CHECK_WIN32_LIBS
dnl Checks for pthread library
FLX_CHECK_PTHREAD_LIB
dnl Check for --enable-speed flag
FLX_ENABLE_SPEED
dnl Check for --disable-post-install flag
FLX_ENABLE_POST_INSTALL
dnl Check for -pedantic -Wall compiler switch
AX_CXXFLAGS_WARN_ALL
dnl Check for specific compiler warning support
CC_CHECK_FLAGS_APPEND([CFLAGS],[CFLAGS],[\
        -std=gnu99 \
        -Wall \
        -Wextra \
        -Wpedantic \
        -Wshadow \
])
AC_LANG_PUSH([C++])
CC_CHECK_FLAGS_APPEND([CXXFLAGS],[CXXFLAGS],[\
        -Wextra \
        -Wshadow \
        -Wswitch-enum \
        -Wuseless-cast \
        -Wfloat-conversion \
        -Wundef \
])
AC_LANG_POP([C++])

dnl check for qt libraries and tools
AX_HAVE_QT

dnl Checks for typedefs
AC_C_CONST
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_SIZE_T

dnl Checks for structures

dnl Checks for compiler characteristics
AC_C_BIGENDIAN
dnl Check if bitfields are big endian, set result in config.h
AC_LANG_PUSH([C++])
FLX_BITFIELDS_BIGENDIAN
AC_LANG_POP([C++])
AC_C_CHAR_UNSIGNED
AC_CHECK_SIZEOF(short)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(long long)
AC_CHECK_SIZEOF(void *)
AC_CHECK_DECLS([setenv, unsetenv])

dnl Checks for library functions
AC_FUNC_VPRINTF
AC_PROG_GCC_TRADITIONAL

dnl Check for system services
case $host_os in
       *mingw32*) FLEXEMU_LDADD="-mwindows" ;;
       *cygwin*)  FLEXEMU_LDADD="-mwindows" ;;
       *)         FLEXEMU_LDADD="" ;;
esac
case $host_os in
       *mingw32*) FLEX_RC='flexemu.${OBJEXT}' ;;
       *cygwin*)  FLEX_RC='flexemu.${OBJEXT}' ;;
       *)         FLEX_RC="" ;;
esac
case $host_os in
       *linux*)   OS_TYPE="-D__LINUX" ;;
       *freebsd*) OS_TYPE="-D__BSD -D__FREEBSD" ;;
       *openbsd*) OS_TYPE="-D__OPENBSD" ;;
       *mingw32*) OS_TYPE="-D__MINGW32 -DWIN32" ;;
       *cygwin*)  OS_TYPE="-D__CYGWIN32 -DWIN32" ;;
       *)         OS_TYPE="" ;;
esac
AC_EXEEXT
AC_OBJEXT
AC_CHECK_PROG(WINDRES_EXE, [windres], [windres])
AC_PATH_PROG(UPDATE_DESKTOP_DATABASE,[update-desktop-database],[true])
AC_PATH_PROG(UPDATE_MIME_DATABASE,[update-mime-database],[true])
AC_PATH_PROG(GTK_UPDATE_ICON_CACHE,[gtk-update-icon-cache],[true])

if test "x$with_x" != "xno"; then
    X_LIBS="$X_LIBS $X_PRE_LIBS $X_EXTRA_LIBS"
    dnl AC_PATH_XTRA only defines the path needed to find the X libs,
    dnl it does not add the libs; therefore we add them here
    X_LIBS="$X_LIBS -lX11"
    AC_SUBST(X_CFLAGS)
    AC_SUBST(X_LIBS)
fi

AC_SUBST(PTHREAD_LD)
AC_SUBST(WIN32_LIBS)
AC_SUBST(WNO_VARIADIC_MACROS)
AC_SUBST([FLEX_RC])
AC_SUBST([FLEXEMU_LDADD])
AC_SUBST([OS_TYPE])
AC_SUBST([FLEXPLORER], ['flexplorer${EXEEXT}'])

dnl which optimizations are possible depends on the cpu architecture
CXXSPEED="-D__CPU=$target_cpu"
if test "x$enable_speed" = "xyes"; then
   case $host_os in
      *linux*)
	case $target_cpu in
           i*86) CXXSPEED="$CXXSPEED -Winline -O3 -finline-limit=4000 \
 -frename-registers -fomit-frame-pointer -mcpu=$target_cpu -DUSE_ASM" ;;
        esac ;;
      *cygwin*)
	case $target_cpu in
           i*86) CXXSPEED="$CXXSPEED -Winline -O3 -finline-limit=4000 \
 -frename-registers -fomit-frame-pointer -mcpu=$target_cpu -DUSE_ASM" ;;
        esac ;;
      *mingw32*)
	case $target_cpu in
           i*86) CXXSPEED="$CXXSPEED -Winline -O3 -finline-limit=4000 \
 -frename-registers -fomit-frame-pointer -mcpu=$target_cpu -DUSE_ASM" ;;
        esac ;;
      *freebsd*)
	case $target_cpu in
           i*86) CXXSPEED="$CXXSPEED -Winline -O3 -finline-limit=4000 \
 -frename-registers -mcpu=$target_cpu -DUSE_ASM" ;;
        esac ;;
   esac
fi

AC_SUBST([CXXSPEED])

AC_CONFIG_FILES([Makefile \
	src/Makefile \
	src/bitmaps/Makefile \
	src/bitmaps/16x16/Makefile \
	src/bitmaps/32x32/Makefile \
	src/bitmaps/48x48/Makefile \
	src/bitmaps/64x64/Makefile \
	src/bitmaps/128x128/Makefile \
	src/bitmaps/256x256/Makefile \
	src/bitmaps/scalable/Makefile \
	src/tools/Makefile \
	disks/Makefile \
	monitor/Makefile \
	monitor/mon24/Makefile \
	monitor/mon24z/Makefile \
        build/Makefile \
        build/windows/Makefile \
        build/windows/installer/Makefile \
	doc/Makefile \
	doc/images/Makefile])
AC_OUTPUT

if test "x$with_x" = "xno"; then
  xlib_text="Not found"
else
  xlib_text="yes"
fi
if test "x$have_qt" = "xno"; then
  qt_text="Not found"
else
  qt_text="yes"
fi

  echo
  echo "************************** summary ************************************"
  echo " CFLAGS:                      $CFLAGS"
  echo " CXXFLAGS:                    $CXXFLAGS"
  echo " WIN32_LIBS:                  $WIN32_LIBS"
  echo " Xlib:                        $xlib_text"
  echo " Qt:                          $qt_text"
  echo " speed optimized compilation: $enable_speed"
  echo " execute post installation:   $enable_post_install"
  echo " canonical build:             $build"
  echo " canonical host:              $host"
  echo " canonical target:            $target"
  echo "***********************************************************************"
  echo

if test "x$with_x" = "xno"; then
    AC_MSG_WARN([
            Please check that libX11.so is installed. On debian based
            systems the package libX11-dev has to be installed.
    ])
fi
if test "x$have_qt" = "xno"; then
    AC_MSG_WARN([
            Please check that libQt 5.x is installed. On debian based
            systems the packages qtbase5-dev and qtbase5-dev-tools have to be
            installed.
    ])
fi
  echo
  echo "Configure finished.  Type 'make' to compile"
if test "x$have_qt" = "xno"; then
  echo "mdcrtool, dsktool and flex2hex"
else
  if test "x$with_x" = "xno"; then
    echo "flexplorer, mdcrtool, dsktool and flex2hex"
  else
    echo "flexemu, flexplorer, mdcrtool, dsktool and flex2hex"
  fi
fi
  echo

