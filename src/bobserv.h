/*
    bobserv.h


    Basic abstract class used to implement an observer pattern
    Copyright (C) 1999-2021  W. Schwotzer

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

#ifndef _BOBSERV_INCLUDED_
#define _BOBSERV_INCLUDED_

#include "bobshelp.h"

/*------------------------------------------------------
 BObserver
 An virtual observer class used to observe any action.
 Only the class implementing the BObserver interface
 knows what to do with pObject
--------------------------------------------------------*/
class BObserver
{
public:
    virtual void UpdateFrom(NotifyId id, void *param = nullptr) = 0;
    virtual ~BObserver() { };
};

#endif // #ifndef _BOBSERV_INCLUDED_

