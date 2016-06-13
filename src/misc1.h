/*
    misc1.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2004  W. Schwotzer

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



#ifndef __misc1_h__
#define __misc1_h__

#ifdef _MSC_VER
    #include "confignt.h"
#else
    #include "config.h"
#endif
#ifdef WIN32
    #include <windows.h>
#endif
#include "typedefs.h"
#include "bstring.h"

/* Uncomment the following if You want native file system support */

#define NAFS

/* uncomment the following if the Disassembler should display FLEX entry
   addresses by symbolic names */

#define FLEX_LABEL

/* uncomment the following line if You want to compile flexemu with
   an alternative MC6809 processor implementation. It's about 10% faster
   but only aproximates processor cycles. Good for checking processor
   implementations against each other */

/* #define FASTFLEX */

#define PROGRAMNAME     PACKAGE
#define PROGRAM_VERSION VERSION

#ifndef X_DISPLAY_MISSING
    #define HAVE_X11
#endif

extern const char *gMemoryAllocationErrorString;

/* adaptions for autoconf to use with/without ANSI C headers */

#ifndef UNIX
    #if defined(__LINUX) || defined(__BSD) || defined(__SOLARIS)
        #define UNIX
    #endif
#endif

#if STDC_HEADERS
    #include <string.h>
#else
    #ifndef HAVE_STRCHR
        #define strchr index
        #define strrchr rindex
    #endif
    char *strchr(), *strrchr();
    #ifndef HAVE_MEMCPY
        #define memcpy(d, s, n) bcopy ((s), (d), (n))
        #define memmove(d, s, n) bcopy ((s), (d), (n))
    #endif
#endif

#ifdef HAVE_STRING_H
    #include <string.h>
#else
    #ifdef HAVE_STRINGS_H
        #include <strings.h>
    #endif
#endif

#ifdef HAVE_STDLIB_H
    #include <stdlib.h>
#endif

/* adaptions for autoconf for POSIX.1 compatibility */
#if HAVE_UNISTD_H
    #include <sys/types.h>
    #ifdef __GNUC__
        #include <unistd.h>
    #endif
#endif

#ifdef _POSIX_VERSION
    /* Code for POSIX.1 systems.  */
    #include <limits.h>
#endif

/* dirent structure: */
#ifdef UNIX
    #ifdef HAVE_DIRENT_H
        #include <dirent.h>
        #define NAMLEN(dirent) strlen((dirent)->d_name)
    #else
        #define dirent direct
        #define NAMLEN(dirent) (dirent)->d_namlen
        #ifdef HAVE_SYS_NDIR_H
            #include <sys/ndir.h>
        #endif
        #ifdef HAVE_SYS_DIR_H
            #include <sys/dir.h>
        #endif
        #ifdef HAVE_NDIR_H
            #include <ndir.h>
        #endif
    #endif
#endif

/* time */
#if TIME_WITH_SYS_TIME
    #include <sys/time.h>
    #include <time.h>
#else
    #if HAVE_SYS_TIME_H
        #include <sys/time.h>
    #else
        #include <time.h>
    #endif
#endif

/* utime */
#ifdef WIN32
    #include <sys/utime.h>
#endif
#ifdef UNIX
    #include <utime.h>
#endif

/* adapt platform specifics: */

#ifdef _MSC_VER
    #define vsnprintf       _vsnprintf
    #include <io.h>
#endif

#if defined(__GNUC__) && !(defined(__MINGW32) || defined (__CYGWIN32) )
    extern int stricmp(const char *string1, const char *string2);
#endif

#ifdef _MSC_VER
    #define W_OK            (2) /* write permission */
    #define S_ISDIR(x)      (x & S_IFDIR)
    #define S_ISREG(x)      (x & S_IFREG)
    #define set_new_handler _set_new_handler
    #define UNUSED(param)
#endif
#ifdef __GNUC__
    #define UNUSED(param) param
#endif

#ifndef WIN32
    #define CALLBACK
#endif

#ifndef PATH_MAX
    #ifdef WIN32
        #ifdef _MSC_VER
            #define PATH_MAX _MAX_PATH
        #endif
        #if defined(__MINGW32__) || defined (__CYGWIN32__)
            #include <limits.h>
        #endif
    #endif
    #ifdef __LINUX
        #include <sys/param.h>
    #endif
    #ifdef __SOLARIS
        #include <limits.h>
    #endif
#endif

/* PATHSEPARATORSTRING shoud be a define to do */
/* implicit concatenation by the compiler!     */
#ifdef WIN32
    const char PATHSEPARATOR = '\\';
    #define PATHSEPARATORSTRING  "\\"
#endif
#ifdef UNIX
    const char PATHSEPARATOR = '/';
    #define PATHSEPARATORSTRING "/"
#endif

#ifndef EXIT_SUCCESS
    #define EXIT_SUCCESS 0
#endif
#ifndef EXIT_FAILURE
    #define EXIT_FAILURE 1
#endif

#if defined(WIN32) && !defined(NO_DEBUGPRINT)
#define DEBUGPRINT(fmt)   OutputDebugString((LPCTSTR)("[" PACKAGE "] " fmt))
#define DEBUGPRINT1(fmt,p1)                        \
    {                                                \
        char str[255];                                 \
        sprintf((char *)str, "[" PACKAGE "] " fmt,p1); \
        OutputDebugString((LPCTSTR)str);                        \
    }
#define DEBUGPRINT2(fmt,p1,p2)                        \
    {                                                   \
        char str[255];                                    \
        sprintf((char *)str, "[" PACKAGE "] " fmt,p1,p2); \
        OutputDebugString((LPCTSTR)str);                           \
    }
#define DEBUGPRINT3(fmt,p1,p2,p3)                        \
    {                                                      \
        char str[255];                                       \
        sprintf((char *)str, "[" PACKAGE "] " fmt,p1,p2,p3); \
        OutputDebugString((LPCTSTR)str);                              \
    }
#endif

#if defined(UNIX) && !defined(NO_DEBUGPRINT)
    #define DEBUGPRINT(fmt)           ::printf("[" PACKAGE "] " fmt)
    #define DEBUGPRINT1(fmt,p1)       ::printf("[" PACKAGE "] " fmt,p1)
    #define DEBUGPRINT2(fmt,p1,p2)    ::printf("[" PACKAGE "] " fmt,p1,p2)
    #define DEBUGPRINT3(fmt,p1,p2,p3) ::printf("[" PACKAGE "] " fmt,p1,p2,p3)
#endif

#if defined(NO_DEBUGPRINT)
    #define DEBUGPRINT(fmt)
    #define DEBUGPRINT1(fmt,p1)
    #define DEBUGPRINT2(fmt,p1,p2)
    #define DEBUGPRINT2(fmt,p1,p2,p3)
#endif

#define BITFIELDS_LSB_FIRST

struct sOptions
{
    BString   drive[4];
    BString   hex_file;
    BString   disk_dir;
    bool      isHiMem;
    bool      use_undocumented;
    bool      term_mode;
    short int reset_key; // must be short int because of sscanf !!!
};

/* Names of Environment or Registry variables */

#ifdef WIN32
    #define FLEXEMUREG       "Software\\Gnu\\Flexemu"
    #define FLEXPLOREREG     "Software\\Gnu\\FLEXplorer"
#endif

#define FLEXEMURC   ".flexemurc"
#define FLEXPLORERRC    ".flexplorerrc"

#define FLEXDISKDIR     "DiskDirectory"
#define FLEXDISK0       "Disk0Path"
#define FLEXDISK1       "Disk1Path"
#define FLEXDISK2       "Disk2Path"
#define FLEXDISK3       "Disk3Path"
#define FLEXCOLOR       "DisplayColor"
#define FLEXNCOLORS     "NoOfColors"
#define FLEXINVERSE     "DisplayInverse"
#define FLEXHIMEM   "HighMemory"
#define FLEXUNDOCUMENTED    "UndocumentedMc6809"
#define FLEXWWWBROWSER  "WwwBrowser"
#define FLEXDOCDIR      "DocDirectory"
#define FLEXMONITOR     "MonitorPath"
#define FLEXVERSION     "Version"
#define FLEXSCREENWIDTH "ScreenWidthFactor"
#define FLEXSCREENHEIGHT "ScreenHeightFactor"

#define FLEXPLORERFILEVIEWER    "FileViewer"
#define FLEXPLORERBOOTSECTORFILE "BootSectorFile"
#define FLEXPLORERTEXTFLAG  "AutoTextConversion"

#define BTST0(x)  ((x) &    0x01)
#define BTST1(x)  ((x) &    0x02)
#define BTST2(x)  ((x) &    0x04)
#define BTST3(x)  ((x) &    0x08)
#define BTST4(x)  ((x) &    0x10)
#define BTST5(x)  ((x) &    0x20)
#define BTST6(x)  ((x) &    0x40)
#define BTST7(x)  ((x) &    0x80)
#define BTST8(x)  ((x) &   0x100)
#define BTST15(x) ((x) &  0x8000)
#define BTST16(x) ((x) & 0x10000)

#define BSET7(x) (x |= 0x80)
#define BSET6(x) (x |= 0x40)
#define BSET5(x) (x |= 0x20)
#define BSET4(x) (x |= 0x10)
#define BSET3(x) (x |= 0x08)
#define BSET2(x) (x |= 0x04)
#define BSET1(x) (x |= 0x02)
#define BSET0(x) (x |= 0x01)

#define BCLR7(x) (x &= 0x7f)

#define EXTEND8(x) (Word)(SWord)(SByte)(x)

#define DECLARE_MENUCOMMAND(function)   void function(wxCommandEvent& event);
#define FORWARD_MENUCOMMAND_TO(cls, destination, function) \
    void cls::function(wxCommandEvent& event)              \
    {                                                      \
        if (destination)                                   \
        {                                                  \
            destination->function(event);                  \
        }                                                  \
    }


#ifdef __cplusplus
    extern int      copyFile(const char *srcPath, const char *destPath);
    extern void     strupper(char *pstr);
    extern void     strlower(char *pstr);
    extern char     *binstr(Byte x);
    extern char     *hexstr(Byte x);
    extern char     *hexstr(Word x);
    extern char     *ascchr(Byte x);
    #ifdef WIN32
        extern int getopt(int argc, char *const argv[], char *optstr);
        extern int  optind;
        extern int  opterr;
        extern char *optarg;
    #endif
#endif /* ifdef __cplusplus */

#endif /* __misc1.h__ */
