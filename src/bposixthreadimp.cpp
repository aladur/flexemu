/*
    bposixthreadimp.cpp


    Basic class for a posix thread implementation
    Copyright (C) 2001-2005  W. Schwotzer

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

#include "bposixthreadimp.h"

#ifdef HAVE_PTHREAD
#include <bthread.h>

typedef  void *(*tThreadProc)(void *);

BPosixThreadImp::BPosixThreadImp() : pThreadObj(NULL), finished(false), thread(0)
{
}

BPosixThreadImp::~BPosixThreadImp()
{
}

bool BPosixThreadImp::Start(BThread *aThreadObject)
{
  // pthread_create returns 0 if thread is successfully created
  // the thread identifier is stored in the 'thread'
  pthread_attr_t attr;
  int ret;

  ret = pthread_attr_init (&attr);
  //ret =  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
  pThreadObj = aThreadObject;
  ret = pthread_create(&thread, &attr, (tThreadProc)BPosixThreadImp::RunImp, this);
  if (ret == 0)
    // detach thread to avoid memory leaks
    pthread_detach(thread);

  return (ret == 0);
}

// Another thread can wait, until thread with id "thread" has terminated
void BPosixThreadImp::Join()
{
   if (!finished)
      pthread_join(thread, NULL);
}

bool BPosixThreadImp::IsFinished()
{
  return finished;
}

void BPosixThreadImp::Exit(void *retval)
{
  // Attention: this function call will never return!
  finished = true;
  pthread_exit(retval);  
}

// this static function is the thread procedure to
// be called with Start(...)
void *BPosixThreadImp::RunImp(BPosixThreadImp *p)
{
  void *result = NULL;

  p->finished = false;
  if (p != NULL && p->pThreadObj != NULL)
    p->pThreadObj->Run();
  p->finished = true;
  return result;
}
#endif

