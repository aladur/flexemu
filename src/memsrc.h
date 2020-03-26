/*
    memsrc.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2018-2020  W. Schwotzer

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



#ifndef MEMSRC_INCLUDED
#define MEMSRC_INCLUDED

#include "typedefs.h"
#include "bintervl.h"
#include <vector>



template<class T>
class MemorySource
{
public:
    using AddressRange = BInterval<T>;
    using AddressRanges = std::vector<AddressRange>;

    virtual const AddressRanges& GetAddressRanges() const = 0;
    virtual void CopyTo(Byte *buffer, T address, T aSize) const = 0;
};
#endif

