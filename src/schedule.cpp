/*
    schedule.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2004  W. Schwotzer

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


#include <limits.h>
#ifndef _WIN32
    #include <sched.h>
#endif
#include "misc1.h"
#include "schedule.h"
#include "mc6809.h"
#include "inout.h"
#include "btime.h"
#include "btimer.h"
#include "bmutex.h"
#include "bcommand.h"


Scheduler::Scheduler(sOptions * /*pOptions*/) : BThread(false),
    commandMutex(NULL), statusMutex(NULL), irqStatMutex(NULL),
    state(S_RUN), events(0), user_input(S_NO_CHANGE), total_cycles(0),
    time0sec(0), cpu(NULL), io(NULL), systemTime(NULL),
    pCurrent_status(NULL),
    target_frequency(0.0), frequency(0.0), time0(0), cycles0(0)
{
    int i;
#ifdef UNIX
    sigset_t sigmask;

    // By default mask the SIGALRM signal
    // ATTENTION: For POSIX compatibility
    // this should be done in the main thread
    // before creating any thread
    sigemptyset(&sigmask);
    sigaddset(&sigmask, SIGALRM);
    sigprocmask(SIG_BLOCK, &sigmask, NULL);
#endif

    memset(&interrupt_status, 0, sizeof(tInterruptStatus));

    for (i = 0; i < MAX_COMMANDS; i++)
    {
        command[i] = NULL;
    }

    commandMutex       = new BMutex;
    statusMutex    = new BMutex;
    irqStatMutex  = new BMutex;
    systemTime = new BTime;
}

Scheduler::~Scheduler()
{
    int i;

    BTimer::Instance()->Stop();
    commandMutex->lock();

    for (i = 0; i < MAX_COMMANDS; i++)
    {
        delete command[i];
    }

    commandMutex->unlock();
    statusMutex->lock();
    delete pCurrent_status;
    pCurrent_status = NULL;
    statusMutex->unlock();

    delete systemTime;
    delete irqStatMutex;
    delete statusMutex;
    delete commandMutex;
}

void Scheduler::set_new_state(Byte x_user_input)
{
    user_input = x_user_input;
    cpu->exit_run();
}

bool Scheduler::is_finished()
{
    // this is the final state. program can savely be shutdown
    return state == S_EXIT;
}

void Scheduler::process_events(void)
{
    if (events != 0)
    {
        if (events & DO_TIMER)
        {
            irqStatMutex->lock();
            cpu->get_interrupt_status(interrupt_status);
            irqStatMutex->unlock();
            QWord time1sec = systemTime->GetTimeUsll();
            total_cycles = cpu->get_cycles(true);

            if (target_frequency > 0.0)
            {
                frequency_control(time1sec);
            }

            if (time1sec - time0sec >= 1000000)
            {
                // Do 1 second update
                update_frequency();
                events |= DO_SET_STATUS;

                if (io != NULL)
                {
                    io->update_1_second();
                }

                time0sec += 1000000;
            }

            events &= ~DO_TIMER;
        }

        if (events & DO_SET_STATUS)
        {
            statusMutex->lock();

            if (io->is_gui_present() && pCurrent_status == NULL)
            {
                events &= ~DO_SET_STATUS;
                pCurrent_status = cpu->create_status_object();
                cpu->get_status(pCurrent_status);
                pCurrent_status->freq   = frequency;
                pCurrent_status->state  = state;
            }

            statusMutex->unlock();
        }

        if (events & DO_SYNCEXEC)
        {
            Execute();
        }
    }
}

// enter with state S_INVALID or S_STOP
// return with any other state
Byte Scheduler::idleloop()
{
    while (user_input == S_NO_CHANGE || user_input == S_STOP)
    {
        process_events();
        BTimer::Instance()->Suspend();

        // S_INVALID only temp. state to update CPU view
        if (state == S_INVALID)
        {
            return S_STOP;
        }
    }

    return user_input;
}

Byte Scheduler::runloop(Word mode)
{
    Byte new_state;

    do
    {
        new_state = cpu->run(mode);

        if (new_state & EXIT_SUSPEND)
            // suspend thread until next timer tick
        {
            BTimer::Instance()->Suspend();
        }

        process_events();

        if (user_input != S_NO_CHANGE)
        {
            return user_input;
        }

        mode = CONTINUE_RUNNING;
    }
    while ((new_state & S_MASK) == S_NO_CHANGE);

    return new_state & S_MASK;
}

Byte Scheduler::statemachine(Byte initial_state)
{
    Byte prev_state = initial_state;

    state = initial_state;

    while (state != S_EXIT)
    {
        user_input = S_NO_CHANGE;

        switch (state)
        {
            case S_RUN:
                prev_state = state;
                state = runloop(START_RUNNING);
                break;

            case S_NEXT:
                state = runloop(SINGLESTEP_OVER);
                break;

            case S_STEP:
                state = runloop(SINGLESTEP_INTO);
                break;

            case S_STOP:
                prev_state = state;
                state = idleloop();
                break;

            case S_RESET:
                do_reset();
                state = prev_state;
                break;

            case S_RESET_RUN:
                do_reset();
                state = S_RUN;
                break;

            case S_INVALID:
                prev_state = S_RUN;
                state = idleloop();
                break;

            case S_EXIT:
                break;

            case S_NO_CHANGE:
                // This case should never happen
                // Set the state to S_RUN to avoid an endless loop
                DEBUGPRINT("Error in Statemachine: Set state to S_RUN\n");
                state = S_RUN;
                break;
        } // switch

        if (io->is_gui_present())
        {
            events |= DO_SET_STATUS;
        }
    } // while

    return state;
} // statemachine

void Scheduler::timer_elapsed(void *p)
{
    if (p != NULL)
    {
        ((Scheduler *)p)->timer_elapsed();
    }
}

void Scheduler::timer_elapsed()
{
    events |= DO_TIMER;
    cpu->exit_run();
#ifdef __BSD
    BTimer::Instance()->Start(false, TIME_BASE);
#endif
}

void Scheduler::do_reset(void)
{
    cpu->do_reset();
    total_cycles = 0;
    cycles0      = 0;
}

// thread support: Start Running CPU Thread
void Scheduler::Run()
{
    bool periodic = true;

#ifdef _WIN32
    HANDLE hThread = (HANDLE)GetCurrentThread();
    // Decrease Thread priority of CPU thread so that
    // the User Interface thread always has best response time
    SetThreadPriority(hThread, GetThreadPriority(hThread) - 1);
#endif

    BTimer::Instance()->SetTimerProc(timer_elapsed, this);
#ifdef __BSD
    // do not use a periodic timer because it is
    // not reliable on FreeBSD 5.1
    periodic = false;
#endif
    BTimer::Instance()->Start(periodic, TIME_BASE);
    time0sec = systemTime->GetTimeUsll();
    statemachine(S_RUN);
}

void Scheduler::sync_exec(BCommand *newCommand)
{
    int i = 0;

    commandMutex->lock();

    while (command[i] != NULL && i < MAX_COMMANDS - 1)
    {
        i++;
    }

    if (i < MAX_COMMANDS - 1)
    {
        command[i] = newCommand;
        events |= DO_SYNCEXEC;
        cpu->exit_run();
    }

    commandMutex->unlock();
}

void Scheduler::Execute()
{
    int i = 0;

    // use a static list for performance reasons
    // after execution delete the command from the list
    commandMutex->lock();

    while (command[i] != NULL)
    {
        command[i]->Execute();
        delete command[i];
        command[i] = NULL;
        i++;
    }

    events &= ~DO_SYNCEXEC;
    commandMutex->unlock();
}

bool Scheduler::status_available()
{
    bool result;

    statusMutex->lock();
    result = (pCurrent_status != NULL);
    statusMutex->unlock();

    return result;
}

CpuStatus *Scheduler::get_status()
{
    CpuStatus *stat = NULL;

    statusMutex->lock();

    if (pCurrent_status != NULL)
    {
        stat = pCurrent_status;
        pCurrent_status = NULL;
    }

    statusMutex->unlock();
    return stat;
}

void Scheduler::get_interrupt_status(tInterruptStatus &stat)
{
    irqStatMutex->lock();
    memcpy(&stat, &interrupt_status, sizeof(tInterruptStatus));
    irqStatMutex->unlock();
}

//#define DEBUG_FILE "time.txt"
void Scheduler::frequency_control(QWord time1)
{
#ifdef DEBUG_FILE
    FILE *fp;
#endif
    t_cycles required_cyclecount;

    if (time0 == 0)
    {
        time0 = time1;
        required_cyclecount = static_cast<t_cycles>
                              (TIME_BASE * target_frequency);
        cpu->set_required_cyclecount(required_cyclecount);
    }
    else
    {
        SQWord timediff = time1 - time0;
        required_cyclecount = static_cast<t_cycles>
                              (timediff * target_frequency);
        cpu->set_required_cyclecount(required_cyclecount);
        time0 = time1;
#ifdef DEBUG_FILE

        if ((fp = fopen(DEBUG_FILE, "a")) != NULL)
        {
            fprintf(fp, "timediff: %llu required_cyclecount: %lu\n",
                    timediff, required_cyclecount);
            fclose(fp);
        }

#endif
    }
}

void Scheduler::update_frequency()
{
    // calculate frequency in MHz
    t_cycles cyclecount;
    QWord cycles1;

    cycles1 = cpu->get_cycles();
    cyclecount = static_cast<t_cycles>(cycles1 - cycles0);
    frequency = (float)(cyclecount / 1000000.0);
    cycles0 = cycles1;
}


void Scheduler::set_frequency(float target_freq)
{
    if (target_freq <= 0)
    {
        target_frequency = (float)0.0;
    }
    else
    {
        target_frequency = target_freq;
        time0 = 0;
    }

    if (cpu != NULL)
    {
        cpu->set_required_cyclecount(ULONG_MAX);
    }
} // set_frequency

