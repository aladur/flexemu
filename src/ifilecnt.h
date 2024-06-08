/*
    ifilecnt.h

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

#ifndef IFILECNT_INCLUDED
#define IFILECNT_INCLUDED

#include "fdirent.h"
#include "ifilcnti.h"
#include <string>


class IFlexDiskByFile;
class IFlexDiskIteratorImp;

class FileContainerIterator
{
public:
    explicit FileContainerIterator(const std::string &p_wildcard = "*.*");
    FileContainerIterator(const FileContainerIterator &src) = delete;
    virtual               ~FileContainerIterator() = default;
    FlexDirEntry          &operator*();
    FlexDirEntry          *operator->();
    bool operator==(const IFlexDiskByFile *aBase) const;
    bool operator!=(const IFlexDiskByFile *aBase) const;
    FileContainerIterator &operator=(IFlexDiskByFile *aBase);
    FileContainerIterator &operator++();
    FileContainerIterator &operator=(const FileContainerIterator &src) =
        delete;
    bool DeleteCurrent();
    bool RenameCurrent(const std::string &newName);
    bool SetDateCurrent(const BDate &date);
    bool SetAttributesCurrent(Byte attributes);
private:
    std::string wildcard;
    IFlexDiskIteratorImpPtr imp;
};

#endif // IFILECNT_INCLUDED
