/*
    misc1.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2024  W. Schwotzer

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


#include "misc1.h"
#include "cistring.h"
#include <cctype>
#include <iostream>
#include <sstream>
#include <functional>
#include <limits>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef UNIX
#include <netdb.h>
#include <pwd.h>
#endif
#include "cvtwchar.h"
#include "flexerr.h"
#include "benv.h"
#include <array>
#include <string>
#include <fstream>
#include <algorithm>
#include <utility>
#include <regex>
#include <fmt/format.h>


#ifdef _WIN32
    static const char * const pathSeparators = "\\/";
#else
    static const char * const pathSeparators = "/";
#endif

const char * const RANDOM_FILE_LIST = "random";
const char * const RANDOM_FILE_LIST_NEW = ".random";
const char * const flx::white_space = " \t\n\r\f\v";

char flx::tolower_value(char ch)
 {
     if (ch >= 'A' && ch <= 'Z')
     {
         return static_cast<char>(ch + ('z' - 'Z'));
     }
     return ch;
 }

 char &flx::tolower_ref(char &ch)
 {
     if (ch >= 'A' && ch <= 'Z')
     {
         ch = static_cast<char>(ch + ('z' - 'Z'));
     }
     return ch;
 }

char flx::toupper_value(char ch)
 {
     if (ch >= 'a' && ch <= 'z')
     {
         return static_cast<char>(ch - ('z' - 'Z'));
     }
     return ch;
 }

 char &flx::toupper_ref(char &ch)
 {
     if (ch >= 'a' && ch <= 'z')
     {
         ch = static_cast<char>(ch - ('z' - 'Z'));
     }
     return ch;
 }

void flx::strlower(std::string& str)
{
    for (auto &ch : str)
    {
        tolower_ref(ch);
    }
}

void flx::strupper(std::string& str)
{
    for (auto &ch : str)
    {
        toupper_ref(ch);
    }
}

std::string flx::tolower(const std::string& src)
{
    std::string result;
    std::transform(src.cbegin(), src.cend(), std::back_inserter(result),
            tolower_value);

    return result;
}

std::string flx::tolower(std::string&& src)
{
    std::string result = std::move(src);

    std::for_each(result.begin(), result.end(), tolower_ref);

    return result;
}

std::string flx::toupper(const std::string& src)
{
    std::string result;
    std::transform(src.cbegin(), src.cend(), std::back_inserter(result),
            toupper_value);

    return result;
}

std::string flx::toupper(std::string&& src)
{
    std::string result = std::move(src);

    std::for_each(result.begin(), result.end(), toupper_ref);

    return result;
}

// Base 2 and Base 16 conversion functions
std::string flx::binstr(Byte x)
{
    std::string result("        ");

    for (int i = 7; i >= 0; --i)
    {
        result[i] = (x & 1U) ? '1' : '0';
        x >>= 1U;
    }

    return result;
}

inline char flx::hex_digit(Byte x)
{
    static const char *digits = "0123456789abcdef";

    return digits[x & 0x0FU];
}

std::string flx::hexstr(Byte x)
{
    std::string result("  ");

    result[1] = hex_digit(x);
    x >>= 4U;
    result[0] = hex_digit(x);

    return result;
}

std::string flx::hexstr(Word x)
{
    std::string result("    ");

    result[3] = hex_digit(static_cast<Byte>(x));
    x >>= 4U;
    result[2] = hex_digit(static_cast<Byte>(x));
    x >>= 4U;
    result[1] = hex_digit(static_cast<Byte>(x));
    x >>= 4U;
    result[0] = hex_digit(static_cast<Byte>(x));

    return result;
}

std::string flx::ascchr(char x)
{
    std::string result(" ");

    result[0] = ((x >= 0x20) && (x < 0x7f)) ? x : '.';

    return result;
}

// Check if 'text' matches the wildcard 'pattern'.
// Supported wildcard characters:
// *  matches 0 up to any number of arbitrary wildcard characters
// ?  matches exactly 1 arbitrary character
// An empty pattern matches an empty text
bool flx::matches(const std::string &text, const std::string &pattern,
                  bool ignorecase)
{
    if (pattern.find_first_of("*?[]") == std::string::npos)
    {
        if (ignorecase)
        {
            return flx::tolower(text) == flx::tolower(pattern);
        }
        return text == pattern;
    }

    static const int MAX = std::numeric_limits<int>::max();
    auto ipattern = pattern.cbegin();
    auto ilaststar = pattern.cend();
    auto itext = text.cbegin();
    char char_pat{};
    int min = 0;
    int max = 0;
    int minlaststar = 0;
    int any_char_count =  0;

    if (ipattern == pattern.cend() || itext == text.cend())
    {
        return ipattern == pattern.cend() && itext == text.cend();
    }

    while (itext != text.cend())
    {
        char char_txt = *itext;

        if (ignorecase)
        {
            char_txt = static_cast<char>(::tolower(char_txt));
        }

        while (any_char_count == 0 && ipattern != pattern.cend())
        {
            char_pat = *(ipattern++);

            if (char_pat == '*')
            {
                ilaststar = ipattern;
                minlaststar = min;
                max = MAX;
                continue;
            }

            if (char_pat == '?')
            {
                ++min;

                if (max < MAX)
                {
                    ++max;
                }

                continue;
            }

            if (ignorecase)
            {
                char_pat = static_cast<char>(::tolower(char_pat));
            }

            break;
        }

        if (char_txt == char_pat)
        {
            if (any_char_count < min || any_char_count > max)
            {
                if (ilaststar == pattern.cend())
                {
                    return false;
                }

                ipattern = ilaststar;
                min = minlaststar;
                max = MAX;
                any_char_count = 0;
                continue;
            }

            any_char_count = 0;
            min = max = 0;
        }
        else
        {
            ++any_char_count;

            if (any_char_count > max)
            {
                if (ilaststar == pattern.cend())
                {
                    return false;
                }

                ipattern = ilaststar;
                min = minlaststar;
                max = MAX;
                any_char_count = 0;
                continue;
            }
        }
        ++itext;
    }

    if (any_char_count < min || any_char_count > max)
    {
        return false;
    }

    bool only_stars =
        std::all_of(ipattern, pattern.cend(), [](char ch){ return ch == '*'; });
    return ipattern == pattern.cend() || only_stars;
}

bool flx::multimatches(const std::string &text, const std::string &multipattern,
                       char delimiter /* = ';'*/, bool ignorecase /* = false */)
{
    const auto patterns = flx::split(multipattern, delimiter, true);

    return std::any_of(patterns.cbegin(), patterns.cend(),
            [&](const std::string &pattern)
            {
                return flx::matches(text, pattern, ignorecase);
            });
}

#ifdef _WIN32
std::string flx::getExecutablePath()
{
    wchar_t path[MAX_PATH];

    HMODULE hModule = GetModuleHandle(nullptr);
    std::string retval;

    if (hModule != nullptr)
    {
        size_t index;

        GetModuleFileName(hModule, path, MAX_PATH);
        index = wcslen(path);
        while (index > 0)
        {
            if (path[index - 1] == wchar_t(PATHSEPARATOR))
            {
                path[index - 1] = '\0';
                break;
            }
            --index;
        }
        retval = ConvertToUtf8String(path);
    }

    return retval;
}
#endif

#ifdef UNIX
std::string flx::getHomeDirectory()
{
    std::string result;

    if (!BEnvironment::GetValue("HOME", result))
    {
        struct passwd *pwd = getpwuid(getuid());
        if (pwd == nullptr)
        {
            throw FlexException(FERR_ERROR_IN_SYSTEM_CALL, errno,
                                std::string("getpwuid"));
        }
        result = pwd->pw_dir;
    }

    return result;
}
#endif
#ifdef _WIN32
std::string flx::getHomeDirectory()
{
    std::string homeDrive;
    std::string homePath;

    if (!BEnvironment::GetValue("HOMEDRIVE", homeDrive))
    {
        return "";
    }
    if (!BEnvironment::GetValue("HOMEPATH", homePath))
    {
        return "";
    }

    return homeDrive + homePath;
}
#endif

std::string flx::getTempPath()
{
#ifdef _WIN32
    wchar_t tempPath[MAX_PATH];

    if (!GetTempPath(MAX_PATH, tempPath))
    {
       throw FlexException(GetLastError(),
                           std::string("In function GetTempPath"));
    }

    return ConvertToUtf8String(tempPath);
#else
    // On POSIX compliant file systems /tmp has to be available
    return "/tmp";
#endif
}

std::string flx::getFileName(const std::string &path)
{
    auto pos = path.find_last_of(pathSeparators);

    if (pos != std::string::npos)
    {
        if (pos == (path.size() - 1))
        {
            // If path == "/", return "/".
            // If path is "/foo/bar/" return ".".
            return pos == 0 ? path : ".";
        }

        // If path == "/foo/bar.txt" return "bar.txt".
        // If path == "/foo/bar" return "bar".
        return path.substr(pos + 1);
    }

    // If path == "bar.txt" return "bar.txt".
    // If path == "." return ".".
    // If path == ".." return "..".
    return path;
}

std::string flx::getParentPath(const std::string &path)
{
    auto pos = path.find_last_of(pathSeparators);

    if (pos != std::string::npos)
    {
        if (pos == (path.size() - 1))
        {
            // If path == "/", return "".
            if (pos == 0)
            {
                return "";
            }
        }

        // If path is "/foo/bar/" return "/foo/bar".
        // If path == "/foo/bar.txt" return "/foo".
        // If path == "/foo/bar" return "/foo".
        return path.substr(0, pos);
    }

    // If path == "bar.txt" return "".
    // If path == "." return "".
    // If path == ".." return "".
    return "";
}

std::string flx::toAbsolutePath(const std::string &path)
{
    if (flx::isAbsolutePath(path))
    {
        return path;
    }

    auto directory = flx::getCurrentPath();
    if (!flx::endsWithPathSeparator(path))
    {
        directory += PATHSEPARATORSTRING;
    }

    return directory + path;
}

std::string flx::getCurrentPath()
{
#ifdef _WIN32
    DWORD size = GetCurrentDirectory(0, nullptr);
    auto buffer = std::unique_ptr<wchar_t[]>(new wchar_t[size]);

    if (GetCurrentDirectory(size, buffer.get()) > 0)
    {
        return ConvertToUtf8String(buffer.get());
    }
#else
    std::array<char, PATH_MAX> buffer{};

    if (getcwd(buffer.data(), buffer.size()))
    {
        return {buffer.data()};
    }
#endif

    return {};
}

bool flx::isAbsolutePath(const std::string &path)
{
#ifdef _WIN32
    return path.size() >= 2 &&
        ((isalpha(path[0]) && path[1] == ':') ||
         (path[0] == '\\' && path[1] == '\\') ||
         (path[0] == '/' && path[1] == '/'));
#else
    return !path.empty() && path[0] == '/';
#endif
}

bool flx::isPathsEqual(const std::string &path1, const std::string &path2)
{
    if (path1.size() != path2.size())
    {
        return false;
    }

#ifdef _WIN32
    // TODO: utf-8 case insensitive compare
    // std::string with different type traits can not be copy-constructed.
    // A conversion to const char * is needed. False-positive to be ignored.
    // NOLINTNEXTLINE(readability-redundant-string-cstr)
    ci_string ci_path1(path1.c_str());
    return ci_path1.compare(path2.c_str()) == 0;
#else
    return path1.compare(path2) == 0;
#endif
}

std::string flx::getFileExtension(const std::string &path)
{
    std::string fileName = getFileName(path);

    if (!path.empty())
    {
        auto ch = path.at(path.size() - 1);

        if (ch == PATHSEPARATOR
#ifdef _WIN32
            || ch == '/'
#endif
        )
        {
            return "";
        }
    }

    auto pos = fileName.find_last_of('.');

    if (pos != std::string::npos && fileName != "." && fileName != "..")
    {
        // If fileName == "bar.txt" return ".txt".
        // If fileName == "foo.bar.txt" return ".txt".
        return fileName.substr(pos);
    }

    // If fileName == "." or ".." return "".
    // If fileName contains no dot return "".
    return "";
}

std::string flx::getFileStem(const std::string &path)
{
    std::string fileName = flx::getFileName(path);

    if (fileName == "." || fileName == "..")
    {
        return fileName;
    }

    auto pos = fileName.find_last_of('.');

    if (pos != std::string::npos)
    {
        if (pos == 0)
        {
            // If path == ".txt" return "".
            return "";
        }
        // If path == "bar.txt" return "bar".
        // If path == "foo.bar.txt" return "foo.bar".
        return fileName.substr(0, pos);
    }

    // If path == "bar" return "bar".
    return fileName;
}

std::string flx::getHostName()
{
    std::string dnsHostName;

#ifdef _WIN32
    wchar_t hostname[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(hostname) / sizeof(hostname[0]);

    if (GetComputerNameEx(ComputerNameDnsFullyQualified, hostname, &size))
    {
        dnsHostName = ConvertToUtf8String(hostname);
    }
#else
    std::array<char, _POSIX_HOST_NAME_MAX> hostname{};
    if (gethostname(hostname.data(), hostname.size()) == 0)
    {
        struct hostent *host_entry = gethostbyname(hostname.data());
        if (host_entry != nullptr)
        {
            dnsHostName = host_entry->h_name;
        }
        else
        {
            dnsHostName = std::string(hostname.data());
        }
    }
#endif

    return dnsHostName;
}

bool flx::endsWithPathSeparator(const std::string &path)
{
    auto pos = path.find_last_of(pathSeparators);

    return (pos != std::string::npos && (pos == (path.size() - 1)));
}

std::string flx::getFlexemuSystemConfigFile()
{
    static const auto flexemuConfigFile = std::string("flexemu.conf");

#ifdef _WIN32
    return getExecutablePath() + PATHSEPARATORSTRING + flexemuConfigFile;
#endif
#ifdef UNIX
    return std::string(F_SYSCONFDIR) + PATHSEPARATORSTRING + flexemuConfigFile;
#endif
}

std::string flx::getFlexLabelFile()
{
    static const auto flexLabelFile = std::string("flexlabl.conf");

#ifdef _WIN32
    return getExecutablePath() + PATHSEPARATORSTRING + flexLabelFile;
#endif
#ifdef UNIX
    return std::string(F_SYSCONFDIR) + PATHSEPARATORSTRING + flexLabelFile;
#endif
}

#ifdef _WIN32
// uses its own implementation of getopt
int optind = 1;
int opterr = 1;
int optopt = 0;
const char *optarg = nullptr;
static int argvind = 1;

static void next_opt(char *const argv[]);

int getopt(int argc, char *const argv[], const char *optstr)
{
    int opt;
    const char *popt;

    if (optind < argc)
    {
        for (popt = optstr; *popt != '\0'; ++popt)
        {
            opt = *popt;
            if (argv[optind][0] == '-' && argv[optind][argvind] == opt)
            {
                // found option
                if (*(popt + 1) == ':')
                {
                    // option has an argument
                    if (argv[optind][argvind + 1] != '\0')
                    {
                        optarg = &argv[optind++][argvind + 1];
                        argvind = 1;
                        return opt;
                    }
                    else
                    {
                        if (++optind < argc)
                        {
                            optarg = &argv[optind++][0];
                            return opt;
                        }
                        else
                        {
                            // missing argument
                            if (opterr)
                            {
                                std::cerr << argv[0] << ": option requires "
                                    "an argument -- '" << opt << "'\n";
                            }
                            optopt = opt;
                            return '?';
                        }
                    }
                }
                else
                {
                    // option has no argument
                    next_opt(argv);
                    return opt;
                }
            }
            else if (argv[optind][0] != '-')
            {
                // First argument without option prefix '-'
                return -1;
            }
        }

        // Unknown option
        if (opterr)
        {
            std::cerr << argv[0] << ": illegal option -- '" <<
                    argv[optind][argvind] << "'\n";
        }
        optopt = argv[optind][argvind];
        next_opt(argv);
        return '?';
    }

    return -1;
}

void next_opt(char *const argv[])
{
    if (argv[optind][argvind + 1] == '\0')
    {
        ++optind;
    }
    else
    {
        ++argvind;
    }
}

#endif

void flx::dumpSector(std::ostream &os, uint32_t indent_count,
                const Byte *buffer, uint32_t size)
{
    uint32_t i;

    std::string indent(indent_count, ' ');
    os << indent << "  ";
    for (i = 0; i < 16; ++i)
    {
        os << fmt::format(" -{:X}", i);
    }
    os << "\n";

    for (i = 0; i < size; i += 16)
    {
        uint32_t j;

        os << fmt::format("{}{:X}-", indent, i >> 4U);
        for (j = 0; j < 16; ++j)
        {
            os << fmt::format(" {:02X}", static_cast<Word>(buffer[i + j]));
        }
        os << " ";
        for (j = 0; j < 16; ++j)
        {
            const char ch = static_cast<char>(buffer[i + j] & 0x7FU);

            os << ((ch >= ' ' && ch <= '~') ? ch : '_');
        }
        os << "\n";
    }
}

void flx::hex_dump(std::ostream &os, const char *buffer, unsigned count)
{
    const char *p = &buffer[0];
    unsigned i = 0;

    for (; i < count; ++i)
    {
        char ch = *(p++);
        os << fmt::format("{:02X} ", ch);
        if ((i & 0x0FU) == 0x0FU)
        {
            os << "\n";
        }
    }
    if ((i & 0x0FU) != 0U)
    {
        os << "\n";
    }
}

bool flx::askForInput(const std::string &question, const std::string &answers,
                      char default_answer)
{
    char input = ' ';
    char dummy;

    if (answers.empty())
    {
        return false;
    }

    while(answers.find_first_of(input) == std::string::npos)
    {
        if (default_answer == '?')
        {
            std::cout << question << " [";
            for (auto answer : answers)
            {
                if (answer == answers[0])
                {
                    std::cout << static_cast<char>(::toupper(answer));
                }
                else
                {
                    std::cout << "/" << answer;
                }
            }
            std::cout << "]: ";

            // Ask user for an input. One character is sufficient.
            input = ' ';
            while (input == ' ' || input == '\t')
            {
                std::cin >> std::noskipws >> input;
            }

            dummy = input;
            if (input == '\n')
            {
                input = answers[0];
            }

            while (dummy != '\n')
            {
                // Read input until end of line.
                std::cin >> std::noskipws >> dummy;
            }
        }
        else
        {
            // Use default_answer as input.
            input = default_answer;
        }
    }

    // Return true if only Return was entered or the first answer character.
    return input == '\n' || ::tolower(input) == answers.at(0);
}

std::vector<std::string> flx::split(const std::string &str, char delimiter,
        bool keepEmptyString)
{
    std::vector<std::string> result;
    std::string::size_type start_pos = 0;
    std::string::size_type next_pos = 0;

    while (next_pos != std::string::npos)
    {
        next_pos = str.find(delimiter, start_pos);
        if (next_pos != std::string::npos)
        {
            if (keepEmptyString || (start_pos != next_pos))
            {
                result.emplace_back(
                        str.substr(start_pos, next_pos - start_pos));
            }
            start_pos = next_pos + 1;
        }
        else
        {
            if (keepEmptyString || (start_pos != str.size()))
            {
                result.emplace_back(str.substr(start_pos));
            }
        }
    }

    return result;
}

// Check if filename contains a valid FLEX filename.
// On Unix only lowercase filenames are allowed.
// On success return true otherwise false.
// The rules to be checked:
// - filename and extension are separated by a dot.
// filename:
// - First character is a-z or A-Z
// - Next up to 7 characters are a-z, A-Z, 0-9, _ or -
// extension:
// - First character is a-z or A-Z
// - Next up to 2 characters are a-z, A-Z, 0-9, _ or -
/*
    Some examples:

    allowed:        x.a xx.a xxxxxxxx.a x xx xxxxxxxx
    not allowed:    x. .a xx. xxxxxxxxx.a X.a xxxxxxxxX.a
*/

bool flx::isFlexFilename(const std::string &filename)
{
#ifdef UNIX
    static std::regex re("[a-z][a-z0-9_-]{0,7}\\.[a-z][a-z0-9_-]{0,2}");
#else
    static const std::regex re("[a-zA-Z][a-zA-Z0-9_-]{0,7}\\.[a-zA-Z][a-zA-Z0-9_-]{0,2}");
#endif

    return std::regex_match(filename, re);
}

// For a given path replace or append fileExtension.
// prepend filestem with defaultFilestem if it begins with a dot.
// insert filestem if it is empty.
// . and .. are treated as empty filestem.
std::string flx::updateFilename(std::string path,
        const std::string &defaultFilestem, const std::string &fileExtension)
{
    auto pIdx = path.find_last_of(PATHSEPARATOR);
    auto index = path.find_last_of('.');
    auto stem = flx::getFileStem(path);

    if (stem == "." || stem == "..")
    {
        if (pIdx != std::string::npos)
        {
            path = path.substr(0U, pIdx + 1);
        }
        else
        {
            path.clear();
        }
        index = std::string::npos;
        pIdx = path.find_last_of(PATHSEPARATOR);
        stem.clear();
    }

    if (index != std::string::npos &&
            (pIdx == std::string::npos || index > pIdx))
    {
        if (stem.empty())
        {
            path = path.substr(0U, index) + defaultFilestem + fileExtension;
        }
        else
        {
            path = path.substr(0U, index) + fileExtension;
            const auto newStem = flx::getFileStem(path);
            if (!newStem.empty() && newStem[0] == '.')
            {
                auto newIdx = path.find_last_of(PATHSEPARATOR);
                if (newIdx != std::string::npos)
                {
                    path.insert(newIdx + 1, defaultFilestem, 0U);
                }
                else
                {
                    path.insert(0U, defaultFilestem, 0U);
                }
            }
        }
    }
    else if (path.empty() ||
            (pIdx != std::string::npos && pIdx == path.size() - 1))
    {
        path = path + defaultFilestem + fileExtension;
    }
    else if (pIdx == std::string::npos ||
            (pIdx != std::string::npos && pIdx < path.size() - 1))
    {
        path = path + fileExtension;
    }

    return path;
}

