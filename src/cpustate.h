/*
    cpustate.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2018  W. Schwotzer

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



#ifndef __cpustate_h__
#define __cpustate_h__

#define S_MASK          0xF

#define S_NO_CHANGE             1
#define S_RUN                   2
#define S_STOP                  3
#define S_STEP                  4
#define S_EXIT                  5
#define S_RESET                 6
#define S_NEXT                  7
#define S_RESET_RUN             8
#define S_INVALID               9

#define EXIT_SUSPEND        0x80

#define TIME_BASE               10000

#include "typedefs.h"

class CpuStatus
{
public:
    CpuStatus() : freq(0.0), state(0) { };
    ~CpuStatus() { };

    float   freq;
    Byte    state;
};
#endif // __cpustate_h__

