/*
    sfpopts.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2022-2025  W. Schwotzer

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

#include "efiletim.h"
#include "efilesiz.h"
#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;


struct sFPOptions
{
    sFPOptions() = default;
    sFPOptions(const sFPOptions &src) = default;
    sFPOptions& operator=(const sFPOptions &src) = default;

    FileTimeAccess ft_access{};
    FileSizeType fileSizeType{};
    int iconSize{};
    std::string version;
    fs::path bootSectorFile;
    bool injectTextFileConvert{};
    bool injectTextFileAskUser{};
    bool extractTextFileConvert{};
    bool extractTextFileAskUser{};
    bool onTrack0OnlyDirSectors{};
    fs::path openDiskPath;
    fs::path openDirectoryPath;
    fs::path openInjectFilePath;
    std::vector<fs::path> recentDiskPaths;
    std::vector<fs::path> recentDirectoryPaths;

    static const int maxRecentFiles = 24;
    static const int maxRecentDirectories = 24;
};

#endif

