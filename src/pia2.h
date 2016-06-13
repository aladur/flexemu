/*
    pia2.h


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



#ifndef __pia2__h
#define __pia2__h

#include <misc1.h>
#include "inout.h"
#include "mc6821.h"
#include "mc6809.h"

#ifdef  LINUX_JOYSTICK_IS_PRESENT
    class BJoystick;
#endif

class Pia2 : public Mc6821
{

    // Terminal bell and Joystick connected to Port B

private:

    Inout              *io;
    Mc6809             *cpu;
    QWord               cycles;

#ifdef  LINUX_JOYSTICK_IS_PRESENT
    BJoystick          *joystick;
#endif

    // Processor status functions

protected:

    virtual void            writeOutputB(Byte val);
    virtual Byte            readInputB(void);
public:
    virtual void            resetIo(void);
    virtual const char      *getName(void)
    {
        return "pia2";
    };
    Pia2(Inout *x_io, Mc6809 *x_cpu);
    virtual             ~Pia2();
};

#endif // __pia2__h

