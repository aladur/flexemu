/*
    sfpopts.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2022-2024  W. Schwotzer

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



#ifndef SFPOPTS_INCLUDED
#define SFPOPTS_INCLUDED

#include <string>
#include <vector>
#include "efiletim.h"
#include "efilesiz.h"

struct sFPOptions
{
    sFPOptions() = default;
    sFPOptions(const sFPOptions &src) = default;
    sFPOptions& operator=(const sFPOptions &src) = default;

    FileTimeAccess ft_access;
    FileSizeType fileSizeType;
    std::string bootSectorFile;
    bool injectTextFileConvert;
    bool injectTextFileAskUser;
    bool extractTextFileConvert;
    bool extractTextFileAskUser;
    bool onTrack0OnlyDirSectors;
    std::string openDiskPath;
    std::string openDirectoryPath;
    std::string openInjectFilePath;
    std::vector<std::string> recentDiskPaths;

    static const int maxRecentFiles = 24;
};

#endif

