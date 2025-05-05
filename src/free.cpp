/*
    free.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2025  W. Schwotzer

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
#include "typedefs.h"
#include "asciictl.h"
#include "free.h"
#include "fversion.h"
#include <cstring>
#include <regex>
#include <fmt/format.h>


std::string flx::find_regex_string(const std::regex &regex,
        Byte eos, const std::vector<Byte> &data)
{
    std::string parsed_string;
    std::smatch match;
    DWord index = 0U;
    Byte ch;

    while (index < data.size())
    {
        ch = data[index];
        while ((index < (data.size() - 1U) && ch >= ' ' && ch <= 0x7EU) ||
                ch == CR || ch == LF)
        {
            if (ch != CR && ch != LF)
            {
                parsed_string.append(1U, static_cast<char>(ch));
            }
            ch = data[++index];
        }

        if (ch == eos && std::regex_match(parsed_string, match, regex) &&
            match.size() >= 2)
        {
            return match[1].str();
        }

        ++index;
        parsed_string.clear();
    }

    return {};
}

bool flx::is_range_in_ranges(const BInterval<DWord> &range,
        const MemorySource<DWord>::AddressRanges &ranges)
{
    return (std::any_of(ranges.cbegin(), ranges.cend(),
        [&](const auto &addressRange) {
            return subset(range, addressRange);
        }));
}

void flx::print_versions(std::ostream &os, const std::string &program_name)
{
    os <<
        program_name << " " << VERSION << "\n" <<
        "compiled for " << OSTYPE << ", using:\n";
    for (const auto &pair : FlexemuVersions::GetVersions())
    {
        os << "- " << pair.first << " ";
        if (pair.second.size() == 1)
        {
            os << pair.second[0] + "\n";
        }
        else
        {
            bool isWithComma = false;
            for (const auto &item : pair.second)
            {
                if (isWithComma)
                {
                    os << ", ";
                }
                os << item;
                isWithComma = true;
            }
            os << "\n";
        }
    }
    os << program_name << " " << COPYRIGHT_MESSAGE;
}


void flx::hex_dump(std::ostream &os, const char *buffer, unsigned count)
{
    const char *p = &buffer[0];
    unsigned i = 0;

    for (; i < count; ++i)
    {
        char ch = *(p++);
        os << fmt::format("{:02X} ", ch);
        if ((i & 0x0FU) == 0x0FU)
        {
            os << "\n";
        }
    }
    if ((i & 0x0FU) != 0U)
    {
        os << "\n";
    }
}
