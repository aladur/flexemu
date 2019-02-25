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
#include "bfileptr.h"
#include "cvtwchar.h"

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
#ifdef _WIN32
    const char *pathSeparators = "\\/";
#else
    const char *pathSeparators = "/";
#endif
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
int opterr = 0;
int optopt = 0;
const char *optarg = nullptr;

int getopt(int argc, char *const argv[], const char *optstr)
{
    int     i;

    while (1)
    {
        char    opt;

        optarg = optstr;

        for (i = 1; i < optind; i++)
        {
            if (*(++optarg) == ':')
            {
                optarg++;
            }
        }

        if ((opt = *optarg) == '\0')
        {
            return -1;
        }

        optind++;
        i = 1;

        while (i < argc)
        {
            if (argv[i][0] == '-' && argv[i][1] == opt)
            {
                // found option
                if (*(optarg + 1) == ':')
                {
                    // option has a parameter
                    if (argv[i][2] != '\0')
                    {
                        optarg = &argv[i][2];
                        return opt;
                    }
                    else
                    {
                        if (++i < argc)
                        {
                            optarg = &argv[i][0];
                            return opt;
                        }
                        else
                        {
                            break;
                        }
                    } // else
                }
                else
                    // option has no parameter
                {
                    return opt;
                }
            } // if

            i++;
        } // while
    } // while

    return -1;
} // getopt
#endif
