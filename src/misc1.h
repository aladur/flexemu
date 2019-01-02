/*
    misc1.h


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



#ifndef MISC1_INCLUDED
#define MISC1_INCLUDED

#ifdef _MSC_VER
    #include "confignt.h"
#else
    #include "config.h"
#endif
#ifdef _WIN32
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #ifndef _CRT_SECURE_NO_WARNINGS
        #define _CRT_SECURE_NO_WARNINGS 1
    #endif
    #ifndef _CRT_NONSTDC_NO_DEPRECATE
        #define _CRT_NONSTDC_NO_DEPRECATE 1
    #endif
#include <windows.h>
#endif
#include "typedefs.h"
#include <string>
#include <array>

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

// Don't use PACKAGE. It has been changed by automake to be all lowercase.
// For compatibility the program name should begin with a capital F.
// Use PACKAGE_NAME instead.
#define PROGRAMNAME     PACKAGE_NAME
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
#ifdef _WIN32
    #include <sys/utime.h>
#endif
#ifdef UNIX
    #include <utime.h>
#endif

/* adapt platform specifics: */

#ifdef _MSC_VER
    #define vsnprintf _vsnprintf
    #define stricmp _stricmp
    #define access _access
    #define unlink _unlink
    #define getcwd _getcwd
    #define chdir _chdir
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

#ifndef _WIN32
    #define CALLBACK
#endif

#ifndef PATH_MAX
    #ifdef _WIN32
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
#ifdef _WIN32
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
#define DEBUGPRINT(fmt)   OutputDebugString((LPCTSTR)("[" PROGRAMNAME "] " fmt))
#define DEBUGPRINT1(fmt,p1)                        \
    {                                                \
        char str[255];                                 \
        sprintf((char *)str, "[" PROGRAMNAME "] " fmt,p1); \
        OutputDebugString((LPCTSTR)str);                        \
    }
#define DEBUGPRINT2(fmt,p1,p2)                        \
    {                                                   \
        char str[255];                                    \
        sprintf((char *)str, "[" PROGRAMNAME "] " fmt,p1,p2); \
        OutputDebugString((LPCTSTR)str);                           \
    }
#define DEBUGPRINT3(fmt,p1,p2,p3)                        \
    {                                                      \
        char str[255];                                       \
        sprintf((char *)str, "[" PROGRAMNAME "] " fmt,p1,p2,p3); \
        OutputDebugString((LPCTSTR)str);                              \
    }
#endif

#if defined(UNIX) && !defined(NO_DEBUGPRINT)
    #define DEBUGPRINT(fmt)           ::printf("[" PROGRAMNAME "] " fmt)
    #define DEBUGPRINT1(fmt,p1)       ::printf("[" PROGRAMNAME "] " fmt,p1)
    #define DEBUGPRINT2(fmt,p1,p2)    ::printf("[" PROGRAMNAME "] " fmt,p1,p2)
    #define DEBUGPRINT3(fmt,p1,p2,p3) ::printf("[" PROGRAMNAME "] " \
                fmt,p1,p2,p3)
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
    sOptions();

    std::string drive[4];
    std::array<std::string, 2> mdcrDrives;
    std::string hex_file;
    std::string disk_dir;
    bool isRamExtension;  // Use RAM extension cards/No RAM extension
    bool isHiMem;         // Use 2 x 288K RAM extension/2 x 96 K RAM ext.
    bool isFlexibleMmu;   // Use flexible MMU/Normal MMU
    bool isEurocom2V5;    // Emulate an Eurocom II/V5 (instead of Eurocom II/V7)
    bool use_undocumented;
    bool useRtc;
    bool term_mode;
    short int reset_key; // must be short int because of sscanf !!!
    float frequency;
};

/* Names of Environment or Registry variables */

#ifdef _WIN32
    #define FLEXEMUREG       "SOFTWARE\\Gnu\\Flexemu"
    #define FLEXPLOREREG     "SOFTWARE\\Gnu\\FLEXplorer"
#endif

#define FLEXEMURC   ".flexemurc"
#define FLEXPLORERRC    ".flexplorerrc"

#define FLEXDISKDIR     "DiskDirectory"
#define FLEXDISK0       "Disk0Path"
#define FLEXDISK1       "Disk1Path"
#define FLEXDISK2       "Disk2Path"
#define FLEXDISK3       "Disk3Path"
#define FLEXMDCRDRIVE0  "MdcrDrive0Path"
#define FLEXMDCRDRIVE1  "MdcrDrive1Path"
#define FLEXCOLOR       "DisplayColor"
#define FLEXNCOLORS     "NoOfColors"
#define FLEXINVERSE     "DisplayInverse"
#define FLEXHIMEM   "HighMemory"
#define FLEXFLEXIBLEMMU "UseFlexibleMmu"
#define FLEXRAMEXTENSION "UseRamExtension"
#define FLEXEUROCOM2V5 "UseEurocom2V5"
#define FLEXUNDOCUMENTED    "UndocumentedMc6809"
#define FLEXRTC         "UseRTC"
#define FLEXHTMLVIEWER  "HTMLViewer"
#define FLEXDOCDIR      "DocDirectory"
#define FLEXMONITOR     "MonitorPath"
#define FLEXVERSION     "Version"
#define FLEXSCREENWIDTH "ScreenWidthFactor"
#define FLEXSCREENHEIGHT "ScreenHeightFactor"

#define FLEXPLORERFILEVIEWER    "FileViewer"
#define FLEXPLORERBOOTSECTORFILE "BootSectorFile"
#define FLEXPLORERTEXTFLAG  "AutoTextConversion"

#define BTST0(x)  (((x) & 0x01) != 0)
#define BTST1(x)  (((x) & 0x02) != 0)
#define BTST2(x)  (((x) & 0x04) != 0)
#define BTST3(x)  (((x) & 0x08) != 0)
#define BTST4(x)  (((x) & 0x10) != 0)
#define BTST5(x)  (((x) & 0x20) != 0)
#define BTST6(x)  (((x) & 0x40) != 0)
#define BTST7(x)  (((x) & 0x80) != 0)
#define BTST8(x)  (((x) & 0x100) != 0)
#define BTST15(x) (((x) & 0x8000) != 0)
#define BTST16(x) (((x) & 0x10000) != 0)

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

#if __cplusplus > 201402L && __has_cpp_attribute(fallthrough)
#define FALLTHROUGH [[fallthrough]]
#elif __cplusplus && __GNUC__
#define FALLTHROUGH __attribute__ ((fallthrough))
#elif __cplusplus && __has_cpp_attribute(clang::fallthrough)
#define FALLTHROUGH [[clang::fallthrough]]
#else
#define FALLTHROUGH
#endif

#ifdef __cplusplus
    extern int      copyFile(const char *srcPath, const char *destPath);
    extern void     strupper(char *pstr);
    extern void     strlower(char *pstr);
    extern char     *binstr(Byte x);
    extern char     *hexstr(Byte x);
    extern char     *hexstr(Word x);
    extern char     *ascchr(Byte x);
    extern std::string tohexstr(Byte x);
    extern std::string tohexstr(Word x);

    #ifdef _WIN32
        extern int getopt(int argc, char *const argv[], char *optstr);
        extern int  optind;
        extern int  opterr;
        extern char *optarg;
    #endif
#endif /* ifdef __cplusplus */

extern bool matches(const char *text, const char *pattern,
             bool ignorecase /* = false */);
extern bool multimatches(const char *text, const char *multipattern,
                  const char delimiter /* = ';'*/,
                  bool ignorecase /* = false */);
#ifdef _WIN32
extern std::string getExecutablePath();
#endif
extern std::string getFlexemuSystemConfigFile();

extern const char* white_space;

// trim from end of string (right)
inline std::string& rtrim(std::string& str, const char* t = white_space)
{
    str.erase(str.find_last_not_of(t) + 1);
    return str;
}

// trim from beginning of string (left)
inline std::string& ltrim(std::string& str, const char* t = white_space)
{
    str.erase(0, str.find_first_not_of(t));
    return str;
}

// trim from both ends of string (left & right)
inline std::string& trim(std::string& str, const char* t = white_space)
{
    return ltrim(rtrim(str, t), t);
}

#endif /* __misc1.h__ */
