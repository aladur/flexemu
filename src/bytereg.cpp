/*
    bytereg.cpp


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


#include "misc1.h"
#include "bytereg.h"

ByteRegister::ByteRegister()
{
}

ByteRegister::~ByteRegister()
{
}

Byte ByteRegister::requestReadValue()
{
    // If data bus for read is not addressed typically
    // 0xff is returned due to pull-up resistors.
    return 0xff;
}

void ByteRegister::requestWriteValue(Byte)
{
}

void ByteRegister::resetIo()
{
    // default: Ignore.
    // Register not necessarily has to be connected with /RESET
}

Byte ByteRegister::readIo(Word offset)
{
    if (offset == 0)
    {
        return requestReadValue();
    }

    return 0; // default, should never be used!
}


void ByteRegister::writeIo(Word offset, Byte value)
{
    if (offset == 0)
    {
        requestWriteValue(value);
    }
}

