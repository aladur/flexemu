/*
    breltime.cpp


    Basic class for platform independent high resolution time support
    Copyright (C) 2001-2022  W. Schwotzer

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

#ifdef _WIN32
    #include "misc1.h"
#endif
#include "breltime.h"

// This class could also be realized as a envelope/letter pattern
// but this is still the most efficient C++ implementation

BRelativeTime::BRelativeTime()
{
}

BRelativeTime::~BRelativeTime()
{
}

#ifdef UNIX
// return time in us as a unsigned int 64 Bit value
QWord BRelativeTime::GetTimeUsll()
{
    struct timeval tv;

    gettimeofday(&tv, nullptr);
    return ((QWord)tv.tv_sec * 1000000 + tv.tv_usec);
}
#endif

#ifdef _WIN32
// return time in us as a unsigned int 64 Bit value
QWord BRelativeTime::GetTimeUsll()
{
    LARGE_INTEGER count, freq;

    if (QueryPerformanceCounter(&count))
    {
        QueryPerformanceFrequency(&freq);
        return (QWord)count.QuadPart * 1000000 / (QWord)freq.QuadPart;
    }
    else
    {
        return 0;
    }

}
#endif
