/*
    schedcpu.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2004-2025  W. Schwotzer

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



#ifndef SCHEDCPU_INCLUDED
#define SCHEDCPU_INCLUDED

#include "typedefs.h"
#include "cpustate.h"


enum tIrqType : uint8_t
{
    INT_IRQ = 0,
    INT_FIRQ,
    INT_NMI,
    INT_RESET
};

enum class RunMode : uint8_t
{
    SingleStepOver,
    SingleStepInto,
    RunningStart,
    RunningContinue,
};

/* The following struct represents the current Mc6809 CPU interrupt status */
/* For performance reasons it uses plain C arrays. */
/* NOLINTBEGIN(modernize-avoid-c-arrays) */
struct sInterruptStatus
{
    DWord count[INT_RESET + 1];
};
/* NOLINTEND(modernize-avoid-c-arrays) */

using tInterruptStatus = struct sInterruptStatus;

// Polymorphic interface, virtual dtor is required.
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class ScheduledCpu
{
public:
    virtual ~ScheduledCpu() = default;
    virtual void do_reset() = 0;
    virtual CpuState run(RunMode mode) = 0;
    virtual void exit_run() = 0;
    virtual QWord get_cycles(bool reset = false) = 0;
    virtual void get_status(CpuStatus *cpu_status) = 0;
    virtual CpuStatusPtr create_status_object() = 0;
    virtual void get_interrupt_status(tInterruptStatus &s) = 0;
    virtual void set_required_cyclecount(cycles_t required_cyclecount) = 0;
};

#endif // SCHEDCPU_INCLUDED

