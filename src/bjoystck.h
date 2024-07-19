/*
    bjoystck.h: a basic class for reading from standard analog joystick


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

#ifndef BJOYSTICK_INCLUDED
#define BJOYSTICK_INCLUDED

#ifdef LINUX_JOYSTICK_IS_PRESENT

#include <cstdint>
#include <array>

#define JOYSTICK_DEVICE_0   "/dev/js0"
#define JOYSTICK_DEVICE_1   "/dev/js1"

class BJoystick
{

public:
    explicit BJoystick(int which = 0);
    BJoystick(const BJoystick &src) = delete;
    ~BJoystick();
    bool IsOpened() const;
    bool Actualize();
    int32_t XAxis();
    int32_t YAxis();
    uint32_t Buttons() const;
    bool IsButtonSet(uint32_t which) const;
private:
    int js{}; // input stream from joystick
    uint32_t buttons{};
    std::array<int32_t, 2> axis{};
};

inline int32_t BJoystick::XAxis()
{
    return axis[0];
}
inline int32_t BJoystick::YAxis()
{
    return axis[1];
}
inline uint32_t BJoystick::Buttons() const
{
    return buttons;
}

#endif // #ifdef LINUX_JOYSTICK_IS_PRESENT
#endif // #ifdef BJOYSTICK_INCLUDED

