/*
    bthreadimp.h


    Basic class defining a platform independent thread interface
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

#ifndef BTHREADIMP_H
#define BTHREADIMP_H

#include "misc1.h"
#include "bthread.h"

// This class describes a platform independant Thread interface
// According to the Bridge or Body/Handle Pattern

class BThreadImp
{
public:
	BThreadImp();
	virtual ~BThreadImp();
  virtual bool Start(BThread *pThread) = 0;
  virtual void Join() = 0;
  virtual bool IsFinished() = 0;
  virtual void Exit(void *retval = NULL) = 0;
};

#endif
