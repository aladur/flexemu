/*
    bytereg.h


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


#ifndef __bytereg_h__
#define __bytereg_h__

#include "misc1.h"
#include <stdio.h>

#include "iodevice.h"

// ByteRegister is a single byte general purpose
// I/O mapped read/write register.
// It can be specialized by a sub class overwriting 
// requestReadValue() and requestWriteValue() methods.

class ByteRegister : public IoDevice
{
protected:

    virtual Byte requestReadValue();
    virtual void requestWriteValue(Byte value);

public:
    // IoDevice member functions

    virtual void resetIo();
    virtual Byte readIo(Word offset);
    virtual void writeIo(Word offset, Byte value);
    virtual Word sizeIo() const
    {
        return 1;
    }
    virtual const char *getName()
    {
        return "bytereg";
    };
    virtual int sizeOfIo()
    {
        return 1;
    };

public:

    ByteRegister();
    virtual ~ByteRegister();
};

#endif

