/*
    btime.h


    Basic class for platform independent high resolution time support
    Copyright (C) 2001-2020  W. Schwotzer

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

#ifndef BTIME_INCLUDED
#define BTIME_INCLUDED

#include "misc1.h"


class BTime
{
public:
    BTime();
    ~BTime();
    void     ResetRelativeTime();
    QWord    GetRelativeTimeUsll(bool reset = false);
    double   GetRelativeTimeUsf(bool  reset = false);
    QWord GetRelativeTimeMsl(bool reset = false);
    QWord    GetTimeUsll();
    double   GetTimeUsf();
    QWord GetTimeMsl();

private:
    QWord    lapTime;
};

#endif // BTIME_INCLUDED
