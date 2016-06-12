dnl ---------------------------------------------------------------------------
dnl FLX_CHECK_PTHREAD_LIB
dnl
dnl Check for the pthead library
dnl For FreeBSD no thread has to be added. Instead when linking with gcc
dnl the option -pthread has to be set
dnl ---------------------------------------------------------------------------

AC_DEFUN(FLX_CHECK_PTHREAD_LIB,
[
  PTHREAD_LIB=""
  AC_CHECK_LIB(pthread, open, PTHREAD_LD="-lpthread",,)
  case $target_os in
    *freebsd* ) PTHREAD_LD="-pthread";;
  esac
])

dnl ---------------------------------------------------------------------------
dnl FLX_CHECK_WIN32_LIBS
dnl
dnl Check for the Win32 libraries gdi32, comdlg32
dnl ---------------------------------------------------------------------------

AC_DEFUN(FLX_CHECK_WIN32_LIBS,
[
  case $target_os in
    *mingw32* ) WIN32_LIBS="-lgdi32 -lcomdlg32";;
            * ) WIN32_LIBS="";;
  esac
])

dnl ---------------------------------------------------------------------------
dnl FLX_CHECK_XEXTENSIONS
dnl
dnl Check for the X extension libraries Xt, Xaw and Xpm
dnl ---------------------------------------------------------------------------

AC_DEFUN(FLX_CHECK_XEXTENSIONS,
[
if test "$have_x" = "yes"
then
  X_LIBS="$X_LIBS -lXext -lX11"
  X_P_LIBS=""
  ac_save_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$CPPFLAGS $X_CFLAGS"

  dnl *** All of the following tests require X11/Xlib.h
  AC_CHECK_HEADERS(X11/Xlib.h,
    [
      dnl *** Check for XToolkit extension
      AC_CHECK_HEADERS(X11/Intrinsic.h,
        [ 
          dnl *** If X11/Intrinsic.h exists...
          AC_CHECK_LIB(Xt, XtOpenDisplay,
          [AC_DEFINE(HAVE_XTK, 1, [Define if you have the XToolkit extension])
          X_P_LIBS="-lXt $X_P_LIBS"],,
          $X_LIBS $X_EXTRA_LIBS)
        ],
        AC_MSG_WARN([[XToolkit not found, try to build flexemu without it]]),
        [#include <X11/Xlib.h>]
      )

      dnl *** Check for Xmu extension
      AC_CHECK_HEADERS(X11/Xmu/Xmu.h,
        [ 
          dnl *** If X11/Xmu/Converters.h exists...
          AC_CHECK_LIB(Xmu, XmuCvtStringToOrientation,
          [AC_DEFINE(HAVE_XMU, 1, [Define if you have the Xmu extension])
          X_P_LIBS="-lXmu $X_P_LIBS"],,
          $X_LIBS $X_EXTRA_LIBS)
        ],
        AC_MSG_WARN([[Xmu not found, try to build flexemu without it]]),
        [#include <X11/Xlib.h>]
      )

      dnl *** Check for XPixmap extension
      AC_CHECK_HEADERS(X11/xpm.h,
        [ 
          dnl *** If X11/Xpm.h exists...
          AC_CHECK_LIB(Xpm, XpmFree,
          [AC_DEFINE(HAVE_XPM, 1, [Define if you have the Xpm extension])
           X_P_LIBS="-lXpm $X_P_LIBS"],,
          $X_LIBS $X_EXTRA_LIBS)
        ],
        AC_MSG_WARN([[Xpm not found, flexemu will be built without it]]),
        [#include <X11/Xlib.h>]
      )
      dnl *** Check for X Athena widget extension
      AC_CHECK_HEADERS(X11/Xaw/XawInit.h,
        [ 
          dnl *** If X11/Xaw/XawInit.h exists...
          dnl *** Check for both libXaw3d and libXaw
          AC_CHECK_LIB(Xaw3d, XawToggleSetCurrent,
          [AC_DEFINE(HAVE_XAW3D, 1, [Define if you have the Xaw3d extension])
           X_P_LIBS="-lXaw3d $X_P_LIBS"],
             [AC_CHECK_LIB(Xaw, XawToggleSetCurrent,
             [AC_DEFINE(HAVE_XAW, 1, [Define if you have the Xaw extension])
              X_P_LIBS="-lXaw $X_P_LIBS"],,
             $X_LIBS $X_EXTRA_LIBS)],
          $X_LIBS $X_EXTRA_LIBS)
        ],
        AC_MSG_WARN([[Xaw or Xaw3d not found]]),
        [#include <X11/Xlib.h>]
      )
      X_PRE_LIBS="$X_PRE_LIBS $X_P_LIBS"
    ])
fi
])

dnl ---------------------------------------------------------------------------
dnl WX_CPP_BOOL checks whether the C++ compiler has a built in bool type
dnl
dnl call WX_CPP_BOOL - will define HAVE_BOOL if the compiler supports bool
dnl ---------------------------------------------------------------------------

AC_DEFUN(WX_CPP_BOOL,
[
  AC_CACHE_CHECK([if C++ compiler supports bool], wx_cv_cpp_bool,
  [
    AC_LANG_SAVE
    AC_LANG_CPLUSPLUS

    AC_TRY_COMPILE(
      [
      ],
      [
        bool b = true;

        return 0;
      ],
      [
        wx_cv_cpp_bool=1
      ],
      [
        wx_cv_cpp_bool=0
      ]
    )

    AC_LANG_RESTORE
  ])

  if test "x$wx_cv_cpp_bool" = x ; then
    AC_DEFINE(HAVE_BOOL,1,1)
  else
    AC_DEFINE(HAVE_BOOL,0,1)
  fi
])

dnl ---------------------------------------------------------------------------
dnl FLX_ENABLE_SPEED
dnl ---------------------------------------------------------------------------

AC_DEFUN(FLX_ENABLE_SPEED,
[
   AC_ARG_ENABLE(speed,
     [  --enable-speed    compile flexemu for optimized speed],,enable_speed="no")
  AC_MSG_CHECKING(for optimized speed compilation)
  AC_MSG_RESULT($enable_speed)
])

dnl ---------------------------------------------------------------------------
dnl WX_PATH_WXCONFIG(VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl
dnl Test for wxWindows, and define WX_CFLAGS and WX_LIBS. Set WX_CONFIG
dnl environment variable to override the default name of the wx-config script
dnl to use.
dnl ---------------------------------------------------------------------------

AC_DEFUN(WX_PATH_WXCONFIG,
[
dnl 
dnl Get the cflags and libraries from the wx-config script
dnl
AC_ARG_WITH(wx-prefix, [  --with-wx-prefix=PREFIX   Prefix where wxWindows is installed (optional)],
            wx_config_prefix="$withval", wx_config_prefix="")
AC_ARG_WITH(wx-exec-prefix,[  --with-wx-exec-prefix=PREFIX Exec prefix where wxWindows is installed (optional)],
            wx_config_exec_prefix="$withval", wx_config_exec_prefix="")
AC_ARG_ENABLE(wx-shared, [  --disable-wx-shared    statically link wxWindows],,enable_wx_shared="yes")
  AC_MSG_CHECKING(linking wxWindows as shared lib)
  AC_MSG_RESULT($enable_wx_shared)

  dnl deal with optional prefixes
  if test x$wx_config_exec_prefix != x ; then
     wx_config_args="$wx_config_args --exec-prefix=$wx_config_exec_prefix"
     if test x${WX_CONFIG+set} != xset ; then
        WX_CONFIG=$wx_config_exec_prefix/bin/wx-config
     fi
  fi
  if test x$wx_config_prefix != x ; then
     wx_config_args="$wx_config_args --prefix=$wx_config_prefix"
     if test x${WX_CONFIG+set} != xset ; then
        WX_CONFIG=$wx_config_prefix/bin/wx-config
     fi
  fi

  AC_PATH_PROG(WX_CONFIG, wx-config, no)
  min_wx_version=ifelse([$1], ,2.2.1,$1)
  AC_MSG_CHECKING(for wxWindows version >= $min_wx_version)
  no_wx=""
  if test "$WX_CONFIG" = "no" ; then
    no_wx=yes
  else
    wx_include_dir=`$WX_CONFIG $wx_config_args --exec-prefix`
    WX_CFLAGS="-I$wx_include_dir/include `$WX_CONFIG $wx_config_args --cflags`"
    if test x$enable_wx_shared = "xyes" ; then
       WX_LIBS=`$WX_CONFIG $wx_config_args --libs`
    else
       WX_LIBS=`$WX_CONFIG $wx_config_args --static --libs`
    fi
    wx_config_major_version=`$WX_CONFIG $wx_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    wx_config_minor_version=`$WX_CONFIG $wx_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    wx_config_micro_version=`$WX_CONFIG $wx_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
  fi
  WX_VERSION="$wx_config_major_version.$wx_config_minor_version.$wx_config_micro_version"
  if test "x$no_wx" = x ; then
     AC_MSG_RESULT(yes (version $WX_VERSION))
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     WX_CFLAGS=""
     WX_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(WX_CFLAGS)
  AC_SUBST(WX_LIBS)
])


