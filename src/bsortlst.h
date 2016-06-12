/*
    bsortlst.h

    Basic class containing a sorted list of any type

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

#ifndef __bsortlst_h__
#define __bsortlst_h__

#include "typedefs.h"

template <class Item> class BBinaryTreeItem;
class BIdentifier;
class BUInt;


template <class Item> class BSortedList {

public:
	BSortedList();
	virtual ~BSortedList();

	Item GetFirst() const;
	Item GetNext() const;
	void Add(const Item &i);
	void Remove(const Item &i);
	void SetEmpty();
	bool Contains(const Item &i) const;
private:
	BBinaryTreeItem<Item> *pRoot;
	mutable const BBinaryTreeItem<Item> *pCurrentItem; // current binary tree item for GetFirst/GetNext
}; // class BSortedList

typedef BSortedList<BIdentifier> BIdentifierSList;
typedef BSortedList<BUInt> BUIntSList;

#endif // #ifndef __bsortlst_h__
