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


#if defined(UNIX) || defined(USE_CMAKE)
#include "config.h"
#else
#include "confignt.h"
#endif
#include "typedefs.h"
#include "ostype.h"
#include "bintervl.h"
#include "memsrc.h"
#include "asciictl.h"
#include "free.h"
#include "fversion.h"
#include "warnoff.h"
#include <fmt/format.h>
#include "warnon.h"
#include <cstring>
#include <utility>
#include <string>
#include <vector>
#include <ostream>
#include <regex>


constexpr const char *COPYRIGHT_MESSAGE = \
    "comes with ABSOLUTELY NO WARRANTY. This is free software,\n" \
    "and You are welcome to redistribute it under certain conditions.\n" \
    "Please notice that this project was developed under the terms of the\n" \
    "GNU GENERAL PUBLIC LICENCE V2.\n" \
    "Copyright (C) 1998-2025 Wolfgang Schwotzer\n" \
    "http://flexemu.neocities.org\n";

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
