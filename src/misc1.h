/*
    misc1.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2021  W. Schwotzer

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
    #if !defined(_UNICODE) && !defined(UNICODE)
        #error This program can only be compiled with Unicode support.
    #endif
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
#include <algorithm>


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
#define COPYRIGHT_MESSAGE \
    "comes with ABSOLUTELY NO WARRANTY. This is free software,\n" \
    "and You are welcome to redistribute it under certain conditions.\n" \
    "Please notice that this project was developed under the terms of the\n" \
    "GNU GENERAL PUBLIC LICENCE V2.\n" \
    "Copyright (C) 1998-2021 Wolfgang Schwotzer\n" \
    "http://flexemu.neocities.org\n"

extern const char *gMemoryAllocationErrorString;

/* adaptions for autoconf to use with/without ANSI C headers */

#ifndef UNIX
    #if defined(__LINUX) || defined(__BSD)
        #define UNIX
    #endif
#endif

#include <string.h>

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
#if HAVE_SYS_TIME_H
    #include <sys/time.h>
#endif
#include <time.h>

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

#ifdef __LINUX
#define OSTYPE "Linux"
#else
#ifdef __BSD
#define OSTYPE "BSD"
#else
#ifdef _WIN32
#define OSTYPE "Windows"
#else
#define OSTYPE "unknown"
#endif
#endif
#endif

#ifndef EXIT_SUCCESS
    #define EXIT_SUCCESS 0
#endif
#ifndef EXIT_FAILURE
    #define EXIT_FAILURE 1
#endif
#define EXIT_RESTART 25262728 // (pseudo) Exit code to restart flexemu

#if defined(WIN32) && !defined(NO_DEBUGPRINT)
#define DEBUGPRINT(fmt)   OutputDebugString((LPCTSTR)("[" PROGRAMNAME "] " fmt))
#define DEBUGPRINT1(fmt,p1)                        \
    {                                                \
        char str[255];                                 \
        sprintf(str, "[" PROGRAMNAME "] " fmt,p1); \
        OutputDebugString((LPCTSTR)str);                        \
    }
#define DEBUGPRINT2(fmt,p1,p2)                        \
    {                                                   \
        char str[255];                                    \
        sprintf(str, "[" PROGRAMNAME "] " fmt,p1,p2); \
        OutputDebugString((LPCTSTR)str);                           \
    }
#define DEBUGPRINT3(fmt,p1,p2,p3)                        \
    {                                                      \
        char str[255];                                       \
        sprintf(str, "[" PROGRAMNAME "] " fmt,p1,p2,p3); \
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

extern "C" struct sRGBDef
{
    const char *colorName;
    Byte red;
    Byte green;
    Byte blue;
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
#define FLEXDOCDIR      "DocDirectory"
#define FLEXMONITOR     "MonitorPath"
#define FLEXVERSION     "Version"
#define FLEXSCREENFACTOR "ScreenFactor"
#define FLEXFREQUENCY    "Frequency"

#define FLEXPLORERBOOTSECTORFILE "BootSectorFile"
#define FLEXPLORERTEXTFLAG  "AutoTextConversion"

// This macro defines the name of a file. It contains a list of files
// which have to be handled as random files. It is used in directory containers
// to identify random files.
#define RANDOM_FILE_LIST "random"

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

#if __GNUC__ >= 5 || __clang__ >= 4 || (__clang__ == 3 && __clang_minor__ >= 6)
#if __cplusplus > 201402L && __has_cpp_attribute(fallthrough)
#define FALLTHROUGH [[fallthrough]]
#elif __cplusplus && __has_cpp_attribute(clang::fallthrough)
#define FALLTHROUGH [[clang::fallthrough]]
#elif __cplusplus && __has_cpp_attribute(gnu::fallthrough)
#define FALLTHROUGH [[gnu::fallthrough]]
#else
#define FALLTHROUGH
#endif
#else
#define FALLTHROUGH
#endif

#ifdef __cplusplus
    extern int      copyFile(const char *srcPath, const char *destPath);
    extern void     strupper(char *pstr);
    extern void     strlower(char *pstr);
    extern void     strlower(std::string& str);
    extern void     strupper(std::string& str);
    extern char     *binstr(Byte x);
    extern char     *hexstr(Byte x);
    extern char     *hexstr(Word x);
    extern char     *ascchr(Byte x);
    extern std::string tohexstr(Byte x);
    extern std::string tohexstr(Word x);

    #ifdef _WIN32
        extern int getopt(int argc, char *const argv[], const char *optstr);
        extern int  optind;
        extern int  opterr;
        extern int  optopt;
        extern const char *optarg;
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
extern void dumpSector(FILE *fp, const char *indent,
                       const Byte *buffer, uint32_t size);
extern std::string getTempPath();
extern std::string getFlexemuSystemConfigFile();
extern std::string getFileName(const std::string &path);
extern std::string getFileStem(const std::string &path);
extern std::string getFileExtension(const std::string &path);
extern std::string getParentPath(const std::string &path);
extern std::string getCurrentPath();
extern std::string getHostName();
extern bool endsWithPathSeparator(const std::string &path);
extern bool isAbsolutePath(const std::string &path);
extern bool isPathsEqual(const std::string &path1, const std::string &path2);
extern "C" int getRGBForName(const char *colorName, Word *red, Word *green, Word *blue);
extern "C" int getColorForName(const char *colorName, DWord *color);
extern "C" struct sRGBDef colors[];
extern "C" const size_t color_count;

extern const char* white_space;

extern bool isListedInFileRandom(const char *directory, const char *filename);
extern bool hasRandomFileAttribute(const char *directory, const char *filename);

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

template<typename T> T reverseBytes(const T& value)
{
    union values_t
    {
        std::array<char, sizeof(T)> bytes;
        T value;
    } temp;

    temp.value = value;

    std::reverse(temp.bytes.begin(), temp.bytes.end());

    return temp.value;
}

// Convert data type from current CPU architecture to big endian.
template<typename T> T toBigEndian(const T& value)
{
#if defined WORDS_BIGENDIAN
    return value;
#else
    return reverseBytes<T>(value);
#endif
}

// Convert data type from big endian to current CPU architecture.
template<typename T> T fromBigEndian(const T& value)
{
#if defined WORDS_BIGENDIAN
    return value;
#else
    return reverseBytes<T>(value);
#endif
}

// Convert data type from current CPU architecture to little endian.
template<typename T> T toLittleEndian(const T& value)
{
#if defined WORDS_BIGENDIAN
    return reverseBytes<T>(value);
#else
    return value;
#endif
}

// Convert data type from little endian to current CPU architecture.
template<typename T> T fromLittleEndian(const T& value)
{
#if defined WORDS_BIGENDIAN
    return reverseBytes<T>(value);
#else
    return value;
#endif
}

// Read a big endian data value from buffer p.
template<typename T> T getValueBigEndian(const Byte *p)
{
    T result = 0U;

    for (size_t index = 0; index < sizeof(T); ++index)
    {
        result = (result << 8) | p[index];
    }

    return result;
}

// Write a big endian data value to buffer p.
template<typename T> void setValueBigEndian(Byte *p, T value)
{
    for (size_t index = sizeof(T); index != 0; --index)
    {
        p[index - 1] = value & 0xFF;
        value >>= 8;
    }
}

extern bool AskForInput(const std::string &question,
                        const std::string &answers,
                        char default_answer);

#endif /* __misc1.h__ */
