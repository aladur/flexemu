/*
    iffilcnt.h

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

#ifndef IFFILCNT_INCLUDED
#define IFFILCNT_INCLUDED

#include "ifilcnti.h"
#include "filecont.h"
#include "filecnts.h"

class FlexFileContainer;

class FlexFileContainerIteratorImp : public FileContainerIteratorImp
{
public:
    FlexFileContainerIteratorImp(FlexFileContainer *);
    virtual ~FlexFileContainerIteratorImp();
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

    FlexFileContainerIteratorImp();
    FlexFileContainerIteratorImp(const FlexFileContainerIteratorImp &);

    FlexFileContainer *base;
    int dirIndex;
    st_t dirTrackSector;
    struct s_dir_sector dirSector;
    FlexDirEntry dirEntry;
};

#endif // IFFILCNT_INCLUDED

