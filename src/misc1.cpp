/*
    misc1.cpp


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


#include "misc1.h"
#include <ctype.h>
#include <stdio.h>
#include "bfileptr.h"

const char *gMemoryAllocationErrorString =
    "Bad memory allocation.\n"
    "Close other applications\n"
    "and try again.";

int copyFile(const char *srcPath, const char *destPath)
{
    BFilePtr sFp(srcPath,  "rb");
    BFilePtr dFp(destPath, "wb");
    int c;

    if (sFp == NULL || dFp == NULL)
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
        *pstr = toupper(*pstr);
        pstr++;
    }
} // strupper


void strlower(char *pstr)
{
    while (*pstr)
    {
        *pstr = tolower(*pstr);
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

    if (pattern == NULL)
    {
        return false;
    }

    while (*p_src != '\0')
    {
        char char_src = *p_src;

        if (ignorecase)
        {
            char_src = tolower(char_src);
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
                    char_pat = tolower(char_pat);
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

    if (multipattern == NULL)
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

