/*
    fattrib.h

    FLEXplorer, An explorer for FLEX disk image files and directory disks.
    Copyright (C) 2024  W. Schwotzer

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

#ifndef FATTRIB_INCLUDED
#define FATTRIB_INCLUDED

#include "typedefs.h"
#include <array>
#include <utility>


// Supported file attributes.
// They are used as bit masks and can be combined.
// File attributes are used in struct s_dir_entry in field file_attr.
// All other bits of file_attr should remain 0.
// (WRITE_PROTECT also used for container attribute)
const unsigned WRITE_PROTECT = 0x80;
const unsigned DELETE_PROTECT = 0x40;
const unsigned READ_PROTECT = 0x20;
const unsigned CATALOG_PROTECT = 0x10;
const unsigned ALL_PROTECT = WRITE_PROTECT | DELETE_PROTECT | READ_PROTECT |
                         CATALOG_PROTECT;

using AttributeCharToFlag_t = std::array<std::pair<char, unsigned>, 4>;
extern const AttributeCharToFlag_t &GetAttributeCharToFlag();

#endif

