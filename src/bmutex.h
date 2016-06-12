/*
    bmutex.h


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

#ifndef __bmutex_h__
#define __bmutex_h__

// This class could also be realized as a envelope/letter pattern
// but this is still the most efficient C++ implementation

#include <misc1.h>
#ifdef UNIX
#include <pthread.h>
#endif
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif


class BMutex {
public:
   BMutex();
   virtual ~BMutex();

   void lock();
   void unlock();
   bool locked();
   bool tryLock();
private:
   bool isLocked;

#ifdef UNIX
private:
   pthread_mutex_t mutex;
#endif
#ifdef WIN32
   HANDLE mutex;
#endif
};

#endif // #ifndef __bmutex_h__

