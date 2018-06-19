/* confignt.h.  config.h adapted to Windows 95/98/NT  */
/*
    confignt.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2018  W. Schwotzer

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef CONFIG_INCLUDED
#define CONFIG_INCLUDED

/* Uncomment the following if You want native file system support */

#define NAFS

/* uncomment the following if the Disassembler should display FLEX entry
   addresses by symbolic names */

#define FLEX_LABEL

/* uncomment the following line if You want to compile flexemu with
   an alternative MC6809 processor implementation. It's about 10% faster
   but only aproximates processor cycles. Good for checking processor
   implementations against each other */

/*#define FASTFLEX*/



/********************************************************************/
/* following lines will be automatically be configured by           */
/* autoheader as part of autoconf                                   */
/********************************************************************/


/* Define if type char is unsigned and you are not using gcc.  */
#ifndef __CHAR_UNSIGNED__
    /* #undef __CHAR_UNSIGNED__ */
#endif

/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define if you don't have vprintf but do have _doprnt.  */
/* #undef HAVE_DOPRNT */

/* Define if the `long double' type works.  */
#define HAVE_LONG_DOUBLE 1

/* Define if you support file names longer than 14 characters.  */
#define HAVE_LONG_FILE_NAMES 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
/* #undef HAVE_NDIR_H */

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
/*#define HAVE_STRINGS_H 1*/

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define if you have <sys/wait.h> that is POSIX.1 compatible.  */
#define HAVE_SYS_WAIT_H 1

/* Define if you have the vprintf function.  */
#define HAVE_VPRINTF 1

/* Define as __inline if that's what the C compiler calls it.  */
/* #undef inline */

/* Define to `long' if <sys/types.h> doesn't define.  */
/* #undef off_t */

/* Define as the return type of signal handlers (int or void).  */
/* #define RETSIGTYPE void */

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
/* #undef size_t */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if your processor stores words with the most significant
   byte first (like Motorola and SPARC, unlike Intel and VAX).  */
/* #undef WORDS_BIGENDIAN */

/* Define if the X Window System is missing or not being used.  */
#define X_DISPLAY_MISSING

/* #undef JOYSTICK_IS_PRESENT */
/* #undef LINUX_JOYSTICK_IS_PRESENT */
/* #undef HAVE_XPM */
/* #undef HAVE_XTK */

/* The number of bytes in a int.  */
#define SIZEOF_INT 4

/* The number of bytes in a short.  */
#define SIZEOF_SHORT 2

/* The number of bytes in a long.  */
#define SIZEOF_LONG 4

/* The number of bytes in a long.  */
#define SIZEOF_LONG_LONG 8

/* Define if you have the <dirent.h> header file.  */
/* #undef HAVE_DIRENT_H */

/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H 1

/* Define if you have the <ndir.h> header file.  */
/* #undef HAVE_NDIR_H */

/* Define if you have the <sys/dir.h> header file.  */
/* #undef HAVE_SYS_DIR_H */

/* Define if you have the <sys/ioctl.h> header file.  */
#define HAVE_SYS_IOCTL_H 1

/* Define if you have the <sys/ndir.h> header file.  */
/* #undef HAVE_SYS_NDIR_H */

/* Define if you have the <termios.h> header file.  */
/* #undef HAVE_TERMIOS_H */

/* Define if you have the <unistd.h> header file.  */
/* undef HAVE_UNISTD_H */

/* Define if you have the X11 library (-lX11).  */
/* undef HAVE_LIBX11 */

/* Define if you have the Xaw library (-lXaw).  */
/* undef HAVE_LIBXAW */

/* Define if you have the Xpm library (-lXpm).  */
/* undef HAVE_LIBXPM */

/* Define if you have the Xt library (-lXt).  */
/* undef HAVE_LIBXT */

/* Define if you have the gdk library (-lgdk).  */
/* undef HAVE_LIBGDK */

/* Define if you have the gtk library (-lgtk).  */
/* undef HAVE_LIBGTK */

/* Define if you have the pthread library (-lpthread).  */
/* undef HAVE_LIBPTHREAD */

/* Define if you have the wx_gtk library (-lwx_gtk).  */
/* #undef HAVE_LIBWX_GTK */

/* Name of package */
#define PACKAGE "flexemu"

/* Define to the full name of this package. */
#define PACKAGE_NAME "Flexemu"

/* Version number of package */
#define VERSION "2.20"

#endif /* CONFIG_INCLUDED */

