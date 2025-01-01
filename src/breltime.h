/*
    breltime.h


    Basic class for platform independent high resolution time support
    Copyright (C) 2001-2025  W. Schwotzer

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

#ifndef BRELTIME_INCLUDED
#define BRELTIME_INCLUDED

#include "typedefs.h"

// Get relative time values with us resolution with unknown reference.
// Interface can be used to calculate precise time differences between
// two time values.
//
class BRelativeTime
{
public:
    BRelativeTime() = delete;
    ~BRelativeTime() = delete;
    static QWord GetTimeUsll();
};

#endif
