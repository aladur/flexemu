dnl ---------------------------------------------------------------------------
dnl FLX_BITFIELDS_BIGENDIAN
dnl
dnl Check for command line argument
dnl    --enable-bitfields-bigendian or
dnl    --disable-bitfields-bigendian (the default).
dnl
dnl It defines if the first bit in a bitfield starts with the most significant
dnl bit (big endian) or with the least significant bit (little endian).
dnl Typically this value corresponds with the endianness of the used CPU 
dnl architecture but there may be compilers with different behaviour.
dnl As of today most CPU architectures, like x86, x64, arm, arm64 use little
dnl endian bitfields. For this reason this has been chosen as default.
dnl Big endian bitfields are used by e.g mips or m68k.
dnl
dnl When not setting the command line argument and this is no cross compilation
dnl A C++ program is executed to determine the compiler behaviour.
dnl ---------------------------------------------------------------------------

AC_DEFUN([FLX_BITFIELDS_BIGENDIAN],
[
  AC_ARG_ENABLE([bitfields-bigendian],
     AS_HELP_STRING([--enable-bitfields-bigendian], [Enable bitfields bit ordering is bigendian, default is littleendian, only needed for cross compilation]),,)
  AC_MSG_CHECKING([whether bitfield bit ordering is bigendian])

  AS_IF([test "x$enable_bitfields_bigendian" = "xyes"],,
    AC_RUN_IFELSE([
       AC_LANG_PROGRAM([],[[
         union u_t
         {
             char all;
             struct { bool first : 1; } bit;
         };
 
         u_t v{0};
         v.bit.first = true;
         return !(v.all & '\x80');
       ]])
    ],[enable_bitfields_bigendian="yes"],[enable_bitfields_bigendian="no"],[])
  )
  AC_MSG_RESULT($enable_bitfields_bigendian)
  AS_IF([test "x$enable_bitfields_bigendian" = "xyes"],,[
    AC_DEFINE([BITFIELDS_LSB_FIRST],[],[bitfields ordering is little endian])
  ])
])

