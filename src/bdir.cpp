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

#include "misc1.h"
#include <sys/stat.h>
#include <sys/types.h>
#ifdef HAVE_UNISTD_H
    #include <unistd.h>
#endif
#ifdef HAVE_DIRENT_H
    #include <dirent.h>
#else
    #define dirent direct
#endif

#ifdef _WIN32
    #ifdef _MSC_VER
        #include <direct.h>
    #endif
#endif

#include "bdir.h"
#include "cvtwchar.h"


/********************************************
 static functions
********************************************/

bool BDirectory::Exists(const std::string &p_path)
{
    struct stat sbuf{};

    return !stat(p_path.c_str(), &sbuf) && (S_ISDIR(sbuf.st_mode));
}

bool BDirectory::Remove(const std::string &p_path)
{
#if defined(_MSC_VER) || defined(__MINGW32)
    return _rmdir(p_path.c_str()) >= 0;
#endif
#if defined(UNIX) || defined(__CYGWIN32)
    return rmdir(p_path.c_str()) >= 0;
#endif
}

#if defined(_MSC_VER) || defined(__MINGW32)
bool BDirectory::Create(const std::string &p_path, int /* [[maybe_unused]] int mode = 0755 */)
{
    return _mkdir(p_path.c_str()) >= 0;
}
#endif
#if defined(UNIX) || defined(__CYGWIN32)
bool BDirectory::Create(const std::string &p_path, int mode /* = 0755 */)
{
    return mkdir(p_path.c_str(), mode) >= 0;
}
#endif

#ifdef _WIN32
bool BDirectory::RemoveRecursive(const std::string &p_path)
{
    std::string basePath(p_path);
    WIN32_FIND_DATA pentry;

    if (basePath[basePath.length()-1] != PATHSEPARATOR)
    {
        basePath += PATHSEPARATOR;
    }

    const auto wFilePattern (ConvertToUtf16String(basePath + "*.*"));
    auto hdl = FindFirstFile(wFilePattern.c_str(), &pentry);
    if (hdl != INVALID_HANDLE_VALUE)
    {
        do
        {
            const auto dirEntry(
                basePath + ConvertToUtf8String(pentry.cFileName));

            if (pentry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (pentry.cFileName[0] != '.')
                {
                    RemoveRecursive(dirEntry);
                }
            }
            else
            {
                remove(dirEntry.c_str());
            }
        }
        while (FindNextFile(hdl, &pentry) != 0);

        FindClose(hdl);
    }

    BDirectory::Remove(basePath);
    return true;
}
#else
bool BDirectory::RemoveRecursive(const std::string &p_path)
{
    std::string basePath;
    std::string dirEntry;
    struct stat sbuf{};

    basePath = p_path;

    if (basePath[basePath.length()-1] == PATHSEPARATOR)
    {
        std::string temp = basePath.substr(0, basePath.length() - 1);

        basePath = temp;
    }

    auto *pd = opendir(basePath.c_str());
    if (pd != nullptr)
    {
        struct dirent *pentry;

        while ((pentry = readdir(pd)) != nullptr)
        {
            dirEntry = basePath + PATHSEPARATORSTRING +
                       pentry->d_name;

            if (!stat(dirEntry.c_str(), &sbuf) && (S_ISREG(sbuf.st_mode)))
            {
                remove(dirEntry.c_str());
            }
            else if (S_ISDIR(sbuf.st_mode) && pentry->d_name[0] != '.')
            {
                RemoveRecursive(dirEntry);
            }
        }

        closedir(pd);
    }

    BDirectory::Remove(basePath);
    return true;
}
#endif

PathList_t BDirectory::GetSubDirectories(const std::string &p_path)
{
    std::vector<std::string> subDirList;
    std::string basePath(p_path);
#ifdef _WIN32
    WIN32_FIND_DATA pentry;

    if (basePath[basePath.length()-1] != PATHSEPARATOR)
    {
        basePath += PATHSEPARATOR;
    }

    const auto wFilePattern(ConvertToUtf16String(basePath + "*.*"));
    auto hdl = FindFirstFile(wFilePattern.c_str(), &pentry);
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
    std::string dirEntry;
    struct stat sbuf{};

    if (basePath[basePath.length()-1] == PATHSEPARATOR)
    {
        std::string temp = basePath.substr(0, basePath.length() - 1);

        basePath = temp;
    }

    auto *pd = opendir(basePath.c_str());
    if (pd != nullptr)
    {
        struct dirent *pentry;

        while ((pentry = readdir(pd)) != nullptr)
        {
            dirEntry = basePath + PATHSEPARATORSTRING + pentry->d_name;

            if (stat(dirEntry.c_str(), &sbuf) == 0 &&
                S_ISDIR(sbuf.st_mode) &&
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

PathList_t BDirectory::GetFiles(const std::string &p_path)
{
    std::vector<std::string> fileList;
    std::string basePath(p_path);
#ifdef _WIN32
    WIN32_FIND_DATA pentry;

    if (basePath[basePath.length()-1] != PATHSEPARATOR)
    {
        basePath += PATHSEPARATOR;
    }

    const auto wFilePattern(ConvertToUtf16String(basePath + "*.*"));
    auto hdl = FindFirstFile(wFilePattern.c_str(), &pentry);
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
    std::string dirEntry;
    struct stat sbuf{};

    if (basePath[basePath.length()-1] == PATHSEPARATOR)
    {
        std::string temp = basePath.substr(0, basePath.length() - 1);

        basePath = temp;
    }

    auto *pd = opendir(basePath.c_str());
    if (pd != nullptr)
    {
        struct dirent *pentry;

        while ((pentry = readdir(pd)) != nullptr)
        {
            dirEntry = basePath + PATHSEPARATORSTRING + pentry->d_name;

            if (stat(dirEntry.c_str(), &sbuf) == 0 && S_ISREG(sbuf.st_mode))
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

bool BDirectory::Exists() const
{
    return Exists(path);
}

bool BDirectory::Remove() const
{
    return Remove(path);
}

bool BDirectory::RemoveRecursive() const
{
    return RemoveRecursive(path);
}

bool BDirectory::Create(int mode /* = 0755 */) const
{
    return Create(path, mode);
}

PathList_t BDirectory::GetSubDirectories() const
{
    return GetSubDirectories(path);
}

PathList_t BDirectory::GetFiles() const
{
    return GetFiles(path);
}
