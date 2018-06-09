/*
    bthread.cpp


    Basic class providing a platform independent thread implementation
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

#include "bthread.h"
#include "bthreadimp.h"
#include "bthreadfactory.h"
#include "misc1.h"

class BThreadImp;

BThread::BThread(bool autoStart /* = true */) : imp(nullptr)
{
    imp = SThreadFactory::Instance().CreateBThreadImp();

    if (imp != nullptr && autoStart)
    {
        imp->Start(this);
    }
}

// Attention: It is unsave to delete the thread object when the
// corresponding thread still is active (IsFinished() returns false)
BThread::~BThread()
{
    delete imp;
}

bool BThread::Start()
{
    if (imp == nullptr)
    {
        return false;
    }

    return imp->Start(this);
}

void BThread::Join()
{
    if (imp != nullptr)
    {
        imp->Join();
    }
}

bool BThread::IsFinished()
{
    if (imp == nullptr)
    {
        return false;
    }

    return imp->IsFinished();
}

void BThread::Run()
{
    // to be implemented by subclass
}

// Attention: This will finish the current process.
// This function does not return
// because it's thread gets closed
// Can be used within the Run() function to finish
// the current thread
void BThread::Exit(void *retval)
{
    if (imp == nullptr)
    {
        return;
    }

    imp->Exit(retval);
}
