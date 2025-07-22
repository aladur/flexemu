/*
    free.h


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


#ifndef FREE_INCLUDED
#define FREE_INCLUDED

#include "typedefs.h"
#include "bintervl.h"
#include "memsrc.h"
#include <string>
#include <vector>
#include <utility>
#include <regex>
#include <optional>
#include <iostream>


using ItemPairList_t = std::vector<std::pair<
    std::string, std::vector<std::string> > >;

namespace flx
{
    extern std::string find_regex_string(const std::regex &regex,
            Byte eos, const std::vector<Byte> &data);
    extern bool is_range_in_ranges(
            const BInterval<DWord> &range,
            const MemorySource<DWord>::AddressRanges &ranges);
    extern void print_versions(std::ostream &os,
            const std::string &program_name);
    extern void hex_dump(std::ostream &os, const Byte *data, DWord size,
            DWord bytesPerLine,
            bool withAscii = true,
            bool isDisplayAddress = false,
            DWord startAddress = 0x0000U,
            std::optional<DWord> extraSpace = std::nullopt);
    extern void hex_dump_scale(std::ostream &os, DWord bytesPerLine,
            bool withAscii = true,
            bool isDisplayAddress = false,
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
            DWord row, DWord column,
            size_t size, DWord bytesPerLine,
            bool withAscii = true,
            bool isDisplayAddress = true,
            DWord startAddress = 0U,
            std::optional<DWord> extraSpace = std::nullopt);

    extern std::optional<std::pair<DWord, DWord> >
        get_hex_dump_position_for_address(
            DWord address,
            size_t size,
            DWord bytesPerLine,
            bool withAscii = true,
            bool isDisplayAddress = true,
            bool isAscii = false,
            bool isUpperNibble = true,
            DWord startAddress = 0U,
            std::optional<DWord> extraSpace = std::nullopt);
}
#endif
