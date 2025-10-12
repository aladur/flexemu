/*
    brcfile.cpp


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
#include "brcfile.h"
#include <sstream>
#include <fstream>
#include <cstring>
#include "warnoff.h"
#include <optional>
#include "warnon.h"
#include <string>
#include <filesystem>

namespace fs = std::filesystem;


BRcFile::BRcFile(fs::path p_path) : path(std::move(p_path))
{
}

int BRcFile::SetValue(const char *key, const std::string &value)
{
    std::ofstream fs(path, std::ios::out | std::ios::app);

    if (!fs.is_open())
    {
        return BRC_FILE_ERROR;
    }

    fs << key << "\t\t" << std::quoted(value) << "\n";

    if (fs.fail())
    {
        return BRC_FILE_ERROR;
    }

    return BRC_NO_ERROR;
}

int BRcFile::SetValue(const char *key, const fs::path &value)
{
    std::ofstream fs(path, std::ios::out | std::ios::app);

    if (!fs.is_open())
    {
        return BRC_FILE_ERROR;
    }

    fs << key << "\t\t" << value << "\n";

    if (fs.fail())
    {
        return BRC_FILE_ERROR;
    }

    return BRC_NO_ERROR;
}

int BRcFile::SetValue(const char *key, int value)
{
    std::ofstream fs(path, std::ios::out | std::ios::app);

    if (!fs.is_open())
    {
        return BRC_FILE_ERROR;
    }

    fs << key << "\t\t" << value << "\n";

    if (fs.fail())
    {
        return BRC_FILE_ERROR;
    }

    return BRC_NO_ERROR;
}

int BRcFile::GetValue(const char *key, std::string &value)
{
    std::optional<bool> isInteger;

    return GetValue(key, value, isInteger);
}

int BRcFile::GetValue(const char *key, fs::path &value)
{
    std::string stringValue;

    auto result = GetValue(key, stringValue);

    if (result == BRC_NO_ERROR)
    {
        value = fs::u8path(stringValue);
    }

    return result;
}

int BRcFile::GetValue(const char *key, std::string &value,
        std::optional<bool> &isInteger)
{
    std::ifstream fs(path, std::ios::in);
    auto keyLength = std::strlen(key);

    isInteger = true;

    if (!fs.is_open())
    {
        return BRC_FILE_ERROR;
    }

    while (!fs.eof())
    {
        std::stringbuf strbuf;
        std::string skey;

        fs >> skey;
        if (!skey.empty())
        {
            fs.get(strbuf);
            fs.get(); // skip delimiter char.
            if (skey.size() == keyLength && skey.compare(key) == 0)
            {
                value = flx::trim(strbuf.str());
                if (!value.empty() && value[0] == '"')
                {
                    isInteger = false;
                    value = value.substr(1, value.length() - 2);
                }

                return BRC_NO_ERROR;
            }
        }
    }

    return BRC_NOT_FOUND;
}

int BRcFile::GetValue(const char *key, int &value)
{
    std::string str;
    std::optional<bool> isInt;

    if (int res = GetValue(key, str, isInt))
    {
        return res;
    }

    if (isInt.has_value() && !isInt.value())
    {
        return BRC_NO_INTEGER;
    }

    std::stringstream stream(str);
    if ((stream >> value).fail())
    {
        return BRC_NO_INTEGER; // returned value is no integer
    }

    return BRC_NO_ERROR;
}

int BRcFile::Initialize()
{
    std::ofstream fs(path, std::ios::out | std::ios::trunc);

    if (!fs.is_open())
    {
        return BRC_FILE_ERROR;
    }

    return BRC_NO_ERROR;
}

int BRcFile::GetValues(const char *keyPrefix,
        std::map<std::string, std::string> &values)
{
    std::ifstream fs(path);
    const auto lcKeyPrefix = flx::tolower(keyPrefix);

    if (!fs.is_open())
    {
        return BRC_FILE_ERROR;
    }

    values.clear();
    while (!fs.eof())
    {
        std::stringbuf strbuf;
        std::string key;

        fs >> key;
        fs.get(strbuf);
        auto value = flx::ltrim(strbuf.str());
        if (key.size() > std::strlen(keyPrefix))
        {
            const auto lcPrefixOfKey =
                flx::tolower(key.substr(0, lcKeyPrefix.size()));
            auto subKey = key.substr(lcKeyPrefix.size());

            if (fs.good() && lcKeyPrefix.compare(lcPrefixOfKey) == 0)
            {
                if (value[0] == '"')
                {
                    value = value.substr(1, value.length() - 2);
                }
                values[subKey] = value;
            }
        }
    }

    return BRC_NO_ERROR;
}

