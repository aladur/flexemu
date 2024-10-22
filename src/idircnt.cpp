/*
    idircnt.cpp

    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2024  W. Schwotzer

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
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <cerrno>
#include "fattrib.h"
#include "filecont.h"
#include "dircont.h"
#include "idircnt.h"
#include "filecnts.h"
#include "cvtwchar.h"
#include <ctime>
#include <fstream>


FlexDirectoryDiskIteratorImp::FlexDirectoryDiskIteratorImp(
    FlexDirectoryDiskByFile *p_base)
    : base(p_base), dirHdl(nullptr), searchOneFileAtEnd(false)
{
}

FlexDirectoryDiskIteratorImp::~FlexDirectoryDiskIteratorImp()
{
    if (dirHdl != nullptr)
    {
#ifdef _WIN32
        FindClose(dirHdl);
#endif
#ifdef UNIX
        closedir(dirHdl);
#endif
        dirHdl = nullptr;
    }

    if (base != nullptr)
    {
        base->randomFileCheck.UpdateRandomListToFile();
    }

    base = nullptr;
}

bool FlexDirectoryDiskIteratorImp::operator==(const IFlexDiskByFile *rhs) const
{
    return (base == nullptr && rhs == nullptr) ||
           ((base == rhs) && (dirHdl == nullptr));
}

void FlexDirectoryDiskIteratorImp::AtEnd()
{
    if (base != nullptr)
    {
        base->randomFileCheck.UpdateRandomListToFile();
    }
    searchOneFileAtEnd = true;

    base = nullptr;
}

bool FlexDirectoryDiskIteratorImp::NextDirEntry(const std::string &wildcard)
{
    std::string fileName;
    bool isValid = true;
    bool searchOneFile = (wildcard.find_first_of("*?[];") == std::string::npos);

    dirEntry.SetEmpty();
    if (searchOneFile && searchOneFileAtEnd)
    {
        return false;
    }

    // repeat until a valid directory entry found
#ifdef _WIN32
    const auto path = base->GetPath() + PATHSEPARATORSTRING;
    const auto pattern = path + "*.*";
    WIN32_FIND_DATA findData{};

    while (isValid &&
            (!flx::isFlexFilename(fileName) ||
            (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ||
            (findData.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE) ||
            (findData.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY) ||
            findData.nFileSizeLow > (MAX_FILE_SECTORS * DBPS) ||
            !flx::multimatches(fileName, wildcard, ';', true)))
    {
        isValid = false;

        if (searchOneFile)
        {
            fileName = wildcard;
            const auto filePattern = path + fileName;
            dirHdl = FindFirstFile(ConvertToUtf16String(filePattern).c_str(),
                &findData);
            if (dirHdl != INVALID_HANDLE_VALUE && !searchOneFileAtEnd)
            {
                isValid = true;
            }
            searchOneFileAtEnd = true;
            continue;
        }

        if (dirHdl == nullptr)
        {
            dirHdl = FindFirstFile(ConvertToUtf16String(pattern).c_str(),
                                   &findData);
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

                fileName = ConvertToUtf8String(findData.cFileName);
                if (fileName.size() > 12)
                {
                    fileName = ConvertToUtf8String(findData.cAlternateFileName);
                }
            }
        }
    }

    if (isValid)
    {
        SYSTEMTIME systemTime{};
        SYSTEMTIME localTime{};

        // ok, found a valid directory entry
        Byte attributes = 0;
        int sectorMapFlag = 0;

        if (base->randomFileCheck.IsRandomFile(fileName))
        {
            sectorMapFlag = IS_RANDOM_FILE;
        }

        if (findData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
        {
            attributes |= WRITE_PROTECT;
        }

        flx::strupper(fileName);
        dirEntry.SetTotalFileName(fileName);
        auto fileSize = (findData.nFileSizeLow + 251U) / 252U * SECTOR_SIZE;
        dirEntry.SetFileSize(fileSize);
        FileTimeToSystemTime(&findData.ftLastWriteTime, &systemTime);
        SystemTimeToTzSpecificLocalTime(nullptr, &systemTime, &localTime);
        dirEntry.SetDate({localTime.wDay, localTime.wMonth, localTime.wYear});
        dirEntry.SetTime({localTime.wHour, localTime.wMinute, 0U});
        dirEntry.SetAttributes(attributes);
        dirEntry.SetSectorMap(sectorMapFlag);
        dirEntry.SetStartTrkSec(0, 0);
        dirEntry.SetEndTrkSec(0, 0);
        dirEntry.ClearEmpty();
    }

#endif
#ifdef UNIX
    auto path = base->GetPath() + PATHSEPARATORSTRING;
    struct stat sbuf {};
    struct dirent* findData = nullptr;

    while (isValid &&
            (!flx::isFlexFilename(fileName) ||
            stat((path + fileName).c_str(), &sbuf) != 0 ||
            !S_ISREG(sbuf.st_mode) ||
            sbuf.st_size < 0 || sbuf.st_size > (MAX_FILE_SECTORS * DBPS) ||
            !flx::multimatches(fileName, wildcard, ';', true)))
    {
        isValid = false;

        if (searchOneFile)
        {
            fileName = flx::tolower(wildcard);
            if (stat((path + fileName).c_str(), &sbuf) == 0 &&
                    !searchOneFileAtEnd)
            {
                isValid = true;
            }
            searchOneFileAtEnd = true;
            continue;
        }

        if (dirHdl == nullptr)
        {
            dirHdl = opendir(path.c_str());
            if (dirHdl != nullptr)
            {
                isValid = true;
            }
        }

        if (dirHdl != nullptr)
        {
            findData = readdir(dirHdl);
            if (findData != nullptr)
            {
                isValid = true;
                fileName = findData->d_name;
            }
        }
    }

    if (isValid)
    {
        // ok, found a valid directory entry
        Byte attributes = 0;
        int sectorMapFlag = 0;

        if (base->randomFileCheck.IsRandomFile(fileName))
        {
            sectorMapFlag = IS_RANDOM_FILE;
        }

        if (!(sbuf.st_mode & S_IWUSR))
        {
            attributes |= WRITE_PROTECT;
        }

        flx::strupper(fileName);
        dirEntry.SetTotalFileName(fileName);
        auto fileSize = (sbuf.st_size + 251) / 252 * SECTOR_SIZE;
        dirEntry.SetFileSize(static_cast<int>(fileSize));
        const struct tm *lt = localtime(&sbuf.st_mtime);
        dirEntry.SetDate({lt->tm_mday, lt->tm_mon + 1, lt->tm_year + 1900});
        if ((base->ft_access & FileTimeAccess::Get) == FileTimeAccess::Get)
        {
            dirEntry.SetTime({lt->tm_hour, lt->tm_min});
        }
        dirEntry.SetAttributes(attributes);
        dirEntry.SetSectorMap(sectorMapFlag);
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
bool FlexDirectoryDiskIteratorImp::DeleteCurrent()
{
    std::string filePath;

    if (dirEntry.IsEmpty())
    {
        return false;
    }

    filePath = flx::tolower(dirEntry.GetTotalFileName());
    filePath = base->GetPath() + PATHSEPARATOR + filePath;
#ifdef UNIX

    if (remove(filePath.c_str()))
    {
        if (errno == ENOENT)
        {
            throw FlexException(FERR_NO_FILE_IN_CONTAINER,
                                dirEntry.GetTotalFileName(),
                                base->GetPath());
        }

        throw FlexException(FERR_REMOVE_FILE,
                            dirEntry.GetTotalFileName(),
                            base->GetPath());
    }

#endif
#ifdef _WIN32
    // evtl. remove read-only attribute
    // to be able to delete it
    const auto wFilePath(ConvertToUtf16String(filePath));
    DWORD attributes = GetFileAttributes(wFilePath.c_str());

    if (attributes & FILE_ATTRIBUTE_READONLY)
    {
        SetFileAttributes(wFilePath.c_str(), attributes & ~FILE_ATTRIBUTE_READONLY);
    }

    BOOL success = DeleteFile(wFilePath.c_str());

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
bool FlexDirectoryDiskIteratorImp::RenameCurrent(const std::string &newName)
{
    if (dirEntry.IsEmpty())
    {
        return false;
    }

    std::string src(dirEntry.GetTotalFileName());
    // When renaming always prefer lowercase filenames
    auto dst(flx::tolower(newName));
    FlexDirEntry tempDirEntry;
#ifdef UNIX
    flx::strlower(src);
#endif

    // prevent overwriting of an existing file
    if (base->FindFile(dst, tempDirEntry))
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

        if (errno == EACCES)
        {
            throw FlexException(FERR_RENAME_FILE,
                                dirEntry.GetTotalFileName(),
                                base->GetPath());
        }

        if (errno == ENOENT)
        {
            throw FlexException(FERR_NO_FILE_IN_CONTAINER,
                                dirEntry.GetTotalFileName(),
                                base->GetPath());
        }
    }

    return true;
}

// Set date for the file on which the iterator currently
// is pointing on
// Only valid if the iterator has a valid directory entry
bool FlexDirectoryDiskIteratorImp::SetDateCurrent(const BDate &date)
{
    struct stat sbuf{};
    std::string filePath;

    if (dirEntry.IsEmpty())
    {
        return false;
    }

    filePath = flx::tolower(dirEntry.GetTotalFileName());
    filePath = base->GetPath() + PATHSEPARATORSTRING + filePath;

    if (stat(filePath.c_str(), &sbuf) == 0)
    {
        struct utimbuf timebuf{};
        struct tm file_time{};

        timebuf.actime = sbuf.st_atime;
        file_time.tm_sec = 0;
        file_time.tm_min = 0;
        file_time.tm_hour = 0;
        file_time.tm_mon = date.GetMonth() - 1;
        file_time.tm_mday = date.GetDay();
        file_time.tm_year = date.GetYear() - 1900;
        file_time.tm_isdst = -1;
        timebuf.modtime = mktime(&file_time);

        return (timebuf.modtime >= 0 && utime(filePath.c_str(), &timebuf) == 0);
    }

    return false;
}

// set the date in the actual selected directory entry
// Only valid if the iterator has a valid directory entry
// Only the WRITE_PROTECT flag is supported
bool FlexDirectoryDiskIteratorImp::SetAttributesCurrent(Byte attributes)
{
    std::string filePath;

    if (dirEntry.IsEmpty())
    {
        return false;
    }

#ifdef _WIN32
    const auto wFilePath(
        ConvertToUtf16String(base->GetPath() + PATHSEPARATORSTRING +
                             dirEntry.GetTotalFileName()));
    DWORD attrs = GetFileAttributes(wFilePath.c_str());

    if (attributes & WRITE_PROTECT)
    {
        attrs |= FILE_ATTRIBUTE_READONLY;
    }
    else
    {
        attrs &= ~FILE_ATTRIBUTE_READONLY;
    }

    SetFileAttributes(wFilePath.c_str(), attrs);
#endif
#ifdef UNIX
    struct stat sbuf{};

    filePath = flx::tolower(dirEntry.GetTotalFileName());
    filePath = base->GetPath() + PATHSEPARATORSTRING + filePath;

    if (!stat(filePath.c_str(), &sbuf))
    {
        if (attributes & WRITE_PROTECT)
        {
            chmod(filePath.c_str(), sbuf.st_mode &
                    static_cast<unsigned>(~S_IWUSR));
        }
        else
        {
            chmod(filePath.c_str(), sbuf.st_mode | S_IWUSR);
        }
    }

#endif
    return true;
}

