/*
    fcinfo.cpp


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


#include "fcinfo.h"
#include "filecntb.h"
#include <string>


std::string FlexDiskAttributes::GetTypeString() const
{
    std::string str;

    switch (type)
    {
        case DiskType::DSK:
            str = "Disk image file, DSK format";
            break;

        case DiskType::FLX:
            str = "Disk image file, FLX format";
            break;

        case DiskType::Directory:
            str = "directory";
            break;

        default:
            str = "Unknown type";
            break;
    }

    return str;
}

