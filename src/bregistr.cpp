/*
    bregistr.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2018 W. Schwotzer

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
#include "bregistr.h"
#ifdef UNICODE
#include "cvtwchar.h"
#endif

BRegistry BRegistry::classesRoot    = HKEY_CLASSES_ROOT;
BRegistry BRegistry::currentUser    = HKEY_CURRENT_USER;
BRegistry BRegistry::localMachine   = HKEY_LOCAL_MACHINE;
BRegistry BRegistry::users          = HKEY_USERS;

BRegistry::BRegistry(void) : lastError(0), hKey(NULL)
{
}

BRegistry::BRegistry(HKEY aHKey) :  lastError(0), hKey(aHKey)
{
}

BRegistry::BRegistry(const BRegistry &regKey, const std::string &subKey)
{
    DWORD flags;

#ifdef UNICODE
    lastError = RegCreateKeyEx(regKey, ConvertToUtf16String(subKey).c_str(), 0,
        L"", 0, KEY_ALL_ACCESS, NULL, &hKey, &flags);
#else
    lastError = RegCreateKeyEx(regKey.hKey, subKey.c_str(), 0, "", 0, KEY_ALL_ACCESS, NULL,
        &hKey, &flags);
#endif

    if (lastError != ERROR_SUCCESS)
    {
        hKey = NULL;
    }
}

BRegistry::~BRegistry()
{
    if (hKey != NULL)
    {
        lastError = RegCloseKey(hKey);
    }
}

LONG BRegistry::Delete()
{
    lastError = RegCloseKey(hKey);
    hKey = NULL;

    return lastError;
}

LONG BRegistry::SetValue(const std::string &name, const std::string &value)
{
#ifdef UNICODE

    std::wstring wideCharValue = ConvertToUtf16String(value);

    lastError = RegSetValueEx(hKey, ConvertToUtf16String(name).c_str(), 0,
        REG_SZ, reinterpret_cast<CONST BYTE *>(wideCharValue.c_str()),
        static_cast<DWORD>(((wideCharValue.size() + 1) * sizeof(wchar_t))));
#else
    lastError = RegSetValueEx(hKey, name.c_str(), 0, REG_SZ,
        reinterpret_cast<CONST BYTE *>(value.c_str()),
        static_cast<DWORD>(value.size() + 1));
#endif

    return lastError;
}

LONG BRegistry::SetValue(const std::string &name, const int value)
{
#ifdef UNICODE
    lastError = RegSetValueEx(hKey, ConvertToUtf16String(name).c_str(), 0, REG_DWORD,
                              (CONST BYTE *)&value, sizeof(DWORD));
#else
    lastError = RegSetValueEx(hKey, name.c_str(), 0, REG_DWORD,
        (CONST BYTE *)&value, sizeof(DWORD));
#endif

    return lastError;
}

LONG BRegistry::SetValue(const std::string &name, const BYTE *value, int size)
{
#ifdef UNICODE
    lastError = RegSetValueEx(hKey, ConvertToUtf16String(name).c_str(), 0, REG_DWORD, value, size);
#else
    lastError = RegSetValueEx(hKey, name.c_str(), 0, REG_DWORD, value, size);
#endif
    return lastError;
}

LONG BRegistry::GetValue(const std::string &name, std::string &value)
{
    DWORD aSize;
    DWORD type;

#ifdef UNICODE
    wchar_t *str;

    if ((lastError = RegQueryValueEx(hKey, ConvertToUtf16String(name).c_str(), 0, &type, NULL,
                                     &aSize)) == ERROR_SUCCESS)
    {
        if ((str = new wchar_t [aSize / sizeof(wchar_t)]) != NULL)
        {
            lastError = RegQueryValueEx(hKey, ConvertToUtf16String(name).c_str(), 0, &type, (BYTE *)str,
                                        &aSize);

            if (lastError == ERROR_SUCCESS)
            {
                value = ConvertToUtf8String(std::wstring(str).c_str());
            }

            delete[] str;
        }
    }
#else
    char *str;

    if ((lastError = RegQueryValueEx(hKey, name.c_str(), 0, &type, NULL,
        &aSize)) == ERROR_SUCCESS)
    {
        if ((str = new char[aSize]) != NULL)
        {
            lastError = RegQueryValueEx(hKey, name.c_str(), 0, &type, (BYTE *)str,
                &aSize);

            if (lastError == ERROR_SUCCESS)
            {
                value = str;
            }

            delete[] str;
        }
    }
#endif

    return lastError;
}

LONG BRegistry::GetValue(const std::string &name, int *pvalue)
{
    DWORD aSize;
    DWORD type;

    aSize = sizeof(DWORD);
#ifdef UNICODE
    lastError = RegQueryValueEx(hKey, ConvertToUtf16String(name).c_str(), 0,
        &type, (BYTE *)pvalue, &aSize);
#else
    lastError = RegQueryValueEx(hKey, name.c_str(), 0, &type, (BYTE *)pvalue,
        &aSize);
#endif

    return lastError;
}

#endif
