/*
    blist.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 2003-2004 W. Schwotzer

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

#include <string.h>  // needed for NULL
#include "blist.h"
#include "bintervl.h"


template <class Item>BList<Item>::BList(const BList &i) : next(NULL), previous(NULL), item(i.item)
{
}

template <class Item> BList<Item>::BList(const Item &i) : next(NULL), previous(NULL), item(i)
{
}

template <class Item>BList<Item>::~BList()
{
	delete next;
}

template <class Item>BList<Item>const *BList<Item>::Find(const Item &i) const
{
	const BList<Item> *l;

	l = this;
	do {
		if (l->item == i)
			return l;
		l = l->next;
	} while (l);
	return NULL;
}

template <class Item>BList<Item>const *BList<Item>::ReverseFind(const Item &i) const
{
	if (item == i)
		return this;
	if (previous)
		return previous->Find(i);
	return NULL;
}

template <class Item>bool BList<Item>::Contains(const Item &i) const
{
	return Find(i) != NULL;
}

template <class Item>bool BList<Item>::ReverseContains(const Item &i) const
{
	return ReverseFind(i) != NULL;
}

template <class Item>void BList<Item>::Unlink()
{
	if (previous)
		previous->next = next;
	if (next)
		next->previous = previous;
	previous = NULL;
	next = NULL;
}

// insert a new linked item before *this
template <class Item>void BList<Item>::Insert(const Item &i)
{
	BList *newLink;

	newLink = new BList<Item>(i);
	if (previous)
		previous->next = newLink;
	newLink->previous = previous;
	newLink->next = this;
	previous = newLink;
}

// append a new linked item after the last link in the list
template <class Item>void BList<Item>::Append(const Item &i)
{
	BList<Item> *link;

	link = GetLast();
	link->next = new BList<Item>(i);
	link->next->previous = link;
}

template <class Item>BList<Item> *BList<Item>::GetFirst()
{
	BList<Item> *link;

	link = this;
	while (link->previous)
		link = link->previous;
	return link;
}

template <class Item>BList<Item> *BList<Item>::GetLast()
{
	BList<Item> *link;

	link = this;
	while (link->next)
		link = link->next;
	return link;
}

// explicit instanciation of complete template class:

template class BList<BInterval>;
