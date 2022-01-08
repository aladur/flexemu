/*
    benv.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2022  W. Schwotzer

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
#include <stdio.h>
#include <locale>
#include <algorithm>
#include "benv.h"
#include "cvtwchar.h"


BEnvironment::BEnvironment()
{
}

BEnvironment::~BEnvironment()
{
}

bool BEnvironment::RemoveKey(const char *key)
{
    std::string upperKey(key);

    strupper(upperKey);
#ifdef _WIN32
    SetEnvironmentVariable(ConvertToUtf16String(upperKey).c_str(), nullptr);
#endif
#ifdef UNIX
#if (HAVE_DECL_UNSETENV==1)
    unsetenv(upperKey.c_str());
#endif
#endif
    return true;
}

bool BEnvironment::SetValue(const char *key, const char *value)
{
    std::string upperKey(key);

    strupper(upperKey);
#ifdef _WIN32
    return (SetEnvironmentVariable(
        ConvertToUtf16String(upperKey).c_str(),
        ConvertToUtf16String(value).c_str()) != 0);
#endif
#ifdef UNIX
#if (HAVE_DECL_SETENV==1)
    return (setenv(upperKey.c_str(), value, 1) == 0);
#else
    return false;
#endif
#endif
}

bool BEnvironment::SetValue(const char *key, int value)
{
    char str[32];
    std::string upperKey(key);

    strupper(upperKey);
    sprintf(str, "%i", value);
#ifdef _WIN32
    return (SetEnvironmentVariable(
        ConvertToUtf16String(upperKey).c_str(),
        ConvertToUtf16String(str).c_str()) != 0);
#endif
#ifdef UNIX
#if (HAVE_DECL_SETENV==1)
    return (setenv(upperKey.c_str(), str, 1) == 0);
#else
    return false;
#endif
#endif
}

/*
bool BEnvironment::GetValue(const char *key, char **pValue)
{
    std::string upperKey(key);

    strupper(upperKey);
#ifdef _WIN32
    return (SetEnvironmentVariable(
        ConvertToUtf16String(upperKey).c_str(),
        ConvertToUtf16String(str).c_str()) != 0);
#endif
#ifdef UNIX
    return (*pValue = getenv(upperKey));
#endif
}
*/

bool BEnvironment::GetValue(const char *key, std::string &value)
{
    std::string upperKey(key);
    bool ret = false;

    strupper(upperKey);
#ifdef _WIN32
    const auto wUpperKey(ConvertToUtf16String(upperKey));
    auto size = GetEnvironmentVariable(wUpperKey.c_str(), nullptr, 0);
    if (size)
    {
        auto p = new wchar_t[size + 1U];

        if (GetEnvironmentVariable(wUpperKey.c_str(), p, size + 1U))
        {
            value = ConvertToUtf8String(p);
            ret = true;
        }

        delete [] p;
    }
#endif
#ifdef UNIX
    char* p;

    if ((p = getenv(upperKey.c_str())))
    {
        value = p;
        ret = true;
    }

#endif
    return ret;
}

bool BEnvironment::GetValue(const char *key, int *pValue)
{
    std::string str;
    std::string upperKey(key);

    strupper(upperKey);

    if (!GetValue(upperKey.c_str(), str))
    {
        return false;
    }

    return (sscanf(str.c_str(), "%i", pValue) == 1);
}

