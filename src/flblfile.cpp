/*
    flblfile.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2024-2026  W. Schwotzer

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

#include "flblfile.h"
#include "binifile.h"
#include <ios>
#include <string>
#include <map>
#include <ostream>
#include <sstream>
#include <regex>
#include <filesystem>


std::map<unsigned, std::string> FlexLabelFile::ReadFile(
        std::ostream &os,
        const fs::path &path,
        const std::string &section)
{
    static std::regex regexKey("^[A-F0-9]{1,4}$");
    static std::regex regexValue("^([A-Z][A-Z0-9]*)(\\s*;.*)?$");
    std::map<unsigned, std::string> result;
    BIniFile iniFile(path);

    if (!iniFile.IsValid())
    {
        return {};
    }

    const auto valueForKey = iniFile.ReadSection(section);
    for (const auto &[key, value] : valueForKey)
    {
        if (std::regex_match(key, regexKey))
        {
            std::smatch match;

            if (std::regex_match(value, match, regexValue) && match.size() >= 2)
            {
                std::stringstream stream(key);
                unsigned addr{};

                stream >> std::hex >> addr;

                result[addr] = match[1];
            }
            else
            {
                os << "Syntax error: " << key << "=" << value << ". Ignored\n";
            }
        }
        else
        {
            os << "Invalid address: " << key << "=" << value << ". Ignored.\n";
        }
    }

    return result;
}
