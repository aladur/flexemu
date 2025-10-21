/*
    joystick.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2018-2025  W. Schwotzer

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
#include <mutex>


JoystickIO::JoystickIO()
{
    reset();
}

void JoystickIO::reset()
{
    std::lock_guard<std::mutex> guard(joystick_mutex);
    deltaX = 0;
    deltaY = 0;
    buttonMask = 0;
    newValues = false;
}

bool JoystickIO::get_values(int *p_deltaX, int *p_deltaY,
                            unsigned int *p_buttonMask)
{
    bool result;

    std::lock_guard<std::mutex> guard(joystick_mutex);
    result = newValues;

    if (p_deltaX != nullptr)
    {
        *p_deltaX = deltaX;
    }

    if (p_deltaY != nullptr)
    {
        *p_deltaY = deltaY;
    }

    if (p_buttonMask != nullptr)
    {
        *p_buttonMask = buttonMask;
    }

    newValues = false;
    return result;
}

void JoystickIO::put_values(int p_deltaX, int p_deltaY)
{
    std::lock_guard<std::mutex> guard(joystick_mutex);
    deltaX = p_deltaX;
    deltaY = p_deltaY;
    newValues = true;
}

void JoystickIO::put_value(unsigned int p_buttonMask)
{
    std::lock_guard<std::mutex> guard(joystick_mutex);
    buttonMask = p_buttonMask;
}
