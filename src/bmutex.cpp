/*
    bmutex.cpp


    Basic class for access serialization between multiple threads
    Copyright (C) 2003-2004  W. Schwotzer

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

#include "bmutex.h"

#ifdef UNIX
BMutex::BMutex() : isLocked(false)
{
   pthread_mutex_init(&mutex, NULL);
}

BMutex::~BMutex()
{
   if (isLocked)
      unlock();
   pthread_mutex_destroy(&mutex);
}

void BMutex::lock()
{
   // prevent multiple locks
   if (!isLocked)
      pthread_mutex_lock(&mutex);
   isLocked = true;
}

void BMutex::unlock()
{
   if (isLocked)
      pthread_mutex_unlock(&mutex);
   isLocked = false;
}

bool BMutex::locked()
{
   return isLocked;
}

bool BMutex::tryLock()
{
   if (isLocked)
      return true;
   isLocked = (pthread_mutex_trylock(&mutex) == 0);
   return isLocked;
}
#endif

#ifdef WIN32
BMutex::BMutex() : isLocked(false), mutex(NULL)
{
	mutex = CreateMutex(NULL, FALSE, NULL);
}

BMutex::~BMutex()
{
	if (mutex != NULL)
	{
		unlock();
		CloseHandle(mutex);
	}
	mutex = NULL;
}


void BMutex::lock()
{
   WaitForSingleObject(mutex, INFINITE);
   isLocked = true;
}

void BMutex::unlock()
{
	ReleaseMutex(mutex);
	isLocked = false;
}

bool BMutex::locked()
{
   return isLocked;
}

bool BMutex::tryLock()
{
	// wait for ownership for max. 10 ms
	if (isLocked)
		return true;
	isLocked = (WaitForSingleObject(mutex, 10) == WAIT_OBJECT_0);
	return isLocked;
}
#endif

