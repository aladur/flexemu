/*
cvtwchar.cpp


FLEXplorer, An explorer for any FLEX file or disk container
Copyright (C) 2016-2018 W. Schwotzer

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

std::wstring ConvertToUtf16String(const std::string &value)
{
    std::wstring result;

    int size = MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, NULL, 0);
    if (size > 0)
    {
        LPWSTR wideString = new wchar_t[size];

        MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, wideString, size);
        result = wideString;

        delete[] wideString;
    }

    if (size == 0)
    {
        DWORD lastError = GetLastError();
        throw FlexException(lastError, "Conversion to UTF16 string failed.");
    }

    return result;
}

std::string ConvertToUtf8String(const std::wstring &value)
{
    std::string result;

    int size = WideCharToMultiByte(CP_UTF8, 0, value.c_str(), -1, NULL, 0,
        NULL, NULL);
    if (size > 0)
    {
        LPSTR multiByteString = new char[size];

        size = WideCharToMultiByte(CP_UTF8, 0, value.c_str(), -1,
            multiByteString, size, NULL, NULL);
        if (size > 0)
        {
            result = multiByteString;
        }
 
       delete [] multiByteString;
    }

    if (size == 0)
    {
        DWORD lastError = GetLastError();
        throw FlexException(lastError, "Conversion to UTF8 string failed.");
    }

    return result;
}
#endif
