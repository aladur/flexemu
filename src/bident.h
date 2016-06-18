/*
    bident.h

    Basic class containing an identifier. An integer value with an optional
    string representation.

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

#ifndef __bident_h__
#define __bident_h__

#include <stdlib.h>
#include "misc1.h"

class BIdentifier
{

private:

    DWord value;
    std::string name;

public:

    BIdentifier();
    BIdentifier(DWord value, const char *pName = NULL);
    BIdentifier(const BIdentifier &id);
    ~BIdentifier(); // public destructor

    DWord   GetValue();
    DWord   GetKey();   // to be hashable
    std::string GetName();
    void    SetTo(DWord value, const char *name = NULL);
    BIdentifier &operator = (const BIdentifier &id);
    bool    operator < (const BIdentifier &d) const;
    bool    operator == (const BIdentifier &d) const;
    bool    operator > (const BIdentifier &d) const;
    bool    operator >= (const BIdentifier &d) const;
    bool    operator <= (const BIdentifier &d) const;
}; // class BIdentifier

inline DWord BIdentifier::GetValue()
{
    return value;
}
inline DWord BIdentifier::GetKey()
{
    return value;
}
inline std::string BIdentifier::GetName()
{
    return name;
}
inline BIdentifier &BIdentifier::operator = (const BIdentifier &id)
{
    value = id.value;
    name = id.name;
    return *this;
}
inline bool BIdentifier::operator < (const BIdentifier &id) const
{
    return value <  id.value;
}
inline bool BIdentifier::operator == (const BIdentifier &id) const
{
    return value == id.value;
}
inline bool BIdentifier::operator > (const BIdentifier &id) const
{
    return value >  id.value;
}
inline bool BIdentifier::operator <= (const BIdentifier &id) const
{
    return value <= id.value;
}
inline bool BIdentifier::operator >= (const BIdentifier &id) const
{
    return value >= id.value;
}
inline void BIdentifier::SetTo(DWord aValue, const char *pName)
{
    value = aValue;
    name = pName;
}
#endif // #ifndef __bident_h__
