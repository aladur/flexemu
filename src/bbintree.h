/*
    bbintree.h

    Basic template class for a binary tree item
    Copyright (C) 1999-2004  W. Schwotzer

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

#ifndef __bbintree_h__
#define __bbintree_h__


template <class Item> class BBinaryTreeItem {

private:

	Item			item;
	BBinaryTreeItem	*left;
	BBinaryTreeItem	*right;

public:

	BBinaryTreeItem(const Item &anItem);		// public constructor
	virtual ~BBinaryTreeItem();			// public destructor

	const Item &GetItem() const;
	const BBinaryTreeItem *GetLeftMostItem() const;
	const BBinaryTreeItem *GetLeftItem() const;
	const BBinaryTreeItem *GetRightItem() const;
	void RemoveLeftItem();
	void RemoveRightItem();
	void Add(const Item &i);
	bool Remove(const Item &i);
	bool Contains(const Item &i) const;
	bool	operator <  (const BBinaryTreeItem& i) const;
	bool	operator == (const BBinaryTreeItem& i) const;
	bool	operator >  (const BBinaryTreeItem& i) const;
	bool	operator >= (const BBinaryTreeItem& i) const;
	bool	operator <= (const BBinaryTreeItem& i) const;
}; // class BBinaryTreeItem

template <class Item>inline const Item &BBinaryTreeItem<Item>::GetItem() const { return item; };
template <class Item>inline const BBinaryTreeItem<Item> *BBinaryTreeItem<Item>::GetLeftItem()  const { return left;  };
template <class Item>inline const BBinaryTreeItem<Item> *BBinaryTreeItem<Item>::GetRightItem() const { return right; };

template <class Item>inline bool BBinaryTreeItem<Item>::operator <  (const BBinaryTreeItem<Item>& i) const { return item <  i.item; };
template <class Item>inline bool BBinaryTreeItem<Item>::operator == (const BBinaryTreeItem<Item>& i) const { return item == i.item; };
template <class Item>inline bool BBinaryTreeItem<Item>::operator >  (const BBinaryTreeItem<Item>& i) const { return item >  i.item; };
template <class Item>inline bool BBinaryTreeItem<Item>::operator >= (const BBinaryTreeItem<Item>& i) const { return item >= i.item; };
template <class Item>inline bool BBinaryTreeItem<Item>::operator <= (const BBinaryTreeItem<Item>& i) const { return item <= i.item; };

#endif // #ifndef __bbintree_h__
