/*
    tstdev.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2025  W. Schwotzer

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


#include "tstdev.h"

TestDevice::TestDevice(Word byte_size) :
    data(byte_size, '\0')
{
}

void TestDevice::resetIo()
{
}

Byte TestDevice::readIo(Word offset)
{
    if (offset < data.size())
    {
        return data[offset];
    }

    return '\0'; // default, should never be used!
}


void TestDevice::writeIo(Word offset, Byte value)
{
    if (offset < data.size())
    {
        data[offset] = value;
    }
}

Word TestDevice::sizeOfIo()
{
    return static_cast<Word>(data.size());
}
