/*
    crc.h

    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2020-2025  W. Schwotzer

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


#ifndef CRC_INCLUDED
#define CRC_INCLUDED

#include "typedefs.h"
#include <array>


template<typename T>
class Crc
{
    T crc;
    std::array<T, 256> crc_table;

    void Initialize(T polynomial)
    {
        for (Word dividend = 0U;
             dividend < static_cast<Word>(crc_table.size());
             ++dividend)
        {
            T current = static_cast<T>(dividend) << ((sizeof(T) * 8U) - 8U);
            for (Byte bit = 0U; bit < 8U; ++bit)
            {
                if ((current & (1U << ((sizeof(T) * 8U) - 1U))) != 0U)
                {
                    current <<= 1U;
                    current ^= polynomial;
                }
                else
                {
                    current <<= 1U;
                }
            }
            crc_table[dividend] = current;
        }
    }

public:
    Crc() = delete;
    Crc(const Crc&) = delete;
    Crc(Crc&&) = delete;
    Crc &operator=(Crc&&) = delete;
    Crc &operator=(const Crc&) = delete;
    ~Crc() = default;

    explicit Crc(T p_polynomial) : crc(0U)
    {
        Initialize(p_polynomial);
    }

    void Add(Byte value)
    {
        static const Byte shift = (sizeof(T) - 1U) * 8U;
        auto pos = static_cast<Byte>(
                (crc ^ (static_cast<unsigned>(value) << shift)) >> shift);
        crc = (crc * 256U) ^ crc_table[pos];
    }

    T GetResult(const Byte *begin, const Byte *end)
    {
        Reset();
        for (const auto *iter = begin; iter != end; ++iter)
        {
            Add(*iter);
        }

        return GetResult();
    }

    T GetResult()
    {
        return crc;
    }

    void Reset()
    {
        crc = 0U;
    }
};

#endif

