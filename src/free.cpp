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
#include "flexerr.h"
#include <cstring>
#include <regex>
#include <optional>
#include <iostream>
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

// hex dump of a byte array.
// parameters:
//    os:           Output stream
//    data:         The pointer to the data to dump
//    size:         Byte size of data
//    bytesPerLine: Number of bytes output on one line.
//    withAscii:    If true dump 7-bit ASCII equivalent on the right side
//    startAddress: Optional, dump the address.
// Address at the beginning on the line is always a multiple of bytesPerLine.
void flx::hex_dump(std::ostream &os, const Byte *data, DWord size,
        DWord bytesPerLine, bool withAscii, std::optional<DWord> startAddress)
{
    const bool withAddress = startAddress.has_value();
    DWord idx = 0U;
    DWord address = 0U;
    DWord offset = 0U;
    std::string asciiString;
    auto asciiIter = std::back_inserter(asciiString);

    if (bytesPerLine == 0U)
    {
        throw FlexException(FERR_WRONG_PARAMETER);
    }

    if (data == nullptr || size == 0U)
    {
        return;
    }

    if (startAddress.has_value())
    {
        offset = startAddress.value() % bytesPerLine;
        address = startAddress.value() - offset;
    }

    if (withAddress)
    {
        os << fmt::format("{:04X}  ", address);
    }

    // Fill up with spaces to align to base address.
    os << std::string(3U * offset, ' ');
    if (withAscii)
    {
        for (DWord j = 0U; j < offset; ++j)
        {
            *(asciiIter++) = ' ';
        }
        address += offset;
    }

    const auto *spacer = "";
    for (idx = 0U; idx < size; ++idx)
    {
        const auto ch = static_cast<char>(*(data++));

        if (address % bytesPerLine == 0U)
        {
            if (withAddress && idx > 0U)
            {
                os << fmt::format("{:04X}  ", address);
            }

            if (withAscii)
            {
                asciiString.clear();
                asciiIter = std::back_inserter(asciiString);
            }
        }

        os << spacer << fmt::format("{:02X}", ch);
        spacer = " ";

        if (withAscii)
        {
            *(asciiIter++) = (ch >= ' ' && ch <= '~') ? ch : '_';
        }
        if ((address % bytesPerLine + 1U) == bytesPerLine)
        {
            if (withAscii)
            {
                os << "  " << asciiString;
            }
            os << "\n";
            spacer = "";
        }

        ++address;
    }

    if (withAscii && (address % bytesPerLine) != 0U)
    {
        offset = bytesPerLine - (address % bytesPerLine);
        os << std::string(3U * offset, ' ') << "  " << asciiString << "\n";
    }
}
