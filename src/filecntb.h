/*
    filecntb.h

    FLEXplorer, An explorer for FLEX disk image files and directory disks.
    Copyright (C) 1998-2025  W. Schwotzer

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

#ifndef FILECNTB_INCLUDED
#define FILECNTB_INCLUDED

#include "typedefs.h"
#include <cstdint>
#include <filesystem>

namespace fs = std::filesystem;


// This macro defines the name of a file. It contains the boot sector.
// It is used in directory containers to be able to boot from them.
constexpr const auto *BOOT_FILE = u8"boot";

const int SECTOR_SIZE = 256;

class FlexDiskAttributes;

// Magic number to mark a file as random access file. This byte is stored
// in s_dir_entry in field sector_map.
const Byte IS_RANDOM_FILE = 0x02;
// Number of directory entries in one directory sector, struct s_dir_sector
const Byte DIRENTRIES = 10;

// Max. length of the diskname, without terminating NUL
const size_t FLEX_DISKNAME_LENGTH = 8U;
// Max. length of the disk extension, without terminating NUL
const size_t FLEX_DISKEXT_LENGTH = 3U;
// Max. length of the file basename, without extension, without term. NUL
const size_t FLEX_BASEFILENAME_LENGTH = 8U;
// Max. length of the file extension, without terminating NUL
const size_t FLEX_FILEEXT_LENGTH = 3U;
// Max. length of a FLEX filename incl. dot and terminating NUL
const size_t FLEX_FILENAME_LENGTH =
                 FLEX_BASEFILENAME_LENGTH + FLEX_FILEEXT_LENGTH + 2U;

enum class DiskType : uint8_t
{
    DSK, // A *.dsk (or *.wta) disk image file.
    FLX, // A *.flx disk image file.
    Directory, // A directory disk.
};

// The read-only options a disk image or directory disk can have.
enum class DiskOptions : uint8_t
{
    NONE = 0, // No options set.
    JvcHeader = 1, // A *.dsk (or *.wta) disk image file with a JVC header.
    HasSectorIF = 2, // Has a sector oriented interface.
    RAM = 4, // a disk image file fully loaded in RAM.
};

// This interface gives basic properties access to a FLEX disk image.
// Rename: FileContainerIfBase => IFlexDiskBase
class IFlexDiskBase
{
public:
    virtual bool IsWriteProtected() const = 0;
    virtual bool GetDiskAttributes(FlexDiskAttributes
            &diskAttributes) const = 0;
    virtual DiskType GetFlexDiskType() const = 0;
    virtual DiskOptions GetFlexDiskOptions() const = 0;
    virtual fs::path GetPath() const = 0;

    virtual ~IFlexDiskBase() = default;
};

inline DiskOptions operator| (DiskOptions lhs, DiskOptions rhs)
{
    using TYPE = std::underlying_type_t<DiskOptions>;

    return static_cast<DiskOptions>(static_cast<TYPE>(lhs) |
                                    static_cast<TYPE>(rhs));
}

inline DiskOptions operator& (DiskOptions lhs, DiskOptions rhs)
{
    using TYPE = std::underlying_type_t<DiskOptions>;

    return static_cast<DiskOptions>(static_cast<TYPE>(lhs) &
                                    static_cast<TYPE>(rhs));
}

inline DiskOptions operator|= (DiskOptions &lhs, DiskOptions rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

inline DiskOptions operator&= (DiskOptions &lhs, DiskOptions rhs)
{
    lhs = lhs & rhs;
    return lhs;
}

#endif /* FILECNTB_INCLUDED */

