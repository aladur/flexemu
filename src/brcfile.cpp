/*
    brcfile.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2024  W. Schwotzer

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

#include <stdio.h>
#include <errno.h>
#include <sstream>
#include <fstream>
#include "misc1.h"
#include "brcfile.h"
#include "bfileptr.h"


BRcFile::BRcFile(const char *aFileName) : fileName(aFileName)
{
}

void BRcFile::SetFileName(const char *aFileName)
{
    fileName = aFileName;
}

int BRcFile::SetValue(const char *key, const char *value)
{
    BFilePtr fp(fileName, "a");

    if (fp == nullptr)
    {
        return errno;
    }

    auto res = fprintf(fp, "%s\t\t\"%s\"\n", key, value);

    if (res < 0)
    {
        return errno;
    }

    return BRC_NO_ERROR;
}

int BRcFile::SetValue(const char *key, int value)
{
    std::string str;
    BFilePtr fp(fileName, "a");

    if (fp == nullptr)
    {
        return errno;
    }

    auto res = fprintf(fp, "%s\t\t%i\n", key, value);

    if (res < 0)
    {
        return errno;
    }

    return BRC_NO_ERROR;
}

int BRcFile::GetValue(const char *key, std::string &value, int *isInteger)
{
    char def[256];
    char strparm[PATH_MAX];
    auto keyLength = strlen(key);
    BFilePtr fp(fileName, "r");

    if (isInteger)
    {
        *isInteger = 1;
    }

    if (fp == nullptr)
    {
        return errno;
    }

    while (!feof(fp))
    {
        if (fscanf(fp, "%79s %[^\n]\n", def, strparm) == 2 &&
            strlen(def) == keyLength && stricmp(def, key) == 0)
        {
            value = strparm;

            if (value[0] == '"')
            {

                if (isInteger)
                {
                    *isInteger = 0;
                }

                value = value.substr(1, value.length() - 2);
            }

            return BRC_NO_ERROR;
        }
    }

    return BRC_NOT_FOUND;
}

int BRcFile::GetValue(const char *key, int &value)
{
    std::string str;
    int isInt;

    if (int res = GetValue(key, str, &isInt))
    {
        return res;
    }

    if (!isInt)
    {
        return BRC_NO_INTEGER;
    }

    if (sscanf(str.c_str(), "%i", &value) != 1)
    {
        return BRC_NO_INTEGER; // returned value is no integer
    }

    return BRC_NO_ERROR;
}

int BRcFile::Initialize()
{
    BFilePtr fp(fileName, "w");

    if (fp == nullptr)
    {
        return errno;
    }

    return BRC_NO_ERROR;
}

int BRcFile::GetValues(const char *keyPrefix,
        std::map<std::string, std::string> &values)
{
    std::ifstream fs(fileName.c_str());

    values.clear();
    if (!fs.is_open())
    {
        return BRC_NOT_FOUND;
    }

    while (!fs.eof())
    {
        std::stringbuf strbuf;
        std::string key;

        fs >> key;
        fs.get(strbuf);
        auto value = strbuf.str();
        ltrim(value);
        if (key.size() > strlen(keyPrefix))
        {
            auto prefixOfKey = key.substr(0, strlen(keyPrefix));
            auto subKey = key.substr(strlen(keyPrefix));

            if (fs.good() && stricmp(keyPrefix, prefixOfKey.c_str()) == 0)
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

