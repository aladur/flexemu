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

   IFlexDiskByFile *m_container = new FlexDisk(...);
   FlexDiskIterator it("*.*");
   std::string fileName;
   int i = 0;

   for (it = m_container->begin(); it != m_container->end(); ++it)
   {
      fileName = (*it).GetTotalFileName();
      printf("%d. file: %s\n", ++i, fileName);
   }
*/

FlexDiskIterator::FlexDiskIterator(std::string p_wildcard /* = "*.*" */)
    : wildcard(std::move(p_wildcard))
{
}

FlexDirEntry &FlexDiskIterator::operator*()
{
    if (imp != nullptr)
    {
        return imp->GetDirEntry();
    }

    // maybe you used the iterator in the wrong way ?
    throw FlexException(FERR_INVALID_ITERATOR_USE);
}

FlexDirEntry *FlexDiskIterator::operator->()
{
    if (imp != nullptr)
    {
        return &imp->GetDirEntry();
    }

    // maybe you used the iterator in the wrong way ?
    throw FlexException(FERR_INVALID_ITERATOR_USE);

}

bool FlexDiskIterator::operator==(const IFlexDiskByFile *aBase) const
{
    if (imp == nullptr)
    {
        return aBase == nullptr;
    }

    return imp->operator==(aBase);
}

bool FlexDiskIterator::operator!=(const IFlexDiskByFile *aBase) const
{
    if (imp == nullptr)
    {
        return aBase != nullptr;
    }

    return !(imp->operator==(aBase));
}

FlexDiskIterator &FlexDiskIterator::operator++()
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

FlexDiskIterator &FlexDiskIterator::operator=(IFlexDiskByFile *aBase)
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

bool FlexDiskIterator::DeleteCurrent()
{
    if (imp == nullptr)
    {
        return false;
    }

    return imp->DeleteCurrent();
}

bool FlexDiskIterator::RenameCurrent(const std::string &newName)
{
    if (imp == nullptr)
    {
        return false;
    }

    return imp->RenameCurrent(newName);
}

bool FlexDiskIterator::SetDateCurrent(const BDate &date)
{
    if (imp == nullptr)
    {
        return false;
    }

    return imp->SetDateCurrent(date);
}

bool FlexDiskIterator::SetAttributesCurrent(Byte attributes)
{
    if (imp == nullptr)
    {
        return false;
    }

    return imp->SetAttributesCurrent(attributes);
}

