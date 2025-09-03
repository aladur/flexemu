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
#include <utility>
#include "warnoff.h"
#include <fmt/format.h>
#include "warnon.h"


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

// Get byte array from a hex-digits string.
std::vector<Byte> flx::get_bytes_from_hex(const std::string &hex_values)
{
    std::vector<Byte> result;

    if (hex_values.empty() || hex_values.size() % 2 == 1)
    {
        return { };
    }

    result.reserve(hex_values.size() / 2);
    for (size_t idx = 0; idx < hex_values.size(); idx += 2)
    {
        Byte value{};

        const auto [ptr, ec] = std::from_chars(
                hex_values.data() + idx,
                hex_values.data() + idx + 2U, value, 16);
        if (ec != std::errc() || ptr != hex_values.data() + idx + 2U)
        {
            return { };
        }
        result.emplace_back(value);
    }

    return result;
}

// Write hex dump of a byte array to a stream.
// Parameters:
//    os:           Where the result is streamed to.
//    data:         Pointer to the data to dump.
//    size:         Byte size of data.
//    bytesPerLine: Number of bytes to stream per line.
//    withAscii:    If true stream ASCII values on the right.
//    isDisplayAddress: Stream the 4-digit address on the left.
//    startAddress: First address to stream data for.
//    extraSpace:   Optional, stream extra space after "extraSpace" bytes.
// Address at the beginning on the line is always a multiple of bytesPerLine.
void flx::hex_dump(
        std::ostream &os,
        const Byte *data,
        size_t size,
        DWord bytesPerLine,
        bool withAscii,
        bool isDisplayAddress,
        DWord startAddress,
        std::optional<DWord> extraSpace)
{
    if (bytesPerLine == 0U)
    {
        throw FlexException(FERR_WRONG_PARAMETER);
    }

    if (data == nullptr || size == 0U)
    {
        return;
    }

    DWord idx = 0U;
    const DWord offset = startAddress % bytesPerLine;
    DWord address = startAddress - offset;
    std::string hexString;
    std::string asciiString;
    auto asciiIter = std::back_inserter(asciiString);

    if (extraSpace.has_value() && bytesPerLine <= extraSpace.value())
    {
        extraSpace.reset();
    }

    if (isDisplayAddress)
    {
        convert(address, hexString, 16, 4);
        os << hexString << "  ";
    }

    // Fill up with spaces to align to base address.
    auto extra = extraSpace.has_value() ? offset / extraSpace.value() : 0U;
    DWord extraSpaces = 0U;
    os << std::string(3U * offset + extra, ' ');
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

        if (address % bytesPerLine == 0U)
        {
            if (isDisplayAddress && idx > 0U)
            {
                convert(address, hexString, 16, 4);
                os << hexString << "  ";
            }

            if (withAscii)
            {
                asciiString.clear();
                asciiIter = std::back_inserter(asciiString);
            }

            extraSpaces = 0U;
        }

        convert(static_cast<Byte>(ch), hexString, 16, 2);
        os << spacer << hexString;

        spacer = " ";

        const bool withExtraSpace = (extraSpace.has_value() &&
            address % bytesPerLine + 1U != bytesPerLine &&
            ((address % bytesPerLine) % extraSpace.value() + 1U ==
             extraSpace.value()));

        if (withExtraSpace)
        {
            os << " ";
            ++extraSpaces;
        }

        if (withAscii)
        {
            *(asciiIter++) = flx::ascchr(ch)[0];

            if (withExtraSpace && idx + 1U != size)
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
            if (idx + 1U != size)
            {
                os << "\n";
            }
            spacer = "";
        }

        ++address;
    }

    if (withAscii && (address % bytesPerLine) != 0U)
    {
        extra = 0U;
        if (extraSpace.has_value())
        {
            extra = (bytesPerLine - 1U) / extraSpace.value() - extraSpaces;
        }

        auto endOffset = bytesPerLine - (address % bytesPerLine);
        os << std::string(3U * endOffset + extra, ' ') << "  " << asciiString;
    }
}

// Write a hex dump scale on a stream.
// Parameters:
//    os:           Where the result is streamed to.
//    bytesPerLine: Number of bytes to scale per line.
//    withAscii:    If true stream the scale for ASCII values on the right.
//    isDisplayAddress: Used to reserve columns on the left.
//    extraSpace:   Optional, stream extra space after "extraSpace" bytes.
void flx::hex_dump_scale(
        std::ostream &os,
        DWord bytesPerLine,
        bool withAscii,
        bool isDisplayAddress,
        std::optional<DWord> extraSpace)
{
    auto streamHexDumpScaleFct = [&](bool isSecondLine)
    {
        const auto *spacer = "";

        os << std::string(isDisplayAddress ? 6U : 0U, ' ');
        for (DWord idx = 0U; idx < bytesPerLine; ++idx)
        {
            const auto scale = 0xFFU & (idx % (2U * bytesPerLine));
            const auto *theExtraSpace = (extraSpace.has_value() &&
                bytesPerLine > extraSpace.value() &&
                scale + 1U != bytesPerLine &&
                (idx % extraSpace.value() + 1U == extraSpace.value())) ?
                " " : "";

            const auto scaleString =
                fmt::format("{:02X}", isSecondLine ? scale : scale & 0xF0U);
            os << spacer << ((isSecondLine || !(scale & 0x0FU)) ?
                scaleString : "  ") << theExtraSpace;
            spacer = " ";
        }

        if (!withAscii)
        {
            return;
        }

        os << "  ";
        for (DWord idx = 0U; idx < bytesPerLine; ++idx)
        {
            const auto scale = 0xFFU & (idx % (2U * bytesPerLine));
            const auto *theExtraSpace = (extraSpace.has_value() &&
                bytesPerLine > extraSpace.value() &&
                idx + 1U != bytesPerLine &&
                (idx % extraSpace.value() + 1U == extraSpace.value())) ?
                " " : "";

            const auto scaleString =
                fmt::format("{:X}", isSecondLine ? scale & 0x0FU : scale >> 4U);
            os << ((isSecondLine || !(scale & 0x0FU)) ?
                scaleString : " ") << theExtraSpace;
        }
    };

    streamHexDumpScaleFct(false);
    os << "\n";
    streamHexDumpScaleFct(true);
}

// Return properties for a given row and column within a hex dump .
// Parameters:
//    row:          Zero-based row in hex_dump output.
//    column:       Zero-based column in hex_dump output.
//    size:         Byte size of data.
//    bytesPerLine: Number of bytes to scale per line.
//    withAscii:    If true there are ASCII values on the right.
//    isDisplayAddress: If true the address is output on the right.
//    startAddress: First address with data.
//    extraSpace:   Optional, there is extra space after "extraSpace" bytes.
flx::sHexDumpProperties flx::get_hex_dump_properties(
        DWord row,
        DWord column,
        size_t size,
        DWord bytesPerLine,
        bool withAscii,
        bool isDisplayAddress,
        DWord startAddress,
        std::optional<DWord> extraSpace)
{
    flx::sHexDumpProperties result{};
    const bool isExtraSpc = extraSpace.has_value();
    const auto offset = startAddress % bytesPerLine;
    const auto indent = isDisplayAddress ? 6U : 0U; // skip address columns
    const auto lastRow = (size + offset) / bytesPerLine;
    const auto firstRowBytes = bytesPerLine - offset;
    const auto lastRowBytes = static_cast<DWord>((size + offset) % bytesPerLine);
    const auto extraInOffset = isExtraSpc ? offset / extraSpace.value() : 0U;
    const auto extraInRow =
        isExtraSpc ? (bytesPerLine - 1U) / extraSpace.value() : 0U;
    const auto extraInLastRow =
        isExtraSpc ? (lastRowBytes - 1U) / extraSpace.value() : 0U;
    const auto maxEndHexCol = indent + 3U * bytesPerLine + extraInRow - 2U;
    const auto extraDivHex = isExtraSpc ? extraSpace.value() * 3U + 1U : 0U;

    result.beginHexCol = indent + (!row ? 3U * offset + extraInOffset : 0U);
    result.endHexCol = (row == lastRow) ?
        indent + 3U * lastRowBytes - 2U +
        (isExtraSpc ? (lastRowBytes - 1U) / extraSpace.value() : 0U)
        : maxEndHexCol;
    const auto beginAscCol =
        maxEndHexCol + 3U + (!row ? offset + extraInOffset : 0U);
    result.endCol = withAscii ? maxEndHexCol + 3U - 1U +
        (row == lastRow ?
         lastRowBytes + extraInLastRow : bytesPerLine + extraInRow)
        : result.endHexCol;

    if (column >= result.beginHexCol && column <= result.endHexCol &&
        (!extraDivHex ?  true :
         (column - indent) % extraDivHex + 1U != extraDivHex))
    {
        const auto extraColHex =
            extraDivHex ? (column - indent) / extraDivHex : 0U;
        const auto hexCol = column - indent - extraColHex;
        const auto byteOffset = hexCol / 3 +
            (!row ? (0 - offset) : firstRowBytes + (row - 1U) * bytesPerLine);

        if (byteOffset < size && hexCol % 3U != 2U)
        {
            // Set address an upper nibble flag only when corsor is
            // positioned on a hex byte.
            result.isUpperNibble = (hexCol % 3U == 0U);
            result.address = startAddress + byteOffset;
            result.type = HexDumpType::HexByte;
        }
    }

    if (withAscii && column >= beginAscCol)
    {
        const auto extraDivAsc = isExtraSpc ? extraSpace.value() + 1U : 0U;
        const auto extraColAsc =
            extraDivAsc ? (column - maxEndHexCol - 3U) / extraDivAsc : 0U;
        const auto asciiCol = column - maxEndHexCol - 3U - extraColAsc;
        const auto byteOffset = asciiCol +
                (!row ? (0 - offset) :
                 firstRowBytes + (row - 1U) * bytesPerLine);

        if (byteOffset < size && column <= result.endCol &&
            (!extraDivAsc ? true :
             (column - maxEndHexCol - 3U) % extraDivAsc + 1U != extraDivAsc))
        {
            result.address = startAddress + byteOffset;
            result.type = HexDumpType::AsciiChar;
        }
    }

    return result;
}

// Return row, column for a given address within a hex dump.
// Parameters:
//    address:      Address for which to return row, column
//    size:         Byte size of data
//    bytesPerLine: Number of bytes per line.
//    withAscii:    If true there are ASCII values on the right.
//    isDisplayAddress: If true the address is output on the right.
//    isAscii:      If true return row, column within ASCII values.
//                  If false return row, column within HEX values.
//    startAddress: First address with data.
//    extraSpace:   Optional, there is extra space after "extraSpace" bytes.
// If address is out of valid range std::nullopt is returned.
// If ASCII is not displayed and isAscii is true return row, column within
// HEX values.
std::optional<std::pair<DWord, DWord> > flx::get_hex_dump_position_for_address(
        DWord address,
        size_t size,
        DWord bytesPerLine,
        bool withAscii,
        bool isDisplayAddress,
        bool isAscii,
        bool isUpperNibble,
        DWord startAddress,
        std::optional<DWord> extraSpace)
{
    const auto offset = startAddress % bytesPerLine;
    const bool isExtraSpc = extraSpace.has_value();

    if (address < startAddress || address >= startAddress + size)
    {
         return std::nullopt;
    }

    std::pair<DWord, DWord> row_col;
    row_col.first = (address - (startAddress - offset)) / bytesPerLine;
    const auto indexInRow = (address - (startAddress - offset)) % bytesPerLine;
    const auto extraForAddress =
            isExtraSpc ? indexInRow / extraSpace.value() : 0U;
    row_col.second = isDisplayAddress ? 6U : 0U;
    if (withAscii && isAscii)
    {
        // Calculate column within ASCII values.
        const auto extraInRow =
            isExtraSpc ? (bytesPerLine - 1U) / extraSpace.value() : 0U;
        row_col.second += 3U * bytesPerLine + extraInRow + 1U + indexInRow +
            extraForAddress;
    }
    else
    {
        // Calculate column within HEX values.
        row_col.second += 3U * indexInRow + extraForAddress +
            (isUpperNibble ? 0U : 1U);
    }

    return row_col;
}

std::ostream &operator<<(std::ostream &os, flx::HexDumpType type)
{
    switch (type)
    {
        case flx::HexDumpType::NONE:
            os << "HexDumpType::NONE";
            break;

        case flx::HexDumpType::HexByte:
            os << "HexDumpType::HexByte";
            break;

        case flx::HexDumpType::AsciiChar:
            os << "HexDumpType::AsciiChar";
            break;
    }

    return os;
}
