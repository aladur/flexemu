/*
    dircont.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2021  W. Schwotzer

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

DirectoryContainer::DirectoryContainer(const char *aPath) :
    attributes(0),
    disk_number(0)
{
    struct stat sbuf;
    static Word number = 0;

    if (aPath == nullptr || stat(aPath, &sbuf) || !S_ISDIR(sbuf.st_mode))
    {
        throw FlexException(FERR_UNABLE_TO_OPEN, std::string(aPath));
    }

    if (isAbsolutePath(aPath))
    {
        directory = aPath;
    }
    else
    {
        directory = getCurrentPath();
        if (!endsWithPathSeparator(aPath))
        {
            directory += PATHSEPARATORSTRING;
        }
        directory += aPath;
    }

    if (access(aPath, W_OK))
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
DirectoryContainer *DirectoryContainer::Create(const char *dir,
        const char *name, int, int, int)
{
    struct stat sbuf;
    std::string aPath;

    aPath = dir;

    if (aPath[aPath.length()-1] != PATHSEPARATOR)
    {
        aPath += PATHSEPARATORSTRING;
    }

    aPath += name;

    if (!stat(aPath.c_str(), &sbuf) && S_ISREG(sbuf.st_mode))
    {
        // if a file exists with this name delete it
        remove(aPath.c_str());
    }

    if (stat(aPath.c_str(), &sbuf) || !S_ISDIR(sbuf.st_mode))
    {
        // directory does not exist
        if (!BDirectory::Create(aPath, 0755))
        {
            throw FlexException(FERR_UNABLE_TO_CREATE, aPath);
        }
    }

    return new DirectoryContainer(aPath.c_str());
}

std::string DirectoryContainer::GetPath() const
{
    return directory;
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
    else
    {
        it.RenameCurrent(newName);
    }

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
#ifdef WIN32
    DWORD sectorsPerCluster = 0;
    DWORD bytesPerSector = 1;
    DWORD numberOfFreeClusters = 0;
    DWORD totalNumberOfClusters = 0;
#endif
    const char  *p;
    std::string rootPath;
    struct stat sbuf;

    if (directory.length() > 3)
    {
        rootPath = directory.substr(0, 3);
    }

#ifdef WIN32

#ifdef UNICODE
    BOOL success = GetDiskFreeSpace(ConvertToUtf16String(rootPath).c_str(),
        &sectorsPerCluster, &bytesPerSector, &numberOfFreeClusters,
        &totalNumberOfClusters);
#else
    BOOL success = GetDiskFreeSpace(rootPath.c_str(), &sectorsPerCluster,
        &bytesPerSector, &numberOfFreeClusters,
        &totalNumberOfClusters);
#endif
    if (!success)
    {
        throw FlexException(FERR_READING_DISKSPACE, directory.c_str());
    }

    // free size in KByte
    info.SetFree(numberOfFreeClusters * sectorsPerCluster * bytesPerSector);
    // total size in KByte
    info.SetTotalSize(totalNumberOfClusters * sectorsPerCluster *
                      bytesPerSector);
#endif
#ifdef UNIX
#ifdef __SOLARIS
    statvfs_t fsbuf;
#else
    struct statvfs fsbuf;
#endif

    if (statvfs(directory.c_str(), &fsbuf))
    {
        throw FlexException(FERR_READING_DISKSPACE, directory.c_str());
    }

    info.SetFree(fsbuf.f_bsize * fsbuf.f_bfree);
    info.SetTotalSize(fsbuf.f_bsize * fsbuf.f_blocks);
#endif

    if (stat(directory.c_str(), &sbuf) >= 0)
    {
        struct tm *timeStruct = localtime(&sbuf.st_mtime);
        info.SetDate(timeStruct->tm_mday, timeStruct->tm_mon + 1,
                     timeStruct->tm_year + 1900);
    }
    else
    {
        info.SetDate(0, 0, 0);
    }

    info.SetTrackSector(0, 0);

    if ((p = strrchr(directory.c_str(), PATHSEPARATOR)) != nullptr)
    {
        info.SetName(p + 1);
    }
    else
    {
        info.SetName(directory.c_str());
    }
    info.SetNumber(disk_number);

    info.SetPath(directory.c_str());
    //info.SetType(param.type);
    info.SetType(TYPE_DIRECTORY);
    info.SetAttributes(attributes);
    return true;
}

int DirectoryContainer::GetContainerType() const
{
    return TYPE_DIRECTORY;
}

bool DirectoryContainer::CheckFilename(const char *fileName) const
{
    int     result; // result from sscanf should be int
    char    dot;
    char    name[9];
    char    ext[4];
    const char    *format;

    dot    = '\0';
    format = "%1[A-Za-z]%7[A-Za-z0-9_-]";
    result = sscanf(fileName, format, name, &name[1]);

    if (!result || result == EOF)
    {
        return false;
    }

    if (result == 1)
    {
        format = "%*1[A-Za-z]%c%1[A-Za-z]%2[A-Za-z0-9_-]";
    }
    else
    {
        format = "%*1[A-Za-z]%*7[A-Za-z0-9_-]%c%1[A-Za-z]%2[A-Za-z0-9_-]";
    }

    result = sscanf(fileName, format, &dot, ext, &ext[1]);

    if (!result || result == 1 || result == EOF)
    {
        return false;
    }

    if (strlen(name) + strlen(ext) + (dot == '.' ? 1 : 0) != strlen(fileName))
    {
        return false;
    }

    return true;
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
    if (!stat(filePath.c_str(), &sbuf) && S_ISREG(sbuf.st_mode))
    {
        throw FlexException(FERR_FILE_ALREADY_EXISTS, lowerFileName);
    }

    if (!buffer.WriteToFile(filePath.c_str()))
    {
        throw FlexException(FERR_WRITING_TO, std::string(fileName));
    }

    SetDate(lowerFileName.c_str(), buffer.GetDate());
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

// set the date of a file

bool    DirectoryContainer::SetDate(const char *fileName, const BDate &date)
{
    struct stat    sbuf;
    struct utimbuf timebuf;
    struct tm      file_time;
    std::string filePath;
    std::string lowerFileName(fileName);

    strlower(lowerFileName);
    filePath = directory + PATHSEPARATORSTRING + lowerFileName;

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

// set the file attributes of a file
bool DirectoryContainer::SetAttributes(const char *fileName, Byte setMask,
                                       Byte clearMask /* = ~0 */)
{
    // only WRITE_PROTECT flag is supported
    if ((setMask & WRITE_PROTECT) || (clearMask & WRITE_PROTECT))
    {
        std::string filePath;
#ifdef WIN32
        DWORD attrs;
        filePath = directory + PATHSEPARATORSTRING + fileName;
#ifdef UNICODE
        attrs = GetFileAttributes(ConvertToUtf16String(filePath).c_str());
#else
        attrs = GetFileAttributes(filePath.c_str());
#endif

        if (clearMask & WRITE_PROTECT)
        {
            attrs &= ~FILE_ATTRIBUTE_READONLY;
        }

        if (setMask & WRITE_PROTECT)
        {
            attrs |= FILE_ATTRIBUTE_READONLY;
        }

#ifdef UNICODE
        SetFileAttributes(ConvertToUtf16String(filePath).c_str(), attrs);
#else
        SetFileAttributes(filePath.c_str(), attrs);
#endif
#endif
#ifdef UNIX
        struct stat sbuf;
        std::string lowerFileName(fileName);

        strlower(lowerFileName);
        filePath = directory + PATHSEPARATORSTRING + lowerFileName;

        if (!stat(filePath.c_str(), &sbuf))
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
    std::string filePath;

#ifdef WIN32
    DWORD attrs;
    filePath = directory + PATHSEPARATORSTRING + fileName;
#ifdef UNICODE
    attrs = GetFileAttributes(ConvertToUtf16String(filePath).c_str());
    SetFileAttributes(ConvertToUtf16String(filePath).c_str(),
        attrs | FILE_ATTRIBUTE_HIDDEN);
#else
    attrs = GetFileAttributes(filePath.c_str());
    SetFileAttributes(filePath.c_str(), attrs | FILE_ATTRIBUTE_HIDDEN);
#endif
#endif
#ifdef UNIX
    struct stat sbuf;
    std::string lowerFileName(fileName);

    strlower(lowerFileName);
    filePath = directory + PATHSEPARATORSTRING + lowerFileName;

    if (!stat(filePath.c_str(), &sbuf))
    {
        chmod(filePath.c_str(), sbuf.st_mode | S_IXUSR);
    }

#endif
    return true;
}

// check if pfilename contains a valid FLEX filename
// on Unix only lowercase filenames are allowed
bool DirectoryContainer::IsFlexFilename(const char *pfilename,
                                        char *pname /* = nullptr */,
                                        char *pext  /* = nullptr */) const
{
    int     result; // result from sscanf should be int
    char    dot;
    char    name[9];
    char    ext[4];

    dot    = '\0';
    result = sscanf(pfilename, "%1[a-z]%7[a-z0-9_-]", name, &name[1]);

    if (!result || result == EOF)
    {
        return false;
    }

    if (result == 1)
    {
        result = sscanf(pfilename, "%*1[a-z]%c%1[a-z]%2[a-z0-9_-]",
                        &dot, ext, &ext[1]);
    }
    else
    {
        result = sscanf(pfilename, "%*1[a-z]%*7[a-z0-9_-]%c%1[a-z]%2[a-z0-9_-]",
                        &dot, ext, &ext[1]);
    }

    if (!result || result == 1 || result == EOF)
    {
        return false;
    }

    if (strlen(name) + strlen(ext) + (dot == '.' ? 1 : 0) != strlen(pfilename))
    {
        return false;
    }

    strupper(name);
    strupper(ext);

    if (pname)
    {
        strcpy(pname, name);
    }

    if (pext)
    {
        strcpy(pext, ext);
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

