/*
    mmu.h


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



#ifndef __mmu_h__
#define __mmu_h__

#include "misc1.h"

#include "iodevice.h"

class Memory;
class Inout;


class Mmu : public IoDevice
{

    // Internal registers

protected:

    Memory      *memory;
    Inout       *io;


    // public interface
public:

    void                resetIo();
    virtual Byte        readIo(Word addr);
    virtual void        writeIo(Word addr, Byte val);
    virtual const char *getName()
    {
        return "mmu";
    };

    // Public constructor and destructor
public:
    Mmu(Inout *x_io, Memory *x_memory);
    virtual ~Mmu();
};

#endif // __mmu_h__

