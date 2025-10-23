/*
    pia2.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2025  W. Schwotzer

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



#ifndef PIA2_INCLUDED
#define PIA2_INCLUDED

#include "misc1.h"
#include "mc6821.h"
#include "bjoystck.h"


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
    cycles_t cycles{0};

#ifdef LINUX_JOYSTICK_IS_PRESENT
    BJoystick joystick;
#endif

    // Processor status functions

protected:

    void writeOutputB(Byte value) override;
    Byte readInputB() override;

public:
    void resetIo() override;
    const char *getName() override
    {
        return "pia2";
    };
    Pia2(Mc6809 &p_cpu, KeyboardIO &p_keyboardIO, JoystickIO &p_joystick);
    ~Pia2() override = default;
};

#endif // PIA2_INCLUDED


