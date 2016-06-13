/*
    bmutex.cpp


    Basic class to define mutual exclusive sections to be used
    from multiple threads
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
BMutex::BMutex()
{
    pthread_mutex_init(&mutex, NULL);
}

BMutex::~BMutex()
{
    // assuming the mutex is not locked any more!
    pthread_mutex_destroy(&mutex);
}

void BMutex::lock()
{
    pthread_mutex_lock(&mutex);
}

void BMutex::unlock()
{
    pthread_mutex_unlock(&mutex);
}
#endif

#ifdef WIN32
BMutex::BMutex() : mutex(NULL)
{
    InitializeCriticalSection(&criticalSec);
}

BMutex::~BMutex()
{
    DeleteCriticalSection(&criticalSec);
}


void BMutex::lock()
{
    EnterCriticalSection(&criticalSec);
}

void BMutex::unlock()
{
    LeaveCriticalSection(&criticalSec);
}
#endif

