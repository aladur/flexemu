/*
    tstdev.h


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


#ifndef TSTDEV_INCLUDED
#define TSTDEV_INCLUDED

#include "typedefs.h"
#include "iodevice.h"
#include <vector>

// TestDevice is a test device with a configurable byte size.
// It behaves like normal RAM.

class TestDevice : public IoDevice
{
protected:
    std::vector<Byte> data;

public:
    // IoDevice member functions

    void resetIo() override;
    Byte readIo(Word offset) override;
    void writeIo(Word offset, Byte value) override;
    const char *getName() override
    {
        return "tstdev";
    };
    const char *getDescription() override
    {
        return "Test Device with RAM";
    }
    const char *getClassName() override
    {
        return "TSTDEV";
    }
    const char *getClassDescription() override
    {
        return "";
    }
    const char *getVendor() override
    {
        return "";
    }
    Word sizeOfIo() override;

    explicit TestDevice(Word byte_size = 1U);
    ~TestDevice() override = default;
};

#endif

