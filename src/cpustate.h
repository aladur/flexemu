/*
    cpustate.h


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



#ifndef CPUSTATE_INCLUDED
#define CPUSTATE_INCLUDED

#include <memory>
#include "typedefs.h"

enum class CpuState : uint8_t
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

// The time in micro-seconds on which a timer get's a timeout
// to execute updates.
enum : uint16_t {
TIME_BASE = 10000,
};


class CpuStatus
{
public:
    CpuStatus()  = default;
    virtual ~CpuStatus() = default;

    float freq{0.0};
    CpuState state{CpuState::NONE};
};

using CpuStatusPtr = std::unique_ptr<CpuStatus>;

#endif // CPUSTATE_INCLUDED

