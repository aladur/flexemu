/*
    bjoystck.h: a basic class for reading from standard analog joystick


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2019  W. Schwotzer

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

#ifndef BJOYSTICK_INCLUDED
#define BJOYSTICK_INCLUDED

#ifdef LINUX_JOYSTICK_IS_PRESENT

#define JOYSTICK_DEVICE_0   "/dev/js0"
#define JOYSTICK_DEVICE_1   "/dev/js1"

class  BJoystick
{

public:
    BJoystick(int which = 0);
    ~BJoystick();
    short IsOpened();
    short Actualize();
    short XAxis();
    short YAxis();
    int Buttons();
    int IsButtonSet(int which);
private:
    BJoystick(const BJoystick &d);  // private, should not be used
    int js; // input stream from joystick
    int buttons;
    short   axis[2];
};

inline short BJoystick::XAxis()
{
    return axis[0];
}
inline short BJoystick::YAxis()
{
    return axis[1];
}
inline int   BJoystick::Buttons()
{
    return buttons;
}

#endif // #ifdef LINUX_JOYSTICK_IS_PRESENT
#endif // #ifdef BJOYSTICK_INCLUDED

