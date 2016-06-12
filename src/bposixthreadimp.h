/*
    bposixthreadimp.h


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

#ifndef BPOSIXTHREADIMP_H
#define BPOSIXTHREADIMP_H

#include <misc1.h>
#ifdef HAVE_PTHREAD
#include <pthread.h>
#else
#error libpthread or header file pthread.h is missing
#endif
#include "bthreadimp.h"


class BThread;

class BPosixThreadImp : public BThreadImp
{
public:
	BPosixThreadImp();
	virtual ~BPosixThreadImp();
  bool Start(BThread *pThread);
  void Join();
  bool IsFinished();
  void Exit(void *retval = NULL);
private:
  static void *RunImp(BPosixThreadImp *p);
  BThread *pThreadObj;
  bool finished;
  pthread_t thread;
};

#endif

