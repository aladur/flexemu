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

#ifndef IDIRCNT_INCLUDED
#define IDIRCNT_INCLUDED

#include "ifilcnti.h"
#include "filecont.h"

#ifdef _WIN32
    using DIRHANDLE = HANDLE;
#endif
#ifdef UNIX
    using DIRHANDLE = DIR *;
#endif

class FlexDirectoryDiskByFile;

// Implementation of IFlexDiskIteratorImp interface for FlexDirectoryDiskByFile.
// Rename: FlexDirectoryDiskByFileIteratorImp => FlexDirectoryDiskIteratorImp
class FlexDirectoryDiskIteratorImp : public IFlexDiskIteratorImp
{
public:
    explicit FlexDirectoryDiskIteratorImp(FlexDirectoryDiskByFile *p_base);
    FlexDirectoryDiskIteratorImp() = delete;
    FlexDirectoryDiskIteratorImp(const FlexDirectoryDiskIteratorImp &src) =
        delete;
    ~FlexDirectoryDiskIteratorImp() override;
    bool operator==(const IFlexDiskByFile *rhs) const override;
    void AtEnd() override;
    FlexDirEntry &GetDirEntry() override
    {
        return dirEntry;
    };
    bool NextDirEntry(const std::string &wildcard) override;
private:
    bool DeleteCurrent() override;
    bool RenameCurrent(const std::string &newName) override;
    bool SetDateCurrent(const BDate &date) override;
    bool SetAttributesCurrent(Byte attributes) override;

    FlexDirectoryDiskByFile *base;
    FlexDirEntry dirEntry;
    DIRHANDLE dirHdl;
};

#endif // IDIRCNT_INCLUDED

