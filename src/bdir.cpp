/*
    bdir.cpp


    Basic class used for directory functions
    Copyright (C) 1999-2025  W. Schwotzer

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

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include "config.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_DIRENT_H
    #include <dirent.h>
#endif

#ifdef _WIN32
    #ifdef _MSC_VER
        #include <direct.h>
    #endif
#endif

#include "bdir.h"
#ifdef _WIN32
#include <cvtwchar.h>
#endif
#include <filesystem>


namespace fs = std::filesystem;


/********************************************
 static functions
********************************************/

PathList_t BDirectory::GetSubDirectories(const fs::path &p_path)
{
    PathList_t subDirList;
#ifdef _WIN32
    WIN32_FIND_DATA pentry;

    auto hdl = FindFirstFile((p_path / "*.*").wstring().c_str(), &pentry);
    if (hdl != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (pentry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY &&
                pentry.cFileName[0] != '.')
            {
                subDirList.push_back(ConvertToUtf8String(pentry.cFileName));
            }
        }
        while (FindNextFile(hdl, &pentry) != 0);

        FindClose(hdl);
    }

#else
    fs::path dirEntry;

    auto *pd = opendir(p_path.u8string().c_str());
    if (pd != nullptr)
    {
        struct dirent *pentry;

        while ((pentry = readdir(pd)) != nullptr)
        {
            dirEntry = p_path / pentry->d_name;
            const auto status = fs::status(dirEntry);

            if (fs::exists(status) && fs::is_directory(status) &&
                    pentry->d_name[0] != '.')
            {
                subDirList.emplace_back(pentry->d_name);
            }
        }

        closedir(pd);
    }

#endif
    return subDirList;
}

PathList_t BDirectory::GetFiles(const fs::path &p_path)
{
    PathList_t fileList;
#ifdef _WIN32
    WIN32_FIND_DATA pentry;

    auto hdl = FindFirstFile((p_path / "*.*").wstring().c_str(), &pentry);
    if (hdl != INVALID_HANDLE_VALUE)
    {
        do
        {
            if ((pentry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 &&
                (pentry.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE) == 0 &&
                (pentry.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) == 0)
            {
                fileList.push_back(ConvertToUtf8String(pentry.cFileName));
            }
        }
        while (FindNextFile(hdl, &pentry) != 0);

        FindClose(hdl);
    }

#else
    fs::path dirEntry;

    auto *pd = opendir(p_path.u8string().c_str());
    if (pd != nullptr)
    {
        struct dirent *pentry;

        while ((pentry = readdir(pd)) != nullptr)
        {
            dirEntry = fs::path(p_path) / pentry->d_name;
            const auto status = fs::status(dirEntry);

            if (fs::exists(status) && fs::is_regular_file(status))
            {
                fileList.emplace_back(pentry->d_name);
            }
        }

        closedir(pd);
    }

#endif
    return fileList;
}
/********************************************
 member functions
********************************************/

PathList_t BDirectory::GetSubDirectories() const
{
    return GetSubDirectories(path);
}

PathList_t BDirectory::GetFiles() const
{
    return GetFiles(path);
}
