/*
    bregistr.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2022 W. Schwotzer

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
#include "cvtwchar.h"
#include <memory>


BRegistry BRegistry::classesRoot    = HKEY_CLASSES_ROOT;
BRegistry BRegistry::currentUser    = HKEY_CURRENT_USER;
BRegistry BRegistry::localMachine   = HKEY_LOCAL_MACHINE;
BRegistry BRegistry::users          = HKEY_USERS;

BRegistry::BRegistry() : lastError(0), hKey(nullptr)
{
}

BRegistry::BRegistry(HKEY aHKey) :  lastError(0), hKey(aHKey)
{
}

BRegistry::BRegistry(const BRegistry &regKey, const std::string &subKey)
{
    DWORD flags;

    lastError = RegCreateKeyEx(regKey, ConvertToUtf16String(subKey).c_str(), 0,
        L"", 0, KEY_ALL_ACCESS, nullptr, &hKey, &flags);

    if (lastError != ERROR_SUCCESS)
    {
        hKey = nullptr;
    }
}

BRegistry::~BRegistry()
{
    if (hKey != nullptr)
    {
        lastError = RegCloseKey(hKey);
    }
}

LONG BRegistry::Delete()
{
    lastError = RegCloseKey(hKey);
    hKey = nullptr;

    return lastError;
}

LONG BRegistry::SetValue(const std::string &name, const std::string &value)
{
    const auto wideCharValue(ConvertToUtf16String(value));

    lastError = RegSetValueEx(hKey, ConvertToUtf16String(name).c_str(), 0,
        REG_SZ, reinterpret_cast<CONST BYTE *>(wideCharValue.c_str()),
        static_cast<DWORD>(((wideCharValue.size() + 1) * sizeof(wchar_t))));

    return lastError;
}

LONG BRegistry::SetValue(const std::string &name, const int value)
{
    lastError = RegSetValueEx(hKey, ConvertToUtf16String(name).c_str(), 0,
                              REG_DWORD, (CONST BYTE *)&value, sizeof(DWORD));

    return lastError;
}

LONG BRegistry::SetValue(const std::string &name, const BYTE *value, int size)
{
    lastError = RegSetValueEx(hKey, ConvertToUtf16String(name).c_str(), 0,
                              REG_DWORD, value, size);

    return lastError;
}

LONG BRegistry::GetValue(const std::string &name, std::string &value)
{
    DWORD aSize;
    DWORD type;

    if ((lastError = RegQueryValueEx(hKey, ConvertToUtf16String(name).c_str(),
                                     0, &type, nullptr, &aSize))
                     == ERROR_SUCCESS)
    {
        auto buffer =
            std::unique_ptr<wchar_t>(new wchar_t[aSize / sizeof(wchar_t)]);

        lastError = RegQueryValueEx(hKey, ConvertToUtf16String(name).c_str(),
                                    0, &type, (BYTE *)buffer.get(), &aSize);

        if (lastError == ERROR_SUCCESS)
        {
            value = ConvertToUtf8String(buffer.get());
        }
    }

    return lastError;
}

LONG BRegistry::GetValue(const std::string &name, int &value)
{
    DWORD aSize(sizeof(DWORD));
    DWORD type;

    lastError = RegQueryValueEx(hKey, ConvertToUtf16String(name).c_str(), 0,
        &type, (BYTE *)&value, &aSize);

    return lastError;
}

LONG BRegistry::DeleteValue(const std::string &name)
{
    lastError = RegDeleteValue(hKey, ConvertToUtf16String(name).c_str());

    return lastError;
}

#endif
