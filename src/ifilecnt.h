/*
    ifilecnt.h

    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2023  W. Schwotzer

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


class FileContainerIf;
class FileContainerIteratorImp;

class FileContainerIterator
{
public:
    FileContainerIterator(const char *aFilePattern = "*.*");
    virtual               ~FileContainerIterator();
    FlexDirEntry          &operator*();
    FlexDirEntry          *operator->();
    bool                   operator==(const FileContainerIf *) const;
    bool                   operator!=(const FileContainerIf *) const;
    FileContainerIterator &operator=(FileContainerIf *);
    FileContainerIterator &operator++();
    bool                   DeleteCurrent();
    bool                   RenameCurrent(const char *);
    bool                   SetDateCurrent(const BDate &date);
    bool                   SetAttributesCurrent(Byte attributes);
private:
    std::string            filePattern;
    FileContainerIteratorImpPtr imp;
    FileContainerIterator(const FileContainerIterator &); // should not be used
    FileContainerIterator &operator=(const FileContainerIterator
                                     &); // should not be used
};

#endif // IFILECNT_INCLUDED
