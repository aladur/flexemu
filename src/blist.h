/*
    blist.h

    Basic class representing an element of a linear list
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

#ifndef __blist_h__
#define __blist_h__


template <class Item> class BList {

private:

	BList *next;
	BList *previous;
	Item  item;

public:

	BList(const BList &i);		// public constructor
	BList(const Item &i);		// public constructor
	virtual ~BList();			// public destructor

	BList *GetFirst();
	BList *GetLast();
	BList *GetNext() const;
	void SetNext(BList *next);
	void SetPrevious(BList *previous);
	BList *GetPrevious() const;
	Item &GetItem();
	const BList *Find(const Item &i) const;
	const BList *ReverseFind(const Item &i) const;
	bool Contains(const Item &i) const;
	bool ReverseContains(const Item &i) const;
	void Insert(const Item &i);// Insert a new item before the current item
	void Append(const Item &i);	// Add a new item at the end of the lin. list
	void Unlink();
}; // class BList

template <class Item>inline BList<Item> *BList<Item>::GetNext() const { return next; };
template <class Item>inline BList<Item> *BList<Item>::GetPrevious() const { return previous; };
template <class Item>inline Item &BList<Item>::GetItem() { return item; };
template <class Item>inline void BList<Item>::SetNext(BList *l) { next = l; };
template <class Item>inline void BList<Item>::SetPrevious(BList *l) { previous = l; };

#endif // #ifndef __blist_h__
