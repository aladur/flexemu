dnl ---------------------------------------------------------------------------
dnl FLX_PRECOMPILED_HEADERS
dnl ---------------------------------------------------------------------------

AC_DEFUN([FLX_PRECOMPILED_HEADERS],
[
  AC_ARG_ENABLE(precompiled-headers,
     [  --disable-precompiled-headers  disable build with precompiled headers ],,enable_precompiled_headers="yes")
  AC_MSG_CHECKING(for using precompiled headers)
  AC_MSG_RESULT($enable_precompiled_headers)
])

