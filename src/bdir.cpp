/*
    bdir.cpp


    Basic class used for directory functions
    Copyright (C) 1999-2018  W. Schwotzer

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
#include <stdio.h>
#include <sys/stat.h>
#ifdef HAVE_UNISTD_H
    #include <sys/types.h>
    #include <unistd.h>
#endif
#ifdef HAVE_DIRENT_H
    #include <sys/types.h>
    #include <dirent.h>
    #define NAMLEN(dirent) strlen((dirent)->d_name)
#else
    #define dirent direct
    #define NAMLEN(dirent) (dirent)->d_namlen
#endif

#ifdef _WIN32
    #ifdef _MSC_VER
        #include <direct.h>
    #endif
#endif

#include "bdir.h"
#include "cvtwchar.h"


/********************************************
 ctor / dtor
********************************************/

BDirectory::BDirectory()
{
}

BDirectory::~BDirectory()
{
}

/********************************************
 static functions
********************************************/

bool BDirectory::Exists(const std::string &aPath)
{
    struct stat sbuf;

    return !stat(aPath.c_str(), &sbuf) && (S_ISDIR(sbuf.st_mode));
}

bool BDirectory::Remove(const std::string &aPath)
{
#if defined(_MSC_VER) || defined(__MINGW32)
    return _rmdir(aPath.c_str()) >= 0;
#endif
#if defined(UNIX) || defined(__CYGWIN32)
    return rmdir(aPath.c_str()) >= 0;
#endif
}

#if defined(_MSC_VER) || defined(__MINGW32)
bool BDirectory::Create(const std::string &aPath, int /* [[maybe_unused]] int mode = 0x0755 */)
{
    return _mkdir(aPath.c_str()) >= 0;
}
#endif
#if defined(UNIX) || defined(__CYGWIN32)
bool BDirectory::Create(const std::string &aPath, int mode /* = 0x0755 */)
{
    return mkdir(aPath.c_str(), mode) >= 0;
}
#endif

#ifdef _WIN32
bool BDirectory::RemoveRecursive(const std::string &aPath)
{
    std::string     basePath;
    std::string     dirEntry;
    HANDLE          hdl;
    WIN32_FIND_DATA pentry;

    basePath = aPath;

    if (basePath[basePath.length()-1] != PATHSEPARATOR)
    {
        basePath += PATHSEPARATOR;
    }

    std::string filePattern = basePath + "*.*";
#ifdef UNICODE
    hdl = FindFirstFile(ConvertToUtf16String(filePattern).c_str(), &pentry);
#else
    hdl = FindFirstFile(filePattern.c_str(), &pentry);
#endif
    if (hdl != INVALID_HANDLE_VALUE)
    {
        do
        {
#ifdef UNICODE
            dirEntry = basePath + ConvertToUtf8String(pentry.cFileName);
#else
            dirEntry = basePath + pentry.cFileName;
#endif

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
bool BDirectory::RemoveRecursive(const std::string &aPath)
{
    std::string basePath;
    std::string dirEntry;
    DIR         *pd;
    struct stat sbuf;

    basePath = aPath;

    if (basePath[basePath.length()-1] == PATHSEPARATOR)
    {
        std::string temp = basePath.substr(0, basePath.length() - 1);

        basePath = temp;
    }

    if ((pd = opendir(basePath.c_str())) != nullptr)
    {
        struct dirent   *pentry;

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
        } // while

        closedir(pd);
    }

    BDirectory::Remove(basePath);
    return true;
}
#endif

tPathList BDirectory::GetSubDirectories(const std::string &aPath)
{
    std::vector<std::string> subDirList;
    std::string     basePath(aPath);
#ifdef _WIN32
    HANDLE          hdl;
    WIN32_FIND_DATA pentry;

    if (basePath[basePath.length()-1] != PATHSEPARATOR)
    {
        basePath += PATHSEPARATOR;
    }

    std::string filePattern = basePath + "*.*";
#ifdef UNICODE
    hdl = FindFirstFile(ConvertToUtf16String(filePattern).c_str(), &pentry);
#else
    hdl = FindFirstFile(filePattern.c_str(), &pentry);
#endif
    if (hdl != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (pentry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY &&
                pentry.cFileName[0] != '.')
            {
#ifdef UNICODE
                subDirList.push_back(ConvertToUtf8String(pentry.cFileName));
#else
                subDirList.push_back(pentry.cFileName);
#endif
            }
        }
        while (FindNextFile(hdl, &pentry) != 0);

        FindClose(hdl);
    }

#else
    std::string     dirEntry;
    DIR             *pd;
    struct stat     sbuf;

    if (basePath[basePath.length()-1] == PATHSEPARATOR)
    {
        std::string temp = basePath.substr(0, basePath.length() - 1);

        basePath = temp;
    }

    if ((pd = opendir(basePath.c_str())) != nullptr)
    {
        struct dirent   *pentry;

        while ((pentry = readdir(pd)) != nullptr)
        {
            dirEntry = basePath + PATHSEPARATORSTRING + pentry->d_name;

            if (stat(dirEntry.c_str(), &sbuf) == 0  &&
                S_ISDIR(sbuf.st_mode)   &&
                pentry->d_name[0] != '.')
            {
                subDirList.push_back(pentry->d_name);
            }
        } // while

        closedir(pd);
    }

#endif
    return subDirList;
}

tPathList BDirectory::GetFiles(const std::string &aPath)
{
    std::vector<std::string> fileList;
    std::string     basePath(aPath);
#ifdef _WIN32
    HANDLE          hdl;
    WIN32_FIND_DATA pentry;

    if (basePath[basePath.length()-1] != PATHSEPARATOR)
    {
        basePath += PATHSEPARATOR;
    }

    std::string filePattern = basePath + "*.*";
#ifdef UNICODE
    hdl = FindFirstFile(ConvertToUtf16String(filePattern).c_str(), &pentry);
#else
    hdl = FindFirstFile(filePattern.c_str(), &pentry);
#endif
    if (hdl != INVALID_HANDLE_VALUE)
    {
        do
        {
            if ((pentry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 &&
                (pentry.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE) == 0 &&
                (pentry.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) == 0)
            {
#ifdef UNICODE
                fileList.push_back(ConvertToUtf8String(pentry.cFileName));
#else
                fileList.push_back(pentry.cFileName);
#endif
            }
        }
        while (FindNextFile(hdl, &pentry) != 0);

        FindClose(hdl);
    }

#else
    std::string     dirEntry;
    DIR             *pd;
    struct stat     sbuf;

    if (basePath[basePath.length()-1] == PATHSEPARATOR)
    {
        std::string temp = basePath.substr(0, basePath.length() - 1);

        basePath = temp;
    }

    if ((pd = opendir(basePath.c_str())) != nullptr)
    {
        struct dirent *pentry;

        while ((pentry = readdir(pd)) != nullptr)
        {
            dirEntry = basePath + PATHSEPARATORSTRING + pentry->d_name;

            if (stat(dirEntry.c_str(), &sbuf) == 0 && S_ISREG(sbuf.st_mode))
            {
                fileList.push_back(pentry->d_name);
            }
        } // while

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
    return Exists(m_path);
}

bool BDirectory::Remove() const
{
    return Remove(m_path);
}

bool BDirectory::RemoveRecursive() const
{
    return RemoveRecursive(m_path);
}

bool BDirectory::Create(int mode /* = 0x0755 */) const
{
    return Create(m_path, mode);
}

tPathList BDirectory::GetSubDirectories() const
{
    return GetSubDirectories(m_path);
}

tPathList BDirectory::GetFiles() const
{
    return GetFiles(m_path);
}
