/*
    cpustate.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2025  W. Schwotzer

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

#include <cstdint>
#include <memory>
#include <ostream>

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

inline std::ostream &operator<<(std::ostream &os, CpuState state)
{
    switch (state)
    {
        case CpuState::NONE:
            os << "CpuState::NONE";
            break;

        case CpuState::Run:
            os << "CpuState::Run";
            break;

        case CpuState::Stop:
            os << "CpuState::Stop";
            break;

        case CpuState::Step:
            os << "CpuState::Step";
            break;

        case CpuState::Exit:
            os << "CpuState::Exit";
            break;

        case CpuState::Reset:
            os << "CpuState::Reset";
            break;

        case CpuState::Next:
            os << "CpuState::Next";
            break;

        case CpuState::ResetRun:
            os << "CpuState::ResetRun";
            break;

        case CpuState::Invalid:
            os << "CpuState::Invalid";
            break;

        case CpuState::Suspend:
            os << "CpuState::Suspend";
            break;

        case CpuState::Schedule:
            os << "CpuState::Schedule";
            break;

        case CpuState::_count:
        default:
            os << "CpuState::unknown";
            break;
    }

    return os;
}

// The time in micro-seconds on which a timer get's a timeout
// to execute updates.
enum : uint16_t {
TIME_BASE = 10000,
};


struct CpuStatus
{
    CpuStatus()  = default;
    virtual ~CpuStatus() = default;

    CpuState state{CpuState::NONE};
};

using CpuStatusPtr = std::unique_ptr<CpuStatus>;

#endif // CPUSTATE_INCLUDED

