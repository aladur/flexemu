/*
    fpoptman.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2022-2026  W. Schwotzer

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
#include "efiletim.h"
#include "efilesiz.h"
#include "sfpopts.h"
#include "filecntb.h"
#include "fpoptman.h"
#ifdef _WIN32
#include "bregistr.h"
#endif
#include "brcfile.h"
#include <string>
#include <sstream>
#include <algorithm>
#include <filesystem>


namespace fs = std::filesystem;

#ifdef _WIN32
constexpr const char * const FLEXPLOREREG = u8"SOFTWARE\\Gnu\\FLEXplorer";
#endif

constexpr const auto * const OLDFLEXPLORERRC = u8".flexplorerrc";
constexpr const auto * const FLEXPLORERRC = u8"flexplorerrc";

constexpr const char * const FLEXPLORERBOOTSECTORFILE = "BootSectorFile";
constexpr const char * const FLEXPLOREREXTRACTCNV = "ExtractTextFileConvert";
constexpr const char * const FLEXPLOREREXTRACTASK = "ExtractTextFileAskUser";
constexpr const char * const FLEXPLORERFILESIZETYPE = "FileSizeType";
constexpr const char * const FLEXPLORERFILETIMEACCESS = "FileTimeAccess";
constexpr const char * const FLEXPLORERICONSIZE = "IconSize";
constexpr const char * const FLEXPLORERINJECTASK = "InjectTextFileAskUser";
constexpr const char * const FLEXPLORERINJECTCNV = "InjectTextFileConvert";
constexpr const char * const FLEXPLOREROPENDIRECTORYPATH = "OpenDirectoryPath";
constexpr const char * const FLEXPLOREROPENDISKPATH = "OpenDiskPath";
constexpr const char * const FLEXPLOREROPENINJECTFILEPATH = "OpenInjectFilePath";
constexpr const char * const FLEXPLORERRECENTDIRECTORY = "RecentDirectoryPath";
constexpr const char * const FLEXPLORERRECENTDISKPATH = "RecentDiskPath";
constexpr const char * const FLEXPLORERTRACK0ONLYDIRSEC = "OnTrack0OnlyDirSectors";
constexpr const char * const FLEXPLORERVERSION = "Version";

void FlexplorerOptions::InitOptions(struct sFPOptions &options)
{
    options.version = VERSION;
    options.ft_access = FileTimeAccess::NONE;
    options.iconSize = 32;
#ifdef _WIN32
    options.bootSectorFile = flx::getExecutablePath() / BOOT_FILE;
#else
    options.bootSectorFile = fs::path(F_DATADIR) / BOOT_FILE;
#endif
    options.injectTextFileConvert = true;
    options.injectTextFileAskUser = true;
    options.extractTextFileConvert = true;
    options.extractTextFileAskUser = true;
    options.onTrack0OnlyDirSectors = true;
    options.fileSizeType = FileSizeType::FileSize;
    options.openInjectFilePath = flx::getHomeDirectory();
#ifdef _WIN32
    options.openDiskPath = flx::getExecutablePath() / u8"Data";
#else
    options.openDiskPath = F_DATADIR;
#endif
    options.openDirectoryPath = flx::getHomeDirectory();
}

void FlexplorerOptions::WriteOptions(const struct sFPOptions &options)
{
#ifdef _WIN32
    BRegistry reg(BRegistry::currentUser, FLEXPLOREREG);
    reg.SetValue(FLEXPLORERVERSION, std::string(VERSION));
    reg.SetValue(FLEXPLORERBOOTSECTORFILE, options.bootSectorFile);
    reg.SetValue(FLEXPLORERFILETIMEACCESS,
                 static_cast<int>(options.ft_access));
    reg.SetValue(FLEXPLORERICONSIZE, options.iconSize);
    reg.SetValue(FLEXPLORERINJECTCNV, options.injectTextFileConvert ? 1 : 0);
    reg.SetValue(FLEXPLORERINJECTASK, options.injectTextFileAskUser ? 1 : 0);
    reg.SetValue(FLEXPLOREREXTRACTCNV, options.extractTextFileConvert ? 1 : 0);
    reg.SetValue(FLEXPLOREREXTRACTASK, options.extractTextFileAskUser ? 1 : 0);
    reg.SetValue(FLEXPLORERTRACK0ONLYDIRSEC,
                 options.onTrack0OnlyDirSectors ? 1 : 0);
    reg.SetValue(FLEXPLOREROPENDISKPATH, options.openDiskPath);
    reg.SetValue(FLEXPLOREROPENDIRECTORYPATH, options.openDirectoryPath);
    reg.SetValue(FLEXPLORERFILESIZETYPE,
                 static_cast<int>(options.fileSizeType));
    reg.SetValue(FLEXPLOREROPENINJECTFILEPATH, options.openInjectFilePath);
    for (auto i = 0U; i < options.recentDiskPaths.size(); ++i)
    {
        std::stringstream key;

        key << FLEXPLORERRECENTDISKPATH << i;
        reg.SetValue(key.str(), options.recentDiskPaths[i]);
    }
    for (auto i = 0U; i < options.recentDirectoryPaths.size(); ++i)
    {
        std::stringstream key;

        key << FLEXPLORERRECENTDIRECTORY << i;
        reg.SetValue(key.str(), options.recentDirectoryPaths[i]);
    }
#else
    auto rcFilePath = flx::getFlexemuUserConfigPath() / FLEXPLORERRC;
    fs::create_directories(flx::getFlexemuUserConfigPath());

    BRcFile rcFile(rcFilePath);
    rcFile.Initialize(); // truncate file
    rcFile.SetValue(FLEXPLORERVERSION, std::string(VERSION));
    rcFile.SetValue(FLEXPLORERBOOTSECTORFILE, options.bootSectorFile);
    rcFile.SetValue(FLEXPLORERFILETIMEACCESS,
                    static_cast<int>(options.ft_access));
    rcFile.SetValue(FLEXPLORERICONSIZE, options.iconSize);
    rcFile.SetValue(FLEXPLORERINJECTCNV, options.injectTextFileConvert ? 1 : 0);
    rcFile.SetValue(FLEXPLORERINJECTASK, options.injectTextFileAskUser ? 1 : 0);
    rcFile.SetValue(FLEXPLOREREXTRACTCNV,
                    options.extractTextFileConvert ? 1 : 0);
    rcFile.SetValue(FLEXPLOREREXTRACTASK,
                    options.extractTextFileAskUser ? 1 : 0);
    rcFile.SetValue(FLEXPLORERTRACK0ONLYDIRSEC,
                    options.onTrack0OnlyDirSectors ? 1 : 0);
    rcFile.SetValue(FLEXPLOREROPENDISKPATH, options.openDiskPath);
    rcFile.SetValue(FLEXPLOREROPENDIRECTORYPATH, options.openDirectoryPath);
    rcFile.SetValue(FLEXPLORERFILESIZETYPE,
                    static_cast<int>(options.fileSizeType));
    rcFile.SetValue(FLEXPLOREROPENINJECTFILEPATH, options.openInjectFilePath);

    for (auto i = 0U; i < options.recentDiskPaths.size(); ++i)
    {
        std::stringstream key;

        key << FLEXPLORERRECENTDISKPATH << i;
        rcFile.SetValue(key.str().c_str(), options.recentDiskPaths[i]);
    }
    for (auto i = 0U; i < options.recentDirectoryPaths.size(); ++i)
    {
        std::stringstream key;

        key << FLEXPLORERRECENTDIRECTORY << i;
        rcFile.SetValue(key.str().c_str(), options.recentDirectoryPaths[i]);
    }

    const auto oldRcFilePath = flx::getHomeDirectory() / OLDFLEXPLORERRC;
    if (fs::exists(oldRcFilePath))
    {
        fs::remove(oldRcFilePath);
    }
#endif
}

void FlexplorerOptions::ReadOptions(struct sFPOptions &options)
{
    int int_result;
    fs::path path_result;

#ifdef _WIN32
    BRegistry reg(BRegistry::currentUser, FLEXPLOREREG);

    reg.GetValue(FLEXPLORERVERSION, options.version);
    reg.GetValue(FLEXPLORERBOOTSECTORFILE, options.bootSectorFile);

    if (!reg.GetValue(FLEXPLORERFILETIMEACCESS, int_result))
    {
        int_result = std::max(int_result, 0);
        if (int_result == 2 || int_result > 3)
        {
            int_result = 3;
        }
        options.ft_access = static_cast<FileTimeAccess>(int_result);
    }
    if (!reg.GetValue(FLEXPLORERICONSIZE, int_result))
    {
        options.iconSize = (int_result >= 24) ? 24 : 16;
        options.iconSize = (int_result >= 32) ? 32 : options.iconSize;
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
    if (!reg.GetValue(FLEXPLORERTRACK0ONLYDIRSEC, int_result))
    {
        options.onTrack0OnlyDirSectors = (int_result != 0);
    }
    reg.GetValue(FLEXPLOREROPENDISKPATH, options.openDiskPath);
    reg.GetValue(FLEXPLOREROPENDIRECTORYPATH, options.openDirectoryPath);
    reg.GetValue(FLEXPLORERFILESIZETYPE, int_result);
    int_result = std::max(int_result, 1);
    int_result = std::min(int_result, 2);
    options.fileSizeType = static_cast<FileSizeType>(int_result);
    reg.GetValue(FLEXPLOREROPENINJECTFILEPATH, options.openInjectFilePath);

    for (auto i = 0; i < options.maxRecentFiles; ++i)
    {
        std::stringstream key;

        key << FLEXPLORERRECENTDISKPATH << i;
        if (ERROR_SUCCESS == reg.GetValue(key.str(), path_result))
        {
            options.recentDiskPaths.push_back(path_result);
        }
    }
    for (auto i = 0; i < options.maxRecentDirectories; ++i)
    {
        std::stringstream key;

        key << FLEXPLORERRECENTDIRECTORY << i;
        if (ERROR_SUCCESS == reg.GetValue(key.str(), path_result))
        {
            options.recentDirectoryPaths.push_back(path_result);
        }
    }
#else
    auto rcFilePath = flx::getFlexemuUserConfigPath() / FLEXPLORERRC;
    if (!fs::exists(rcFilePath))
    {
        rcFilePath = flx::getHomeDirectory() / OLDFLEXPLORERRC;
    }
    BRcFile rcFile(rcFilePath);

    rcFile.GetValue(FLEXPLORERVERSION, options.version);
    rcFile.GetValue(FLEXPLORERBOOTSECTORFILE, options.bootSectorFile);

    if (!rcFile.GetValue(FLEXPLORERFILETIMEACCESS, int_result))
    {
        int_result = std::max(int_result, 0);
         if (int_result == 2 || int_result > 3)
        {
            int_result = 3;
        }
        options.ft_access = static_cast<FileTimeAccess>(int_result);
    }
    if (!rcFile.GetValue(FLEXPLORERICONSIZE, int_result))
    {
        options.iconSize = (int_result >= 24) ? 24 : 16;
        options.iconSize = (int_result >= 32) ? 32 : options.iconSize;
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
    if (!rcFile.GetValue(FLEXPLORERTRACK0ONLYDIRSEC, int_result))
    {
        options.onTrack0OnlyDirSectors = (int_result != 0);
    }
    rcFile.GetValue(FLEXPLOREROPENDISKPATH, options.openDiskPath);
    rcFile.GetValue(FLEXPLOREROPENDIRECTORYPATH, options.openDirectoryPath);
    rcFile.GetValue(FLEXPLORERFILESIZETYPE, int_result);
    int_result = std::max(int_result, 1);
    int_result = std::min(int_result, 2);
    options.fileSizeType = static_cast<FileSizeType>(int_result);
    rcFile.GetValue(FLEXPLOREROPENINJECTFILEPATH, options.openInjectFilePath);

    for (auto i = 0; i < sFPOptions::maxRecentFiles; ++i)
    {
        std::stringstream key;

        key << FLEXPLORERRECENTDISKPATH << i;
        if (BRC_NO_ERROR == rcFile.GetValue(key.str().c_str(), path_result))
        {
            options.recentDiskPaths.emplace_back(path_result);
        }
    }
    for (auto i = 0; i < sFPOptions::maxRecentDirectories; ++i)
    {
        std::stringstream key;

        key << FLEXPLORERRECENTDIRECTORY << i;
        if (BRC_NO_ERROR == rcFile.GetValue(key.str().c_str(), path_result))
        {
            options.recentDirectoryPaths.emplace_back(path_result);
        }
    }
#endif
}

