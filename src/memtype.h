/*
    memtype.h


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


#ifndef MEMORY_TYPE_INCLUDED
#define MEMORY_TYPE_INCLUDED

#include "typedefs.h"
#include "bintervl.h"
#include <cstdint>
#include <vector>
#include <ostream>

enum class MemoryType : uint8_t
{
    NONE = 0U,
    ROM = 1U,
    RAM = 3U,
};

inline std::ostream &operator<<(std::ostream &os, MemoryType type)
{
    switch (type)
    {
        case MemoryType::NONE:
            os << "None";
            break;

        case MemoryType::ROM:
            os << "ROM";
            break;

        case MemoryType::RAM:
            os << "RAM";
            break;
    }

    return os;
}

struct sMemoryRangeWithType
{
    MemoryType type;
    BInterval<DWord> addressRange;
};

using MemoryRanges_t = std::vector<sMemoryRangeWithType>;
#endif

