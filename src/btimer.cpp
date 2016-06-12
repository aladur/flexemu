/*
    btimer.cpp

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

#include <misc1.h>
#include <stdio.h>
#ifdef UNIX
    #include <signal.h>
    #include <errno.h>
#endif
#ifdef WIN32
    #include <process.h>
#endif

#include "btimer.h"
#include "bdelete.h"

#ifdef WIN32
    #ifndef _TTHREADPROC_DEFINED_
        typedef  unsigned long (__stdcall *tThreadProc)(void *);
        #define _TTHREADPROC_DEFINED_
    #endif
#endif

BTimer *BTimer::instance = NULL;

BTimer::BTimer() : dueTime(0), periodic(false),
    timerProc(NULL), timerParam(NULL)
#ifdef WIN32
    , timerHandle(NULL), startTimerEvent(NULL),
    timerElapsedEvent(NULL),
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
    instance  = NULL;
    timerProc = NULL;
}

BTimer *BTimer::Instance()
{
    if (instance == NULL)
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

void BTimer::SetTimerProc(BTimerProc x_timerProc, void *x_p /* = NULL*/)
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

bool BTimer::Stop(void)
{
    bool ret = true;
    struct itimerval timerValue;
    struct sigaction act;
    sigset_t sigmask;

    sigemptyset(&sigmask);
#ifdef SIGALRM
    sigaddset(&sigmask, SIGALRM);
    sigprocmask(SIG_BLOCK, &sigmask, NULL);

    periodic   = false;
    timerProc  = NULL;
    timerParam = NULL;

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

    if (sigaction(SIGALRM, &act, NULL) == -1)
    {
        fprintf(stderr, "error %d in sigaction\n", errno);
    }

#endif
    ret = (setitimer(ITIMER_REAL, &timerValue, NULL) == 0);
    return ret;
}

void BTimer::Suspend(void)
{
    sigset_t sigmask, oldmask;
    int sig;

    sigemptyset(&sigmask);
    sigaddset(&sigmask, SIGALRM);
    sigprocmask(SIG_BLOCK, &sigmask, &oldmask);
    sigwait(&sigmask, &sig);
    sigprocmask(SIG_SETMASK, &oldmask, NULL);
    // when catching signal with sigwait
    // the signal handler is not executed
    // automatically so call it here
    UnixTimerCallback(0);
}

bool BTimer::StartTimer(void)
{
    int ret;
    struct itimerval timerValue;
    struct sigaction act;
    sigset_t sigmask;

    sigemptyset(&sigmask);
#ifdef SIGALRM
    sigaddset(&sigmask, SIGALRM);
    sigprocmask(SIG_UNBLOCK, &sigmask, NULL);

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

    if (sigaction(SIGALRM, &act, NULL) == -1)
    {
        fprintf(stderr, "error %d in sigaction\n", errno);
    }

    ret = setitimer(ITIMER_REAL, &timerValue, NULL);

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
    if (instance != NULL)
    {
        instance->TimerElapsed();
    }
}

void BTimer::TimerElapsed()
{
    if (timerProc != NULL)
    {
        (*timerProc)(timerParam);
    }
}
#endif

///////////////////////////////////////////////////////////////
// BTimer Win32 specific implementation
///////////////////////////////////////////////////////////////

#ifdef WIN32
void BTimer::Init()
{
    DWORD threadId; // necessary for Win95/98/ME

    timerHandle = CreateWaitableTimer(NULL, FALSE, NULL);

    if (timerHandle == NULL)
        DEBUGPRINT1("BTimer: CreateWaitableTimer failed (%lu)\n",
                    GetLastError());

    startTimerEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (startTimerEvent == NULL)
        DEBUGPRINT1("BTimer: CreateEvent failed (%lu)\n",
                    GetLastError());

    timerElapsedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (startTimerEvent == NULL)
        DEBUGPRINT1("BTimer: CreateEvent failed (%lu)\n",
                    GetLastError());

    timerThread = CreateThread(NULL, 0,
                               (tThreadProc)BTimer::StartTimerImp, this, 0, &threadId);

    if (timerThread == NULL)
        DEBUGPRINT1("BTimer: CreateThread failed (%lu)\n",
                    GetLastError());
}

void BTimer::UnInit()
{
    if (timerHandle != NULL && timerThread != NULL)
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
        timerHandle = NULL;
        timerThread = NULL;
    }
}

bool BTimer::Stop(void)
{
    if (instance == NULL)
    {
        return true;
    }

    BOOL ret = TRUE;

    periodic   = false;
    timerProc  = NULL;
    timerParam = NULL;

    if (timerHandle != NULL)
    {
        ret = CancelWaitableTimer(timerHandle);

        if (!ret)
            DEBUGPRINT1("BTimer: CancelWaitableTimer failed (%lu)\n",
                        GetLastError());
    }

    return (ret != FALSE);
}

void BTimer::Suspend(void)
{
    DWORD ret;

    if (periodic && timerHandle != NULL)
    {
        ret = WaitForSingleObject(timerElapsedEvent, INFINITE);

        if (ret == WAIT_FAILED)
            DEBUGPRINT1("BTimer: WaitForSingleObject failed (%lu)\n",
                        GetLastError());
    }
}

bool BTimer::StartTimer()
{
    if (timerHandle != NULL && timerThread != NULL)
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
                             &aDueTime, period, NULL, NULL, false) == 0)
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
    if (p == NULL || p->timerHandle == NULL)
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
                DEBUGPRINT1("BTimer: WaitForSingleObject failed (%lu)\n", GetLastError());
            }

            if (ret == WAIT_OBJECT_0 && p->timerProc != NULL)
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
