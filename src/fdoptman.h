/*
    fdoptman.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2024-2025  W. Schwotzer

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

#ifndef FDOPTMAN_INCLUDED
#define FDOPTMAN_INCLUDED

#include "misc1.h"
#include <string>


class FlexDirectoryDiskOptions
{
public:
    FlexDirectoryDiskOptions() = delete;
    explicit FlexDirectoryDiskOptions(std::string directory);
    bool Read();
    bool Write(bool onlyIfNotExists);
    int GetTracks() const;
    int GetSectors() const;
    void SetTracks(int tracks);
    void SetSectors(int sectors);

private:
    static const std::string &GetRcFilename();

    std::string path;
    int tracks;
    int sectors;
};

#endif // FOPTMAN_INCLUDED

