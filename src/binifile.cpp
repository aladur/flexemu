/*
    binifile.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2018-2026  W. Schwotzer

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
#include "binifile.h"
#include "flexerr.h"
#include <cctype>
#include <fstream>
#include <sstream>
#include <set>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

BIniFile::BIniFile(fs::path p_path)
    : path(std::move(p_path))
{
}

BIniFile::BIniFile(BIniFile &&src) noexcept :
    path(std::move(src.path))
{
}

BIniFile &BIniFile::operator=(BIniFile &&src) noexcept
{
    path = std::move(src.path);

    return *this;
}

bool BIniFile::IsValid() const
{
    std::ifstream istream(path);
    bool isValid = istream.is_open();

    return isValid;
}

fs::path BIniFile::GetPath() const
{
    return path;
}

BIniFile::Type BIniFile::ReadLine(int line_number,
                                  std::istream &istream,
                                  std::string &section,
                                  std::string &key,
                                  std::string &value,
                                  bool isSectionOnly = false) const
{
    std::string line;

    if (std::getline(istream, line))
    {
        if (line.empty() || std::isspace(line[0]))
        {
            throw FlexException(FERR_INVALID_LINE_IN_FILE, line_number,
                    line, GetPath());
        }

        if (line[0] == '[')
        {
            std::stringstream stream(line.substr(1));

            if (std::getline(stream, section, ']'))
            {
                section = flx::trim(std::move(section));

                return Type::Section;
            }
        }
        else if (line[0] == ';' || line[0] == '#')
        {
            return Type::Comment;
        }
        else
        {
            if (isSectionOnly)
            {
                return Type::Unknown;
            }

            std::stringstream stream(line);
            key.clear();
            value.clear();

            if (std::getline(stream, key, '='))
            {
                if (!std::getline(stream, value))
                {
                    value.clear();
                }

                key = flx::trim(std::move(key));
                value = flx::trim(std::move(value));
            }
            return Type::KeyValue;
        }
    }

    return Type::Unknown;
}

BIniFile::SectionContent_t BIniFile::ReadSection(
                                             const std::string &section) const
{
    SectionContent_t resultMap;
    std::set<std::string> foundKeys;
    std::ifstream istream(path);
    int line_number = 0;

    if (istream.good())
    {
        bool isSectionActive = false;
        bool isDone = false;
        istream.clear();
        istream.seekg(0);

        isSectionActive = section.empty();
        while(!isDone && !istream.eof())
        {
            std::string sectionName;
            std::string key;
            std::string value;

            ++line_number;
            switch (ReadLine(line_number, istream, sectionName, key, value,
                             !isSectionActive))
            {
                case Type::Section:
                    if (isSectionActive)
                    {
                        isDone = true; // A section can only be specified once.
                    }
                    isSectionActive = (section == sectionName);
                    break;

                case Type::KeyValue:
                    if (isSectionActive)
                    {
                        if (foundKeys.find(key) != foundKeys.end())
                        {
                            throw FlexException(FERR_INVALID_LINE_IN_FILE,
                                    line_number,
                                    key.append("=").append(value),
                                    GetPath());
                        }

                        resultMap.insert({ key, value });;
                        foundKeys.insert(key);
                    }
                    break;

                case Type::Comment:
                case Type::Unknown:
                    break;
            }
        }
    }

    return resultMap;
}

bool BIniFile::HasSection(const std::string &section) const
{
    std::ifstream istream(path);

    if (!istream.good())
    {
        return false;
    }

    istream.clear();
    istream.seekg(0);
    int line_number = 0;

    while(!istream.eof())
    {
        std::string sectionName;
        std::string key;
        std::string value;

        ++line_number;
        switch (ReadLine(line_number, istream, sectionName, key, value, false))
        {
            case Type::Section:
                if(section == sectionName)
                {
                    return true;
                }
                break;

            case Type::KeyValue:
                if (section.empty())
                {
                    return true;
                }
            case Type::Comment:
            case Type::Unknown:
                break;
        }
    }

    return false;
}

int BIniFile::GetLineNumber(const std::string &section, const std::string &key)
    const
{
    std::ifstream istream(path);
    int line_number = 0;
    std::string sectionName;
    std::string keyName;
    std::string value;
    bool isSectionActive = section.empty();

    if (!istream.is_open())
    {
        return 0;
    }

    while (!istream.eof())
    {
        ++line_number;
        switch (BIniFile::ReadLine(line_number, istream, sectionName, keyName,
                    value, !isSectionActive))
        {
            case BIniFile::Type::Section:
                if (isSectionActive)
                {
                    return 0;
                }
                isSectionActive = (section == sectionName);
                break;
            case BIniFile::Type::KeyValue:
                if (isSectionActive)
                {
                    if (key == keyName)
                    {
                        return line_number;
                    }
                }
                break;
            case BIniFile::Type::Comment:
            case BIniFile::Type::Unknown:
                break;
        }
    }

    return 0;
}

