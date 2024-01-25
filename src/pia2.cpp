/*
    pia2.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2024  W. Schwotzer

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

#include "misc1.h"
#include "pia2.h"
#include "mc6809.h"
#include "bjoystck.h"
#include "joystick.h"
#include "keyboard.h"


Pia2::Pia2(Mc6809 &x_cpu, KeyboardIO &x_keyboardIO, JoystickIO &x_joystickIO) :
    cpu(x_cpu), keyboardIO(x_keyboardIO), joystickIO(x_joystickIO), cycles(0)
#ifdef LINUX_JOYSTICK_IS_PRESENT
    , joystick(0)
#endif
{
}

Pia2::~Pia2()
{
}

void Pia2::resetIo()
{
    Mc6821::resetIo();
    keyboardIO.reset_parallel();
    joystickIO.reset();
    cycles = 0;
}

void Pia2::writeOutputB(Byte val)
{
    if (val & 0x40)
    {
        keyboardIO.set_bell(0);
    }
}

Byte Pia2::readInputB()
{
    unsigned int buttonMask;
    unsigned int keyMask;
    int deltaX, deltaY;
    bool newValues;

    newValues = joystickIO.get_values(&deltaX, &deltaY, &buttonMask);
    keyboardIO.get_value(&keyMask);

    orb &= 0xc1;

    if (buttonMask & L_MB)
    {
        if (keyMask & SHIFT_KEY)   // shift L_MB to emulate M_MB
        {
            orb |= 0x20;
        }
        else
        {
            orb |= 0x02;
        }
    }

    if (buttonMask & M_MB)
    {
        if (keyMask & SHIFT_KEY)
        {
            orb |= 0x08;
        }
        else
        {
            if (keyMask & CONTROL_KEY)
            {
                orb |= 0x10;
            }
            else
            {
                orb |= 0x20;
            }
        }
    }

    if (buttonMask & R_MB)
    {
        if (keyMask & SHIFT_KEY)   // shift R_MB to emulate M_MB
        {
            orb |= 0x20;
        }
        else
        {
            orb |= 0x04;
        }
    }

    // joystick input will either be emulated with mouse buttons
    // or with a real joystick (only linux support at the moment):
#ifdef LINUX_JOYSTICK_IS_PRESENT

    if (joystick.IsOpened())
    {
        joystick.Actualize();

        if (joystick.XAxis() < -8)
        {
            orb |= 0x02;    // joystick to left side
        }

        if (joystick.XAxis() > 8)
        {
            orb |= 0x04;    // joystick to left side
        }

        if (joystick.YAxis() < -8)
        {
            orb |= 0x10;    // joystick up
        }

        if (joystick.YAxis() > 8)
        {
            orb |= 0x08;    // joystick down
        }

        if (joystick.IsButtonSet(0))
        {
            orb |= 0x20;    // joystick "shoot"
        }
    }

#endif

    // the emulation of the analog Eltec Joystick is implemented
    // as follows:
    // If there is no pointer movement a middle frequency of
    // about 6.44 KHz will be generated.
    // Pointer moves are available in variable deltaX/deltaY
    // if there is a move in any direction a higher/lower frequency
    // will be generated at the pia port bits 0 (horizontal) or
    // 7 (vertical)
    // The transfromation from mouse move given as delta pixel in
    // x- or y-direction into the joystick frequencies (or better
    // period time) is done with the 1D table tab_period_from_mouse.
    // Before reading the frequency from the table the mouse movement
    // is limited to +/- TAB_OFFSET

#define TAB_OFFSET   (15)
    static short tab_period_from_mouse[(TAB_OFFSET << 1) + 1] =
    {
        8000, 7084, 6272, 5554, 4918, 4354, 3856, 3414, 3023, 2677,
        2370, 2099, 1858, 1645, 1457, 1290, 1142, 1011,  896,  793,
        702,  622,  550,  487,  432,   382,  338,  300,  265,  235,
        208
    };

    static int count     = 0;
    static int Tx = tab_period_from_mouse[TAB_OFFSET];
    static int Ty = tab_period_from_mouse[TAB_OFFSET];
    static int tx = 0;
    static int ty = 0;
    int dX, dY;
    cycles_t cyclediff;

    count++;

    if (newValues)
    {
        count = 0;
    }

    if (count < 300)
    {
        dX = (deltaX > TAB_OFFSET) ? TAB_OFFSET : deltaX;

        if (dX < -TAB_OFFSET)
        {
            dX = -TAB_OFFSET;
        }

        dY = (deltaY > TAB_OFFSET) ? TAB_OFFSET : deltaY;

        if (dY < -TAB_OFFSET)
        {
            dY = -TAB_OFFSET;
        }
    }
    else
    {
        dX = 0;
        dY = 0;
    }

    cycles_t prev_cycles = cycles;
    cycles = cpu.get_cycles();

    if (cycles < prev_cycles)
    {
        // cycle count overflow
        cyclediff = std::numeric_limits<QWord>::max() - prev_cycles +
                    cycles + 1;
    }
    else
    {
        cyclediff = cycles - prev_cycles;
    }

    if (cyclediff > 100) // more than 75 us
    {
        // initialize for a new measurement
        Tx = tab_period_from_mouse[dX + TAB_OFFSET];
        Ty = tab_period_from_mouse[dY + TAB_OFFSET];
        tx = 0;
        ty = 0;
    }
    else
    {
        tx += static_cast<int>(cyclediff << 4);

        if (tx >= Tx)
        {
            tx -= Tx;
            Tx = tab_period_from_mouse[dX + TAB_OFFSET];
            orb ^= 0x01;
        }

        ty += static_cast<int>(cyclediff << 4);

        if (ty >= Ty)
        {
            ty -= Ty;
            Ty = tab_period_from_mouse[dY + TAB_OFFSET];
            orb ^= 0x80;
        }
    }

    return orb;
} // readInputB

