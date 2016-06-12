/*
    bregistr.h

    Basic class containing a windows registry handle
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

#ifndef __registr_h__
#define __registr_h__

#ifdef WIN32

#include <windows.h>
#include "bstring.h"


class BRegistry {

private:

	LONG	lastError;
	HKEY	hKey;

	BRegistry();

public:
	~BRegistry();

	BRegistry(const BRegistry &regKey, const char *subKey = NULL);
	BRegistry(HKEY aHKey);
	bool isOpened() const;
	LONG GetLastError() const;
	LONG Delete();
	LONG SetValue(const char *name, const char *value);
	LONG SetValue(const char *name, const int value);
	LONG SetValue(const char *name, const BYTE *value, int size);
	LONG GetValue(const char *name, BString &value);
	LONG GetValue(const char *name, int *value);
	// implicit type conversions !
	operator HKEY() const ;

	static BRegistry classesRoot;
	static BRegistry currentUser;
	static BRegistry localMachine;
	static BRegistry users;
};

inline bool BRegistry::isOpened() const { return hKey != NULL; };
inline LONG BRegistry::GetLastError() const { return lastError; };
inline BRegistry::operator HKEY() const { return hKey; };

#endif // #ifdef WIN32
#endif // #ifndef __bregistr_h__
