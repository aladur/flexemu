/*
    benv.cpp


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

#include <misc1.h>
#include <stdio.h>
#include "benv.h"


BEnvironment::BEnvironment()
{
}

BEnvironment::~BEnvironment(void)
{
}

bool BEnvironment::RemoveKey(const char *key)
{
	BString upperKey(key);

	upperKey.upcase();
#ifdef WIN32
	SetEnvironmentVariable((const char *)upperKey, NULL);
#endif
#ifdef UNIX
#if (HAVE_DECL_UNSETENV==1)
	unsetenv(upperKey);
#endif
#endif
	return true;
}

bool BEnvironment::SetValue(const char *key, const char *value)
{
	BString upperKey(key);
	
	upperKey.upcase();
#ifdef WIN32
	return (SetEnvironmentVariable((const char *)upperKey, value) != 0);
#endif
#ifdef UNIX
#if (HAVE_DECL_SETENV==1)
	return (setenv(upperKey, value, 1) == 0);
#else
	return false;
#endif
#endif
}

bool BEnvironment::SetValue(const char *key, int value)
{
	char str[32];
	BString upperKey(key);

	upperKey.upcase();
	sprintf(str, "%i", value);
#ifdef WIN32
	return (SetEnvironmentVariable(upperKey, str) != 0);
#endif
#ifdef UNIX
#if (HAVE_DECL_SETENV==1)
	return (setenv(upperKey, str, 1) == 0);
#else
	return false;
#endif
#endif
}

/*
bool BEnvironment::GetValue(const char *key, char **pValue)
{
	BString upperKey(key);

	upperKey.upcase();
#ifdef WIN32
	return (SetEnvironmentVariable(upperKey, str) != 0);
#endif
#ifdef UNIX
	return (*pValue = getenv(upperKey));
}
*/

bool BEnvironment::GetValue(const char *key, BString &value)
{
	char *p;
	BString upperKey(key);
	bool ret = false;

	upperKey.upcase();
#ifdef WIN32
	int size = GetEnvironmentVariable(upperKey, NULL, 0);
	if (size) {
		p = new char[size];
		if (GetEnvironmentVariable(upperKey, p, size)) {
			value = p;
			ret = true;
		}
		delete [] p;
	}
#endif
#ifdef UNIX
	if ((p = getenv(upperKey))) {
		value = p;
		ret = true;
	}
#endif
	return ret;
}

bool BEnvironment::GetValue(const char *key, int *pValue)
{
	BString str;
	BString upperKey(key);

	upperKey.upcase();
	if (!GetValue(upperKey, str))
		return false;
	return (sscanf(str, "%i", pValue) == 1);
}

