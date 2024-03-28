/*
    dircont.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
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
#ifdef HAVE_SYS_MOUNT_H
    #include <sys/param.h>
    #include <sys/mount.h>
#endif

#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include "bdir.h"
#include "bdate.h"
#include "bfileptr.h"
#include "fcinfo.h"
#include "flexerr.h"
#include "dircont.h"
#include "fdirent.h"
#include "fcopyman.h"
#include "ffilebuf.h"
#include "ifilecnt.h"
#include "idircnt.h"
#include "cvtwchar.h"

/****************************************/
/* Constructor                          */
/****************************************/

DirectoryContainer::DirectoryContainer(const std::string &aPath,
                                       const FileTimeAccess &fileTimeAccess)
    : ft_access(fileTimeAccess)
{
    struct stat sbuf;
    static Word number = 0;

    if (stat(aPath.c_str(), &sbuf) != 0 || !S_ISDIR(sbuf.st_mode))
    {
        throw FlexException(FERR_UNABLE_TO_OPEN, aPath);
    }

    if (isAbsolutePath(aPath))
    {
        directory = aPath;
    }
    else
    {
        directory = getCurrentPath();
        if (!directory.empty() && !endsWithPathSeparator(aPath))
        {
            directory += PATHSEPARATORSTRING;
        }
        directory += aPath;
    }

    if (access(aPath.c_str(), W_OK))
    {
        attributes |= FLX_READONLY;
    }

    Initialize_header(attributes & FLX_READONLY);
    disk_number = number++;
}

/****************************************/
/* Destructor                           */
/****************************************/

DirectoryContainer::~DirectoryContainer()
{
}

/****************************************/
/* Public interface                     */
/****************************************/

bool DirectoryContainer::IsWriteProtected() const
{
    return (attributes & FLX_READONLY) ? true : false;
}

// type, track and sectors parameter will be ignored
DirectoryContainer *DirectoryContainer::Create(
        const std::string &directory,
        const std::string &name,
        int /* tracks */,
        int /* sectors */,
        const FileTimeAccess &fileTimeAccess,
        int /* fmt = TYPE_DISK_CONTAINER */)
{
    struct stat sbuf;
    std::string path;

    path = directory;

    if (!path.empty() && !endsWithPathSeparator(path))
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

    return new DirectoryContainer(path, fileTimeAccess);
}

std::string DirectoryContainer::GetPath() const
{
    return directory;
}

std::string DirectoryContainer::GetSupportedAttributes() const
{
    return "W";
}

/*
bool    DirectoryContainer::OpenDirectory(const char *pattern)
{
    CHECK_DDIRECTORY_ALREADY_OPENED;
    filePattern = pattern;
    dirHdl = nullptr;
    return true;
}

}
*/

// return true if file found
// if file found can also be checked by
// !entry.isEmpty
bool DirectoryContainer::FindFile(const char *fileName, FlexDirEntry &entry)
{
    FileContainerIterator it(fileName);

    it = this->begin();

    if (it == this->end())
    {
        return false;
    }

    entry = *it;
    return true;
}

bool    DirectoryContainer::DeleteFile(const char *fileName)
{
    FileContainerIterator it(fileName);

    for (it = this->begin(); it != this->end(); ++it)
    {
        it.DeleteCurrent();
    }

    return true;
}

bool    DirectoryContainer::RenameFile(const char *oldName, const char *newName)
{
    FlexDirEntry de;

    if (strcmp(oldName, newName) == 0)
    {
        return false;
    }

    // prevent conflict with an existing file
    if (FindFile(newName, de))
    {
        throw FlexException(FERR_FILE_ALREADY_EXISTS, std::string(newName));
    }

    FileContainerIterator it(oldName);

    it = this->begin();

    if (it == this->end())
    {
        throw FlexException(FERR_NO_FILE_IN_CONTAINER, std::string(oldName),
                            GetPath());
    }

    it.RenameCurrent(newName);

    return true;
}

bool    DirectoryContainer::FileCopy(
    const char *sourceName, const char *destName,
    FileContainerIf &destination)
{
    FlexCopyManager copyMan;

    return copyMan.FileCopy(sourceName, destName,
                            (FileContainerIf &) * this, destination);
}

bool    DirectoryContainer::GetInfo(FlexContainerInfo &info) const
{
    std::string rootPath;
    struct stat sbuf;

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

    info.SetFree(static_cast<uint64_t>(numberOfFreeClusters) *
                 sectorsPerCluster * bytesPerSector);
    info.SetTotalSize(static_cast<uint64_t>(totalNumberOfClusters) *
                      sectorsPerCluster * bytesPerSector);
#endif
#ifdef UNIX
    struct statvfs fsbuf;

    if (statvfs(directory.c_str(), &fsbuf) != 0)
    {
        throw FlexException(FERR_READING_DISKSPACE, directory);
    }

    info.SetFree(fsbuf.f_bsize * fsbuf.f_bavail);
    info.SetTotalSize(fsbuf.f_bsize * fsbuf.f_blocks);
#endif

    if (stat(directory.c_str(), &sbuf) == 0)
    {
        struct tm *timeStruct = localtime(&sbuf.st_mtime);
        info.SetDate(BDate(timeStruct->tm_mday, timeStruct->tm_mon + 1,
                     timeStruct->tm_year + 1900));
    }
    else
    {
        info.SetDate(BDate());
    }

    info.SetTrackSector(0, 0);

    const auto *p = strrchr(directory.c_str(), PATHSEPARATOR);
    if (p != nullptr)
    {
        info.SetName(p + 1);
    }
    else
    {
        info.SetName(directory);
    }
    info.SetNumber(disk_number);

    info.SetPath(directory);
    //info.SetType(param.type);
    info.SetType(TYPE_DIRECTORY);
    info.SetAttributes(attributes);
    return true;
}

int DirectoryContainer::GetContainerType() const
{
    return TYPE_DIRECTORY;
}

FlexFileBuffer DirectoryContainer::ReadToBuffer(const char *fileName)
{
    FlexFileBuffer buffer;
    std::string filePath(fileName);

    strlower(filePath);
    filePath = directory + PATHSEPARATORSTRING + filePath;

    if (!buffer.ReadFromFile(filePath.c_str()))
    {
        throw FlexException(FERR_READING_FROM, std::string(fileName));
    }

    return buffer;
}

bool DirectoryContainer::WriteFromBuffer(const FlexFileBuffer &buffer,
        const char *fileName /* = nullptr */)
{
    std::string lowerFileName, filePath;
    struct stat sbuf;

    if (fileName == nullptr)
    {
        lowerFileName = buffer.GetFilename();
    }
    else {
        lowerFileName = fileName;
    }

#ifdef UNIX
    strlower(lowerFileName);
#endif
    filePath = directory + PATHSEPARATORSTRING + lowerFileName;

    // prevent to overwrite an existing file
    if (stat(filePath.c_str(), &sbuf) == 0 && S_ISREG(sbuf.st_mode))
    {
        throw FlexException(FERR_FILE_ALREADY_EXISTS, lowerFileName);
    }

    if (!buffer.WriteToFile(filePath.c_str()))
    {
        throw FlexException(FERR_WRITING_TO, filePath);
    }

    SetDateTime(lowerFileName.c_str(), buffer.GetDate(), buffer.GetTime());
    SetAttributes(lowerFileName.c_str(), buffer.GetAttributes());

    if (buffer.IsRandom())
    {
        SetRandom(lowerFileName.c_str());
    }

    return true;
}

/******************************/
/* private interface          */
/******************************/

void DirectoryContainer::Initialize_header(Byte)
{
    /*
        param.offset        = 0;
        param.write_protect = wp;
        param.max_sector    = 0;
        param.max_sector0   = 0;
        param.max_track     = 0;
        param.byte_p_sector = SECTOR_SIZE;
        param.byte_p_track0 = 0;
        param.byte_p_track  = 0;
        param.type          = TYPE_DIRECTORY;
    */
}

// set the date and time of a file
bool DirectoryContainer::SetDateTime(const char *fileName, const BDate &date,
                                     const BTime &time)
{
    struct stat    sbuf;
    struct utimbuf timebuf;
    struct tm      file_time;
    std::string filePath;
    std::string lowerFileName(fileName);

    strlower(lowerFileName);
    filePath = directory + PATHSEPARATORSTRING + lowerFileName;
    const bool setFileTime =
        (ft_access & FileTimeAccess::Set) == FileTimeAccess::Set;

    if (stat(filePath.c_str(), &sbuf) == 0)
    {
        timebuf.actime = sbuf.st_atime;
        file_time.tm_sec   = 0;
        file_time.tm_min   = setFileTime ? time.GetMinute() : 0;
        file_time.tm_hour  = setFileTime ? time.GetHour() : 12;
        file_time.tm_mon   = date.GetMonth() - 1;
        file_time.tm_mday  = date.GetDay();
        file_time.tm_year  = date.GetYear() - 1900;
        file_time.tm_isdst = 0;
        timebuf.modtime    = mktime(&file_time);

        return (timebuf.modtime >= 0 && utime(filePath.c_str(), &timebuf) >= 0);
    }

    return false;
}

// set the file attributes of a file
bool DirectoryContainer::SetAttributes(const char *fileName, Byte setMask,
                                       Byte clearMask /* = ~0 */)
{
    // only WRITE_PROTECT flag is supported
    if ((setMask & WRITE_PROTECT) || (clearMask & WRITE_PROTECT))
    {
#ifdef _WIN32
        const auto wFilePath(
            ConvertToUtf16String(directory + PATHSEPARATORSTRING + fileName));
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
        struct stat sbuf;
        std::string lowerFileName(fileName);

        strlower(lowerFileName);
        auto filePath(directory + PATHSEPARATORSTRING + lowerFileName);

        if (stat(filePath.c_str(), &sbuf) == 0)
        {
            if (clearMask & WRITE_PROTECT)
            {
                chmod(filePath.c_str(), sbuf.st_mode | S_IWUSR);
            }

            if (setMask & WRITE_PROTECT)
            {
                chmod(filePath.c_str(), sbuf.st_mode & ~S_IWUSR);
            }
        }

#endif
    }

    return true;
}

// on WIN32 a random file will be represented by a hidden flag
// on UNIX a random file will be represented by a user execute flag
bool    DirectoryContainer::SetRandom(const char *fileName)
{
#ifdef _WIN32
    const auto wFilePath(
        ConvertToUtf16String(directory + PATHSEPARATORSTRING + fileName));
    DWORD attrs = GetFileAttributes(wFilePath.c_str());
    SetFileAttributes(wFilePath.c_str(), attrs | FILE_ATTRIBUTE_HIDDEN);
#endif
#ifdef UNIX
    struct stat sbuf;
    std::string lowerFileName(fileName);

    strlower(lowerFileName);
    auto filePath(directory + PATHSEPARATORSTRING + lowerFileName);

    if (stat(filePath.c_str(), &sbuf) == 0)
    {
        chmod(filePath.c_str(), sbuf.st_mode | S_IXUSR);
    }

#endif
    return true;
}

// check if pfilename contains a valid FLEX filename
// on Unix only lowercase filenames are allowed
bool DirectoryContainer::IsFlexFilename(const std::string &filename)
{
    int     result; // result from sscanf should be int
    char    dot;
    char    name[9];
    char    ext[4];

    dot    = '\0';
    result = sscanf(filename.c_str(), "%1[a-z]%7[a-z0-9_-]", name, &name[1]);

    if (!result || result == EOF)
    {
        return false;
    }

    if (result == 1)
    {
        result = sscanf(filename.c_str(), "%*1[a-z]%c%1[a-z]%2[a-z0-9_-]",
                        &dot, ext, &ext[1]);
    }
    else
    {
        result = sscanf(filename.c_str(),
                        "%*1[a-z]%*7[a-z0-9_-]%c%1[a-z]%2[a-z0-9_-]",
                        &dot, ext, &ext[1]);
    }

    if (!result || result == 1 || result == EOF)
    {
        return false;
    }

    if (strlen(name) + strlen(ext) + (dot == '.' ? 1 : 0) != filename.size())
    {
        return false;
    }

    return true;
} // IsFlexFilename

/*****************************************************************
Iterator implemenation
*****************************************************************/

FileContainerIteratorImpPtr DirectoryContainer::IteratorFactory()
{
    return FileContainerIteratorImpPtr(new DirectoryContainerIteratorImp(this));
}

