/*
    binifile.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2018-2021  W. Schwotzer

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
#include <sstream>
#include <set>


BIniFile::BIniFile(const char *aFileName) :
    fileName(aFileName)
{
}

BIniFile::BIniFile(BIniFile &&src) :
    fileName(std::move(src.fileName))
{
}

BIniFile &BIniFile::operator=(BIniFile &&src) 
{
    fileName = std::move(src.fileName);

    return *this;
}

BIniFile::~BIniFile()
{
}

bool BIniFile::IsValid() const
{
    std::ifstream istream(fileName);
    bool isValid = istream.good();

    return isValid;
}

std::string BIniFile::GetFileName() const
{
    return fileName;
}

BIniFile::Type BIniFile::ReadLine(std::istream &istream,
                                  std::string &section,
                                  std::string &key,
                                  std::string &value,
                                  bool isSectionOnly = false) const
{
    std::string line;

    if (std::getline(istream, line))
    {
        std::stringstream stream(line);

        if (line.size() > 0 && line[0] == '[')
        {
            stream.get();

            if (std::getline(stream, section, ']'))
            {
                trim(section);

                return Type::Section;
            }
        }
        else
        {
            if (isSectionOnly)
            {
                return Type::Unknown;
            }

            if (line.size() > 0 && line[0] != ';' && line[0] != '#')
            {
                if (std::getline(stream, key, '=') &&
                    std::getline(stream, value))
                {
                    trim(key);
                    trim(value);

                    return Type::KeyValue;
                }
                else
                {
                    throw FlexException(FERR_INVALID_LINE_IN_FILE, line,
                                        fileName);
                }
            }
        }
    }

    return Type::Comment;
}

std::map<std::string, std::string> BIniFile::ReadSection(
                                             const std::string &section) const
{
    std::map<std::string, std::string> resultMap;
    std::set<std::string> foundKeys;
    std::ifstream istream(fileName);

    if (istream.good())
    {
        bool isSectionActive = false;
        istream.clear();
        istream.seekg(0);

        while(!istream.eof())
        {
            std::string sectionName;
            std::string key;
            std::string value;

            switch (ReadLine(istream, sectionName, key, value,
                             !isSectionActive))
            {
                case Type::Section:
                    isSectionActive = (section == sectionName);
                    break;

                case Type::KeyValue:
                    if (isSectionActive)
                    {
                        if (foundKeys.find(key) != foundKeys.end())
                        {
                            throw FlexException(FERR_INVALID_LINE_IN_FILE,
                                    key + "=" + value, fileName);
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

