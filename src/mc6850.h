/*
    mc6850.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2019  W. Schwotzer

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



#ifndef MC6850_INCLUDED
#define MC6850_INCLUDED

#include "misc1.h"
#include <stdio.h>

#include "iodevice.h"

class Mc6850 : public IoDevice
{

    // Internal registers:
    //
    // cr       control register (write only)
    // sr       status register  (read only)
    // rdr, tdr receive/transmit data register (read only/write only)

protected:

    Byte                 cr, sr, rdr, tdr;

public:

    Byte readIo(Word offset) override;
    void writeIo(Word offset, Byte value) override;
    void resetIo() override;
    const char *getName() override
    {
        return "mc6850";
    };
    Word sizeOfIo() override
    {
        return 2;
    }

    // actions to be done when a character is ready to be received
    virtual void            activeTransition();

protected:
    // read data from serial line
    virtual Byte            readInput();
    // write data to serial line
    virtual void            writeOutput(Byte val);
    // set an interrupt
    virtual void            set_irq();
    // request if character is ready to be read, update status register
    virtual void            requestInput();

    // Public constructor and destructor

public:

    Mc6850();
    virtual             ~Mc6850();

};

#endif // MC6850_INCLUDED
