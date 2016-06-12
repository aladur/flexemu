/*
    benv.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2004  W. Schwotzer

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

#ifndef __benv_h__
#define __benv_h__

#include "bstring.h"

class BEnvironment {
public:
	BEnvironment();
	~BEnvironment();

	bool RemoveKey(const char *key);
	bool SetValue(const char *key, const char *value);
	bool SetValue(const char *key, int value);
//	bool GetValue(const char *key, char **pValue);
	bool GetValue(const char *key, BString &value);
	bool GetValue(const char *key, int *pValue);
};

#endif

