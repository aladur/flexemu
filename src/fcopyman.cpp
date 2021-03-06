/*
    fcopyman.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2021  W. Schwotzer

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

bool    FlexCopyManager::FileCopy(const char *srcName, const char *destName,
                                  FileContainerIf &src, FileContainerIf &dst)
{
    if (&src == &dst)
    {
        throw FlexException(FERR_COPY_ON_ITSELF, std::string(srcName));
    }

    if (dst.IsWriteProtected())
    {
        FlexContainerInfo info;

        dst.GetInfo(info);
        throw FlexException(FERR_CONTAINER_IS_READONLY, info.GetName());
    }

    auto fileBuffer = src.ReadToBuffer(srcName);

    if ((src.GetContainerType() & TYPE_CONTAINER) &&
        (dst.GetContainerType() & TYPE_DIRECTORY) &&
        fileBuffer.IsFlexTextFile() && autoTextConversion)
    {
        fileBuffer.ConvertToTextFile();
    }

    if ((src.GetContainerType() & TYPE_DIRECTORY) &&
        (dst.GetContainerType() & TYPE_CONTAINER) &&
        fileBuffer.IsTextFile() && autoTextConversion)
    {
        fileBuffer.ConvertToFlexTextFile();
    }

    if (!dst.WriteFromBuffer(fileBuffer, destName))
    {
        FlexContainerInfo info;

        dst.GetInfo(info);
        throw FlexException(FERR_DISK_FULL_WRITING, info.GetPath().c_str(),
                            destName);
    }

    return true;
}
