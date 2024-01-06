/*
    fpoptman.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2022-2023  W. Schwotzer

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
#include <string>
#include "sfpopts.h"
#include "filecntb.h"
#include "fpoptman.h"
#include "bregistr.h"
#include "brcfile.h"


void FlexplorerOptions::InitOptions(struct sFPOptions &options)
{
    options.ft_access = FileTimeAccess::NONE;
#ifdef UNIX
    options.bootSectorFile = F_DATADIR PATHSEPARATORSTRING BOOT_FILE;
#endif
#ifdef _WIN32
    options.bootSectorFile =
        getExecutablePath() + PATHSEPARATORSTRING BOOT_FILE;
#endif
    options.injectTextFileConvert = true;
    options.injectTextFileAskUser = true;
    options.extractTextFileConvert = true;
    options.extractTextFileAskUser = true;
    options.fileSizeType = FileSizeType::FileSize;
#ifdef UNIX
    options.openContainerPath = F_DATADIR;
#endif
}

void FlexplorerOptions::WriteOptions(const struct sFPOptions &options)
{
#ifdef _WIN32
    BRegistry reg(BRegistry::currentUser, FLEXPLOREREG);
    reg.SetValue(FLEXPLORERBOOTSECTORFILE, options.bootSectorFile);
    reg.SetValue(FLEXFILETIMEACCESS, static_cast<int>(options.ft_access));
    reg.SetValue(FLEXPLORERINJECTCNV, options.injectTextFileConvert ? 1 : 0);
    reg.SetValue(FLEXPLORERINJECTASK, options.injectTextFileAskUser ? 1 : 0);
    reg.SetValue(FLEXPLOREREXTRACTCNV, options.extractTextFileConvert ? 1 : 0);
    reg.SetValue(FLEXPLOREREXTRACTASK, options.extractTextFileAskUser ? 1 : 0);
    reg.SetValue(FLEXPLOREROPENCONTPATH, options.openContainerPath);
    reg.SetValue(FLEXPLOREROPENDIRCONTPATH, options.openDirContainerPath);
    reg.SetValue(FLEXPLORERFILESIZETYPE,
                 static_cast<int>(options.fileSizeType));
#endif
#ifdef UNIX
    const auto rcFileName = getHomeDirectory() +
                            PATHSEPARATORSTRING FLEXPLORERRC;

    BRcFile rcFile(rcFileName.c_str());
    rcFile.Initialize(); // truncate file
    rcFile.SetValue(FLEXPLORERBOOTSECTORFILE, options.bootSectorFile.c_str());
    rcFile.SetValue(FLEXFILETIMEACCESS, static_cast<int>(options.ft_access));
    rcFile.SetValue(FLEXPLORERINJECTCNV, options.injectTextFileConvert ? 1 : 0);
    rcFile.SetValue(FLEXPLORERINJECTASK, options.injectTextFileAskUser ? 1 : 0);
    rcFile.SetValue(FLEXPLOREREXTRACTCNV,
                    options.extractTextFileConvert ? 1 : 0);
    rcFile.SetValue(FLEXPLOREREXTRACTASK,
                    options.extractTextFileAskUser ? 1 : 0);
    rcFile.SetValue(FLEXPLOREROPENCONTPATH, options.openContainerPath.c_str());
    rcFile.SetValue(FLEXPLOREROPENDIRCONTPATH,
                    options.openDirContainerPath.c_str());
    rcFile.SetValue(FLEXPLORERFILESIZETYPE,
                    static_cast<int>(options.fileSizeType));
#endif
}

void FlexplorerOptions::ReadOptions(struct sFPOptions &options)
{
    int int_result;
    std::string string_result;

#ifdef _WIN32
    BRegistry reg(BRegistry::currentUser, FLEXPLOREREG);

    if (!reg.GetValue(FLEXPLORERBOOTSECTORFILE, string_result) &&
        !string_result.empty())
    {
        options.bootSectorFile = string_result;
    }

    if (!reg.GetValue(FLEXFILETIMEACCESS, int_result))
    {
        int_result = std::max(int_result, 0);
        if (int_result == 2 || int_result > 3)
        {
            int_result = 3;
        }
        options.ft_access = static_cast<FileTimeAccess>(int_result);
    }
    if (!reg.GetValue(FLEXPLORERINJECTCNV, int_result))
    {
        options.injectTextFileConvert = (int_result != 0);
    }
    if (!reg.GetValue(FLEXPLORERINJECTASK, int_result))
    {
        options.injectTextFileAskUser = (int_result != 0);
    }
    if (!reg.GetValue(FLEXPLOREREXTRACTCNV, int_result))
    {
        options.extractTextFileConvert = (int_result != 0);
    }
    if (!reg.GetValue(FLEXPLOREREXTRACTASK, int_result))
    {
        options.extractTextFileAskUser = (int_result != 0);
    }
    reg.GetValue(FLEXPLOREROPENCONTPATH, options.openContainerPath);
    reg.GetValue(FLEXPLOREROPENDIRCONTPATH, options.openDirContainerPath);
    reg.GetValue(FLEXPLORERFILESIZETYPE, int_result);
    int_result = std::max(int_result, 1);
    int_result = std::min(int_result, 2);
    options.fileSizeType = static_cast<FileSizeType>(int_result);
#endif
#ifdef UNIX
    const auto rcFileName =
        getHomeDirectory() + PATHSEPARATORSTRING FLEXPLORERRC;
    BRcFile rcFile(rcFileName.c_str());

    if (!rcFile.GetValue(FLEXPLORERBOOTSECTORFILE, string_result) &&
        !string_result.empty())
    {
        options.bootSectorFile = string_result;
    }

    if (!rcFile.GetValue(FLEXFILETIMEACCESS, int_result))
    {
        int_result = std::max(int_result, 0);
         if (int_result == 2 || int_result > 3)
        {
            int_result = 3;
        }
        options.ft_access = static_cast<FileTimeAccess>(int_result);
    }
    if (!rcFile.GetValue(FLEXPLORERINJECTCNV, int_result))
    {
        options.injectTextFileConvert = (int_result != 0);
    }
    if (!rcFile.GetValue(FLEXPLORERINJECTASK, int_result))
    {
        options.injectTextFileAskUser = (int_result != 0);
    }
    if (!rcFile.GetValue(FLEXPLOREREXTRACTCNV, int_result))
    {
        options.extractTextFileConvert = (int_result != 0);
    }
    if (!rcFile.GetValue(FLEXPLOREREXTRACTASK, int_result))
    {
        options.extractTextFileAskUser = (int_result != 0);
    }
    rcFile.GetValue(FLEXPLOREROPENCONTPATH, options.openContainerPath);
    rcFile.GetValue(FLEXPLOREROPENDIRCONTPATH, options.openDirContainerPath);
    rcFile.GetValue(FLEXPLORERFILESIZETYPE, int_result);
    int_result = std::max(int_result, 1);
    int_result = std::min(int_result, 2);
    options.fileSizeType = static_cast<FileSizeType>(int_result);
#endif
}

