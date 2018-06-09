/*
    btimer.cpp

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

#include "misc1.h"
#include <stdio.h>
#ifdef UNIX
    #include <signal.h>
    #include <errno.h>
#endif
#ifdef _WIN32
    #include <process.h>
#endif

#include "btimer.h"
#include "bdelete.h"

#ifdef _WIN32
    #ifndef _TTHREADPROC_DEFINED_
        typedef  unsigned long (__stdcall *tThreadProc)(void *);
        #define _TTHREADPROC_DEFINED_
    #endif
#endif

BTimer *BTimer::instance = nullptr;

BTimer::BTimer() : dueTime(0), periodic(false),
    timerProc(nullptr), timerParam(nullptr)
#ifdef _WIN32
    , timerHandle(nullptr), startTimerEvent(nullptr),
    timerElapsedEvent(nullptr),
    timerThread(0), doFinish(false)
#endif
{
    instance = this;
    Init();
}

BTimer::~BTimer()
{
    Stop();
    UnInit();
    instance  = nullptr;
    timerProc = nullptr;
}

BTimer *BTimer::Instance()
{
    if (instance == nullptr)
    {
        instance = new BTimer;
        static BDeleter<BTimer> deleter(instance);
    }

    return instance;
}

bool BTimer::Start(bool x_periodic, QWord x_dueTime)
{
    dueTime  = x_dueTime;
    periodic = x_periodic;
    return StartTimer();
}

void BTimer::SetTimerProc(BTimerProc x_timerProc, void *x_p /* = nullptr*/)
{
    timerParam = x_p;
    timerProc  = x_timerProc;
}

///////////////////////////////////////////////////////////////
// BTimer UNIX specific implementation
///////////////////////////////////////////////////////////////
#ifdef UNIX
void BTimer::Init()
{
}

void BTimer::UnInit()
{
}

bool BTimer::Stop()
{
    bool ret = true;
    struct itimerval timerValue;
    struct sigaction act;
    sigset_t sigmask;

    sigemptyset(&sigmask);
#ifdef SIGALRM
    sigaddset(&sigmask, SIGALRM);
    sigprocmask(SIG_BLOCK, &sigmask, nullptr);

    periodic   = false;
    timerProc  = nullptr;
    timerParam = nullptr;

    timerValue.it_value.tv_usec    = 0;
    timerValue.it_value.tv_sec     = 0;
    timerValue.it_interval.tv_usec = 0;
    timerValue.it_interval.tv_sec  = 0;

    // set alarm timer to IGNORE avoid
    // spurious alarm events to abort program
    act.sa_handler = SIG_IGN;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
#ifdef SA_INTERRUPT /* SunOS */
    act.sa_flags |= SA_INTERRUPT;
#endif

    if (sigaction(SIGALRM, &act, nullptr) == -1)
    {
        fprintf(stderr, "error %d in sigaction\n", errno);
    }

#endif
    ret = (setitimer(ITIMER_REAL, &timerValue, nullptr) == 0);
    return ret;
}

void BTimer::Suspend()
{
    sigset_t sigmask, oldmask;
    int sig;

    sigemptyset(&sigmask);
    sigaddset(&sigmask, SIGALRM);
    sigprocmask(SIG_BLOCK, &sigmask, &oldmask);
    sigwait(&sigmask, &sig);
    sigprocmask(SIG_SETMASK, &oldmask, nullptr);
    // when catching signal with sigwait
    // the signal handler is not executed
    // automatically so call it here
    UnixTimerCallback(0);
}

bool BTimer::StartTimer()
{
    int ret;
    struct itimerval timerValue;
    struct sigaction act;
    sigset_t sigmask;

    sigemptyset(&sigmask);
#ifdef SIGALRM
    sigaddset(&sigmask, SIGALRM);
    sigprocmask(SIG_UNBLOCK, &sigmask, nullptr);

    timerValue.it_value.tv_usec    = dueTime % 1000000;
    timerValue.it_value.tv_sec     = dueTime / 1000000;

    if (!periodic)
    {
        timerValue.it_interval.tv_usec = 0;
        timerValue.it_interval.tv_sec  = 0;
    }
    else
    {
        timerValue.it_interval.tv_usec = dueTime % 1000000;
        timerValue.it_interval.tv_sec  = dueTime / 1000000;
    }

    // use POSIX sigaction for setting the
    // SIGALRM signal action
    act.sa_handler = UnixTimerCallback;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
#ifdef SA_INTERRUPT /* SunOS */
    act.sa_flags |= SA_INTERRUPT;
#endif

    if (sigaction(SIGALRM, &act, nullptr) == -1)
    {
        fprintf(stderr, "error %d in sigaction\n", errno);
    }

    ret = setitimer(ITIMER_REAL, &timerValue, nullptr);

    if (ret == -1)
    {
        fprintf(stderr, "error %d in setitimer\n", errno);
    }

#else
#error "No SIGALRM available. Can't use timer"
#endif
    return (ret == 0);
}

RETSIGTYPE BTimer::UnixTimerCallback(int)
{
    if (instance != nullptr)
    {
        instance->TimerElapsed();
    }
}

void BTimer::TimerElapsed()
{
    if (timerProc != nullptr)
    {
        (*timerProc)(timerParam);
    }
}
#endif

///////////////////////////////////////////////////////////////
// BTimer Win32 specific implementation
///////////////////////////////////////////////////////////////

#ifdef _WIN32
void BTimer::Init()
{
    DWORD threadId; // necessary for Win95/98/ME

    timerHandle = CreateWaitableTimer(nullptr, FALSE, nullptr);

    if (timerHandle == nullptr)
        DEBUGPRINT1("BTimer: CreateWaitableTimer failed (%lu)\n",
                    GetLastError());

    startTimerEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

    if (startTimerEvent == nullptr)
        DEBUGPRINT1("BTimer: CreateEvent failed (%lu)\n",
                    GetLastError());

    timerElapsedEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

    if (startTimerEvent == nullptr)
        DEBUGPRINT1("BTimer: CreateEvent failed (%lu)\n",
                    GetLastError());

    timerThread = CreateThread(nullptr, 0, (tThreadProc)BTimer::StartTimerImp,
                               this, 0, &threadId);

    if (timerThread == nullptr)
        DEBUGPRINT1("BTimer: CreateThread failed (%lu)\n",
                    GetLastError());
}

void BTimer::UnInit()
{
    if (timerHandle != nullptr && timerThread != nullptr)
    {
        DWORD exitCode;

        doFinish = true;
        Start(false, 0);
        WaitForSingleObject(timerThread, INFINITE);
        GetExitCodeThread(timerThread, &exitCode);
        CloseHandle(timerHandle);
        CloseHandle(startTimerEvent);
        CloseHandle(timerElapsedEvent);
        CloseHandle(timerThread);
        timerHandle = nullptr;
        timerThread = nullptr;
    }
}

bool BTimer::Stop()
{
    if (instance == nullptr)
    {
        return true;
    }

    BOOL ret = TRUE;

    periodic   = false;
    timerProc  = nullptr;
    timerParam = nullptr;

    if (timerHandle != nullptr)
    {
        ret = CancelWaitableTimer(timerHandle);

        if (!ret)
            DEBUGPRINT1("BTimer: CancelWaitableTimer failed (%lu)\n",
                        GetLastError());
    }

    return (ret != FALSE);
}

void BTimer::Suspend()
{
    DWORD ret;

    if (periodic && timerHandle != nullptr)
    {
        ret = WaitForSingleObject(timerElapsedEvent, INFINITE);

        if (ret == WAIT_FAILED)
            DEBUGPRINT1("BTimer: WaitForSingleObject failed (%lu)\n",
                        GetLastError());
    }
}

bool BTimer::StartTimer()
{
    if (timerHandle != nullptr && timerThread != nullptr)
    {
        LARGE_INTEGER aDueTime;
        LONG period = 0;

        aDueTime.QuadPart = -((LONGLONG)dueTime * 10);

        if (periodic)
        {
            period = (LONG)(dueTime / 1000);
        }

        SetEvent(startTimerEvent);

        if (SetWaitableTimer(timerHandle,
                             &aDueTime, period, nullptr, nullptr, false) == 0)
        {
            DEBUGPRINT1("BTimer: SetWaitableTimer failed (%lu)\n",
                        GetLastError());
            return false;
        }

        return true;
    }

    return false;
}

unsigned long BTimer::StartTimerImp(BTimer *p)
{
    if (p == nullptr || p->timerHandle == nullptr)
    {
        return 1;
    }

    DWORD ret;

    // increase Thread priority to avoid delays if timer elapses
    HANDLE hThread = (HANDLE)GetCurrentThread();
    SetThreadPriority(hThread, GetThreadPriority(hThread) + 1);

    do
    {
        WaitForSingleObject(p->startTimerEvent, INFINITE);

        do
        {
            ret = WaitForSingleObject(p->timerHandle, INFINITE);
            // Use separate event with manual reset for Suspend()
            // to wait until next timer tick
            SetEvent(p->timerElapsedEvent);

            if (ret == WAIT_FAILED)
            {
                DEBUGPRINT1("BTimer: WaitForSingleObject failed (%lu)\n",
                            GetLastError());
            }

            if (ret == WAIT_OBJECT_0 && p->timerProc != nullptr)
            {
                (*p->timerProc)(p->timerParam);
            }
        }
        while (p->periodic);
    }
    while (p->doFinish == false);

    return 0;
}
#endif
