/*
    bregistr.cpp


    FLEXplorer, An explorer for FLEX disk image files and directory disks.
    Copyright (C) 1998-2026  W. Schwotzer

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
#include "misc1.h"
#include "bregistr.h"
#include "cvtwchar.h"
#include <memory>
#include <filesystem>

namespace fs = std::filesystem;


BRegistry BRegistry::classesRoot = HKEY_CLASSES_ROOT;
BRegistry BRegistry::currentUser = HKEY_CURRENT_USER;
BRegistry BRegistry::localMachine = HKEY_LOCAL_MACHINE;
BRegistry BRegistry::users = HKEY_USERS;

BRegistry::BRegistry() : lastError(0), hKey(nullptr)
{
}

BRegistry::BRegistry(HKEY p_hKey) :  lastError(0), hKey(p_hKey)
{
}

BRegistry::BRegistry(const BRegistry &regKey, const std::string &subKey)
{
    DWORD flags;

    lastError = RegCreateKeyEx(regKey, ConvertToUtf16String(subKey).c_str(), 0,
        nullptr, 0, KEY_ALL_ACCESS, nullptr, &hKey, &flags);

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

LONG BRegistry::SetValue(const std::string &name, const fs::path &value)
{
    const auto wideCharValue(ConvertToUtf16String(value.u8string()));

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
    DWORD size;
    DWORD type;

    if ((lastError = RegQueryValueEx(hKey, ConvertToUtf16String(name).c_str(),
                                     0, &type, nullptr, &size))
                     == ERROR_SUCCESS)
    {
        auto buffer =
            std::unique_ptr<wchar_t>(new wchar_t[size / sizeof(wchar_t)]);

        lastError = RegQueryValueEx(hKey, ConvertToUtf16String(name).c_str(),
                                    0, &type, (BYTE *)buffer.get(), &size);

        if (lastError == ERROR_SUCCESS)
        {
            value = ConvertToUtf8String(buffer.get());
        }
    }

    return lastError;
}

LONG BRegistry::GetValue(const std::string &name, fs::path &value)
{
    std::string stringValue;

    auto result = GetValue(name, stringValue);
    if (result == ERROR_SUCCESS)
    {
        value = fs::u8path(stringValue);
    }

    return result;
}

LONG BRegistry::GetValue(const std::string &name, int &value)
{
    DWORD size(sizeof(DWORD));
    DWORD type;

    lastError = RegQueryValueEx(hKey, ConvertToUtf16String(name).c_str(), 0,
        &type, (BYTE *)&value, &size);

    return lastError;
}

LONG BRegistry::DeleteValue(const std::string &name)
{
    lastError = RegDeleteValue(hKey, ConvertToUtf16String(name).c_str());

    return lastError;
}

LONG BRegistry::GetValues(const std::string &keyPrefix,
        std::map<std::string, std::string> &values)
{
    const DWORD size(16384);
    DWORD keySize;
    DWORD index = 0U;
    DWORD type;
    bool isFinished = false;
    const auto lcKeyPrefix = flx::tolower(keyPrefix);
    auto keyBuf =
        std::unique_ptr<wchar_t>(new wchar_t[size * sizeof(wchar_t)]);

    values.clear();
    while (!isFinished)
    {
        keySize = size;
        lastError = RegEnumValue(hKey, index, keyBuf.get(), &keySize, nullptr,
                                 &type, nullptr, nullptr);
        if (lastError == ERROR_SUCCESS && type == REG_SZ)
        {
            auto key = ConvertToUtf8String(keyBuf.get());
            if (key.size() > keyPrefix.size())
            {
                std::string value;
                const auto lcPrefixOfKey =
                    flx::tolower(key.substr(0, keyPrefix.size()));
                if (lcKeyPrefix.compare(lcPrefixOfKey) == 0 &&
                    GetValue(key, value) == ERROR_SUCCESS)
                {
                    auto subKey = key.substr(keyPrefix.size());
                    values[subKey] = value;
                }
            }
        }

        if (lastError == ERROR_NO_MORE_ITEMS)
        {
            isFinished = true;
            lastError = ERROR_SUCCESS;
        }
        ++index;
    }

    return lastError;
}

#endif
