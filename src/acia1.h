/*
    acia1.h


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



#ifndef __acia1_h__
#define __acia1_h__

#include <stdio.h>
#include "inout.h"
#include "mc6809.h"
#include "mc6850.h"

class Acia1 : public Mc6850
{

protected:

    Mc6809          *cpu;
    Inout           *io;

public:
    // read data from serial line
    virtual Byte            readInput(void);

    // write data to serial line
    virtual void            writeOutput(Byte val);

    // set an interrupt
    virtual void            set_irq(void);

    // request for an input ready to be read
    virtual void            requestInput(void);

public:
    virtual void            resetIo(void);

    // Public constructor and destructor

public:

    virtual const char      *getName(void)
    {
        return "acia1";
    };
    Acia1(Inout *x_io, Mc6809 *x_cpu);
    virtual             ~Acia1();

};

#endif // __acia1_h__
