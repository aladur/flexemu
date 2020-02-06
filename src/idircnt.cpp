/*
    idircnt.cpp

    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2019  W. Schwotzer

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

#ifndef __idircnt_cpp__
#define __idircnt_cpp__

#include "misc1.h"
#include <algorithm>
#ifdef _MSC_VER
    #include <io.h>         // needed for access
    #include <direct.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "filecont.h"
#include "dircont.h"
#include "idircnt.h"
#include "cvtwchar.h"


DirectoryContainerIteratorImp::DirectoryContainerIteratorImp(
    DirectoryContainer *aBase)
    : base(aBase), dirHdl(nullptr)
{
}

DirectoryContainerIteratorImp::~DirectoryContainerIteratorImp()
{
    if (dirHdl != nullptr)
    {
#ifdef WIN32
        FindClose(dirHdl);
#endif
#ifdef UNIX
        closedir(dirHdl);
#endif
        dirHdl = nullptr;
    }

    base = nullptr;
}

bool DirectoryContainerIteratorImp::operator==(const FileContainerIf *src) const
{
    return (base == nullptr && src == nullptr) ||
           (((FileContainerIf *)base == src) && (dirHdl == nullptr));
}

void DirectoryContainerIteratorImp::AtEnd()
{
    base = nullptr;
}

bool DirectoryContainerIteratorImp::NextDirEntry(const char *filePattern)
{
    std::string str, fileName;
    bool isValid;
#ifdef WIN32
    WIN32_FIND_DATA findData;
    SYSTEMTIME systemTime;
#endif
#ifdef UNIX
    struct dirent *findData = nullptr;
    struct stat sbuf;
#endif
    dirEntry.SetEmpty();
    // repeat until a valid directory entry found
#ifdef WIN32
    str = base->GetPath();
    str += PATHSEPARATOR;
    str += "*.*";

    do
    {
        isValid = false;

        if (dirHdl == nullptr)
        {
#ifdef UNICODE
            dirHdl = FindFirstFile(ConvertToUtf16String(str).c_str(), &findData);
#else
            dirHdl = FindFirstFile(str.c_str(), &findData);
#endif
            if (dirHdl != INVALID_HANDLE_VALUE)
            {
                isValid = true;
            }
            else
            {
                dirHdl = nullptr;
            }
        }
        else
        {
            if (FindNextFile(dirHdl, &findData))
            {
                isValid = true;

#ifdef UNICODE
                fileName = ConvertToUtf8String(findData.cFileName);
#else
                fileName = findData.cFileName;
#endif
                if (fileName.size() > 12)
                {
#ifdef UNICODE
                    fileName = ConvertToUtf8String(findData.cAlternateFileName);
#else
                    fileName = findData.cAlternateFileName;
#endif
                }
            }
        }
    }
    while (isValid &&
           ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ||
            (findData.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE) ||
            (findData.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY) ||
            !stricmp(fileName.c_str(), RANDOM_FILE_LIST) ||
            !multimatches(fileName.c_str(), filePattern, ';', true)));

    if (isValid)
    {
        // ok, found a valid directory entry
        Byte attributes = 0;
        int sectorMap = 0;
        dirEntry.SetTotalFileName(fileName.c_str());

        if (findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
        {
            sectorMap = 2;
        }

        // CDFS support:
        if (base->IsRandomFile(base->GetPath().c_str(), fileName.c_str()))
        {
            sectorMap = 2;
        }

        if (findData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
        {
            attributes |= WRITE_PROTECT;
        }

        dirEntry.SetSize((findData.nFileSizeLow + 251U) / 252U * SECTOR_SIZE);
        FileTimeToSystemTime(&findData.ftLastWriteTime, &systemTime);
        dirEntry.SetDate(systemTime.wDay, systemTime.wMonth,
                         systemTime.wYear);
        dirEntry.SetAttributes(attributes);
        dirEntry.SetSectorMap(sectorMap);
        dirEntry.SetStartTrkSec(0, 0);
        dirEntry.SetEndTrkSec(0, 0);
        dirEntry.ClearEmpty();
    }

#endif
#ifdef UNIX
    // unfinished
    str = base->GetPath();

    do
    {
        isValid = false;

        if (dirHdl == nullptr)
        {
            if ((dirHdl = opendir(str.c_str())) != nullptr)
            {
                isValid = true;
            }
        }

        if (dirHdl != nullptr && (findData = readdir(dirHdl)) != nullptr)
        {
            isValid = true;
            fileName = findData->d_name;
        }
    }
    while (isValid &&
           (stat((str + PATHSEPARATORSTRING + findData->d_name).c_str(),
                 &sbuf) ||
            !base->IsFlexFilename(findData->d_name) ||
            !S_ISREG(sbuf.st_mode) ||
            sbuf.st_size < 0 ||
            !multimatches(fileName.c_str(), filePattern, ';', true)));

    if (isValid)
    {
        struct tm *lt;

        // ok, found a valid directory entry
        Byte attributes = 0;
        int sectorMap = 0;

        if (base->IsWriteProtected())
        {
            // CDFS-Support: look for file name in file 'random'
            if (base->IsRandomFile(
                    base->GetPath().c_str(), findData->d_name))
            {
                sectorMap = 2;
            }
        }
        else
        {
            if (sbuf.st_mode & S_IXUSR)
            {
                sectorMap = 2;
            }
        }

        if (!(sbuf.st_mode & S_IWUSR))
        {
            attributes |= WRITE_PROTECT;
        }

        dirEntry.SetTotalFileName(findData->d_name);
        dirEntry.SetSize((sbuf.st_size + 251) / 252 * SECTOR_SIZE);
        lt = localtime(&(sbuf.st_mtime));
        dirEntry.SetDate(lt->tm_mday, lt->tm_mon + 1, lt->tm_year + 1900);
        dirEntry.SetAttributes(attributes);
        dirEntry.SetSectorMap(sectorMap);
        dirEntry.SetStartTrkSec(0, 0);
        dirEntry.SetEndTrkSec(0, 0);
        dirEntry.ClearEmpty();
    }

#endif
    return !dirEntry.IsEmpty();
}

// deletes the file on which the iterator currently
// is pointing on
// Only valid if the iterator has a valid directory entry
bool DirectoryContainerIteratorImp::DeleteCurrent()
{
    std::string filePath;

    if (dirEntry.IsEmpty())
    {
        return false;
    }

    filePath = dirEntry.GetTotalFileName();
    std::transform(filePath.begin(), filePath.end(), filePath.begin(),
         ::tolower);
    filePath = base->GetPath() + PATHSEPARATOR + filePath;
#ifdef UNIX

    if (remove(filePath.c_str()))
    {
        if (errno == ENOENT)
            throw FlexException(FERR_NO_FILE_IN_CONTAINER,
                                dirEntry.GetTotalFileName().c_str(),
                                base->GetPath().c_str());
        else
            throw FlexException(FERR_REMOVE_FILE,
                                dirEntry.GetTotalFileName().c_str(),
                                base->GetPath().c_str());
    }

#endif
#ifdef WIN32
    // evtl. remove read-only attribute
    // to be able to delete it
#ifdef UNICODE
    DWORD attributes = GetFileAttributes(ConvertToUtf16String(filePath).c_str());
#else
    DWORD attributes = GetFileAttributes(filePath.c_str());
#endif

    if (attributes & FILE_ATTRIBUTE_READONLY)
    {
#ifdef UNICODE
        SetFileAttributes(ConvertToUtf16String(filePath).c_str(), attributes & ~FILE_ATTRIBUTE_READONLY);
#else
        SetFileAttributes(filePath.c_str(), attributes & ~FILE_ATTRIBUTE_READONLY);
#endif
    }

#ifdef UNICODE
    BOOL success = DeleteFile(ConvertToUtf16String(filePath).c_str());
#else
    BOOL success = DeleteFile(filePath.c_str());
#endif

    if (!success)
    {
        DWORD lastError = GetLastError();

        if (lastError == ERROR_FILE_NOT_FOUND)
        {
            throw FlexException(FERR_NO_FILE_IN_CONTAINER,
                dirEntry.GetTotalFileName(), base->GetPath());
        }
        else {
            throw FlexException(FERR_REMOVE_FILE,
                dirEntry.GetTotalFileName(), base->GetPath());
        }
    }

#endif
    return true;
}

// Renames the file on which the iterator currently
// is pointing on
// Only valid if the iterator has a valid directory entry
bool DirectoryContainerIteratorImp::RenameCurrent(const char *newName)
{
    if (dirEntry.IsEmpty())
    {
        return false;
    }

    std::string src(dirEntry.GetTotalFileName());
    std::string dst(newName);
    FlexDirEntry de;
#ifdef UNIX
    std::transform(src.begin(), src.end(), src.begin(), ::tolower);
    std::transform(dst.begin(), dst.end(), dst.begin(), ::tolower);
#endif

    // prevent overwriting of an existing file
    if (base->FindFile(dst.c_str(), de))
    {
        throw FlexException(FERR_FILE_ALREADY_EXISTS, newName);
    }

    if (src == dst)
    {
        return true;
    }

    src = base->GetPath() + PATHSEPARATORSTRING + src;
    dst = base->GetPath() + PATHSEPARATORSTRING + dst;

    if (rename(src.c_str(), dst.c_str()))
    {
        // Unfinished
        if (errno == EEXIST)
        {
            throw FlexException(FERR_FILE_ALREADY_EXISTS, newName);
        }
        else if (errno == EACCES)
            throw FlexException(FERR_RENAME_FILE,
                                dirEntry.GetTotalFileName().c_str(),
                                base->GetPath().c_str());
        else if (errno == ENOENT)
            throw FlexException(FERR_NO_FILE_IN_CONTAINER,
                                dirEntry.GetTotalFileName().c_str(),
                                base->GetPath().c_str());
    }

    return true;
}

// Set date for the file on which the iterator currently
// is pointing on
// Only valid if the iterator has a valid directory entry
bool DirectoryContainerIteratorImp::SetDateCurrent(const BDate &date)
{
    struct stat sbuf;
    struct utimbuf timebuf;
    struct tm file_time;
    std::string filePath;

    if (dirEntry.IsEmpty())
    {
        return false;
    }

    filePath = dirEntry.GetTotalFileName();
    std::transform(filePath.begin(), filePath.end(), filePath.begin(),
        ::tolower);
    filePath = base->GetPath() + PATHSEPARATORSTRING + filePath;

    if (stat(filePath.c_str(), &sbuf) >= 0)
    {
        timebuf.actime = sbuf.st_atime;
        file_time.tm_sec   = 0;
        file_time.tm_min   = 0;
        file_time.tm_hour  = 12;
        file_time.tm_mon   = date.GetMonth() - 1;
        file_time.tm_mday  = date.GetDay();
        file_time.tm_year  = date.GetYear() - 1900;
        file_time.tm_isdst = 0;
        timebuf.modtime    = mktime(&file_time);

        if (timebuf.modtime >= 0 && utime(filePath.c_str(), &timebuf) >= 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    } // if

    return false;
}

// set the date in the actual selected directory entry
// Only valid if the iterator has a valid directory entry
// Only the WRITE_PROTECT flag is supported
bool DirectoryContainerIteratorImp::SetAttributesCurrent(Byte attributes)
{
    std::string filePath;

    if (dirEntry.IsEmpty())
    {
        return false;
    }

#ifdef WIN32
    filePath = base->GetPath() + PATHSEPARATORSTRING +
               dirEntry.GetTotalFileName();
#ifdef UNICODE
    DWORD attrs = GetFileAttributes(ConvertToUtf16String(filePath).c_str());
#else
    DWORD attrs = GetFileAttributes(filePath.c_str());
#endif

    if (attributes & WRITE_PROTECT)
    {
        attrs |= FILE_ATTRIBUTE_READONLY;
    }
    else
    {
        attrs &= ~FILE_ATTRIBUTE_READONLY;
    }

#ifdef UNICODE
    SetFileAttributes(ConvertToUtf16String(filePath).c_str(), attrs);
#else
    SetFileAttributes(filePath.c_str(), attrs);
#endif
#endif
#ifdef UNIX
    struct stat sbuf;

    filePath = dirEntry.GetTotalFileName();
    std::transform(filePath.begin(), filePath.end(), filePath.begin(),
         ::tolower);
    filePath = base->GetPath() + PATHSEPARATORSTRING + filePath;

    if (!stat(filePath.c_str(), &sbuf))
    {
        if (attributes & WRITE_PROTECT)
        {
            chmod(filePath.c_str(), sbuf.st_mode | S_IWUSR);
        }
        else
        {
            chmod(filePath.c_str(), sbuf.st_mode & ~S_IWUSR);
        }
    }

#endif
    return true;
}

#endif // __idircnt_h__

