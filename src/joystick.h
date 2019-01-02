/*
    joystick.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2018-2019  W. Schwotzer

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



#ifndef JOYSTICK_INCLUDED
#define JOYSTICK_INCLUDED


#include <mutex>

// Button mask for left, middle and right button
#define L_MB        (4)
#define M_MB        (2)
#define R_MB        (1)

class JoystickIO
{
public:
    JoystickIO();

    void    reset();
    bool    get_values(int *deltaX, int *deltaY, unsigned int *buttonMask);
    void    put_values(int deltaX, int deltaY);
    void    put_value(unsigned int buttonMask);

private:
    int deltaX, deltaY;
    unsigned int buttonMask;
    bool    newValues;
    std::mutex joystick_mutex;
};

#endif

