/*
    idircnt.cpp

    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2025  W. Schwotzer

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
#include "typedefs.h"
#include "misc1.h"
#include "idircnt.h"
#include "filecntb.h"
#include "efiletim.h"
#include "fattrib.h"
#include "filecont.h"
#include "dircont.h"
#include "filecnts.h"
#include "flexerr.h"
#ifdef _WIN32
#include "cvtwchar.h"
#include <sys/utime.h>
#else
#include <utime.h>
#endif
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
#include <sys/stat.h>
#include <ctime>
#include <cerrno>
#include <system_error>
#include <string>
#include <filesystem>
#ifdef _WIN32
#include "windefs.h"
#endif


namespace fs = std::filesystem;

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
    const auto pattern = base->GetPath() / "*.*";
    WIN32_FIND_DATA findData{};

    // do-while loop in this context improves code readability.
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-do-while)
    do
    {
        isValid = false;

        if (searchOneFile)
        {
            fileName = wildcard;
            const auto oneFilePattern = base->GetPath() / fileName;
            dirHdl = FindFirstFile(oneFilePattern.wstring().c_str(),
                &findData);
            if (dirHdl != INVALID_HANDLE_VALUE && !searchOneFileAtEnd)
            {
                isValid = true;
            }
            searchOneFileAtEnd = true;
        }
        else
        {
            if (dirHdl == nullptr)
            {
                dirHdl = FindFirstFile(pattern.wstring().c_str(), &findData);
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
                        fileName =
                            ConvertToUtf8String(findData.cAlternateFileName);
                    }
                }
            }
        }
    }
    while (isValid &&
            (!flx::isFlexFilename(fileName) ||
            (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ||
            (findData.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE) ||
            (findData.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY) ||
            findData.nFileSizeLow > (MAX_FILE_SECTORS * DBPS) ||
            !flx::multimatches(fileName, wildcard, ';', true)));

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
    const auto path = base->GetPath();
    struct stat sbuf {};
    struct dirent* findData = nullptr;
    // This loop is performance sensitive. Keep usage of struct stat.
    // do-while loop in this context improves code readability.
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-do-while)
    do
    {
        isValid = false;

        if (searchOneFile)
        {
            fileName = flx::tolower(wildcard);
#ifdef _WIN32
            if (_wstat((path / fileName).wstring().c_str(), &sbuf) == 0 &&
#else
            if (stat((path / fileName).u8string().c_str(), &sbuf) == 0 &&
#endif
                    !searchOneFileAtEnd)
            {
                isValid = true;
            }
            searchOneFileAtEnd = true;
        }
        else
        {
            if (dirHdl == nullptr)
            {
                dirHdl = opendir(path.u8string().c_str());
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
    }
    while (isValid &&
            (!flx::isFlexFilename(fileName) ||
#ifdef _WIN32
            _wstat((path / fileName).wstring().c_str(), &sbuf) != 0 ||
#else
            stat((path / fileName).u8string().c_str(), &sbuf) != 0 ||
#endif
            !S_ISREG(sbuf.st_mode) ||
            sbuf.st_size < 0 || sbuf.st_size > (MAX_FILE_SECTORS * DBPS) ||
            !flx::multimatches(fileName, wildcard, ';', true)));

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
        auto fileSize = (sbuf.st_size + 251U) / 252U * SECTOR_SIZE;
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
    if (dirEntry.IsEmpty())
    {
        return false;
    }

    const auto filePath = base->GetPath() /
                flx::tolower(dirEntry.GetTotalFileName());
    std::error_code error;

    fs::remove(filePath, error);
    if (error)
    {
        if (error == std::errc::no_such_file_or_directory)
        {
            throw FlexException(FERR_NO_FILE_IN_CONTAINER,
                                dirEntry.GetTotalFileName(),
                                base->GetPath());
        }

        throw FlexException(FERR_REMOVE_FILE,
                            dirEntry.GetTotalFileName(),
                            base->GetPath());
    }

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

    std::error_code error;
    auto src(dirEntry.GetTotalFileName());
    // When renaming always prefer lowercase filenames
    auto dst(flx::tolower(newName));
    FlexDirEntry tempDirEntry;
#ifdef UNIX
    flx::strlower(src);
#endif

    if (src == dst)
    {
        return true;
    }

    fs::rename(base->GetPath() / src, base->GetPath() / dst, error);
    if (error)
    {
        // Unfinished
        if (error == std::errc::file_exists)
        {
            throw FlexException(FERR_FILE_ALREADY_EXISTS, newName);
        }

        if (error == std::errc::no_such_file_or_directory)
        {
            throw FlexException(FERR_NO_FILE_IN_CONTAINER,
                                dirEntry.GetTotalFileName(),
                                base->GetPath());
        }

        throw FlexException(FERR_RENAME_FILE,
                            dirEntry.GetTotalFileName(),
                            base->GetPath());
    }

    return true;
}

// Set date for the file on which the iterator currently
// is pointing on
// Only valid if the iterator has a valid directory entry
bool FlexDirectoryDiskIteratorImp::SetDateCurrent(const BDate &date)
{
    struct stat sbuf{};

    if (dirEntry.IsEmpty())
    {
        return false;
    }

    const auto filePath = base->GetPath() /
        flx::tolower(dirEntry.GetTotalFileName());

#ifdef _WIN32
    if (_wstat(filePath.wstring().c_str(), &sbuf) == 0)
#else
    if (stat(filePath.u8string().c_str(), &sbuf) == 0)
#endif
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

        return (timebuf.modtime >= 0 &&
#ifdef _WIN32
                _wutime(filePath.wstring().c_str(), &timebuf) == 0);
#else
                utime(filePath.u8string().c_str(), &timebuf) == 0);
#endif
    }

    return false;
}

// set the date in the actual selected directory entry
// Only valid if the iterator has a valid directory entry
// Only the WRITE_PROTECT flag is supported
bool FlexDirectoryDiskIteratorImp::SetAttributesCurrent(Byte attributes)
{
#ifdef _WIN32
    const static auto write_perms = fs::perms::all;
#else
    const static auto write_perms = fs::perms::owner_write;
#endif

    if (dirEntry.IsEmpty())
    {
        return false;
    }

    auto fileName = dirEntry.GetTotalFileName();
#ifdef UNIX
    fileName = flx::tolower(fileName);
#endif
    const auto filePath = base->GetPath() / fileName;

    if (fs::exists(filePath))
    {
        if (attributes & WRITE_PROTECT)
        {
            fs::permissions(filePath, write_perms, fs::perm_options::remove);
        }
        else
        {
            fs::permissions(filePath, write_perms, fs::perm_options::add);
        }
    }

    return true;
}

