/*
    bytereg.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2018-2024  W. Schwotzer

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


#ifndef BYTEREG_INCLUDED
#define BYTEREG_INCLUDED

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

    void resetIo() override;
    Byte readIo(Word offset) override;
    void writeIo(Word offset, Byte value) override;
    const char *getName() override
    {
        return "bytereg";
    };
    Word sizeOfIo() override
    {
        return 1;
    };

public:

    ByteRegister();
    ~ByteRegister() override;
};

#endif

