/*
    cvtwchar.cpp


    FLEXplorer, An explorer for FLEX disk image files and directory disks.
    Copyright (C) 2016-2026  W. Schwotzer

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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "cvtwchar.h"
#include "flexerr.h"
#include <memory>


std::wstring ConvertToUtf16String(const std::string &value)
{
    if (!value.empty())
    {
        int size =
            MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, nullptr, 0);
        if (size > 0)
        {
            auto buffer = std::unique_ptr<wchar_t[]>(new wchar_t[size]);

            size = MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1,
                                       buffer.get(), size);

            if (size > 0)
            {
                return std::wstring(buffer.get());
            }
        }
        else if (size == 0)
        {
            DWORD lastError = GetLastError();
            const auto msg = std::string("Conversion to UTF16 string failed.");
            throw FlexException(lastError, msg);
        }
    }

    return std::wstring();
}

std::string ConvertToUtf8String(const std::wstring &value)
{
    if (!value.empty())
    {
        int size = WideCharToMultiByte(CP_UTF8, 0, value.c_str(), -1, nullptr,
                                       0, nullptr, nullptr);
        if (size > 0)
        {
            auto buffer = std::unique_ptr<char[]>(new char[size]);

            size = WideCharToMultiByte(CP_UTF8, 0, value.c_str(), -1,
                buffer.get(), size, nullptr, nullptr);
            if (size > 0)
            {
                return std::string(buffer.get());
            }
        }
        else if (size == 0)
        {
            DWORD lastError = GetLastError();
            const auto msg = std::string("Conversion to UTF8 string failed.");
            throw FlexException(lastError, msg);
        }
    }

    return std::string();
}
#endif
