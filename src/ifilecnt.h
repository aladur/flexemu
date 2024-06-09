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

// class FlexDiskIterator allows to iterate over directory entries
// of a FLEX disk. Depending on the Flex disk type it uses a different
// implementation (see private member imp).
//
// Rename: FileContainerIterator => FlexDiskIterator
class FlexDiskIterator
{
public:
    explicit FlexDiskIterator(const std::string &p_wildcard = "*.*");
    FlexDiskIterator(const FlexDiskIterator &src) = delete;
    virtual               ~FlexDiskIterator() = default;
    FlexDirEntry          &operator*();
    FlexDirEntry          *operator->();
    bool operator==(const IFlexDiskByFile *aBase) const;
    bool operator!=(const IFlexDiskByFile *aBase) const;
    FlexDiskIterator &operator=(IFlexDiskByFile *aBase);
    FlexDiskIterator &operator++();
    FlexDiskIterator &operator=(const FlexDiskIterator &src) =
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
