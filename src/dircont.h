/*
    dircont.h


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2018  W. Schwotzer

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

#ifndef __dircont_h__
#define __dircont_h__

#include "misc1.h"
#include <stdio.h>

#include "filecont.h"
#include "fdirent.h"
#include "flexerr.h"
#include <string>


class FlexContainerInfo;
class FlexDirEntry;
class BDate;
class DirectoryContainerIteratorImp;


#define CHECK_NO_DCONTAINER_OPEN                 \
    if (!IsContainerOpened()) {              \
        throw FlexException(FERR_NO_CONTAINER_OPEN); \
    }

class DirectoryContainer : public FileContainerIf
{
    friend class DirectoryContainerIteratorImp;  // corresponding iterator class

private:
    std::string *path;
    int attributes;
    bool isOpened;

public:
    DirectoryContainer(const char *path);
    virtual ~DirectoryContainer();      // public destructor

    // basic interface
public:
    static DirectoryContainer *Create(const char *dir, const char *name,
                                      int t, int s,
                                      int fmt = TYPE_DSK_CONTAINER);
    int Close();
    bool IsContainerOpened() const;
    bool IsWriteProtected() const;
    bool GetInfo(FlexContainerInfo &info) const;
    int  GetContainerType() const;
    std::string GetPath() const;
    bool CheckFilename(const char *) const;

    // file oriented interface (to be used within flexdisk)
public:
    FileContainerIf *begin()
    {
        return this;
    };
    FileContainerIf *end() const
    {
        return NULL;
    };
    FileContainerIteratorImp *IteratorFactory();
    bool    FindFile(const char *fileName, FlexDirEntry &entry);
    bool    DeleteFile(const char *fileName);
    bool    RenameFile(const char *oldName, const char *newName);
    bool    SetAttributes(const char *fileName, int setMask,
                          int clearMask = ~0);
    void ReadToBuffer(const char *fileName, FlexFileBuffer &buffer);
    bool WriteFromBuffer(const FlexFileBuffer &buffer,
                         const char *fileName = NULL);
    bool    FileCopy(const char *sourceName, const char *destName,
                     FileContainerIf &destination);

    // private interface
private:
    DirectoryContainer();   // should not be used
    bool IsRandomFile(const char *ppath, const char *pfilename) const;
    bool IsFlexFilename(const char *pfilename,
                        char *pname = NULL,
                        char *pext = NULL) const;
    bool    SetDate(const char *fileName, const BDate &date);
    bool    SetRandom(const char *fileName);
    void Initialize_header(Byte wp);

};  // class DirectoryContainer

#endif // __dircont_h__
