/*
    bthread.h


    Basic class providing a platform independent thread implementation
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

#ifndef BTHREAD_H
#define BTHREAD_H

#include "misc1.h"

// This class describes a platform independant Thread interface
// According to the Bridge or Body/Handle Pattern

class BThreadImp;

class BThread {

public: 
	BThread(bool autoStart = true);
	virtual ~BThread();
  bool Start();     // Explicitly start the thread if not started yet
  void Join();      // Wait from another thread until this thread has terminated
  bool IsFinished(); // Check if thread has finished
  virtual void Run();  // entry point when starting the thread. To be reimplemented

protected:
  void Exit(void *retval = NULL);
  BThreadImp *imp;
};

#endif
