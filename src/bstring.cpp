/*
    bstring.cpp


    Basic class containing a string implementation
    Copyright (C) 1998-2005  W. Schwotzer

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
#include <limits.h>

#include "bstring.h"

const int BString::INVALID_STR         = -1;
const int BString::STRING_DEFAULT_SIZE = 16;


void BString::initForChar(const char c)
{
    sz = 2;
    str = new char[sz];
    str[0] = c;
    str[1] = '\0';
}

void BString::init(const char *s, int maxlen)
{
    if (maxlen == 0 || s == NULL)
    {
        initForChar('\0');
        return;
    }

    if (maxlen < 0)
    {
        sz = strlen(s) + 1;
    }
    else
    {
        sz = maxlen + 1;
    }

    if (sz < 2)
    {
        sz = 2;
    }

    char *pTmp = str;
    str = new char[sz];
    strncpy(str, s, sz - 1);
    str[sz - 1] = '\0';
    delete [] pTmp;
}

BString::BString(void) : str(NULL), sz(0)
{
    initForChar('\0');
}

BString::BString(const char c) : str(NULL), sz(0)
{
    initForChar(c);
}

BString::BString(const char *s, int maxlen) : str(NULL), sz(0)
{
    if (s == NULL)
    {
        initForChar('\0');
    }
    else
    {
        init(s, maxlen);
    }
}

BString::BString(const BString &s, int maxlen) : str(NULL), sz(0)
{
    init(s.c_str(), maxlen);
}

BString::~BString(void)
{
    delete [] str;
    sz = 0;
}

void FScat(const char *s1, const char *s2, BString &s3)
{
    if (s1 == NULL || s2 == NULL)
    {
        return;
    }

    int size;

    size = strlen(s1) + strlen(s2) + 1;
    char *newS = new char[size];
    strcpy(newS, s1);
    strcat(newS, s2);
    s3 = BString(newS);
    delete [] newS;
}

void BString::FSadd(const char *s1)
{
    if (s1 == NULL)
    {
        return;
    }

    int size = length() + strlen(s1) + 1;

    if (size <= capacity())
    {
        strcat(str, s1);
    }
    else
    {
        char *newS = new char[size];
        strcpy(newS, str);
        strcat(newS, s1);
        delete [] str;
        str = newS;
        sz = size;
    }
}

void BString::alloc(int s)
{
    if (s > sz)
    {
        sz = s;
        char *newS = new char[s];
        strcpy(newS, str);
        delete [] str;
        str = newS;
    }
}
void BString::alloc(const char *s)
{
    if (s == NULL)
    {
        return;
    }

    int size;

    size = strlen(s) + 1;

    if (size > sz)
    {
        sz = size;
        char *newS = new char[size];
        strcpy(newS, s);
        delete [] str;
        str = newS;
    }
    else
    {
        strcpy(str, s);
    }
}

void BString::upcase(void)
{
    char *p;

    p = &str[0];

    while (*p != '\0')
    {
        *p = toupper(*p);
        p++;
    }
}

void BString::downcase(void)
{
    char *p;

    p = &str[0];

    while (*p != '\0')
    {
        *p = tolower(*p);
        p++;
    }
}

void BString::at(unsigned int pos, int len, BString &s)
{
    if (pos >= strlen(str))
    {
        s = "";
    }
    else
    {
        s = BString(&str[pos], len);
    }
}

bool BString::operator < (const BString &s) const
{
    return strcmp(str, s.str)  < 0;
}

bool BString::operator <= (const BString &s) const
{
    return strcmp(str, s.str)  <= 0;
}

bool BString::operator == (const BString &s) const
{
    return strcmp(str, s.str)  == 0;
}

bool BString::operator == (const char *s) const
{
    if (s == NULL)
    {
        return false;
    }

    return strcmp(str, s)  == 0;
}

bool BString::operator != (const BString &s) const
{
    return strcmp(str, s.str)  != 0;
}

bool BString::operator != (const char *s) const
{
    if (s == NULL)
    {
        return false;
    }

    return strcmp(str, s)  != 0;
}

bool BString::operator >= (const BString &s) const
{
    return strcmp(str, s.str)  >= 0;
}

bool BString::operator > (const BString &s) const
{
    return strcmp(str, s.str)  > 0;
}

unsigned int BString::length(void) const
{
    return strlen(str);
}


