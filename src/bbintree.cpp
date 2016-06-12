/*
    bbintree.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2004 W. Schwotzer

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
#include "bbintree.h"
#include "bintervl.h"
#include "bident.h"
#include "buint.h"

template <class Item> BBinaryTreeItem<Item>::BBinaryTreeItem(
    const Item &anItem) : item(anItem), left(NULL), right(NULL)
{
}

template <class Item> BBinaryTreeItem<Item>::~BBinaryTreeItem()
{
    delete left;
    delete right;
}

/* not implemented recusive to save stack size */
template <class Item> void BBinaryTreeItem<Item>::RemoveLeftItem()
{
    BBinaryTreeItem<Item> *rightItem, *leftItem, *next;

    leftItem = left;

    if (left)
    {
        left = leftItem->left;
        next = this;
        rightItem = leftItem->right;

        if (rightItem)
        {
            bool done = false;

            do
            {
                if (rightItem->item > next->item)
                {
                    if (!next->right)
                    {
                        next->right = rightItem;
                        done = true;
                    }
                    else
                    {
                        next = next->right;
                    }
                }
                else
                {
                    if (!next->left)
                    {
                        next->left = rightItem;
                        done = true;
                    }
                    else
                    {
                        next = next->left;
                    }
                }
            }
            while (!done);
        }
    }

    leftItem->left  = NULL;
    leftItem->right = NULL;
    delete leftItem;
}

template <class Item> void BBinaryTreeItem<Item>::RemoveRightItem()
{
    BBinaryTreeItem<Item> *rightItem, *leftItem, *next;

    rightItem = right;

    if (right)
    {
        right = rightItem->right;
        next = this;
        leftItem = rightItem->left;

        if (leftItem)
        {
            bool done = false;

            do
            {
                if (leftItem->item < next->item)
                {
                    if (!next->left)
                    {
                        next->left = leftItem;
                        done = true;
                    }
                    else
                    {
                        next = next->left;
                    }
                }
                else
                {
                    if (!next->right)
                    {
                        next->right = leftItem;
                        done = true;
                    }
                    else
                    {
                        next = next->right;
                    }
                }
            }
            while (!done);
        }
    }

    rightItem->left  = NULL;
    rightItem->right = NULL;
    delete rightItem;
}

template <class Item> bool BBinaryTreeItem<Item>::Contains(const Item &i) const
{
    if (i == item)
    {
        return true;
    }

    if (left && left->Contains(i)) // recursive
    {
        return true;
    }

    if (right && right->Contains(i)) // recursive
    {
        return true;
    }

    return false;
}

template <class Item> const BBinaryTreeItem<Item>
*BBinaryTreeItem<Item>::GetLeftMostItem() const
{
    const BBinaryTreeItem<Item> *pTreeItem;

    pTreeItem = this;

    while (pTreeItem->left)
    {
        pTreeItem = pTreeItem->left;
    }

    return pTreeItem;
}

template <class Item> void BBinaryTreeItem<Item>::Add(const Item &)
{
    // unfinished
}

template <class Item> bool BBinaryTreeItem<Item>::Remove(const Item &i)
{
    const BBinaryTreeItem<Item> *pTreeItem;

    // unfinished
    pTreeItem = this;

    if (pTreeItem->left && (pTreeItem->left->GetItem() == i))
    {
        pTreeItem = pTreeItem->left;
    }

    //return pTreeItem;
    return true;
}

// explicit instanciation of complete template class:

template class BBinaryTreeItem<BInterval>;
template class BBinaryTreeItem<BIdentifier>;
template class BBinaryTreeItem<BUInt>;
