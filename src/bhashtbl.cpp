/*
    bhashtbl.cpp


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

#include "bhashtbl.h"

// Each hash table entry can contain multiple entries with the same
// hash. They are stored in a linear list

template <class Item> BHashTable<Item>::BHashTable<Item>(int size /* = 1000 */) : n(size)
{
	int i;

	n = size;
	hash_table = new BList<Item> *[size];
	for (i = 0; i < size; i++)
		hash_table[i] = (BList<Item> *) NULL;
}

template <class Item> BHashTable<Item>::~BHashTable()
{
	if (!hash_table)
		return;
	for (int i = 0; i < n; i++)
		delete hash_table[i];
	delete[] hash_table;
	hash_table = NULL;
}

template <class Item>Item *BHashTable<Item>::Get(Item &i)
{
	DWord key;

	key = i.GetKey() % n;
	if (!hash_table[key])
		return NULL;
	else
		return hash_table[key]->Find(i);
}

template <class Item>void BHashTable<Item>::Put(Item &i)
{
	DWord key;

	key = i.GetKey() % n;
	if (!hash_table[key])
		hash_table[key] = new BList<Item>(i);
	else
		hash_table[key]->Add(i);
}

template <class Item>bool BHashTable<Item>::Contains(Item &i)
{
	return Get(i) != NULL;
}

/* the returned linear list is created on the heap and must be freed
   after use */
template <class Item>BList<Item> *BHashTable<Item>::GetAsLinearList()
{
	BList<Item> *first, *next;
	BList<Item> *source;

	for (int i = 0; i < size; i++) {
		source = hash_table[i];
		while(source) {
			// walk through linear list
			if (!first) {
				first = new BList<Item>(source->GetItem());
				next = first;
			} else {
				next->SetNext(new BList<Item>(source->GetItem()));
				next = next->GetNext();
			}

		}
	}
	return first;
}
