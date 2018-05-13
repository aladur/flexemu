/*
    bsortlst.cpp


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

#include <string.h> // needed for declaration of NULL
#include "bsortlst.h"
#include "bbintree.h"
#include "bintervl.h"
#include "bident.h"
#include "buint.h"


template <class Item>BSortedList<Item>::BSortedList() : pRoot(NULL),
    pCurrentItem(NULL)
{
}

template <class Item>BSortedList<Item>::~BSortedList()
{
    delete pRoot;
}

template <class Item>Item BSortedList<Item>::GetFirst() const
{
    if (!pRoot)
    {
        pCurrentItem = NULL;
        return *((Item *)NULL);
    }
    else
    {
        pCurrentItem = pRoot->GetLeftMostItem();
        return pCurrentItem->GetItem();
    }
}

template <class Item>Item BSortedList<Item>::GetNext() const
{
    // unfinished
    if (pRoot)
    {
    }

    return pCurrentItem->GetItem();
}

template <class Item>void BSortedList<Item>::Add(const Item &i)
{
    if (!pRoot)
    {
        pRoot = new BBinaryTreeItem<Item>(i);
    }
    else
    {
        pRoot->Add(i);
    }
}

template <class Item>void BSortedList<Item>::Remove(const Item &i)
{
    // unfinished
    if (pRoot)
    {
        if (pRoot->GetItem() == i)
        {
        }
        else
        {
            pRoot->Remove(i);
        }
    }
}

template <class Item>void BSortedList<Item>::SetEmpty()
{
    delete pRoot;
    pRoot = NULL;
    pCurrentItem = NULL;
}

template <class Item>bool BSortedList<Item>::Contains(const Item &i) const
{
    if (!pRoot)
    {
        return false;
    }

    return pRoot->Contains(i);
}

// explicit instanciation of complete template class:

template class BSortedList<BInterval>;
template class BSortedList<BIdentifier>;
template class BSortedList<BUInt>;
