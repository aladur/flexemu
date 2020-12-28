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
dnl FLX_ENABLE_SPEED
dnl ---------------------------------------------------------------------------

AC_DEFUN([FLX_ENABLE_SPEED],
[
  AC_ARG_ENABLE(speed,
     [  --enable-speed    compile flexemu for optimized speed],,enable_speed="no")
  AC_MSG_CHECKING(for optimized speed compilation)
  AC_MSG_RESULT($enable_speed)
])

