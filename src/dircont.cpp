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
#include <filesystem>


namespace fs = std::filesystem;

/****************************************/
/* Constructor                          */
/****************************************/

FlexDirectoryDiskByFile::FlexDirectoryDiskByFile(
        const fs::path &path, const FileTimeAccess &fileTimeAccess)
    : randomFileCheck(path.u8string())
    , ft_access(fileTimeAccess)
{
    static Word number = 0;
    auto new_path(path.u8string());
    if (new_path.size() > 1 && flx::endsWithPathSeparator(new_path))
    {
        new_path.resize(new_path.size() - 1);
    }

    directory = fs::absolute(fs::u8path(new_path));

    const auto status = fs::status(directory);
    if (!fs::exists(status) || !fs::is_directory(status))
    {
        throw FlexException(FERR_UNABLE_TO_OPEN, directory);
    }

    if ((access(directory.u8string().c_str(), W_OK) != 0) ||
         randomFileCheck.IsWriteProtected())
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
        const fs::path &directory,
        const std::string &name,
        int /* tracks */,
        int /* sectors */,
        const FileTimeAccess &fileTimeAccess,
        DiskType disk_type)
{
    if (disk_type != DiskType::Directory)
    {
        using T = std::underlying_type_t<DiskType>;
        auto id = static_cast<T>(disk_type);

        throw FlexException(FERR_INVALID_FORMAT, id);
    }

    const auto path = directory / name;
    const auto status = fs::status(path);

    if (fs::exists(status) && fs::is_regular_file(status))
    {
        // if a file exists with this name delete it
        fs::remove(path);
    }

    std::error_code error;

    fs::create_directory(path, error);
    if (error)
    {
        throw FlexException(FERR_UNABLE_TO_CREATE, path);
    }

    return new FlexDirectoryDiskByFile(path, fileTimeAccess);
}

fs::path FlexDirectoryDiskByFile::GetPath() const
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
#ifdef _WIN32
    DWORD sectorsPerCluster;
    DWORD bytesPerSector;
    DWORD numberOfFreeClusters;
    DWORD totalNumberOfClusters;
    const auto rootPath =
        (directory.root_name() / directory.root_directory()).u8string();

    BOOL success = GetDiskFreeSpace(ConvertToUtf16String(rootPath).c_str(),
        &sectorsPerCluster, &bytesPerSector, &numberOfFreeClusters,
        &totalNumberOfClusters);

    if (!success)
    {
        throw FlexException(FERR_READING_DISKSPACE, directory);
    }

    diskAttributes.SetFree(static_cast<uint64_t>(numberOfFreeClusters) *
                           sectorsPerCluster * bytesPerSector);
    diskAttributes.SetTotalSize(static_cast<uint64_t>(totalNumberOfClusters) *
                                sectorsPerCluster * bytesPerSector);
#endif
#ifdef UNIX
    struct statvfs fsbuf{};

    if (statvfs(directory.u8string().c_str(), &fsbuf) != 0)
    {
        throw FlexException(FERR_READING_DISKSPACE, directory);
    }

    diskAttributes.SetFree(fsbuf.f_bsize * fsbuf.f_bavail);
    diskAttributes.SetTotalSize(fsbuf.f_bsize * fsbuf.f_blocks);
#endif

    struct stat sbuf{};

    if (stat(directory.u8string().c_str(), &sbuf) == 0)
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

    auto diskname = getDiskName(directory.filename().u8string());

    diskAttributes.SetName(diskname);
    diskAttributes.SetNumber(disk_number);
    diskAttributes.SetPath(directory.u8string());
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

    if (fileName.find_first_of("*?[]") != std::string::npos)
    {
        throw FlexException(FERR_WILDCARD_NOT_SUPPORTED, fileName);
    }

    const auto filePath = directory /
#ifdef _WIN32
        fileName;
#else
        flx::tolower(fileName);
#endif
    if (!buffer.ReadFromFile(filePath.u8string(), ft_access, false))
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
    std::string sFileName;

    if (fileName == nullptr)
    {
        sFileName = buffer.GetFilename();
    }
    else {
        sFileName = fileName;
    }

    const auto filePath = directory /
#ifdef _WIN32
        sFileName;
#else
        flx::tolower(sFileName);
#endif
    const auto status = fs::status(filePath);

    // prevent to overwrite an existing file
    if (fs::exists(status) && fs::is_regular_file(status))
    {
        throw FlexException(FERR_FILE_ALREADY_EXISTS, sFileName);
    }

    if (IsWriteProtected())
    {
        throw FlexException(FERR_CONTAINER_IS_READONLY, GetPath());
    }

    if (!buffer.WriteToFile(filePath.u8string(), ft_access))
    {
        throw FlexException(FERR_WRITING_TO, filePath);
    }

    SetDateTime(sFileName, buffer.GetDate(), buffer.GetTime());
    SetAttributes(sFileName, buffer.GetAttributes());

    if (buffer.IsRandom())
    {
        randomFileCheck.AddToRandomList(sFileName);
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

    const auto filePath = directory /
#ifdef _WIN32
        fileName;
#else
        flx::tolower(fileName);
#endif
    const bool setFileTime =
        (ft_access & FileTimeAccess::Set) == FileTimeAccess::Set;

    if (stat(filePath.u8string().c_str(), &sbuf) == 0)
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

        return (timebuf.modtime >= 0 &&
                utime(filePath.u8string().c_str(), &timebuf) == 0);
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

