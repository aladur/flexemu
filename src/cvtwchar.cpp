/*
cvtwchar.cpp


FLEXplorer, An explorer for any FLEX file or disk container
Copyright (C) 2016-2021 W. Schwotzer

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

#ifdef _WIN32

#include "misc1.h"
#include "cvtwchar.h"
#include "flexerr.h"
#include <memory>


std::wstring ConvertToUtf16String(const std::string &value)
{
    int size = MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, nullptr, 0);
    if (size > 0)
    {
        auto wideString = std::unique_ptr<wchar_t[]>(new wchar_t[size]);

        MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, wideString.get(), size);
        return std::wstring(wideString.get());
    }

    if (size == 0)
    {
        DWORD lastError = GetLastError();
        throw FlexException(lastError, "Conversion to UTF16 string failed.");
    }

    return std::wstring();
}

std::string ConvertToUtf8String(const std::wstring &value)
{
    std::string result;

    int size = WideCharToMultiByte(CP_UTF8, 0, value.c_str(), -1, nullptr, 0,
        nullptr, nullptr);
    if (size > 0)
    {
        auto multiByteString = std::unique_ptr<char[]>(new char[size]);

        size = WideCharToMultiByte(CP_UTF8, 0, value.c_str(), -1,
                               multiByteString.get(), size, nullptr, nullptr);
        if (size > 0)
        {
            return std::string(multiByteString.get());
        }
    }

    if (size == 0)
    {
        DWORD lastError = GetLastError();
        throw FlexException(lastError, "Conversion to UTF8 string failed.");
    }

    return std::string();
}
#endif
