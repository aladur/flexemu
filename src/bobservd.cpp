/*
    bobservd.cpp


    Basic abstract class used to implement an observer pattern
    Copyright (C) 2020-2022  W. Schwotzer

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

#include "bobservd.h"
#include "bobserv.h"
#include <algorithm>

void BObserved::Attach(BObserver &x_observer)
{
    if (std::find_if(observers.cbegin(), observers.cend(), 
        [&x_observer](std::reference_wrapper<BObserver> observerRef)
        {
            return &observerRef.get() == &x_observer;
        }) == observers.cend())
    {
        observers.push_back(std::ref(x_observer));
    }
}

void BObserved::Detach(BObserver &x_observer)
{
    const auto iter =
        std::find_if(observers.cbegin(), observers.cend(), 
            [&x_observer](std::reference_wrapper<BObserver> observerRef)
            {
                return &observerRef.get() == &x_observer;
            });

    if (iter != observers.cend())
    {
        observers.erase(iter);
    }
}

void BObserved::Notify(NotifyId id, void *param)
{
    for (auto &observer : observers)
    {
        observer.get().UpdateFrom(id, param);
    }
}

