/*
    bwin32threadimp.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2001-2004  W. Schwotzer

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

#ifdef WIN32
#include <misc1.h>

#ifdef _POSIX_
#undef _POSIX_
#endif

#include <process.h>
#include "bwin32threadimp.h"
#include "bthread.h"

#ifndef _TTHREADPROC_DEFINED_
typedef  unsigned long ( __stdcall *tThreadProc )( void * );
#define _TTHREADPROC_DEFINED_
#endif

BWin32ThreadImp::BWin32ThreadImp() : pThreadObj(NULL), finished(false),
	hThread(NULL), finishedEvent(NULL)
{
	finishedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
}

BWin32ThreadImp::~BWin32ThreadImp()
{
	if (finishedEvent != NULL)
		CloseHandle(finishedEvent);
	finishedEvent = NULL;
	if (hThread != NULL)
		CloseHandle(hThread);
	hThread = NULL;
}

void BWin32ThreadImp::Join()
{
	if (finishedEvent != NULL)
		WaitForSingleObject(finishedEvent, INFINITE);
}

bool BWin32ThreadImp::Start(BThread *aThreadObject)
{
  DWORD threadId; // necessary for Win95/98/ME

  pThreadObj = aThreadObject;
  hThread = CreateThread(NULL, 0,
		(tThreadProc)BWin32ThreadImp::RunImp, this, 0, &threadId);
  // return true if thread successfully has been created
  return (hThread != NULL);
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
  p->finished = false;
  if (p != NULL && p->pThreadObj != NULL)
    p->pThreadObj->Run();
  p->finished = true;
  if (p->finishedEvent != NULL)
	  SetEvent(p->finishedEvent);
  return 0;
}

#endif

