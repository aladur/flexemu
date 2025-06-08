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

// Write a hex dump of a byte array on a stream.
// parameters:
//    os:           Where the result is streamed to.
//    data:         The pointer to the data to dump
//    size:         Byte size of data
//    bytesPerLine: Number of bytes to scale on one line.
//    withAscii:    If true stream the scale for ASCII values on the right.
//    startAddress: Optional, stream the address.
//    extraSpace:   Optional, stream extra space after "extraSpace" bytes.
// Address at the beginning on the line is always a multiple of bytesPerLine.
void flx::hex_dump(std::ostream &os, const Byte *data, DWord size,
        DWord bytesPerLine, bool withAscii, std::optional<DWord> startAddress,
        std::optional<DWord> extraSpace)
{
    const bool withAddress = startAddress.has_value();
    bool endsWithNewline = false;
    DWord idx = 0U;
    const DWord offset = withAddress ? startAddress.value() % bytesPerLine : 0U;
    DWord address = withAddress ? startAddress.value() - offset : 0U;
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
    if (extraSpace.has_value() && bytesPerLine <= extraSpace.value())
    {
        extraSpace.reset();
    }

    if (withAddress)
    {
        os << fmt::format("{:04X}  ", address);
    }

    // Fill up with spaces to align to base address.
    auto extra = extraSpace.has_value() ? offset / extraSpace.value() : 0U;
    os << std::string(3U * offset + extra, ' ');
    auto column = offset;
    if (withAscii)
    {
        for (DWord j = 0U; j < offset + extra; ++j)
        {
            *(asciiIter++) = ' ';
        }
    }
    address += offset;

    const auto *spacer = "";
    for (idx = 0U; idx < size; ++idx)
    {
        const auto ch = static_cast<char>(*(data++));

        endsWithNewline = false;
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

            column = 0U;
        }

        os << spacer << fmt::format("{:02X}", ch);
        spacer = " ";

        const bool withExtraSpace = (extraSpace.has_value() &&
            address % bytesPerLine + 1U != bytesPerLine &&
            (column % extraSpace.value() + 1U == extraSpace.value()));

        if (withExtraSpace)
        {
            os << " ";
        }

        if (withAscii)
        {
            *(asciiIter++) = (ch >= ' ' && ch <= '~') ? ch : '_';

            if (withExtraSpace)
            {
                *(asciiIter++) = ' ';
            }
        }

        if ((address % bytesPerLine + 1U) == bytesPerLine)
        {
            if (withAscii)
            {
                os << "  " << asciiString;
            }
            os << "\n";
            spacer = "";
            endsWithNewline = true;
        }

        ++address;
        ++column;
    }

    if (withAscii && (address % bytesPerLine) != 0U)
    {
        extra = 0U;
        if (extraSpace.has_value())
        {
            extra = bytesPerLine / extraSpace.value() -
                    (column / extraSpace.value());
        }

        auto endOffset = bytesPerLine - (address % bytesPerLine);
        os << std::string(3U * endOffset + extra, ' ') << "  " << asciiString;
    }

    if (!endsWithNewline)
    {
        os << "\n";
    }
}

// Write a hex dump scale on a stream.
// Intentionally use the same parameters as for hex_dump() used to format
// the scale.
//
// Parameters:
//    os:           Where the result is streamed to.
//    bytesPerLine: Number of bytes to scale on one line.
//    withAscii:    If true stream the scale for ASCII values on the right.
//    startAddress: Optional, if set indent the scale.
//    extraSpace:   Optional, stream extra space after "extraSpace" bytes.
void flx::hex_dump_scale(std::ostream &os, DWord bytesPerLine, bool withAscii,
        std::optional<DWord> startAddress,
        std::optional<DWord> extraSpace)
{
    auto streamHexDumpScaleFct = [&](bool isSecondLine)
    {
        const auto *spacer = "";

        os << std::string(startAddress.has_value() ? 6U : 0U, ' ');
        for (DWord idx = 0U; idx < bytesPerLine; ++idx)
        {
            const auto *theExtraSpace = (extraSpace.has_value() &&
                bytesPerLine > extraSpace.value() &&
                idx + 1U != bytesPerLine &&
                (idx % extraSpace.value() + 1U == extraSpace.value())) ?
                " " : "";

            os << spacer << ((isSecondLine || (idx & 0x0FU) == 0U) ?
                fmt::format("{:02X}", idx) : "  ") << theExtraSpace;
            spacer = " ";
        }

        if (!withAscii)
        {
            return;
        }

        os << "  ";
        for (DWord idx = 0U; idx < bytesPerLine; ++idx)
        {
            const auto *theExtraSpace = (extraSpace.has_value() &&
                bytesPerLine > extraSpace.value() &&
                idx + 1U != bytesPerLine &&
                (idx % extraSpace.value() + 1U == extraSpace.value())) ?
                " " : "";

            os << ((isSecondLine || (idx & 0x0FU) == 0U) ?
                fmt::format("{:X}", isSecondLine ? idx & 0x0FU : idx >> 4U) :
                " ") << theExtraSpace;
        }
    };

    streamHexDumpScaleFct(false);
    os << "\n";
    streamHexDumpScaleFct(true);
}
