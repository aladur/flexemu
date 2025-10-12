/*
    benv.cpp


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


#include "misc1.h"
#include "benv.h"
#ifdef _WIN32
#include "cvtwchar.h"
#endif
#include <string>
#include <sstream>
#include <cstdlib>


bool BEnvironment::RemoveKey(const char *key)
{
#ifdef _WIN32
    SetEnvironmentVariable(ConvertToUtf16String(key).c_str(), nullptr);
#endif
#ifdef UNIX
#if (HAVE_DECL_UNSETENV==1)
    unsetenv(key);
#endif
#endif
    return true;
}

bool BEnvironment::SetValue(const char *key, const std::string &value)
{
#ifdef _WIN32
    return (SetEnvironmentVariable(
        ConvertToUtf16String(key).c_str(),
        ConvertToUtf16String(value).c_str()) != 0);
#endif
#ifdef UNIX
#if (HAVE_DECL_SETENV==1)
    return (setenv(key, value.c_str(), 1) == 0);
#else
    return false;
#endif
#endif
}

bool BEnvironment::SetValue(const char *key, int value)
{
    std::stringstream stream;

    stream << value;
#ifdef _WIN32
    return (SetEnvironmentVariable(
        ConvertToUtf16String(key).c_str(),
        ConvertToUtf16String(stream.str()).c_str()) != 0);
#endif
#ifdef UNIX
#if (HAVE_DECL_SETENV==1)
    return (setenv(key, stream.str().c_str(), 1) == 0);
#else
    return false;
#endif
#endif
}

bool BEnvironment::GetValue(const char *key, std::string &value)
{
    bool ret = false;

#ifdef _WIN32
    const auto wKey(ConvertToUtf16String(key));
    auto size = GetEnvironmentVariable(wKey.c_str(), nullptr, 0);
    if (size)
    {
        auto p = new wchar_t[size + 1U];

        if (GetEnvironmentVariable(wKey.c_str(), p, size + 1U))
        {
            value = ConvertToUtf8String(p);
            ret = true;
        }

        delete [] p;
    }
#endif
#ifdef UNIX
    const auto *envValue = getenv(key);
    if (envValue != nullptr)
    {
        value = envValue;
        ret = true;
    }

#endif
    return ret;
}

bool BEnvironment::GetValue(const char *key, int &value)
{
    std::string str;

    if (!GetValue(key, str))
    {
        return false;
    }

    std::stringstream stream{str};

    return static_cast<bool>(stream >> value);
}

