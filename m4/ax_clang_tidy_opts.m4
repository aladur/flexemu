# SYNOPSIS
#
#   AX_CLANG_TIDY_OPTS(CXXFLAGS)
#
# DESCRIPTION
#
#   Create a clang-tidy option list for given CXXFLAGS.
#
#   The following variable is exported:
#
#     CLANG_TIDY_OPTS
#
#   It contains the created option list
#
#   Example for calling clang-tidy:
#
#     clang-tidy mysource.cpp -- @CLANG_TIDY_OPTS@
#
# LICENSE
#
#   Copyright (c) 2024 Wolfgang Schwotzer <wolfgang.schwotzer@gmx.net>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

#serial 1

AC_DEFUN([AX_CLANG_TIDY_OPTS],
[
  AC_REQUIRE([AC_PROG_SED])
  [ CLANG_TIDY_OPTS=`echo $1 | $SED "s/-I\([a-zA-Z0-9_/-]\+\)/-isystem \1/g"` ]dnl
  AC_SUBST([CLANG_TIDY_OPTS])
])

