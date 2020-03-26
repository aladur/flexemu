/*
    ifilcnti.h

    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2020  W. Schwotzer

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

#include "fdirent.h"
#include <memory>


class FileContainerIterator;
class FileContainerIf;
class FlexDirEntry;


class FileContainerIteratorImp
{
    friend class FileContainerIterator;
public:
    virtual ~FileContainerIteratorImp() { };
    virtual bool operator==(const FileContainerIf *aBase) const = 0;
    virtual bool NextDirEntry(const char *filePattern) = 0;
    virtual void AtEnd() = 0;
    virtual FlexDirEntry &GetDirEntry() = 0;
private:
    virtual bool DeleteCurrent() = 0;
    virtual bool RenameCurrent(const char *) = 0;
    virtual bool SetDateCurrent(const BDate &date) = 0;
    virtual bool SetAttributesCurrent(Byte attributes) = 0;
};

using FileContainerIteratorImpPtr = std::unique_ptr<FileContainerIteratorImp>;

#endif // IFILCNTI_INCLUDED

