/*
    typedefs.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2025  W. Schwotzer

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


#ifndef TYPEDEFS_INCLUDED
#define TYPEDEFS_INCLUDED

#include <cstdint>

using Byte = uint8_t;
using SByte = int8_t;
using Word = uint16_t;
using SWord = int16_t;
using DWord = uint32_t;
using SDWord = int32_t;
using QWord = uint64_t;
using SQWord = int64_t;
using cycles_t = uint64_t;

#endif /* TYPEDEFS_INCLUDED */

