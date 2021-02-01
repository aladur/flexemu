/*
    joystick.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2018-2021  W. Schwotzer

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


#include "joystick.h"


JoystickIO::JoystickIO() : deltaX(0), deltaY(0), buttonMask(0), newValues(false)
{
    reset();
}

void JoystickIO::reset()
{
    std::lock_guard<std::mutex> guard(joystick_mutex);
    deltaX           = 0;
    deltaY           = 0;
    buttonMask       = 0;
    newValues        = false;
}

bool JoystickIO::get_values(int *pDeltaX, int *pDeltaY,
                            unsigned int *pButtonMask)
{
    bool result;

    std::lock_guard<std::mutex> guard(joystick_mutex);
    result = newValues;

    if (pDeltaX     != nullptr)
    {
        *pDeltaX     = deltaX;
    }

    if (pDeltaY     != nullptr)
    {
        *pDeltaY     = deltaY;
    }

    if (pButtonMask != nullptr)
    {
        *pButtonMask = buttonMask;
    }

    newValues  = false;
    return result;
}

void JoystickIO::put_values(int x_deltaX, int x_deltaY)
{
    std::lock_guard<std::mutex> guard(joystick_mutex);
    deltaX     = x_deltaX;
    deltaY     = x_deltaY;
    newValues  = true;
}

void JoystickIO::put_value(unsigned int x_buttonMask)
{
    std::lock_guard<std::mutex> guard(joystick_mutex);
    buttonMask = x_buttonMask;
}
