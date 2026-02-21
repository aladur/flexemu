/*
    bjoystck.cpp  A basic class for reading from standard analog joystick

    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2026  W. Schwotzer

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


#include "config.h"
#ifdef LINUX_JOYSTICK_IS_PRESENT
#include <sys/stat.h>
#ifdef HAVE_FCNTL_H
    #include <fcntl.h>
#endif
#include <linux/joystick.h>
#include "bjoystck.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif


BJoystick::BJoystick(int which)
{
    if (which == 0)
    {
        // There is no alternative to this system call.
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        js = open(JOYSTICK_DEVICE_0, O_RDONLY);
    }
    else if (which == 1)
    {
        // There is no alternative to this system call.
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        js = open(JOYSTICK_DEVICE_1, O_RDONLY);
    }
    else
    {
        js = -1;
    }
}

BJoystick::~BJoystick()
{
    if (js >= 0)
    {
        close(js);
    }

    js = -1;
}

bool BJoystick::IsOpened() const
{
    return js >= 0;
}

bool BJoystick::Actualize()
{
    struct JS_DATA_TYPE raw_js_data{};

    if (js < 0)
    {
        return false;
    }

    auto status = read(js, &raw_js_data, JS_RETURN);

    if (status == JS_RETURN)
    {
        auto x = raw_js_data.x;
        auto y = raw_js_data.y;

        if (x < 0)
        {
            x = 0;
        }
        else if (x > 255)
        {
            x = 255;
        }

        if (y < 0)
        {
            y = 0;
        }
        else if (y > 255)
        {
            y = 255;
        }

        axis[0] = x - 128;
        axis[1] = y - 128;
        buttons = raw_js_data.buttons ;

        return true;
    }

    return false;
}

// check for button "which" is set. Which is in the range of 0 .. 31
bool BJoystick::IsButtonSet(uint32_t which) const
{
    if (which > 31U)
    {
        return false;
    }

    return (buttons & (1U << which)) != 0U;
}

#endif //#ifdef LINUX_JOYSTICK_IS_PRESENT
