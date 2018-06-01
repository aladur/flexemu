/*
    mc6821.h


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



#ifndef __mc6821_h__
#define __mc6821_h__

#include <stdio.h>
#include "iodevice.h"

// PIA control lines

#define CB1 0
#define CB2 1
#define CA2 2
#define CA1 3

class Mc6821 : public IoDevice
{

    // Internal registers:
    //
    // cra, crb control register A, B
    // ddra, ddrb   data direction register A, B
    // ora, orb output register A, B

protected:

    Byte                 cra, ora, ddra, crb, orb, ddrb, cls;

public:

    virtual Byte             readIo(Word offset);
    virtual void             writeIo(Word offset, Byte val);
    virtual void             resetIo();
    virtual const char      *getName()
    {
        return "mc6821";
    };

    // generate an active transition on CA1, CA2, CB1 or CB2

public:

    virtual void            activeTransition(Byte control_line);

    // test contol lines CB1 or CB2

public:

    virtual Byte            testControlLine(Byte control_line);

    // read non strobed data

protected:

    virtual Byte            readInputA();
    virtual Byte            readInputB();
    virtual void            set_irq_A();
    virtual void            set_irq_B();

    // read strobed data

protected:

    virtual void            requestInputA();
    virtual void            requestInputB();

    // write data to port-pins

protected:

    virtual void            writeOutputA(Byte val);
    virtual void            writeOutputB(Byte val);


    // Public constructor and destructor

public:

    Mc6821();
    virtual             ~Mc6821();

};

#endif // __mc6821_h__
