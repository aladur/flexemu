/*
    pia2.h


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



#ifndef __pia2__h
#define __pia2__h

#include "misc1.h"
#include "mc6821.h"
#include "bjoystck.h"
#include <memory>


class Mc6809;
class JoystickIO;
class KeyboardIO;

class Pia2 : public Mc6821
{

    // Terminal bell and Joystick connected to Port B

private:

    Mc6809 &cpu;
    KeyboardIO &keyboardIO;
    JoystickIO &joystickIO;
    QWord               cycles;

#ifdef  LINUX_JOYSTICK_IS_PRESENT
    BJoystick joystick;
#endif

    // Processor status functions

protected:

    virtual void            writeOutputB(Byte val);
    virtual Byte            readInputB();
public:
    virtual void            resetIo();
    virtual const char      *getName()
    {
        return "pia2";
    };
    Pia2(Mc6809 &x_cpu, KeyboardIO &x_keyboardIO, JoystickIO &x_joystick);
    virtual             ~Pia2();
};

#endif // __pia2__h

