/*
    bhashtbl.h

    Basic template class for a hash table
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

#ifndef __bhashtbl_h__
#define __bhashtbl_h__

#include "typedefs.h"
template <class Item>class BList;
class BIdentifier;

template <class Item> class BHashTable {

private:

	BList<Item> **hash_table;
	int n;

public:

	BHashTable(int size = 1000);	// public constructor
	~BHashTable();			// public destructor

	Item *Get(Item &i);
	void Put(Item &i);
	bool Contains(Item &i);
	BList<Item> *GetAsLinearList();
}; // class BHashTable

typedef BHashTable<BIdentifier> BIdentifierHTable;

#endif // #ifndef __bhashtbl_h__
