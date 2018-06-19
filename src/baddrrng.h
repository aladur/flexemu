/*
    baddrrng.h

    Basic class for management of address ranges
    Copyright (C) 2003-2018  W. Schwotzer

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

#ifndef BADDRRNG_INCLUDED
#define BADDRRNG_INCLUDED

#include <stdio.h>
#include "blist.h"
#include "bintervl.h"

typedef BList<BInterval> BIntervalItem;

class FlexFileBuffer;


class BAddressRanges
{

private:
    DWord startAddress;
    BIntervalItem *root;
    mutable BIntervalItem *current;
    void Reduce();
    void BasicAdd(const BInterval &i);

public:
    BAddressRanges();   // public constructor
    ~BAddressRanges();  // public destructor

    void Clear();
    DWord GetMin() const;
    DWord GetMax() const;
    void SetStartAddress(DWord address);
    DWord GetStartAddress() const;
    void Add(const BInterval &i);
    void Add(const BAddressRanges &r);
    void Remove(const BInterval &i);
    const BInterval *GetFirst() const;
    const BInterval *GetNext() const;
    bool Contains(DWord address) const;
    bool Contains(const BInterval &i) const;
    bool Overlaps(const BInterval &i) const;
    bool ReadFrom(FlexFileBuffer *buffer);
    void PrintOn(FILE *fp) const;
}; // class BAddressRanges

inline void BAddressRanges::SetStartAddress(DWord address)
{
    startAddress = address;
}
inline DWord BAddressRanges::GetStartAddress() const
{
    return startAddress;
}

#endif // BADDRRNG_INCLUDED
