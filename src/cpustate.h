/*
    cpustate.h


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



#ifndef CPUSTATE_INCLUDED
#define CPUSTATE_INCLUDED

#include <memory>
#include "typedefs.h"

enum class CpuState : Byte
{
    NONE,
    Run,
    Stop,
    Step,
    Exit,
    Reset,
    Next,
    ResetRun,
    Invalid,
    Suspend,
    Schedule,
    _count
};

#define TIME_BASE               10000


class CpuStatus
{
public:
    CpuStatus() : freq(0.0), state(CpuState::NONE) { };
    ~CpuStatus() { };

    float freq;
    CpuState state;
};

using CpuStatusPtr = std::unique_ptr<CpuStatus>;

#endif // CPUSTATE_INCLUDED

