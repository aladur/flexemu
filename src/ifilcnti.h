/*
    ifilcnti.h

    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2026  W. Schwotzer

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

#ifndef IFILCNTI_INCLUDED
#define IFILCNTI_INCLUDED

#include "typedefs.h"
#include <memory>
#include <string>


class BDate;
class FlexDiskIterator;
class IFlexDiskByFile;
class FlexDirEntry;

// This interface defines a FLEX disk iterator implementation.
// Rename: FlexDiskIteratorImp => IFlexDiskIteratorImp
// Polymorphic interface, virtual dtor is required.
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class IFlexDiskIteratorImp
{
    friend class FlexDiskIterator;
public:
    virtual ~IFlexDiskIteratorImp() = default;
    virtual bool operator==(const IFlexDiskByFile *base) const = 0;
    virtual bool NextDirEntry(const std::string &wildcard) = 0;
    virtual void AtEnd() = 0;
    virtual FlexDirEntry &GetDirEntry() = 0;
private:
    virtual bool DeleteCurrent() = 0;
    virtual bool RenameCurrent(const std::string &newName) = 0;
    virtual bool SetDateCurrent(const BDate &date) = 0;
    virtual bool SetAttributesCurrent(Byte attributes) = 0;
};

using IFlexDiskIteratorImpPtr = std::unique_ptr<IFlexDiskIteratorImp>;

#endif // IFILCNTI_INCLUDED

