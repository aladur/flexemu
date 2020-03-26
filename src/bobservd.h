/*
    bobservd.h


    Basic abstract class used to implement an observer pattern
    Copyright (C) 2020  W. Schwotzer

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

#ifndef _BOBSERVD_INCLUDED_
#define _BOBSERVD_INCLUDED_

#include <vector>
#include <functional>
#include "bobshelp.h"
#include "bobserv.h"

/*------------------------------------------------------
 BObserved
 An observed class used by an Observer interface
 to be observed.
--------------------------------------------------------*/

class BObserved
{
public:
    BObserved() = default;
    virtual ~BObserved() = default;

    void Attach(BObserver &x_observer);
    void Detach(BObserver &x_observer);

protected:
    void Notify(NotifyId id, void *param = nullptr);

private:
    std::vector<std::reference_wrapper<BObserver> > observers;
};

#endif // #ifndef _BOBSERVD_INCLUDED_

