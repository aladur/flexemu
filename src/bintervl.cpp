/*
    binterval.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 2003-2018 W. Schwotzer

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

#include "bintervl.h"


BInterval::BInterval(const BInterval &i)
{
    minimum = i.minimum;
    maximum = i.maximum;
}

BInterval::BInterval() : minimum(0), maximum(0)
{
}

BInterval::~BInterval()
{
}

bool BInterval::ExtendBy(const BInterval &i)
{
    if (maximum + 1 < i.minimum || minimum > i.maximum + 1)
        // two separate intervals can't be joined together
    {
        return false;
    }

    minimum = minimum <= i.minimum ? minimum : i.minimum;
    maximum = maximum >= i.maximum ? maximum : i.maximum;
    return true;
}

bool BInterval::Exclude(const BInterval &i)
{
    if (!Overlaps(i) ||
        (Contains(i) && (minimum != i.minimum || maximum != i.maximum)))
        // nothing to exclude or exclusion would result in two intervals
    {
        return false;
    }

    if (maximum > i.minimum)
    {
        maximum = i.minimum - 1;
    }
    else if (minimum < i.maximum)
    {
        minimum = i.maximum + 1;
    }

    return true;
}

bool BInterval::Touches(const BInterval &i) const
{
    return ((maximum + 1 == i.minimum && minimum < i.maximum) ||
            (minimum == i.maximum + 1 && maximum > i.minimum));
}

bool BInterval::Overlaps(const BInterval &i) const
{
    return ((maximum >= i.minimum && minimum < i.maximum) ||
            (minimum <= i.maximum && maximum > i.minimum));
}
