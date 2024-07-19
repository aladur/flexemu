/*
    dircont.cpp


    FLEXplorer, An explorer for FLEX disk image files and directory disks.
    Copyright (C) 1998-2024  W. Schwotzer

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
#include <cctype>
#include <string>
#include <algorithm>
#include <locale>
#ifdef HAVE_SYS_STATVFS_H
    #include <sys/statvfs.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include "bdir.h"
#include "bdate.h"
#include "fattrib.h"
#include "fcinfo.h"
#include "flexerr.h"
#include "dircont.h"
#include "fdirent.h"
#include "fcopyman.h"
#include "ffilebuf.h"
#include "ifilecnt.h"
#include "idircnt.h"
#include "cvtwchar.h"
#include <cstring>

/****************************************/
/* Constructor                          */
/****************************************/

FlexDirectoryDiskByFile::FlexDirectoryDiskByFile(
        const std::string &aPath, const FileTimeAccess &fileTimeAccess)
    : ft_access(fileTimeAccess)
{
    struct stat sbuf{};
    static Word number = 0;

    if (stat(aPath.c_str(), &sbuf) != 0 || !S_ISDIR(sbuf.st_mode))
    {
        throw FlexException(FERR_UNABLE_TO_OPEN, aPath);
    }

    if (flx::isAbsolutePath(aPath))
    {
        directory = aPath;
    }
    else
    {
        directory = flx::getCurrentPath();
        if (!directory.empty() && !flx::endsWithPathSeparator(directory))
        {
            directory += PATHSEPARATORSTRING;
        }
        directory += aPath;
    }

    if (flx::endsWithPathSeparator(directory))
    {
        directory.resize(directory.size() - 1);
    }

    if (access(aPath.c_str(), W_OK))
    {
        attributes |= FLX_READONLY;
    }

    Initialize_header(attributes & FLX_READONLY);
    disk_number = number++;
}

/****************************************/
/* Public interface                     */
/****************************************/

bool FlexDirectoryDiskByFile::IsWriteProtected() const
{
    return (attributes & FLX_READONLY) != 0;
}

// type, track and sectors parameter will be ignored
FlexDirectoryDiskByFile *FlexDirectoryDiskByFile::Create(
        const std::string &directory,
        const std::string &name,
        int /* tracks */,
        int /* sectors */,
        const FileTimeAccess &fileTimeAccess,
        unsigned /* fmt = TYPE_DISK_CONTAINER */)
{
    struct stat sbuf{};
    std::string path;

    path = directory;

    if (!path.empty() && !flx::endsWithPathSeparator(path))
    {
        path += PATHSEPARATORSTRING;
    }

    path += name;

    if (stat(path.c_str(), &sbuf) == 0 && S_ISREG(sbuf.st_mode))
    {
        // if a file exists with this name delete it
        remove(path.c_str());
    }

    if (stat(path.c_str(), &sbuf) != 0 || !S_ISDIR(sbuf.st_mode))
    {
        // directory does not exist
        if (!BDirectory::Create(path, 0755))
        {
            throw FlexException(FERR_UNABLE_TO_CREATE, path);
        }
    }

    return new FlexDirectoryDiskByFile(path, fileTimeAccess);
}

std::string FlexDirectoryDiskByFile::GetPath() const
{
    return directory;
}

std::string FlexDirectoryDiskByFile::GetSupportedAttributes() const
{
    return "W";
}

// return true if file found
// if file found can also be checked by
// !entry.isEmpty
bool FlexDirectoryDiskByFile::FindFile(const std::string &fileName,
        FlexDirEntry &entry)
{
    FlexDiskIterator it(fileName);

    it = this->begin();

    if (it == this->end())
    {
        return false;
    }

    entry = *it;
    return true;
}

bool FlexDirectoryDiskByFile::DeleteFile(const std::string &wildcard)
{
    FlexDiskIterator it(wildcard);

    for (it = this->begin(); it != this->end(); ++it)
    {
        it.DeleteCurrent();
    }

    return true;
}

bool FlexDirectoryDiskByFile::RenameFile(const std::string &oldName,
        const std::string &newName)
{
    FlexDirEntry de;

    if (oldName.compare(newName) == 0)
    {
        return false;
    }

    // prevent conflict with an existing file
    if (FindFile(newName, de))
    {
        throw FlexException(FERR_FILE_ALREADY_EXISTS, newName);
    }

    FlexDiskIterator it(oldName);

    it = this->begin();

    if (it == this->end())
    {
        throw FlexException(FERR_NO_FILE_IN_CONTAINER, oldName, GetPath());
    }

    it.RenameCurrent(newName);

    return true;
}

bool FlexDirectoryDiskByFile::FileCopy(
    const std::string &sourceName, const std::string &destName,
    IFlexDiskByFile &destination)
{
    return FlexCopyManager::FileCopy(sourceName, destName,
                                     static_cast<IFlexDiskByFile &>(*this),
                                     destination);
}

bool FlexDirectoryDiskByFile::GetAttributes(
        FlexDiskAttributes &diskAttributes) const
{
    std::string rootPath;
    struct stat sbuf{};

    if (directory.length() > 3)
    {
        rootPath = directory.substr(0, 3);
    }

#ifdef _WIN32
    DWORD sectorsPerCluster;
    DWORD bytesPerSector;
    DWORD numberOfFreeClusters;
    DWORD totalNumberOfClusters;

    BOOL success = GetDiskFreeSpace(ConvertToUtf16String(rootPath).c_str(),
        &sectorsPerCluster, &bytesPerSector, &numberOfFreeClusters,
        &totalNumberOfClusters);

    if (!success)
    {
        throw FlexException(FERR_READING_DISKSPACE, directory.c_str());
    }

    diskAttributes.SetFree(static_cast<uint64_t>(numberOfFreeClusters) *
                           sectorsPerCluster * bytesPerSector);
    diskAttributes.SetTotalSize(static_cast<uint64_t>(totalNumberOfClusters) *
                                sectorsPerCluster * bytesPerSector);
#endif
#ifdef UNIX
    struct statvfs fsbuf{};

    if (statvfs(directory.c_str(), &fsbuf) != 0)
    {
        throw FlexException(FERR_READING_DISKSPACE, directory);
    }

    diskAttributes.SetFree(fsbuf.f_bsize * fsbuf.f_bavail);
    diskAttributes.SetTotalSize(fsbuf.f_bsize * fsbuf.f_blocks);
#endif

    if (stat(directory.c_str(), &sbuf) == 0)
    {
        struct tm *timeStruct = localtime(&sbuf.st_mtime);
        diskAttributes.SetDate(BDate(timeStruct->tm_mday,
                     timeStruct->tm_mon + 1,
                     timeStruct->tm_year + 1900));
    }
    else
    {
        diskAttributes.SetDate(BDate());
    }

    diskAttributes.SetTrackSector(0, 0);

    const auto *p = std::strrchr(directory.c_str(), PATHSEPARATOR);
    if (p != nullptr)
    {
        diskAttributes.SetName(p + 1);
    }
    else
    {
        diskAttributes.SetName(directory);
    }
    diskAttributes.SetNumber(disk_number);

    diskAttributes.SetPath(directory);
    //info.SetType(param.type);
    diskAttributes.SetType(TYPE_DIRECTORY);
    diskAttributes.SetAttributes(attributes);

    return true;
}

unsigned FlexDirectoryDiskByFile::GetFlexDiskType() const
{
    return TYPE_DIRECTORY;
}

FlexFileBuffer FlexDirectoryDiskByFile::ReadToBuffer(const std::string &fileName)
{
    FlexFileBuffer buffer;
    auto filePath(flx::tolower(fileName));

    filePath = directory + PATHSEPARATORSTRING + filePath;

    if (!buffer.ReadFromFile(filePath))
    {
        throw FlexException(FERR_READING_FROM, fileName);
    }

    return buffer;
}

bool FlexDirectoryDiskByFile::WriteFromBuffer(const FlexFileBuffer &buffer,
        const char *fileName /* = nullptr */)
{
    std::string lowerFileName;
    struct stat sbuf{};

    if (fileName == nullptr)
    {
        lowerFileName = buffer.GetFilename();
    }
    else {
        lowerFileName = fileName;
    }

#ifdef UNIX
    flx::strlower(lowerFileName);
#endif
    const auto filePath = directory + PATHSEPARATORSTRING + lowerFileName;

    // prevent to overwrite an existing file
    if (stat(filePath.c_str(), &sbuf) == 0 && S_ISREG(sbuf.st_mode))
    {
        throw FlexException(FERR_FILE_ALREADY_EXISTS, lowerFileName);
    }

    if (!buffer.WriteToFile(filePath))
    {
        throw FlexException(FERR_WRITING_TO, filePath);
    }

    SetDateTime(lowerFileName, buffer.GetDate(), buffer.GetTime());
    SetAttributes(lowerFileName, buffer.GetAttributes());

    if (buffer.IsRandom())
    {
        SetRandom(lowerFileName);
    }

    return true;
}

/******************************/
/* private interface          */
/******************************/

void FlexDirectoryDiskByFile::Initialize_header(bool /*isWriteProtect*/)
{
    /*
        param.offset = 0;
        param.write_protect = wp;
        param.max_sector = 0;
        param.max_sector0 = 0;
        param.max_track = 0;
        param.byte_p_sector = SECTOR_SIZE;
        param.byte_p_track0 = 0;
        param.byte_p_track = 0;
        param.type = TYPE_DIRECTORY;
    */
}

// set the date and time of a file
bool FlexDirectoryDiskByFile::SetDateTime(
        const std::string &fileName, const BDate &date, const BTime &time)
{
    struct stat sbuf{};
    struct utimbuf timebuf{};
    struct tm file_time{};
    std::string filePath;
    auto lowerFileName(flx::tolower(std::string(fileName)));

    filePath = directory + PATHSEPARATORSTRING + lowerFileName;
    const bool setFileTime =
        (ft_access & FileTimeAccess::Set) == FileTimeAccess::Set;

    if (stat(filePath.c_str(), &sbuf) == 0)
    {
        timebuf.actime = sbuf.st_atime;
        file_time.tm_sec = 0;
        file_time.tm_min = setFileTime ? time.GetMinute() : 0;
        file_time.tm_hour = setFileTime ? time.GetHour() : 12;
        file_time.tm_mon = date.GetMonth() - 1;
        file_time.tm_mday = date.GetDay();
        file_time.tm_year = date.GetYear() - 1900;
        file_time.tm_isdst = 0;
        timebuf.modtime = mktime(&file_time);

        return (timebuf.modtime >= 0 && utime(filePath.c_str(), &timebuf) >= 0);
    }

    return false;
}

// set the file attributes of a file
bool FlexDirectoryDiskByFile::SetAttributes(const std::string &wildcard,
                                       unsigned setMask,
                                       unsigned clearMask /* = ~0U */)
{
    // only WRITE_PROTECT flag is supported
    if ((setMask & WRITE_PROTECT) || (clearMask & WRITE_PROTECT))
    {
#ifdef _WIN32
        const auto wFilePath(
            ConvertToUtf16String(directory + PATHSEPARATORSTRING + wildcard));
        DWORD attrs = GetFileAttributes(wFilePath.c_str());

        if (clearMask & WRITE_PROTECT)
        {
            attrs &= ~FILE_ATTRIBUTE_READONLY;
        }

        if (setMask & WRITE_PROTECT)
        {
            attrs |= FILE_ATTRIBUTE_READONLY;
        }

        SetFileAttributes(wFilePath.c_str(), attrs);
#endif
#ifdef UNIX
        struct stat sbuf{};
        auto lowerFileName(flx::tolower(wildcard));

        auto filePath(directory + PATHSEPARATORSTRING + lowerFileName);

        if (stat(filePath.c_str(), &sbuf) == 0)
        {
            if (clearMask & WRITE_PROTECT)
            {
                chmod(filePath.c_str(), sbuf.st_mode | S_IWUSR);
            }

            if (setMask & WRITE_PROTECT)
            {
                chmod(filePath.c_str(),
                        sbuf.st_mode & static_cast<unsigned>(~S_IWUSR));
            }
        }

#endif
    }

    return true;
}

// on WIN32 a random file will be represented by a hidden flag
// on UNIX a random file will be represented by a user execute flag
bool FlexDirectoryDiskByFile::SetRandom(const std::string &fileName)
{
#ifdef _WIN32
    const auto wFilePath(
        ConvertToUtf16String(directory + PATHSEPARATORSTRING + fileName));
    DWORD attrs = GetFileAttributes(wFilePath.c_str());
    SetFileAttributes(wFilePath.c_str(), attrs | FILE_ATTRIBUTE_HIDDEN);
#endif
#ifdef UNIX
    struct stat sbuf{};
    auto lowerFileName(flx::tolower(fileName));

    auto filePath(directory + PATHSEPARATORSTRING + lowerFileName);

    if (stat(filePath.c_str(), &sbuf) == 0)
    {
        chmod(filePath.c_str(), sbuf.st_mode | S_IXUSR);
    }

#endif
    return true;
}

/*****************************************************************
Iterator implemenation
*****************************************************************/

IFlexDiskIteratorImpPtr FlexDirectoryDiskByFile::IteratorFactory()
{
    return IFlexDiskIteratorImpPtr(new FlexDirectoryDiskIteratorImp(this));
}

