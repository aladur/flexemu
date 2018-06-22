/*
    e2video.h

    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2018  W. Schwotzer

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



#ifndef E2VIDEO_INCLUDED
#define E2VIDEO_INCLUDED

#include "misc1.h"
#include "iodevice.h"

class Memory;


class E2video : public IoDevice
{

    // Internal registers
protected:
    Memory &memory;

public:
    Byte        vico1, vico2;

public:

    void resetIo() override;
    Byte readIo(Word offset) override;
    void writeIo(Word offset, Byte val) override;
    const char *getName() override
    {
        return "e2video";
    };
    int sizeOfIo() override
    {
        return 2;
    }

public:
    E2video(Memory &x_memory);
    virtual ~E2video();

};

#endif // E2VIDEO_INCLUDED

