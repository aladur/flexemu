/*
    ifilecnt.cpp

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

#include "misc1.h"  // needed for nullptr
#include "filecont.h"
#include "ifilecnt.h"
#include "ifilcnti.h"
#include "flexerr.h"

/* iterator class for file container
   Can be used to iterate over the files in a container.
   The iterator interface is similar to those of the STL container.
   Multiple iteraters can be used to read file properties or
   add files.
   Only one iterator at a time should be used when deleting files.

   Example:

   IFlexDiskByFile *m_container = new FlexFileContainer(...);
   FileContainerIterator it("*.*");
   std::string fileName;
   int i = 0;

   for (it = m_container->begin(); it != m_container->end(); ++it)
   {
      fileName = (*it).GetTotalFileName();
      printf("%d. file: %s\n", ++i, fileName);
   }
*/

FileContainerIterator::FileContainerIterator(
        const std::string &p_wildcard /* = "*.*" */)
    : wildcard(p_wildcard)
{
}

FlexDirEntry &FileContainerIterator::operator*()
{
    if (imp != nullptr)
    {
        return imp->GetDirEntry();
    }

    // maybe you used the iterator in the wrong way ?
    throw FlexException(FERR_INVALID_ITERATOR_USE);
}

FlexDirEntry *FileContainerIterator::operator->()
{
    if (imp != nullptr)
    {
        return &imp->GetDirEntry();
    }

    // maybe you used the iterator in the wrong way ?
    throw FlexException(FERR_INVALID_ITERATOR_USE);

}

bool FileContainerIterator::operator==(const IFlexDiskByFile *aBase) const
{
    if (imp == nullptr)
    {
        return aBase == nullptr;
    }

    return imp->operator==(aBase);
}

bool FileContainerIterator::operator!=(const IFlexDiskByFile *aBase) const
{
    if (imp == nullptr)
    {
        return aBase != nullptr;
    }

    return !(imp->operator==(aBase));
}

FileContainerIterator &FileContainerIterator::operator++()
{
    if (imp != nullptr)
    {
        if (!imp->NextDirEntry(wildcard))
        {
            imp->AtEnd();
        }
    }

    return *this;
}

FileContainerIterator &FileContainerIterator::operator=(IFlexDiskByFile *aBase)
{
    if (aBase == nullptr)
    {
        throw FlexException(FERR_INVALID_ITERATOR_USE);
    }

    if (imp != nullptr)
    {
        imp->AtEnd();
    }

    imp = aBase->IteratorFactory();

    if (!imp->NextDirEntry(wildcard))
    {
        imp->AtEnd();
    }

    return *this;
}

bool FileContainerIterator::DeleteCurrent()
{
    if (imp == nullptr)
    {
        return false;
    }

    return imp->DeleteCurrent();
}

bool FileContainerIterator::RenameCurrent(const std::string &newName)
{
    if (imp == nullptr)
    {
        return false;
    }

    return imp->RenameCurrent(newName);
}

bool FileContainerIterator::SetDateCurrent(const BDate &date)
{
    if (imp == nullptr)
    {
        return false;
    }

    return imp->SetDateCurrent(date);
}

bool FileContainerIterator::SetAttributesCurrent(Byte attributes)
{
    if (imp == nullptr)
    {
        return false;
    }

    return imp->SetAttributesCurrent(attributes);
}

