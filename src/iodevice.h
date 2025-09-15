/*
    iodevice.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2025  W. Schwotzer

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



#ifndef IODEVICE_INCLUDED
#define IODEVICE_INCLUDED

#include "misc1.h"

class IoDevice
{
public:

    virtual Byte readIo(Word offset) = 0;
    virtual void writeIo(Word offset, Byte value) = 0;
    virtual void resetIo() = 0;
    // Unique name of device instance.
    virtual const char *getName() = 0;
    // Description of device instance.
    virtual const char *getDescription() = 0;
    // Name of the device class.
    virtual const char *getClassName() = 0;
    // Description of device class.
    virtual const char *getClassDescription() = 0;
    // Vendor of device.
    virtual const char *getVendor() = 0;
    virtual Word sizeOfIo() = 0;
    virtual ~IoDevice() = default;
};

#endif // IODEVICE_INCLUDED

