/*
    schedcpu.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2004-2018  W. Schwotzer

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

typedef unsigned long int t_cycles; // must be unsigned long because of sprintf

enum tIrqType
{
    INT_IRQ = 0,
    INT_FIRQ,
    INT_NMI,
    INT_RESET
};

enum class RunMode : Byte
{
    SingleStepOver,
    SingleStepInto,
    RunningStart,
    RunningContinue,
};

struct sInterruptStatus
{
    DWord count[INT_RESET + 1];
};

typedef struct sInterruptStatus tInterruptStatus;

class ScheduledCpu
{
public:
    virtual ~ScheduledCpu() { };
    virtual void do_reset() = 0;
    virtual CpuState run(RunMode mode) = 0;
    virtual void exit_run() = 0;
    virtual QWord get_cycles(bool reset = false) = 0;
    virtual void get_status(CpuStatus *cpu_status) = 0;
    virtual CpuStatusPtr create_status_object() = 0;
    virtual void get_interrupt_status(tInterruptStatus &s) = 0;
    virtual void set_required_cyclecount(t_cycles required_cyclecount) = 0;
};

#endif // SCHEDCPU_INCLUDED

