/*
    binifile.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2018-2024  W. Schwotzer

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
#include <fstream>
#include <sstream>
#include <set>


BIniFile::BIniFile(std::string p_fileName)
    : fileName(std::move(p_fileName))
{
}

BIniFile::BIniFile(BIniFile &&src) noexcept :
    fileName(std::move(src.fileName))
{
}

BIniFile &BIniFile::operator=(BIniFile &&src) noexcept
{
    fileName = std::move(src.fileName);

    return *this;
}

bool BIniFile::IsValid() const
{
    std::ifstream istream(fileName);
    bool isValid = istream.is_open();

    return isValid;
}

std::string BIniFile::GetFileName() const
{
    return fileName;
}

BIniFile::Type BIniFile::ReadLine(int line_number,
                                  std::istream &istream,
                                  std::string &section,
                                  std::string &key,
                                  std::string &value,
                                  bool isSectionOnly = false) const
{
    static const std::string whiteSpace{flx::white_space};
    std::string line;

    if (std::getline(istream, line))
    {
        if (line.empty() || whiteSpace.find(line[0]) != std::string::npos)
        {
            throw FlexException(FERR_INVALID_LINE_IN_FILE, line_number,
                    line, GetFileName());
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

std::map<std::string, std::string> BIniFile::ReadSection(
                                             const std::string &section) const
{
    std::map<std::string, std::string> resultMap;
    std::set<std::string> foundKeys;
    std::ifstream istream(fileName);
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
                                    key.append("=").append(value), fileName);
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

