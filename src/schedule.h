/*
    schedule.h


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



#ifndef SCHEDULE_INCLUDED
#define SCHEDULE_INCLUDED

#include "misc1.h"
#include <signal.h>
#include <mutex>
#include <vector>
#include "cpustate.h"
#include "schedcpu.h"
#include "btime.h"


#define DO_SYNCEXEC     0x80
#define DO_TIMER        0x100
#define DO_SET_STATUS       0x200


class BCommand;
class BTime;
class BTimer;
class Inout;
class ScheduledCpu;

class Scheduler
{
public:
    Scheduler() = delete;
    Scheduler(ScheduledCpu &x_cpu, Inout &x_inout);
    ~Scheduler();

    Byte        statemachine(Byte initial_state);
    bool        is_finished();
    void        set_new_state(Byte x_user_input);
    void        process_events();
    Byte        idleloop();
    Byte        runloop(Word mode);

    // Thread support
public:
    void        sync_exec(BCommand *newCommand);
    void    run();
protected:
    void        execute();
    std::mutex      command_mutex;
    std::mutex      status_mutex;
    std::mutex      irq_status_mutex;
    std::vector<BCommand *> commands;

    // Timer interface:
public:
    QWord       get_total_cycles()
    {
        return total_cycles;
    };
protected:
    static void timer_elapsed(void *p);
    void timer_elapsed();
    void set_timer();

    ScheduledCpu &cpu;
    Inout &inout;
    Byte        state;
    Word        events;
    Byte        user_input;
    QWord       total_cycles;
    QWord       time0sec;
    BTime       systemTime;
    static Scheduler *instance;

    // CPU status
public:
    void        get_interrupt_status(tInterruptStatus &s);
    CpuStatus  *get_status();
protected:
    tInterruptStatus interrupt_status;
    void        do_reset();
    CpuStatus   *pCurrent_status;

    // CPU frequency
public:
    void        set_frequency(float target_freq);
    float       get_frequency()
    {
        return frequency;
    };
protected:
    void        update_frequency();
    void        frequency_control(QWord time1);
    float       target_frequency;
    float       frequency;      // current frequency
    QWord       time0;          // time for freq control
    QWord       cycles0;        // cycle count for freq calc
};

#endif // SCHEDULE_INCLUDED
