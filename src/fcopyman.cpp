/*
    fcopyman.cpp


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

#include "misc1.h"
#include "filecont.h"
#include "fcopyman.h"
#include "fdirent.h"
#include "flexerr.h"
#include "fcinfo.h"
#include "ffilebuf.h"

bool FlexCopyManager::autoTextConversion = false;

// Return true if the copied file has been detected as text file.
// If false the file has been treated as binary file.
bool FlexCopyManager::FileCopy(const std::string &sourcName,
                               const std::string &destName,
                               IFlexDiskByFile &src, IFlexDiskByFile &dst)
{
    bool isTextFile = false;

    if (&src == &dst)
    {
        throw FlexException(FERR_COPY_ON_ITSELF, sourcName);
    }

    if (dst.IsWriteProtected())
    {
        FlexDiskAttributes diskAttributes;

        dst.GetAttributes(diskAttributes);
        throw FlexException(FERR_CONTAINER_IS_READONLY,
                            diskAttributes.GetPath());
    }

    auto fileBuffer = src.ReadToBuffer(sourcName);

    if ((src.GetFlexDiskType() & TYPE_DISKFILE) &&
        (dst.GetFlexDiskType() & TYPE_DIRECTORY) &&
        fileBuffer.IsFlexTextFile() && autoTextConversion)
    {
        fileBuffer.ConvertToTextFile();
        isTextFile = true;
    }

    if ((src.GetFlexDiskType() & TYPE_DIRECTORY) &&
        (dst.GetFlexDiskType() & TYPE_DISKFILE) &&
        fileBuffer.IsTextFile() && autoTextConversion)
    {
        fileBuffer.ConvertToFlexTextFile();
        isTextFile = true;
    }

    if (!dst.WriteFromBuffer(fileBuffer, destName.c_str()))
    {
        FlexDiskAttributes diskAttributes;

        dst.GetAttributes(diskAttributes);
        throw FlexException(FERR_DISK_FULL_WRITING, diskAttributes.GetPath(),
                            destName);
    }

    return isTextFile;
}
