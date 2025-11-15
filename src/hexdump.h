/*
    hexdump.h


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


#ifndef HEXDUMP_INCLUDED
#define HEXDUMP_INCLUDED

#include "typedefs.h"
#include <optional>
#include <string>
#include <vector>
#include <ostream>


namespace flx
{
    extern std::vector<Byte> get_bytes_from_hex(const std::string &hex_values);
    extern void hex_dump(
            std::ostream &os,
            const Byte *data,
            size_t size,
            DWord bytesPerLine,
            bool withAscii,
            bool isDisplayAddress,
            DWord startAddress,
            std::optional<DWord> extraSpace = std::nullopt);
    extern void hex_dump_scale(
            std::ostream &os,
            DWord bytesPerLine,
            bool withAscii,
            bool isDisplayAddress,
            std::optional<DWord> extraSpace = std::nullopt);

    enum class HexDumpType : uint8_t
    {
        NONE,
        HexByte,
        AsciiChar,
    };

    struct sHexDumpProperties
    {
        HexDumpType type{};  // The item type
        bool isUpperNibble{};// if type is HexByte: if true it is the upper
                             // nibble, if false the lower nibble
        DWord beginHexCol{}; // The column of first hex byte
        DWord endHexCol{};   // The column of last hex byte
        DWord endCol{};      // The column of last char in line
        DWord address{};     // The address if type is HexByte or AsciiChar
    };

    extern sHexDumpProperties get_hex_dump_properties(
            DWord row,
            DWord column,
            size_t size,
            DWord bytesPerLine,
            bool withAscii,
            bool isDisplayAddress,
            DWord startAddress,
            std::optional<DWord> extraSpace = std::nullopt);

    extern std::optional<std::pair<DWord, DWord> >
        get_hex_dump_position_for_address(
            DWord address,
            size_t size,
            DWord bytesPerLine,
            bool withAscii,
            bool isDisplayAddress,
            bool isAscii,
            bool isUpperNibble,
            DWord startAddress,
            std::optional<DWord> extraSpace = std::nullopt);
}

extern std::ostream &operator<<(std::ostream &os, flx::HexDumpType type);

#endif
