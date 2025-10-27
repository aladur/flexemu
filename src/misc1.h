/*
    misc1.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2025  W. Schwotzer

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

#if defined(UNIX) || defined(USE_CMAKE)
    #include "config.h"
#else
    #include "confignt.h"
#endif
#ifdef _WIN32
    #if !defined(_UNICODE) && !defined(UNICODE)
        #error This program can only be compiled with Unicode support.
    #endif
#endif
#include "typedefs.h"
#include <cassert>
#include <ctime>
#include <limits>
#include <locale>
#include <utility>
#include <string>
#include <array>
#include <vector>
#include <sstream>
#include <ostream>
#include <iterator>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;


/* uncomment the following if the Disassembler should display FLEX entry
   addresses by symbolic names */

#define FLEX_LABEL

/* adapt platform specifics: */

/* PATHSEPARATORSTRING shoud be a define to do */
/* implicit concatenation by the compiler!     */
#ifdef _WIN32
constexpr const char PATHSEPARATOR = '\\';
constexpr const char *PATHSEPARATORSTRING = "\\";
#endif
#ifdef UNIX
constexpr const char PATHSEPARATOR = '/';
constexpr const char *PATHSEPARATORSTRING = "/";
#endif

enum {
EXIT_RESTART = 25262728 // (pseudo) Exit code to restart flexemu
};

#ifdef __cplusplus
namespace flx
{
extern char tolower_value(char ch);
extern char &tolower_ref(char &ch);
extern char toupper_value(char ch);
extern char &toupper_ref(char &ch);
extern void strlower(std::string& str);
extern void strupper(std::string& str);
extern std::string tolower(const std::string& src);
extern std::string tolower(std::string&& src);
extern std::string toupper(const std::string& src);
extern std::string toupper(std::string&& src);
extern std::string binstr(Byte x);
extern std::string hexstr(Byte x);
extern std::string hexstr(Word x);
extern std::string ascchr(char x, char default_value = '_');
extern char hex_digit(Byte x);
extern Byte hexval(char x, bool &isValid);

extern std::vector<std::string> split(const std::string &str, char delimiter,
        bool keepEmptyString, size_t max = std::numeric_limits<size_t>::max());
extern bool matches(const std::string &text, const std::string &pattern,
             bool ignorecase);
extern bool multimatches(const std::string &text,
                  const std::string &multipattern,
                  char delimiter /* = ';'*/,
                  bool ignorecase /* = false */);
#ifdef _WIN32
extern fs::path getExecutablePath();
#endif
extern fs::path getHomeDirectory();
extern void dumpSector(std::ostream &os, uint32_t indent_count,
                       const Byte *buffer, uint32_t size);
extern fs::path getFlexemuUserConfigPath();
extern fs::path getFlexemuConfigFile();
extern fs::path getFlexLabelFile();
extern std::string getHostName();
extern std::string updateFilename(std::string path,
        const std::string &defaultFilestem, const std::string &fileExtension);
extern bool isFlexFilename(const std::string &filename);

extern const char * const white_space;

// trim white spaces from end of string (right)
inline std::string rtrim(const std::string &str, const char* t = white_space)
{
    auto result(str);
    result.erase(result.find_last_not_of(t) + 1);
    return result;
}

inline std::string rtrim(std::string &&str, const char* t = white_space)
{
    auto result(std::move(str));
    result.erase(result.find_last_not_of(t) + 1);
    return result;
}

// trim white spaces from beginning of string (left)
inline std::string ltrim(const std::string &str, const char* t = white_space)
{
    auto result(str);
    result.erase(0, result.find_first_not_of(t));
    return result;
}

inline std::string ltrim(std::string &&str, const char* t = white_space)
{
    auto result(std::move(str));
    result.erase(0, result.find_first_not_of(t));
    return result;
}

// trim white spaces from both ends of string (left & right)
inline std::string trim(const std::string &str, const char* t = white_space)
{
    auto result(str);
    result.erase(result.find_last_not_of(t) + 1);
    result.erase(0, result.find_first_not_of(t));
    return result;
}

inline std::string trim(std::string &&str, const char* t = white_space)
{
    auto result(std::move(str));
    result.erase(result.find_last_not_of(t) + 1);
    result.erase(0, result.find_first_not_of(t));
    return result;
}

template<typename T> T reverseBytes(const T& value)
{
    union values_t
    {
        std::array<char, sizeof(T)> bytes;
        T value;
    } temp{};

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
        result = (result * 256U) | p[index];
    }

    return result;
}

// Write a big endian data value to buffer p.
template<typename T> void setValueBigEndian(Byte *p, T value)
{
    for (size_t index = sizeof(T); index != 0; --index)
    {
        p[index - 1] = value & 0xFFU;
        value >>= 8U;
    }
}

extern bool askForInput(const std::string &question,
                        const std::string &answers,
                        char default_answer);

// Locale independent conversion from numeric value to string.
// Use "C" locale to avoid any locale specific conversions.
template<typename T>
std::string toString(T value, bool &success)
{
   std::stringstream value_stream;

    value_stream.imbue(std::locale("C"));
    success = !(value_stream << value).fail();

    return value_stream.str();
}

// Locale independent conversion from string to numeric value.
// Use "C" locale to avoid any locale specific conversions.
template<typename T>
bool fromString(const std::string &str, T &value)
{
    std::stringstream value_stream(str);

    value_stream.imbue(std::locale("C"));

    return !(value_stream >> value).fail();
}

// template to create a string from a cstyle character array with given length.
// The cstyle array not necessarily has a terminating NUL, std::string
// constructors do not support this.
/* Template function needed to read from POD data types into a string. */
/* NOLINTNEXTLINE(modernize-avoid-c-arrays) */
template<size_t N> std::string getstr(const char (&array)[N])
{
    std::string result;
    result.reserve(N);
    auto iter = std::back_inserter(result);

    std::ignore = std::any_of(std::begin(array), std::end(array),
                              [&iter](const char &ch)
    {
        if (ch == '\0')
        {
            return true;
        }
        *(iter++) = ch;
        return false;
    });

    return result;
}

std::time_t to_time_t(fs::file_time_type file_time);
}
#endif /* ifdef __cplusplus */
#endif

