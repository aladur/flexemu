/*
    misc1.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2019  W. Schwotzer

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
#include <ctype.h>
#include <stdio.h>
#ifdef UNIX
#include <linux/param.h>
#include <netdb.h>
#endif
#include "bfileptr.h"
#include "cvtwchar.h"
#include "flexerr.h"

#ifdef _WIN32
    static const char *pathSeparators = "\\/";
#else
    static const char *pathSeparators = "/";
#endif

const char* white_space = " \t\n\r\f\v";

const char *gMemoryAllocationErrorString =
    "Bad memory allocation.\n"
    "Close other applications\n"
    "and try again.";

int copyFile(const char *srcPath, const char *destPath)
{
    BFilePtr sFp(srcPath,  "rb");
    BFilePtr dFp(destPath, "wb");
    int c;

    if (sFp == nullptr || dFp == nullptr)
    {
        return 0;
    }

    while ((c = fgetc(sFp)) != EOF)
    {
        fputc(c, dFp);
    }

    return 1;
}

void strupper(char *pstr)
{
    while (*pstr)
    {
        *pstr = static_cast<char>(toupper(*pstr));
        pstr++;
    }
} // strupper


void strlower(char *pstr)
{
    while (*pstr)
    {
        *pstr = static_cast<char>(tolower(*pstr));
        pstr++;
    }
} // strlower

// Base 2 and Base 16 conversion functions
char *binstr(Byte x)
{
    static char             tmp[9] = "        ";

    for (SWord i = 7; i >= 0; --i)
    {
        tmp[i] = (x & 1) + '0';
        x >>= 1;
    }

    return tmp;
}

static char hex_digit(Byte x)
{
    x &= 0x0f;

    if (x <= 9)
    {
        return '0' + x;
    }
    else
    {
        return 'a' + x - 10;
    }
}

char *hexstr(Byte x)
{
    static char             tmp[3] = "  ";

    tmp[1] = hex_digit(x);
    x >>= 4;
    tmp[0] = hex_digit(x);

    return tmp;
}

char *hexstr(Word x)
{
    static char             tmp[5] = "    ";

    tmp[3] = hex_digit((Byte)x);
    x >>= 4;
    tmp[2] = hex_digit((Byte)x);
    x >>= 4;
    tmp[1] = hex_digit((Byte)x);
    x >>= 4;
    tmp[0] = hex_digit((Byte)x);

    return tmp;
}

std::string tohexstr(Byte x)
{
    return std::string(hexstr(x));
}

std::string tohexstr(Word x)
{
    return std::string(hexstr(x));
}

char *ascchr(Byte x)
{
    static char             tmp[2] = " ";

    x &= 0x7f;
    tmp[0] = ((x >= 0x20) && (x < 0x7f)) ? x : '.';

    return tmp;
}

#if defined(__GNUC__) && !(defined(__MINGW32) || defined (__CYGWIN32) )
int stricmp(const char *string1, const char *string2)
{
    unsigned int i;

    for (i = 0; i < strlen(string1); i++)
    {
        if (tolower(*(string1 + i)) < tolower(*(string2 + i)))
        {
            return -1;
        }

        if (tolower(*(string1 + i)) > tolower(*(string2 + i)))
        {
            return 1;
        }

        if (!*string1)
        {
            return 0;
        }
    }

    return 0;
} // stricmp
#endif

bool matches(const char *text, const char *pattern,
             bool ignorecase /* = false */)
{
    const char *p_pat = pattern;
    const char *p_src = text;
    char char_pat     = '*'; // prepare for first while loop
    int min = 0;
    int max = 0;
    int notmatched =  0;

    if (pattern == nullptr)
    {
        return false;
    }

    while (*p_src != '\0')
    {
        char char_src = *p_src;

        if (ignorecase)
        {
            char_src = static_cast<char>(tolower(char_src));
        }

        while (notmatched == 0 && char_pat != '\0')
        {
            char_pat = *p_pat;
            p_pat++;

            if (char_pat == '*')
            {
                // wildchard for any char
                max = INT_MAX;
                continue;
            }
            else if (char_pat == '?')
            {
                // wildchard for exactly one char
                min++;

                if (max < INT_MAX)
                {
                    max++;
                }

                continue;
            }
            else if (char_pat != '\0')
            {
                // any other character
                if (ignorecase)
                {
                    char_pat = static_cast<char>(tolower(char_pat));
                }

                break;
            }
        }

        if (char_src == char_pat)
        {
            if (notmatched < min || notmatched > max)
            {
                return false;
            }

            notmatched = 0;
            min = max = 0;
        }
        else
        {
            notmatched++;

            if (notmatched > max)
            {
                return false;
            }
        }

        p_src++;
    }

    if (notmatched < min || notmatched > max)
    {
        return false;
    }

    return (char_pat == '\0' && notmatched > 0) || //pattern ends with ? or *
           (*p_pat == '\0' && notmatched == 0); // pattern end with any char
}

bool multimatches(const char *text, const char *multipattern,
                  const char delimiter /* = ';'*/,
                  bool ignorecase /* = false */)

{
    int pos;

    if (multipattern == nullptr)
    {
        return false;
    }

    pos = 0;

    while (multipattern[pos] != '\0')
    {
        int begin = pos;

        while (multipattern[pos] != '\0' && (multipattern[pos] != delimiter))
        {
            pos++;
        }

        std::string pattern = std::string(&multipattern[begin], pos - begin);

        if (matches(text, pattern.c_str(), ignorecase))
        {
            return true;
        }

        if (multipattern[pos] == delimiter)
        {
            pos++;
        }
    }

    return false;
}

#ifdef _WIN32
std::string getExecutablePath()
{
#ifdef UNICODE
    WCHAR path[MAX_PATH];
#else
    CHAR path[MAX_PATH];
#endif

    HMODULE hModule = GetModuleHandle(nullptr);
    std::string retval;

    if (hModule != nullptr)
    {
        size_t index;

        GetModuleFileName(hModule, path, MAX_PATH);
#ifdef UNICODE
        index = wcslen(path);
#else
        index = strlen(path);
#endif
        while (index > 0)
        {
            if (path[index - 1] == PATHSEPARATOR)
            {
                path[index - 1] = '\0';
                break;
            }
            --index;
        }
#ifdef UNICODE
        retval = ConvertToUtf8String(path);
#else
        retval = path;
#endif
    }

    return retval;
}
#endif

std::string getTempPath()
{
#ifdef _WIN32
#ifdef UNICODE
    wchar_t tempPath[MAX_PATH];

    if (!GetTempPath(MAX_PATH, tempPath))
    {
       throw FlexException(GetLastError(),
                           std::string("In function GetTempPath"));
    }

    return ConvertToUtf8String(tempPath);
#else
    char tempPath[MAX_PATH];

    if (!GetTempPath(MAX_PATH, tempPath))
    {
       throw FlexException(GetLastError(),
                           std::string("In function GetTempPath"));
    }

    return std::string(tempPath);
#endif
#else
    // On POSIX compliant file systems /tmp has to be available
    return "/tmp";
#endif
}

std::string getFileName(const std::string &path)
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

std::string getParentPath(const std::string &path)
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
        return path.substr(pos - 1);
    }

    // If path == "bar.txt" return "".
    // If path == "." return "".
    // If path == ".." return "".
    return "";
}

std::string getCurrentPath()
{
    std::string cwd;
#ifdef _WIN32
#ifdef UNICODE
    DWORD size = GetCurrentDirectory(0, nullptr);
    wchar_t *buffer = new wchar_t[size];

    if (GetCurrentDirectory(size, buffer) > 0)
    {
        cwd = ConvertToUtf8String(buffer);
    }
    delete [] buffer;
#else
    DWORD size = GetCurrentDirectory(0, nullptr);
    char *buffer = new char[size];

    if (GetCurrentDirectory(size, buffer) > 0)
    {
        cwd = buffer;
    }
    delete [] buffer;
#endif
#else
    char buffer[PATH_MAX];

    if (getcwd(buffer, sizeof(buffer)))
    {
        cwd = buffer;
    }
#endif

    return cwd;
}

bool isAbsolutePath(const std::string &path)
{
#ifdef _WIN32
    return path.size() >= 2 && isalpha(path[0]) && path[1] == ':';
#else
    return path.size() >= 1 && path[0] == '/';
#endif
}

std::string getFileExtension(const std::string &path)
{
    std::string fileName = getFileName(path);

    auto pos = fileName.find_last_of(".");

    if (pos != std::string::npos)
    {
        // If path == "bar.txt" return ".txt".
        // If path == "foo.bar.txt" return ".txt".
        return fileName.substr(pos);
    }

    // If path == "." return "".
    // If path == ".." return "".
    return "";
}

std::string getFileStem(const std::string &path)
{
    std::string fileName = getFileName(path);

    auto pos = fileName.find_last_of(".");

    if (pos != std::string::npos)
    {
        if (pos == 0)
        {
            // If path == "." return "".
            // If path == ".." return "".
            if (fileName.size() <= 2 && fileName.at(0) == '.')
            {
                return fileName;
            }

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

std::string getHostName()
{
    std::string dnsHostName;

#ifdef _WIN32
#ifdef UNICODE
    wchar_t hostname[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(hostname) / sizeof(hostname[0]);

    if (GetComputerNameEx(ComputerNameDnsFullyQualified, hostname, &size))
    {
        dnsHostName = ConvertToUtf8String(hostname);
    }
#else
    char hostname[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(hostname) / sizeof(hostname[0]);

    if (GetComputerNameEx(ComputerNameDnsFullyQualified, hostname, &size))
    {
        dnsHostName = hostname;
    }
#endif
#else
    char hostname[MAXHOSTNAMELEN];
    if (gethostname(hostname, sizeof(hostname)) == 0)
    {
        struct hostent *host_entry = gethostbyname(hostname);
        if (host_entry != nullptr)
        {
            dnsHostName = host_entry->h_name;
        }
        else
        {
            dnsHostName = hostname;
        }
    }
#endif

    return dnsHostName;
}

bool endsWithPathSeparator(const std::string &path)
{
    auto pos = path.find_last_of(pathSeparators);

    return (pos != std::string::npos && (pos == (path.size() - 1)));
}

std::string getFlexemuSystemConfigFile()
{
    static const auto flexemuConfigFile = std::string("flexemu.conf");

#ifdef _WIN32
    return getExecutablePath() + PATHSEPARATORSTRING + flexemuConfigFile;
#endif
#ifdef __LINUX
    return std::string(F_SYSCONFDIR) + PATHSEPARATORSTRING + flexemuConfigFile;
#endif
}

sOptions::sOptions() :
    isRamExtension(false), isHiMem(false), isFlexibleMmu(false),
    isEurocom2V5(false),
    use_undocumented(false), useRtc(false), term_mode(false), reset_key(0),
    frequency(0.0f)
{
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
                                fprintf(stderr, "%s: option requires an "
                                        "argument -- '%c'\n",
                                        argv[0], opt);
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
            fprintf(stderr, "%s: illegal option -- '%c'\n",
                    argv[0], argv[optind][argvind]);
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
