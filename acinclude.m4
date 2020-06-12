dnl ---------------------------------------------------------------------------
dnl FLX_CHECK_PTHREAD_LIB
dnl
dnl Check for the pthead library
dnl For FreeBSD no thread has to be added. Instead when linking with gcc
dnl the option -pthread has to be set
dnl ---------------------------------------------------------------------------

AC_DEFUN([FLX_CHECK_PTHREAD_LIB],
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

AC_DEFUN([FLX_CHECK_WIN32_LIBS],
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

AC_DEFUN([FLX_CHECK_XEXTENSIONS],
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
            AC_MSG_ERROR([[Athena Widgets (libXaw.so or libXaw3d.so) not found. Unable to build flexemu]]),
            [
               #include <X11/Xlib.h>
               #include <X11/Intrinsic.h>
            ]
          )
        ],
        AC_MSG_WARN([[XToolkit (libXt.so) not found. Unable to build flexemu]]),
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
      X_PRE_LIBS="$X_PRE_LIBS $X_P_LIBS"
    ])
fi
])

dnl ---------------------------------------------------------------------------
dnl FLX_ENABLE_SPEED
dnl ---------------------------------------------------------------------------

AC_DEFUN([FLX_ENABLE_SPEED],
[
  AC_ARG_ENABLE(speed,
     [  --enable-speed    compile flexemu for optimized speed],,enable_speed="no")
  AC_MSG_CHECKING(for optimized speed compilation)
  AC_MSG_RESULT($enable_speed)
])

