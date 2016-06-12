/*
    btimer.h

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
#ifdef WIN32
  //#define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#endif
#include "bmutex.h"

typedef void (*BTimerProc)(void *p);

class BTimer 
{
public:
	virtual ~BTimer();
	static BTimer *Instance();
	bool IsPeriodic() { return periodic; };
	QWord GetDueTime() { return dueTime; };
	bool Start(bool periodic, QWord dueTime);
	bool Stop(void);
	void Suspend(void); // suspend thread until timer elapses
	void SetTimerProc(BTimerProc, void *x_p = NULL); // async func after timer elapses	
protected:
	static BTimer *instance;
	QWord dueTime;
	bool periodic;
	BTimerProc timerProc;
	void *timerParam;
	void Init(void);
	void UnInit(void);
	bool StartTimer();

#ifdef WIN32
	static unsigned long StartTimerImp(BTimer *p);
	HANDLE   timerHandle;
	HANDLE	 startTimerEvent;
	HANDLE	 timerElapsedEvent;
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
