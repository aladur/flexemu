/*
    filecntb.h

    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2018  W. Schwotzer

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

#ifndef __filecntb_h__
#define __filecntb_h__

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

#define RANDOM_FILE_LIST "random"
const unsigned int SECTOR_SIZE  = 256;

class FlexContainerInfo;

/* possible file attributes */
/* (WRITE_PROTECT also used for container attribute) */
enum
{
    WRITE_PROTECT   = 0x80,
    DELETE_PROTECT  = 0x40,
    READ_PROTECT    = 0x20,
    CATALOG_PROTECT = 0x10,
    ALL_PROTECT     = WRITE_PROTECT | DELETE_PROTECT | READ_PROTECT |
                      CATALOG_PROTECT
};

class FileContainerIfBase
{
public:
    virtual int  Close(void) = 0;
    virtual bool IsContainerOpened(void) const = 0;
    virtual bool IsWriteProtected(void) const = 0;
    virtual bool  GetInfo(FlexContainerInfo &info) const = 0;
    virtual int  GetContainerType(void) const = 0;
    virtual std::string GetPath(void) const = 0;
    virtual ~FileContainerIfBase() { };
};  /* class FileContainerIfBase */

#endif /* __filecntb_h__ */

