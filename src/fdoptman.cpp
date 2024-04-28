/*
    fdoptman.cpp


    flexemu, an MC6809 emulator running FLEX
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

#include "misc1.h"
#include "fdoptman.h"
#include "brcfile.h"
#include <sys/stat.h>


const char * const FLEXDIRECTORYDISKTRACKS = "DirectoryDiskTracks";
const char * const FLEXDIRECTORYDISKSECTORS = "DirectoryDiskSectors";

const std::string FlexDirectoryDiskOptions::filename{".flexdiskrc"};

FlexDirectoryDiskOptions::FlexDirectoryDiskOptions(
        const std::string &directory)
    : path(directory)
    , tracks(0)
    , sectors(0)
{
    if (path[path.length() - 1] != PATHSEPARATOR)
    {
        path += PATHSEPARATOR;
    }
    path += filename;
}

bool FlexDirectoryDiskOptions::Read()
{
    struct stat sbuf{};

    if (!stat(path.c_str(), &sbuf) && (S_ISREG(sbuf.st_mode)))
    {
        BRcFile rcFile(path.c_str());

        if (!rcFile.GetValue(FLEXDIRECTORYDISKTRACKS, tracks) &&
            !rcFile.GetValue(FLEXDIRECTORYDISKSECTORS, sectors))
        {
            return true;
        }
    }

    return false;
}

bool FlexDirectoryDiskOptions::Write(bool onlyIfNotExists)
{
    struct stat sbuf{};

    if (onlyIfNotExists && !stat(path.c_str(), &sbuf))
    {
        return true;
    }

    BRcFile rcFile(path.c_str());

    return !rcFile.SetValue(FLEXDIRECTORYDISKTRACKS, tracks) &&
           !rcFile.SetValue(FLEXDIRECTORYDISKSECTORS, sectors);
}

int FlexDirectoryDiskOptions::GetTracks() const
{
    return tracks;
}

int FlexDirectoryDiskOptions::GetSectors() const
{
    return sectors;
}

void FlexDirectoryDiskOptions::SetTracks(int p_tracks)
{
    tracks = p_tracks;
}

void FlexDirectoryDiskOptions::SetSectors(int p_sectors)
{
    sectors = p_sectors;
}

