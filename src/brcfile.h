/*
    brcfile.h


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

#ifndef BRCFILE_INCLUDED
#define BRCFILE_INCLUDED

#include <string>
#include <map>
#include "warnoff.h"
#include <optional>
#include "warnon.h"
#include <filesystem>

namespace fs = std::filesystem;


enum : uint8_t
{
    BRC_NO_ERROR = 0,
    BRC_NOT_FOUND = 99,
    BRC_NO_INTEGER = 100,
    BRC_FILE_ERROR = 101,
};

class BRcFile
{
public:
    BRcFile() = delete;
    explicit BRcFile(fs::path p_path);

    int SetValue(const char *key, const std::string &value);
    int SetValue(const char *key, const fs::path &value);
    int SetValue(const char *key, int value);
    int GetValue(const char *key, std::string &value);
    int GetValue(const char *key, fs::path &value);
    int GetValue(const char *key, int &value);
    int GetValues(const char *keyPrefix,
            std::map<std::string, std::string> &values);
    int Initialize();

private:
    int GetValue(const char *key, std::string &value,
            std::optional<bool> &isInteger);

    fs::path path;
};

#endif

