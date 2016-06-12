/*
    buint.h

    Basic class for unsigned integer
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

#ifndef __buint_h__
#define __buint_h__

#include "misc1.h"


class BUInt {

private:
	DWord value;

public:
	BUInt() : value(0) {};					// public constructor
	BUInt(const BUInt &ui) : value(ui.value) {};
	BUInt(DWord i) : value(i) {};
	BUInt(Word i) : value(i) {};
	~BUInt() {};			// public destructor
	
	const BUInt& operator = (DWord i);
	bool operator == (DWord i) const;
	bool operator <= (DWord i) const;
	bool operator <  (DWord i) const;
	bool operator >= (DWord i) const;
	bool operator >  (DWord i) const;
	bool operator != (DWord i) const;
	operator DWord () const ;
	const BUInt operator + (DWord i) const { return BUInt(value + i); };
	const BUInt operator - (DWord i) const { return BUInt(value - i); };
	const BUInt &operator += (DWord i);
	const BUInt &operator -= (DWord i);
}; // class BUInt

inline const BUInt& BUInt::operator = (DWord i) { value = i; return *this; }
inline bool BUInt::operator == (DWord i) const { return value == i; }
inline bool BUInt::operator <= (DWord i) const { return value <= i; }
inline bool BUInt::operator <  (DWord i) const { return value <  i; }
inline bool BUInt::operator >= (DWord i) const { return value >= i; }
inline bool BUInt::operator >  (DWord i) const { return value >  i; }
inline bool BUInt::operator != (DWord i) const { return value != i; }
inline BUInt::operator DWord () const { return value; }
inline const BUInt &BUInt::operator += (DWord i) { value += i; return *this; }
inline const BUInt &BUInt::operator -= (DWord i) { value -= i; return *this; }

#endif // #ifndef __buint_h__

