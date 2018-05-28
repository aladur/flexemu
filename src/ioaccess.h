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



#ifndef __ioaccess_h__
#define __ioaccess_h__

#include "iodevice.h"


class IoAccess
{
public:
    IoAccess() : device(NULL), addressOffset(0) { };
    IoAccess(IoDevice *pdevice, Word paddressOffset) :
        device(pdevice), addressOffset(paddressOffset) { };
    inline Byte read();
    inline void write(Byte value);

public:
    IoDevice *device;
    Word addressOffset;
};

inline Byte IoAccess::read()
{
    return device->readIo(addressOffset);
}

inline void IoAccess::write(Byte value)
{
    device->writeIo(addressOffset, value);
}

#endif

