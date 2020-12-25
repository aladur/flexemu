/*
    schedule.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2020  W. Schwotzer

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
#include <mutex>
#include <vector>
#include <memory>
#include <condition_variable>
#include <mutex>
#include "cpustate.h"
#include "schedcpu.h"
#include "btime.h"
#include "bcommand.h"



class Inout;

class Scheduler
{
public:

    enum class Event
    {
        NONE = 0,
        SyncExec = (1 << 0),  // synchronous execute commands
        Timer = (1 << 1),     // execute timer events
        SetStatus = (1 << 2), // set cpu status
    };

    Scheduler() = delete;
    Scheduler(ScheduledCpu &x_cpu, Inout &x_inout);
    ~Scheduler();

    CpuState statemachine(CpuState initial_state);
    bool        is_finished();
    void        request_new_state(CpuState x_user_state);
    void        process_events();
    CpuState idleloop();
    CpuState runloop(RunMode mode);

    // Thread support
public:
    void        sync_exec(BCommandPtr new_command);
    void    run();

protected:
    void        execute_commands();
    void suspend();
    void resume();
    std::mutex condition_mutex;
    std::condition_variable condition;
    std::mutex      command_mutex;
    std::mutex      status_mutex;
    std::mutex      irq_status_mutex;
    std::vector<BCommandPtr> commands;

    // Timer interface:
public:
    QWord       get_total_cycles()
    {
        return total_cycles;
    }
    void timer_elapsed();
protected:
//    static void timer_elapsed(void *p);
//    void set_timer();

    ScheduledCpu &cpu;
    Inout &inout;
    CpuState state;
    Event       events;
    CpuState user_state;
    QWord       total_cycles;
    QWord       time0sec;
    BTime       systemTime;

    // CPU status
public:
    void        get_interrupt_status(tInterruptStatus &s);
    CpuStatus  *get_status();
protected:
    tInterruptStatus interrupt_status;
    void        do_reset();
    CpuStatusPtr cpu_status;
    bool        is_status_valid;
    bool        is_resume;

    // CPU frequency
public:
    void        set_frequency(float target_freq);
    float       get_frequency()
    {
        return frequency;
    }
    float       get_target_frequency()
    {
        return target_frequency;
    }
protected:
    void        update_frequency();
    void        frequency_control(QWord time1);
    float       target_frequency;
    float       frequency;      // current frequency
    QWord       time0;          // time for freq control
    QWord       cycles0;        // cycle count for freq calc
};

inline Scheduler::Event operator| (Scheduler::Event lhs, Scheduler::Event rhs)
{
    using T1 = std::underlying_type<Scheduler::Event>::type;

    return static_cast<Scheduler::Event>(static_cast<T1>(lhs) |
                                         static_cast<T1>(rhs));
}

inline Scheduler::Event operator& (Scheduler::Event lhs, Scheduler::Event rhs)
{
    using T1 = std::underlying_type<Scheduler::Event>::type;

    return static_cast<Scheduler::Event>(static_cast<T1>(lhs) &
                                         static_cast<T1>(rhs));
}

inline Scheduler::Event operator|= (Scheduler::Event &lhs, Scheduler::Event rhs)
{
    return lhs = lhs | rhs;
}

inline Scheduler::Event operator&= (Scheduler::Event &lhs, Scheduler::Event rhs)
{
    return lhs = lhs & rhs;
}

inline Scheduler::Event operator~ (Scheduler::Event rhs)
{
    using T1 = std::underlying_type<Scheduler::Event>::type;

    return static_cast<Scheduler::Event>(~static_cast<T1>(rhs));
}

inline bool operator! (Scheduler::Event rhs)
{
    using T1 = std::underlying_type<Scheduler::Event>::type;

    return static_cast<T1>(rhs) == 0;
}

#endif // SCHEDULE_INCLUDED
