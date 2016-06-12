/*
    bdir.cpp


    Basic class used for directory functions
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

#include <misc1.h>
#include <stdio.h>
#include <sys/stat.h>

#ifdef _MSC_VER
#  include <direct.h>
#endif

#include "bdir.h"

/********************************************
 ctor / dtor
********************************************/

BDirectory::BDirectory()
{
}

BDirectory::~BDirectory()
{
}

/********************************************
 static functions
********************************************/

bool BDirectory::Exists(const BString &aPath)
{
	struct stat sbuf;

	return !stat(aPath, &sbuf) && (S_ISDIR(sbuf.st_mode));
}

bool BDirectory::Remove(const BString &aPath)
{
#ifdef _MSC_VER
	return _rmdir(aPath) >= 0;
#endif
#ifdef __GNUC__
	return rmdir(aPath) >= 0;
#endif
}

bool BDirectory::Create(const BString &aPath, int mode /* = 0x0755 */)
{
#if defined(_MSC_VER) || defined(__MINGW32)
	return _mkdir(aPath) >= 0;
#endif
#if defined(UNIX) || defined(__CYGWIN32)        
	return mkdir(aPath, mode) >= 0;
#endif
}

#ifdef WIN32
bool BDirectory::RemoveRecursive(const BString &aPath)
{
	BString         basePath;
	BString         dirEntry;
	HANDLE          hdl;
	WIN32_FIND_DATA pentry;

	basePath = aPath;
	if (basePath.lastchar() != PATHSEPARATOR) {
		basePath += PATHSEPARATOR;
	}
	if ((hdl = FindFirstFile(basePath + "*.*", &pentry)) != INVALID_HANDLE_VALUE) {
		do {
			dirEntry = basePath + pentry.cFileName;
			if (pentry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if (pentry.cFileName[0] != '.')
					RemoveRecursive(dirEntry);
			} else
				remove(dirEntry);
		} while (FindNextFile(hdl, &pentry) != 0);
		FindClose(hdl);
	}
	BDirectory::Remove(basePath);
	return true;
}
#endif
#ifdef UNIX        
bool BDirectory::RemoveRecursive(const BString &aPath)
{
	BString         basePath;
	BString         dirEntry;
	DIR             *pd;
	struct dirent   *pentry;
	struct stat     sbuf;

	basePath = aPath;
	if (basePath.lastchar() == PATHSEPARATOR)
		basePath.at(0, basePath.length() - 1, basePath);
	if ((pd = opendir(basePath)) != NULL) {
		while ((pentry = readdir(pd)) != NULL) {
			dirEntry = basePath + PATHSEPARATORSTRING +
				pentry->d_name;
			if (!stat(dirEntry, &sbuf) && (S_ISREG(sbuf.st_mode))) {
				remove(dirEntry);
			} else
			if (S_ISDIR(sbuf.st_mode) && pentry->d_name[0] != '.') {
				RemoveRecursive(dirEntry);
			}
		} // while
		closedir(pd);
	}
	BDirectory::Remove(basePath);
	return true;
}
#endif

/********************************************
 member functions
********************************************/

bool BDirectory::Exists(void) const
{
	return Exists(m_path);
}

bool BDirectory::Remove(void) const
{
	return Remove(m_path);
}

bool BDirectory::RemoveRecursive(void) const
{
	return RemoveRecursive(m_path);
}

bool BDirectory::Create(int mode /* = 0x0755 */) const
{
	return Create(m_path, mode);
}
