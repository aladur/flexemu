/*
    bintervl.h

    Basic class representing an interval with lower and upper bound
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

#ifndef __bintervl_h__
#define __bintervl_h__

#include "typedefs.h"


class BInterval
{

private:
    DWord minimum;
    DWord maximum;

public:

    BInterval(DWord aMinimum, DWord aMaximum);  // public constructor
    BInterval();                    // public constructor
    BInterval(const BInterval &i);  // public constructor
    ~BInterval();           // public destructor

    DWord GetMin() const;
    DWord GetMax() const;
    DWord GetLength() const;
    void SetMin(DWord aMinimum);
    void SetMax(DWord aMaximum);
    void SetMinMax(DWord aMinimum, DWord aMaximum);
    void SetLength(DWord aLength);
    BInterval &operator = (const BInterval &i);
    bool operator == (const BInterval &i) const;
    bool operator <= (const BInterval &i) const;
    bool operator < (const BInterval &i) const;
    bool operator >= (const BInterval &i) const;
    bool operator > (const BInterval &i) const;
    bool operator != (const BInterval &i) const;
    bool Contains(const BInterval &i) const;
    bool Contains(DWord i) const;
    bool Overlaps(const BInterval &i) const;
    bool Touches(const BInterval &i) const; // two intervals which don't overlap
    // but can be joined to one interval
    bool ExtendBy(const BInterval &i);
    bool Exclude(const BInterval &i);
}; // class BInterval

inline BInterval::BInterval(DWord aMinimum, DWord aMaximum) : minimum(aMinimum),
    maximum(aMaximum) { }
inline DWord BInterval::GetMin()  const
{
    return minimum;
}
inline DWord BInterval::GetMax() const
{
    return maximum;
}
inline DWord BInterval::GetLength() const
{
    return maximum - minimum + 1;
}
inline void BInterval::SetMin(DWord aMinimum)
{
    minimum = aMinimum;
}
inline void BInterval::SetMax(DWord aMaximum)
{
    maximum = aMaximum;
}
inline void BInterval::SetMinMax(DWord aMinimum, DWord aMaximum)
{
    minimum = aMinimum;
    maximum = aMaximum;
}
inline void BInterval::SetLength(DWord aLength)
{
    maximum = minimum + aLength - 1;
}
inline BInterval &BInterval::operator = (const BInterval &i)
{
    minimum = i.minimum;
    maximum = i.maximum;
    return *this;
}
inline bool BInterval::Contains(DWord i) const
{
    return (i >= minimum && i <= maximum);
}
inline bool BInterval::Contains(const BInterval &i)     const
{
    return (i.minimum >= minimum && i.maximum <= maximum);
}
inline bool BInterval::operator == (const BInterval &i) const
{
    return (minimum == i.minimum && maximum == i.maximum);
}
inline bool BInterval::operator != (const BInterval &i) const
{
    return (minimum != i.minimum || maximum != i.maximum);
}
// comparison operators only compare lower bound
inline bool BInterval::operator <= (const BInterval &i) const
{
    return minimum <= i.minimum;
}
inline bool BInterval::operator < (const BInterval &i) const
{
    return minimum <  i.minimum;
}
inline bool BInterval::operator >= (const BInterval &i) const
{
    return minimum >= i.minimum;
}
inline bool BInterval::operator > (const BInterval &i) const
{
    return minimum >  i.minimum;
}
#endif // #ifndef __bintervl_h__
