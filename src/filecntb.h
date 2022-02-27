/*
    filecntb.h

    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2022  W. Schwotzer

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

#include "misc1.h"
#include <string>


/* possible constants for container type */

const int TYPE_CONTAINER    = 0x01; /* type: file container */
const int TYPE_DIRECTORY    = 0x02; /* type: directory */
const int TYPE_DSK_CONTAINER    =
    0x10; /* subtype: a file container with DSK format */
const int TYPE_FLX_CONTAINER    =
    0x20; /* subtype: a file container with FLX format */
const int TYPE_NAFS_DIRECTORY   = 0x40; /* subtype: NAFS directory */
/* (means: without text conversion) */
const int TYPE_RAM_CONTAINER    =
    0x80; /* subtype: filecontainer loaded in RAM */

// This macro defines the name of a file. It contains the boot sector.
// It is used in directory containers to be able to boot from them.
#define BOOT_FILE "boot"

const unsigned int SECTOR_SIZE  = 256;

class FlexContainerInfo;

// Supported file attributes.
// They are used as bit masks and can be combined.
// File attributes are used in struct s_dir_entry in field file_attr.
// All other bits of file_attr should remain 0.
// (WRITE_PROTECT also used for container attribute)
enum
{
    WRITE_PROTECT   = 0x80,
    DELETE_PROTECT  = 0x40,
    READ_PROTECT    = 0x20,
    CATALOG_PROTECT = 0x10,
    ALL_PROTECT     = WRITE_PROTECT | DELETE_PROTECT | READ_PROTECT |
                      CATALOG_PROTECT
};

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

class FileContainerIfBase
{
public:
    virtual bool IsWriteProtected() const = 0;
    virtual bool  GetInfo(FlexContainerInfo &info) const = 0;
    virtual int  GetContainerType() const = 0;
    virtual std::string GetPath() const = 0;
    virtual ~FileContainerIfBase() { };
};  /* class FileContainerIfBase */

#endif /* FILECNTB_INCLUDED */

