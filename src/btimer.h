/*
    btimer.h

    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2021  W. Schwotzer

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

#ifndef BTIMER_INCLUDED
#define BTIMER_INCLUDED

#include "misc1.h"

typedef void (*BTimerProc)(void *p);

class BTimer
{
public:
    ~BTimer();
    static BTimer *Instance();
    static void Destroy();
    bool IsPeriodic()
    {
        return periodic;
    };
    QWord GetDueTime()
    {
        return dueTime;
    };
    bool Start(bool periodic, QWord dueTime);
    bool Stop();
    void Suspend(); // suspend thread until timer elapses
    void SetTimerProc(BTimerProc,
                      void *x_p = nullptr); // async func after timer elapses
protected:
    static BTimer *instance;
    QWord dueTime;
    bool periodic;
    BTimerProc timerProc;
    void *timerParam;
    void Init();
    void UnInit();
    bool StartTimer();

#ifdef _WIN32
    static unsigned long StartTimerImp(BTimer *p);
    HANDLE   timerHandle;
    HANDLE   startTimerEvent;
    HANDLE   timerElapsedEvent;
    HANDLE   timerThread;
    bool     doFinish;
#endif
#ifdef UNIX
    static RETSIGTYPE UnixTimerCallback(int i);
    void TimerElapsed();
#endif

private:
    BTimer(); // protect default constructor from being used
};

#endif // BTIMER_INCLUDED

