/*
    bjoystck.h: a basic class for reading from standard analog joystick


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2023  W. Schwotzer
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

#ifdef LINUX_JOYSTICK_IS_PRESENT
#include <sys/stat.h>
#ifdef HAVE_FCNTL_H
    #include <fcntl.h>
#endif
#include <linux/joystick.h>
#include "bjoystck.h"


BJoystick::BJoystick(int which)
{
    axis[0] = axis[1] = buttons = static_cast<decltype(buttons)>(0);

    if (which == 0)
    {
        js = open(JOYSTICK_DEVICE_0, O_RDONLY);
    }
    else if (which == 1)
    {
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

bool BJoystick::IsOpened()
{
#ifdef LINUX_JOYSTICK_IS_PRESENT
    return js >= 0;
#else
    return false;
#endif
}

bool BJoystick::Actualize()
{
    struct JS_DATA_TYPE raw_js_data;

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

        axis[0]   = x - 128;
        axis[1]   = y - 128;
        buttons = raw_js_data.buttons ;

        return true;
    }

    return false;
}

// check for button "which" is set. Which is in the range of 0 .. 31
bool BJoystick::IsButtonSet(int which)
{
    if (which < 0 || which > 31)
    {
        return false;
    }

    return (buttons & (1 << which)) != 0;
}

#endif  //#ifdef LINUX_JOYSTICK_IS_PRESENT
