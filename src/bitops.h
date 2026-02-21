/*
    bitops.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2025-2026  W. Schwotzer

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

#ifndef BITOPERATIONS_INCLUDED
#define BITOPERATIONS_INCLUDED

#include "typedefs.h"
#include <cassert>

template<typename T> bool BTST(T value, unsigned bitpos)
{
    assert((bitpos >> 3U) < sizeof(T));
    return (value & static_cast<T>(static_cast<T>(1U) << bitpos)) != 0;
}

template<typename T> void BSET(T &value, unsigned bitpos)
{
    assert((bitpos >> 3U) < sizeof(T));
    value |= static_cast<T>(static_cast<T>(1U) << bitpos);
}

template<typename T> void BCLR(T &value, unsigned bitpos)
{
    assert((bitpos >> 3U) < sizeof(T));
    value &= static_cast<T>(~static_cast<T>(static_cast<T>(1U) << bitpos));
}

inline Word EXTEND8(Byte value)
{
    return static_cast<Word>(static_cast<SWord>(static_cast<SByte>(value)));
}

#endif
