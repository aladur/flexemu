/*
    ioaccess.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2018  W. Schwotzer

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



#ifndef IOACCESS_INCLUDED
#define IOACCESS_INCLUDED

#include "misc1.h"
#include "iodevice.h"


class IoAccess
{
public:
    IoAccess() = delete;
    IoAccess(const IoAccess &src) :
        device(src.device),
        addressOffset(src.addressOffset)
    {
    }
    IoAccess(IoDevice &pdevice, Word paddressOffset) :
        device(pdevice),
        addressOffset(paddressOffset)
    {
    }

    IoAccess& operator=(const IoAccess &src)
    {
        device = src.device;
        addressOffset = src.addressOffset;
        return *this;
    }

    inline Byte read()
    {
        return device.readIo(addressOffset);
    }

    inline void write(Byte value)
    {
        device.writeIo(addressOffset, value);
    }

private:
    IoDevice &device;
    Word addressOffset;
};


#endif

