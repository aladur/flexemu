/*
    binifile.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2018-2025  W. Schwotzer

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

#ifndef BINIFILE_INCLUDED
#define BINIFILE_INCLUDED

#include <string>
#include <map>
#include <cstdint>


class BIniFile
{
public:
    BIniFile() = delete;
    BIniFile(const BIniFile &src) = delete;
    BIniFile(BIniFile &&src) noexcept;
    explicit BIniFile(std::string p_fileName);
    ~BIniFile() = default;

    BIniFile &operator=(const BIniFile &src) = delete;
    BIniFile &operator=(BIniFile &&src) noexcept;

    bool IsValid() const;
    std::string GetFileName() const;
    std::map<std::string, std::string> ReadSection(const std::string &section)
                                                                          const;
    int GetLineNumber(const std::string &section, const std::string &key) const;

private:
    enum class Type : uint8_t
    {
        Comment,
        Section,
        KeyValue,
        Unknown,
    };

    Type ReadLine(int line_number, std::istream &istream,
                  std::string &section, std::string &key, std::string &value,
                  bool isSectionOnly) const;

    std::string fileName;
};

#endif

