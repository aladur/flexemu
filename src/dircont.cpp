/*
    dircont.cpp


    FLEXplorer, An explorer for FLEX disk image files and directory disks.
    Copyright (C) 1998-2025  W. Schwotzer

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
#include <ctime>

/****************************************/
/* Constructor                          */
/****************************************/

FlexDirectoryDiskByFile::FlexDirectoryDiskByFile(
        const std::string &path, const FileTimeAccess &fileTimeAccess)
    : randomFileCheck(path)
    , ft_access(fileTimeAccess)
{
    struct stat sbuf{};
    static Word number = 0;

    if (stat(path.c_str(), &sbuf) != 0 || !S_ISDIR(sbuf.st_mode))
    {
        throw FlexException(FERR_UNABLE_TO_OPEN, path);
    }

    if (flx::isAbsolutePath(path))
    {
        directory = path;
    }
    else
    {
        directory = flx::getCurrentPath();
        if (!directory.empty() && !flx::endsWithPathSeparator(directory))
        {
            directory += PATHSEPARATORSTRING;
        }
        directory += path;
    }

    if (flx::endsWithPathSeparator(directory))
    {
        directory.resize(directory.size() - 1);
    }

    if ((access(path.c_str(), W_OK) != 0) || randomFileCheck.IsWriteProtected())
    {
        attributes |= WRITE_PROTECT;
    }

    Initialize_header();
    disk_number = number++;
    randomFileCheck.CheckAllFilesAttributeAndUpdate();
}

/****************************************/
/* Public interface                     */
/****************************************/

bool FlexDirectoryDiskByFile::IsWriteProtected() const
{
    return (attributes & WRITE_PROTECT) != 0;
}

// track and sectors parameter will be ignored
FlexDirectoryDiskByFile *FlexDirectoryDiskByFile::Create(
        const std::string &directory,
        const std::string &name,
        int /* tracks */,
        int /* sectors */,
        const FileTimeAccess &fileTimeAccess,
        DiskType disk_type)
{
    struct stat sbuf{};
    std::string path;

    if (disk_type != DiskType::Directory)
    {
        using T = std::underlying_type_t<DiskType>;
        auto id = static_cast<T>(disk_type);

        throw FlexException(FERR_INVALID_FORMAT, id);
    }

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
bool FlexDirectoryDiskByFile::FindFile(const std::string &wildcard,
        FlexDirEntry &dirEntry)
{
    FlexDiskIterator it(wildcard);

    dirEntry.SetEmpty();
    it = this->begin();

    if (it == this->end())
    {
        return false;
    }

    dirEntry = *it;
    return true;
}

bool FlexDirectoryDiskByFile::DeleteFile(const std::string &wildcard)
{
    FlexDiskIterator it(wildcard);

    if (IsWriteProtected())
    {
        throw FlexException(FERR_CONTAINER_IS_READONLY, GetPath());
    }

    for (it = this->begin(); it != this->end(); ++it)
    {
        it.DeleteCurrent();

        if ((*it).IsRandom())
        {
            randomFileCheck.RemoveFromRandomList((*it).GetTotalFileName());
        }
    }
    randomFileCheck.UpdateRandomListToFile();

    return true;
}

bool FlexDirectoryDiskByFile::RenameFile(const std::string &oldName,
        const std::string &newName)
{
    FlexDirEntry dirEntry;

    if (oldName.compare(newName) == 0)
    {
        return false;
    }

    if (oldName.find_first_of("*?[]") != std::string::npos)
    {
        throw FlexException(FERR_WILDCARD_NOT_SUPPORTED, oldName);
    }

    if (newName.find_first_of("*?[]") != std::string::npos)
    {
        throw FlexException(FERR_WILDCARD_NOT_SUPPORTED, newName);
    }

    // prevent conflict with an existing file
    if (FindFile(newName, dirEntry))
    {
        throw FlexException(FERR_FILE_ALREADY_EXISTS, newName);
    }

    if (IsWriteProtected())
    {
        throw FlexException(FERR_CONTAINER_IS_READONLY, GetPath());
    }

    FlexDiskIterator it(oldName);

    it = this->begin();

    if (it == this->end())
    {
        throw FlexException(FERR_NO_FILE_IN_CONTAINER, oldName, GetPath());
    }

    it.RenameCurrent(newName);

    if ((*it).IsRandom())
    {
        randomFileCheck.RemoveFromRandomList(oldName);
        randomFileCheck.AddToRandomList(newName);
        randomFileCheck.UpdateRandomListToFile();
    }

    return true;
}

bool FlexDirectoryDiskByFile::FileCopy(
    const std::string &sourceName, const std::string &destName,
    IFlexDiskByFile &destination)
{
    if (sourceName.find_first_of("*?[]") != std::string::npos)
    {
        throw FlexException(FERR_WILDCARD_NOT_SUPPORTED, sourceName);
    }

    if (destName.find_first_of("*?[]") != std::string::npos)
    {
        throw FlexException(FERR_WILDCARD_NOT_SUPPORTED, destName);
    }

    return FlexCopyManager::FileCopy(sourceName, destName,
                                     static_cast<IFlexDiskByFile &>(*this),
                                     destination);
}

bool FlexDirectoryDiskByFile::GetDiskAttributes(
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
        const struct tm *lt = localtime(&sbuf.st_mtime);
        diskAttributes.SetDate({lt->tm_mday,
                     lt->tm_mon + 1,
                     lt->tm_year + 1900});
    }
    else
    {
        diskAttributes.SetDate(BDate());
    }

    diskAttributes.SetTrackSector(0, 0);

    auto name = flx::toupper(flx::getFileName(directory));
    const auto pos = name.find_first_of('.');
    if (pos != std::string::npos)
    {
        name = name.substr(0, pos);
    }
    if (name.size() > FLEX_DISKNAME_LENGTH)
    {

        name = name.substr(0, FLEX_DISKNAME_LENGTH);
    }
    if (name.empty())
    {
        name = "FLEXDISK";
    }
    diskAttributes.SetName(name);
    diskAttributes.SetNumber(disk_number);
    diskAttributes.SetPath(directory);
    diskAttributes.SetType(GetFlexDiskType());
    diskAttributes.SetOptions(GetFlexDiskOptions());
    diskAttributes.SetAttributes(attributes);
    diskAttributes.SetIsWriteProtected(IsWriteProtected());
    diskAttributes.SetIsFlexFormat(true);

    return true;
}

DiskType FlexDirectoryDiskByFile::GetFlexDiskType() const
{
    return DiskType::Directory;
}

DiskOptions FlexDirectoryDiskByFile::GetFlexDiskOptions() const
{
    return DiskOptions::NONE;
}

FlexFileBuffer FlexDirectoryDiskByFile::ReadToBuffer(const std::string &fileName)
{
    FlexFileBuffer buffer;
    auto filePath(flx::tolower(fileName));

    if (fileName.find_first_of("*?[]") != std::string::npos)
    {
        throw FlexException(FERR_WILDCARD_NOT_SUPPORTED, fileName);
    }

    filePath = directory + PATHSEPARATORSTRING + filePath;
    if (!buffer.ReadFromFile(filePath, ft_access, false))
    {
        throw FlexException(FERR_READING_FROM, fileName);
    }

    if (randomFileCheck.IsRandomFile(fileName))
    {
        buffer.SetSectorMap(IS_RANDOM_FILE);
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

    if (IsWriteProtected())
    {
        throw FlexException(FERR_CONTAINER_IS_READONLY, GetPath());
    }

    if (!buffer.WriteToFile(filePath, ft_access))
    {
        throw FlexException(FERR_WRITING_TO, filePath);
    }

    SetDateTime(lowerFileName, buffer.GetDate(), buffer.GetTime());
    SetAttributes(lowerFileName, buffer.GetAttributes());

    if (buffer.IsRandom())
    {
        randomFileCheck.AddToRandomList(lowerFileName);
        randomFileCheck.UpdateRandomListToFile();
    }

    return true;
}

/******************************/
/* private interface          */
/******************************/

void FlexDirectoryDiskByFile::Initialize_header()
{
    /*
        param.offset = 0;
        param.write_protect = IsWriteProtected() ? 1U : 0U;
        param.max_sector = 0;
        param.max_sector0 = 0;
        param.max_track = 0;
        param.byte_p_sector = SECTOR_SIZE;
        param.byte_p_track0 = 0;
        param.byte_p_track = 0;
        param.type = DiskType::Directory;
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
        file_time.tm_hour = setFileTime ? time.GetHour() : 0;
        file_time.tm_mon = date.GetMonth() - 1;
        file_time.tm_mday = date.GetDay();
        file_time.tm_year = date.GetYear() - 1900;
        file_time.tm_isdst = -1;
        timebuf.modtime = mktime(&file_time);

        return (timebuf.modtime >= 0 && utime(filePath.c_str(), &timebuf) == 0);
    }

    return false;
}

// set the file attributes of a file
bool FlexDirectoryDiskByFile::SetAttributes(const std::string &wildcard,
                                       unsigned setMask,
                                       unsigned clearMask /* = ~0U */)
{
    if (IsWriteProtected())
    {
        throw FlexException(FERR_CONTAINER_IS_READONLY, GetPath());
    }

    FlexDiskIterator it(wildcard);

    for (it = this->begin(); it != this->end(); ++it)
    {
        Byte fileAttributes =
            static_cast<Byte>((it->GetAttributes() & ~clearMask) | setMask);
        it.SetAttributesCurrent(fileAttributes);
    }

    return true;
}

/*****************************************************************
Iterator implemenation
*****************************************************************/

IFlexDiskIteratorImpPtr FlexDirectoryDiskByFile::IteratorFactory()
{
    return IFlexDiskIteratorImpPtr(new FlexDirectoryDiskIteratorImp(this));
}

