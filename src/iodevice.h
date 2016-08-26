/*
    iodevice.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2004  W. Schwotzer

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



#ifndef __iodevice_h__
#define __iodevice_h__

#include "misc1.h"

class IoDevice
{

public:

    virtual Byte         readIo(Word offset) = 0;
    virtual void         writeIo(Word offset, Byte val) = 0;
    virtual void         resetIo(void) = 0;
    virtual const char      *getName(void) = 0;
    virtual ~IoDevice() { }; // necessary otherwise the destructor
    // of subclasses is not called!
};

#endif // __iodevice_h__

