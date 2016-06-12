/*
    btime.cpp


    Basic class for platform independent high resolution time support
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

#ifdef WIN32
#include "misc1.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include "btime.h"

// This class could also be realized as a envelope/letter pattern
// but this is still the most efficient C++ implementation

BTime::BTime() : lapTime(0)
{
}

BTime::~BTime()
{
}

void BTime::ResetRelativeTime()
{
  lapTime = GetTimeUsll();
}

unsigned long BTime::GetRelativeTimeMsl(bool reset /*= false*/)
{
  return (unsigned long)(GetRelativeTimeUsll(reset) / 1000);
}

unsigned long BTime::GetTimeMsl()
{
  return (unsigned long)(GetTimeUsll() / 1000);
}


#ifdef UNIX
QWord BTime::GetRelativeTimeUsll(bool reset /*= false*/)
{
  QWord currentTime = GetTimeUsll();
  QWord result = currentTime - lapTime;
  if (reset) lapTime = currentTime;

  return result;
}

double BTime::GetRelativeTimeUsf(bool  reset /*= false*/)
{
  QWord currentTime = GetTimeUsll();
  double result = (double)(currentTime - lapTime);
  if (reset) lapTime = currentTime;

  return result;
}

// return time in us as a unsigned int 64 Bit value
QWord BTime::GetTimeUsll()
{
  struct timeval tv;

  gettimeofday(&tv, NULL);
  return ((QWord)tv.tv_sec * 1000000 + tv.tv_usec);
}

// return time in us as a double value
double BTime::GetTimeUsf()
{
  struct timeval tv;

  gettimeofday(&tv, NULL);
  return ((double)tv.tv_sec * 1000000.0 + tv.tv_usec);
}
#endif

#ifdef WIN32
QWord BTime::GetRelativeTimeUsll(bool reset /*= false*/)
{
  QWord currentTime = GetTimeUsll();
  QWord result = currentTime - lapTime;
  if (reset) lapTime = currentTime;

  return result;
}

double BTime::GetRelativeTimeUsf(bool  reset /*= false*/)
{
  QWord currentTime = GetTimeUsll();
  double result = (double)(SQWord)(currentTime - lapTime);
  if (reset) lapTime = currentTime;

  return result;
}

// return time in us as a unsigned int 64 Bit value
QWord BTime::GetTimeUsll()
{
  LARGE_INTEGER count, freq;

  if (QueryPerformanceCounter(&count)) {
    QueryPerformanceFrequency(&freq);
    return (QWord)count.QuadPart * 1000000 / (QWord)freq.QuadPart;
  } else
    return 0;

}

// return time in us as a double value
double BTime::GetTimeUsf()
{
  LARGE_INTEGER count, freq;

  if (QueryPerformanceCounter(&count)) {
    QueryPerformanceFrequency(&freq);
    return (double)count.QuadPart * 1000000 / (double)freq.QuadPart;
  } else
    return 0.0;

}
#endif
