/*
    misc1.cpp


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


#if defined(UNIX) || defined(USE_CMAKE)
#include "config.h"
#else
#include "confignt.h"
#endif
#include "typedefs.h"
#include "misc1.h"
#include "cistring.h"
#include "flexerr.h"
#include "benv.h"
#ifdef UNIX
#include <netdb.h>
#include <pwd.h>
#endif
#include "warnoff.h"
#include <fmt/format.h>
#include "warnon.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef _WIN32
#include "cvtwchar.h"
#endif
#include <cctype>
#include <cerrno>
#include <cstdint>
#include <cstddef>
#include <ctime>
#include <limits>
#include <utility>
#include <regex>
#include <string>
#include <array>
#include <vector>
#include <iostream>
#include <iterator>
#include <chrono>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

const char * const RANDOM_FILE_LIST = u8"random";
const char * const RANDOM_FILE_LIST_NEW = u8".random";
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
    constexpr const char *digits = "0123456789ABCDEF";

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

std::string flx::ascchr(char x, char default_value)
{
    std::string result(" ");

    result[0] = ((x >= ' ') && (x <= '~')) ? x : default_value;

    return result;
}

Byte flx::hexval(char x, bool &isValid)
{
    if (x >= '0' && x <= '9')
    {
        isValid = true;
        return static_cast<Byte>(x - '0');
    }

    toupper_ref(x);
    if (x >= 'A' && x <= 'F')
    {
        isValid = true;
        return static_cast<Byte>(x - 'A' + 10U);
    }

    isValid = false;
    return '\0';
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

    constexpr const int MAX = std::numeric_limits<int>::max();
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
fs::path flx::getExecutablePath()
{
    wchar_t path[MAX_PATH];

    HMODULE hModule = GetModuleHandle(nullptr);
    fs::path retval;

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
        retval = path;
    }

    return retval;
}
#endif

#ifdef UNIX
fs::path flx::getHomeDirectory()
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
#else
fs::path flx::getHomeDirectory()
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

    return fs::u8path(homeDrive) / fs::u8path(homePath);
}
#endif

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

fs::path flx::getFlexemuUserConfigPath()
{
    std::string configPath;
    fs::path result = (BEnvironment::GetValue("XDG_CONFIG_HOME", configPath) &&
            !configPath.empty()) ?
        fs::u8path(configPath) : getHomeDirectory();

    result /= fs::path(u8".config") / u8"flexemu";

    return result;
}

fs::path flx::getFlexemuConfigFile()
{
    constexpr const auto * const flexemuConfigFile = u8"flexemu.conf";
    static const auto userPath = getFlexemuUserConfigPath() / flexemuConfigFile;

    if (fs::exists(userPath))
    {
        return userPath;
    }

#ifdef _WIN32
    return getExecutablePath() / flexemuConfigFile;
#else
    return fs::path(F_SYSCONFDIR) / flexemuConfigFile;
#endif
}

fs::path flx::getFlexLabelFile()
{
    constexpr const auto * const flexLabelFile = u8"flexlabl.conf";
    static const auto userPath = getFlexemuUserConfigPath() / flexLabelFile;

    if (fs::exists(userPath))
    {
        return userPath;
    }

#ifdef _WIN32
    return getExecutablePath() / flexLabelFile;
#else
    return fs::path(F_SYSCONFDIR) / flexLabelFile;
#endif
}

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
        bool keepEmptyString, size_t max)
{
    std::vector<std::string> result;
    std::string::size_type start_pos = 0;
    std::string::size_type next_pos = 0;

    while (next_pos != std::string::npos)
    {
        if (result.size() >= max)
        {
            result.emplace_back(str.substr(next_pos));
            break;
        }

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
    auto stem = fs::u8path(path).stem().u8string();

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
            const auto newStem = fs::u8path(path).stem().u8string();
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

// Convert file time to time_t.
std::time_t flx::to_time_t(fs::file_time_type file_time)
{
    using namespace std::chrono;

    const auto duration =
        file_time - fs::file_time_type::clock::now() + system_clock::now();
    const auto sys_time = time_point_cast<system_clock::duration>(duration);

    return system_clock::to_time_t(sys_time);
}
