dnl ---------------------------------------------------------------------------
dnl FLX_ENABLE_POST_INSTALL
dnl ---------------------------------------------------------------------------

AC_DEFUN([FLX_ENABLE_POST_INSTALL],
[
  AC_ARG_ENABLE(post-install,
     [  --disable-post-install  disable executing post installation steps ],,enable_post_install="yes")
  AC_MSG_CHECKING(for executing post installation steps)
  AC_MSG_RESULT($enable_post_install)

IGNORE="#"
if test "x$enable_post_install" = "xyes"
then
    IGNORE=
fi

AC_SUBST(IGNORE)
])

