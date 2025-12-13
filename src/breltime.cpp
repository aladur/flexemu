/*
    breltime.cpp


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

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include "config.h"
#include "typedefs.h"
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include "breltime.h"

// This class could also be realized as a envelope/letter pattern
// but this is still the most efficient C++ implementation

#ifdef UNIX
// return time in us as a unsigned int 64 Bit value
QWord BRelativeTime::GetTimeUsll()
{
    struct timeval tv{};

    gettimeofday(&tv, nullptr);
    return (static_cast<QWord>(tv.tv_sec) * 1000000U + tv.tv_usec);
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
        return static_cast<QWord>(count.QuadPart) * 1000000U /
               static_cast<QWord>(freq.QuadPart);
    }
    else
    {
        return 0;
    }

}
#endif
