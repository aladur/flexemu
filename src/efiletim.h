/*
    efiletim.h

    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2022-2026  W. Schwotzer

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

#ifndef EFILETIM_INCLUDED
#define EFILETIM_INCLUDED


#include <cstdint>
#include <type_traits>

// There is a FLEX extension to support file creation time (hour, minute) when
// creating a file on a FLEX file system.
enum class FileTimeAccess : uint8_t
{
    NONE = 0,
    Get = 1,
    Set = 2,
};

inline FileTimeAccess operator& (FileTimeAccess lhs, FileTimeAccess rhs)
{
    using T1 = std::underlying_type_t<FileTimeAccess>;

    return static_cast<FileTimeAccess>(static_cast<T1>(lhs) &
                                       static_cast<T1>(rhs));
}

inline FileTimeAccess operator| (FileTimeAccess lhs, FileTimeAccess rhs)
{
    using T1 = std::underlying_type_t<FileTimeAccess>;

    return static_cast<FileTimeAccess>(static_cast<T1>(lhs) |
                                       static_cast<T1>(rhs));
}

#endif

