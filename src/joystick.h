/*
    joystick.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2018-2024  W. Schwotzer

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
const unsigned L_MB{4U};
const unsigned M_MB{2U};
const unsigned R_MB{1U};

class JoystickIO
{
public:
    JoystickIO();

    void reset();
    bool get_values(int *p_deltaX, int *p_deltaY, unsigned int *p_buttonMask);
    void put_values(int p_deltaX, int p_deltaY);
    void put_value(unsigned int p_buttonMask);

private:
    int deltaX{0};
    int deltaY{0};
    unsigned int buttonMask{0};
    bool newValues{false};
    std::mutex joystick_mutex;
};

#endif

