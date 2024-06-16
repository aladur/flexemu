/*
    fcinfo.cpp


    FLEXplorer, An explorer for FLEX disk image files and directory disks.
    Copyright (C) 1998-2024  W. Schwotzer

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

#include <stdio.h>

#include "fcinfo.h"
#include "filecntb.h"   // needed for containertypes


std::string FlexDiskAttributes::GetTypeString() const
{
    std::string str;

    if (type & TYPE_DISKFILE)
    {
        if (type & TYPE_DSK_DISKFILE)
        {
            str = "Disk image file, DSK format";
        }
        else if (type & TYPE_FLX_DISKFILE)
        {
            str = "Disk image file, FLX format";
        }
        else
        {
            str = "Disk image file";
        }
    }
    else if (type & TYPE_DIRECTORY)
    {
        str = "directory";
    }
    else
    {
        str = "Unknown type";
    }

    return str;
}

