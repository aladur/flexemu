/*
    baddrrng.cpp


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

#include "misc1.h"
#include "baddrrng.h"
#include "ffilebuf.h"


BAddressRanges::BAddressRanges() : startAddress(0),
    root(nullptr), current(nullptr)
{
}

BAddressRanges::~BAddressRanges()
{
    Clear();
}

void BAddressRanges::Clear()
{
    delete root;
    root = nullptr;
    current = nullptr;
    startAddress = 0;
}

void BAddressRanges::Add(const BInterval &i)
{
    BasicAdd(i);
    Reduce();
}

void BAddressRanges::Add(const BAddressRanges &r)
{
    const BInterval *pi;

    pi = r.GetFirst();

    while (pi)
    {
        BasicAdd(*pi);
        pi = r.GetNext();
    }

    Reduce();
}

// just add an entry into linked list
// don't reduce
void BAddressRanges::BasicAdd(const BInterval &i)
{
    if (!root)
    {
        root = new BIntervalItem(i);
    }
    else
    {
        BIntervalItem *pItem;

        pItem = root;

        while (pItem)
        {
            if (i < pItem->GetItem())
            {
                pItem->Insert(i);
                root = pItem->GetFirst();
                return;
            }

            pItem = pItem->GetNext();
        }

        // append it at the end of the list
        root->Append(i);
    }
}

// reduce address ranges to a minimum needed
// assuming that address ranges are sorted by their start address
void BAddressRanges::Reduce()
{
    BIntervalItem *firstItem;
    BIntervalItem *nextItem;

    firstItem = root;

    while (firstItem)
    {
        nextItem = firstItem->GetNext();

        while (nextItem)
        {
            bool toBeDeleted = false;

            if (firstItem->GetItem().Contains(nextItem->GetItem()))
            {
                toBeDeleted = true;
            }
            else if (firstItem->GetItem().Overlaps(nextItem->GetItem()) ||
                     (firstItem->GetItem().Touches(nextItem->GetItem())))
            {
                firstItem->GetItem().ExtendBy(nextItem->GetItem());
                toBeDeleted = true;
            }

            if (toBeDeleted)
            {
                BIntervalItem *link = nextItem;
                nextItem = nextItem->GetNext();
                link->Unlink();
                delete link;
            }
            else
            {
                nextItem = nextItem->GetNext();
            }
        }

        firstItem = firstItem->GetNext();
    }
}

void BAddressRanges::Remove(const BInterval &)
{
    // unfinished
}

const BInterval *BAddressRanges::GetFirst() const
{
    if (root)
    {
        current = root;
        return &current->GetItem();
    }
    else
    {
        return nullptr;
    }
}

const BInterval *BAddressRanges::GetNext() const
{
    if (current && (current = current->GetNext()))
    {
        return &current->GetItem();
    }
    else
    {
        return nullptr;
    }
}

bool BAddressRanges::Contains(DWord address) const
{
    const BInterval *interval;

    interval = GetFirst();

    while (interval)
    {
        if (interval->Contains(address))
        {
            return true;
        }

        interval = GetNext();
    }

    return false;
}

bool BAddressRanges::Contains(const BInterval &i) const
{
    const BInterval *interval;

    interval = GetFirst();

    while (interval)
    {
        if (interval->Contains(i))
        {
            return true;
        }

        interval = GetNext();
    }

    return false;
}

bool BAddressRanges::Overlaps(const BInterval &i) const
{
    const BInterval *interval;

    interval = GetFirst();

    while (interval)
    {
        if (interval->Overlaps(i))
        {
            return true;
        }

        interval = GetNext();
    }

    return false;
}

void BAddressRanges::PrintOn(FILE *fp) const
{
    const BInterval *interval;

    fprintf(fp, "Startaddress: %x\n", startAddress);
    fprintf(fp, "Ranges:\n");
    interval = GetFirst();

    while (interval)
    {
        fprintf(fp, "   %x-%x\n", interval->GetMin(), interval->GetMax());
        interval = GetNext();
    }

}

DWord BAddressRanges::GetMin() const
{
    if (!root)
    {
        return 0;
    }

    return root->GetItem().GetMin();
}

DWord BAddressRanges::GetMax() const
{
    if (!root)
    {
        return 0;
    }

    return root->GetLast()->GetItem().GetMax();
}

bool BAddressRanges::ReadFrom(FlexFileBuffer *buffer)
{
    const Byte *p;
    DWord address;
    DWord length;
    BInterval interval;

    if (!buffer)
    {
        return false;
    }

    p = buffer->GetBuffer();

    while ((buffer->GetSize() - (p - buffer->GetBuffer())) >= 3)
    {
        switch (*(p++))
        {
            case 0x02: // memory contents
                if (buffer->GetSize() - (p - buffer->GetBuffer()) < 3)
                {
                    return false;
                }

                address = *p << 8 | *(p + 1);
                p += 2;
                length = *(p++);
                p += length;
                interval.SetMinMax(address, address + length - 1);
                Add(interval);
                break;

            case 0x16: // start address
                address = *p << 8 | *(p + 1);
                p += 2;
                SetStartAddress(address);
                break;

            case 0x00: // continue with next sector
                length = 252 - ((p - buffer->GetBuffer()) % 252);
                p += length;
                break;

            default:
                return false;
        }
    }

    return true;
}

