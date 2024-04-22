/*
    schedule.cpp


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


#include <limits>
#include <cinttypes>
#ifndef _WIN32
    #include <sched.h>
#endif
#include "misc1.h"
#include <signal.h>
#include "schedule.h"
#include "mc6809.h"
#include "inout.h"
#include "breltime.h"


Scheduler::Scheduler(ScheduledCpu &p_cpu, Inout &p_inout) :
    cpu(p_cpu), inout(p_inout),
    state(CpuState::Run), events(Event::NONE), user_state(CpuState::NONE),
    total_cycles(0), time0sec(0),
    is_status_valid(false), is_resume(false),
    target_frequency(ORIGINAL_FREQUENCY), frequency(0.0), time0(0), cycles0(0)
{
    memset(&interrupt_status, 0, sizeof(tInterruptStatus));
}

Scheduler::~Scheduler()
{
    commands.clear();
}

void Scheduler::request_new_state(CpuState p_user_state)
{
    if (p_user_state != CpuState::NONE)
    {
        user_state = p_user_state;
        cpu.exit_run();
    }
}

bool Scheduler::is_finished()
{
    // this is the final state. program can safely be shutdown.
    return state == CpuState::Exit;
}

void Scheduler::process_events()
{
    if (events != Event::NONE)
    {
        if ((events & Event::Timer) != Event::NONE)
        {
            irq_status_mutex.lock();
            cpu.get_interrupt_status(interrupt_status);
            irq_status_mutex.unlock();
            auto time1sec = BRelativeTime::GetTimeUsll();
            total_cycles = cpu.get_cycles(true);

            if (target_frequency > 0.0)
            {
                frequency_control(time1sec);
            }

            if (time1sec - time0sec >= 1000000)
            {
                // Do 1 second update
                update_frequency();
                events |= Event::SetStatus;

                inout.update_1_second();

                time0sec += 1000000;
            }

            events &= ~Event::Timer;
        }

        if ((events & Event::SetStatus) != Event::NONE)
        {
            std::lock_guard<std::mutex> guard(status_mutex);

            if (cpu_status == nullptr)
            {
                cpu_status = cpu.create_status_object();
            }

            if (inout.is_gui_present())
            {
                cpu.get_status(cpu_status.get());
                cpu_status->freq = frequency;
                cpu_status->state = state;
                is_status_valid = true;
            }

            events &= ~Event::SetStatus;
        }

        if ((events & Event::SyncExec) != Event::NONE)
        {
            execute_commands();

            events &= ~Event::SyncExec;
        }
    }
}

// Enter with state CpuState::Invalid or CpuState::Stop.
// Return with any other state.
CpuState Scheduler::idleloop()
{
    while (user_state == CpuState::NONE || user_state == CpuState::Stop ||
           user_state == CpuState::Invalid)
    {
        process_events();
        suspend();
    }

    return user_state;
}

CpuState Scheduler::runloop(RunMode mode)
{
    CpuState new_state = CpuState::Schedule;

    while (new_state == CpuState::Schedule)
    {
        new_state = cpu.run(mode);

        if (new_state == CpuState::Suspend)
        {
            // suspend thread until next timer tick
            suspend();
            new_state = CpuState::Schedule;
        }

        process_events();

        if (user_state != CpuState::NONE)
        {
            return user_state;
        }

        mode = RunMode::RunningContinue;
    }

    return new_state;
}

CpuState Scheduler::statemachine(CpuState initial_state)
{
    CpuState prev_state = initial_state;

    state = initial_state;

    while (state != CpuState::Exit)
    {
        user_state = CpuState::NONE;

        switch (state)
        {
            case CpuState::Run:
                prev_state = state;
                state = runloop(RunMode::RunningStart);
                break;

            case CpuState::Next:
                state = runloop(RunMode::SingleStepOver);
                break;

            case CpuState::Step:
                state = runloop(RunMode::SingleStepInto);
                break;

            case CpuState::Stop:
                prev_state = state;
                state = idleloop();
                break;

            case CpuState::Reset:
                do_reset();
                state = prev_state;
                break;

            case CpuState::ResetRun:
                do_reset();
                state = CpuState::Run;
                break;

            case CpuState::Invalid:
                prev_state = CpuState::Run;
                state = idleloop();
                break;

            case CpuState::Exit:
                break;

            case CpuState::NONE:
            case CpuState::Suspend:
            case CpuState::Schedule:
            case CpuState::_count:
                // This case should never happen
                // Set the state to CpuState::Run to avoid an endless loop
                DEBUGPRINT("Error in Statemachine: Set state to CpuState::Run\n");
                state = CpuState::Run;
                break;

        } // switch

        if (inout.is_gui_present())
        {
            events |= Event::SetStatus;
        }
    } // while

    return state;
} // statemachine

void Scheduler::timer_elapsed()
{
    events |= Event::Timer;
    resume();
    cpu.exit_run();
}

void Scheduler::do_reset()
{
    cpu.do_reset();
    total_cycles = 0;
    cycles0 = 0;
}

// thread support: Start Running CPU Thread
void Scheduler::run()
{
#ifdef _WIN32
    HANDLE hThread = (HANDLE)GetCurrentThread();
    // Decrease Thread priority of CPU thread so that
    // the User Interface thread always has best response time
    SetThreadPriority(hThread, GetThreadPriority(hThread) - 1);
#endif

    time0sec = BRelativeTime::GetTimeUsll();
    statemachine(CpuState::Run);
}

void Scheduler::sync_exec(BCommandPtr new_command)
{
    std::lock_guard<std::mutex> guard(command_mutex);

    commands.push_back(std::move(new_command));
    events |= Event::SyncExec;
    cpu.exit_run();
}

void Scheduler::execute_commands()
{
    std::lock_guard<std::mutex> guard(command_mutex);

    for (auto &command : commands)
    {
        command->Execute();
    }
    commands.clear();
}

CpuStatus *Scheduler::get_status()
{
    CpuStatus *status = nullptr;

    std::lock_guard<std::mutex> guard(status_mutex);

    if (is_status_valid)
    {
        status = cpu_status.get();
        is_status_valid = false;
    }

    return status;
}

void Scheduler::get_interrupt_status(tInterruptStatus &stat)
{
    std::lock_guard<std::mutex> guard(irq_status_mutex);
    memcpy(&stat, &interrupt_status, sizeof(tInterruptStatus));
}

//#define DEBUG_FILE "time.txt"
void Scheduler::frequency_control(QWord time1)
{
#ifdef DEBUG_FILE
    FILE *fp;
#endif
    cycles_t required_cyclecount;

    if (time0 == 0)
    {
        time0 = time1;
        required_cyclecount =
            static_cast<cycles_t>(TIME_BASE * target_frequency);
        cpu.set_required_cyclecount(required_cyclecount);
    }
    else
    {
        auto timediff = time1 - time0;
        auto fCyclecount = static_cast<double>(timediff) *
                           static_cast<double>(target_frequency);
        required_cyclecount = static_cast<cycles_t>(fCyclecount);
        cpu.set_required_cyclecount(required_cyclecount);
        time0 = time1;
#ifdef DEBUG_FILE

        if ((fp = fopen(DEBUG_FILE, "a")) != nullptr)
        {
            fprintf(fp, "timediff: %" PRId64 " required_cyclecount: %" PRIu64
                    "\n", timediff, required_cyclecount);
            fclose(fp);
        }

#endif
    }
}

void Scheduler::update_frequency()
{
    // calculate frequency in MHz
    cycles_t cyclecount;
    cycles_t cycles1;

    cycles1 = cpu.get_cycles();
    cyclecount = cycles1 - cycles0;
    frequency = static_cast<float>(static_cast<double>(cyclecount) / 1000000.0);
    cycles0 = cycles1;
}


void Scheduler::set_frequency(float p_target_frequency)
{
    cycles_t cycles;

    if (p_target_frequency == 0.0F)
    {
        target_frequency = p_target_frequency;
        cycles = std::numeric_limits<decltype(cycles)>::max();
    }
    else
    {
        if (p_target_frequency < 0.0F)
        {
            p_target_frequency = ORIGINAL_FREQUENCY;
        }
        target_frequency = p_target_frequency;
        cycles = static_cast<cycles_t>(TIME_BASE * target_frequency);
        time0 = 0;
    }

    cpu.set_required_cyclecount(cycles);
} // set_frequency

void Scheduler::suspend()
{
    std::unique_lock<std::mutex> lock(condition_mutex);
    is_resume = false;
    condition.wait(lock, [&](){ return is_resume; });
}

void Scheduler::resume()
{
    {
        std::unique_lock<std::mutex> lock(condition_mutex);
        is_resume = true;
    }
    condition.notify_one();
}

