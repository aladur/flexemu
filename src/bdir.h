/*
    bdir.h


    Basic class used for directory functions
    Copyright (C) 1999-2005  W. Schwotzer

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

#ifndef __bdir_h__
#define __bdir_h__

#include "misc1.h"
#ifdef WIN32
  #pragma warning (disable: 4786)
#endif
#include <vector>
#include "bstring.h"


typedef std::vector<BString> tPathList;

class BDirectory  
{
private:
	BString m_path;

public:
	static bool Exists(const BString &aPath);
	static bool Remove(const BString &aPath);
	static bool Create(const BString &aPath, int mode = 0x0755);
	static bool RemoveRecursive(const BString &aPath);
	static tPathList GetSubDirectories(const BString &aPath);
	static tPathList GetFiles(const BString &aPath);

	BDirectory();
	BDirectory(BString &path) : m_path(path) { };
	~BDirectory();

	inline void SetPath(BString &path) { m_path = path; };
	inline const BString &GetPath(void) const { return m_path; };
	bool Exists(void) const;
	bool Remove(void) const;
	bool Create(int mode = 0x0755) const;
	bool RemoveRecursive(void) const;
	tPathList GetSubDirectories() const;
	tPathList GetFiles() const;
};

#endif // #ifndef __bdir_h__
