/*
    bwin32threadimp.cpp


    Basic class for a Win32 thread implementation
    Copyright (C) 2001-2018  W. Schwotzer

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

#ifdef _WIN32
#include "misc1.h"

#ifdef _POSIX_
    #undef _POSIX_
#endif

#include <process.h>
#include "bwin32threadimp.h"
#include "bthread.h"

#ifndef _TTHREADPROC_DEFINED_
    typedef  unsigned long (__stdcall *tThreadProc)(void *);
    #define _TTHREADPROC_DEFINED_
#endif

BWin32ThreadImp::BWin32ThreadImp() : pThreadObj(nullptr), finished(false),
    hThread(nullptr), finishedEvent(nullptr)
{
    finishedEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
}

BWin32ThreadImp::~BWin32ThreadImp()
{
    if (finishedEvent != nullptr)
    {
        CloseHandle(finishedEvent);
    }

    finishedEvent = nullptr;

    if (hThread != nullptr)
    {
        CloseHandle(hThread);
    }

    hThread = nullptr;
}

void BWin32ThreadImp::Join()
{
    if (finishedEvent != nullptr)
    {
        WaitForSingleObject(finishedEvent, INFINITE);
    }
}

bool BWin32ThreadImp::Start(BThread *aThreadObject)
{
    DWORD threadId; // necessary for Win95/98/ME

    pThreadObj = aThreadObject;
    hThread = CreateThread(nullptr, 0, (tThreadProc)BWin32ThreadImp::RunImp,
                           this, 0, &threadId);
    // return true if thread successfully has been created
    return (hThread != nullptr);
}

bool BWin32ThreadImp::IsFinished()
{
    return finished;
}

void BWin32ThreadImp::Exit(void *)
{
    // Attention: this function call will never return!
    finished = true;
    ExitThread(0);
}

// this static function is the thread procedure to
// be called with Start(...)
unsigned int BWin32ThreadImp::RunImp(BWin32ThreadImp *p)
{
    if (p != nullptr)
    {
        p->finished = false;

        if (p->pThreadObj != nullptr)
        {
            p->pThreadObj->Run();
        }

        p->finished = true;

        if (p->finishedEvent != nullptr)
        {
            SetEvent(p->finishedEvent);
        }
    }

    return 0;
}

#endif

