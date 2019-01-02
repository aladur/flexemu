/*
    iffilcnt.h

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

#ifndef IDIRCNT_INCLUDED
#define IDIRCNT_INCLUDED

#include "ifilcnti.h"
#include "filecont.h"

#ifdef _WIN32
    typedef HANDLE DIRHANDLE;
#endif
#ifdef UNIX
    typedef DIR *DIRHANDLE;
#endif

class DirectoryContainer;

class DirectoryContainerIteratorImp : public FileContainerIteratorImp
{
public:
    DirectoryContainerIteratorImp(DirectoryContainer *);
    virtual ~DirectoryContainerIteratorImp();
    bool operator==(const FileContainerIf *aBase) const;
    void AtEnd();
    FlexDirEntry &GetDirEntry()
    {
        return dirEntry;
    };
    bool NextDirEntry(const char *filePattern);
private:
    bool DeleteCurrent();
    bool RenameCurrent(const char *);
    bool SetDateCurrent(const BDate &date);
    bool SetAttributesCurrent(Byte attributes);

    DirectoryContainerIteratorImp();
    DirectoryContainerIteratorImp(const DirectoryContainerIteratorImp &);

    DirectoryContainer *base;
    FlexDirEntry dirEntry;
    DIRHANDLE dirHdl;
};

#endif // IDIRCNT_INCLUDED

