/*
    fcinfo.h


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2004  W. Schwotzer

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

#ifndef __fcinfo_h__
#define __fcinfo_h__

#include <stdlib.h>
#include "misc1.h"
#include "bstring.h"
#include "bdate.h"


const int	FLEX_DISKNAME_LENGTH = 12;


class FlexContainerInfo {

private:

	BDate   date;
	BString path;	// path of container file
	int		sectors;	// Number of sectors per track
	int		tracks;		// Number of tracks
	char	name[FLEX_DISKNAME_LENGTH]; // name of disk
	int		type;		// container type
	int		free;		// Number of bytes free
	int		totalSize;	// Number of total bytes writable
	int		attributes;	// Disk attributes

public:
	FlexContainerInfo();		// public constructor
	virtual ~FlexContainerInfo();	// public destructor

	void				SetName(const char *n);
	const BString	GetTypeString(void) const;

	inline void			SetPath(const char *p) { path = p; };
	inline void			SetFree(int f) { free = f; };
	inline int			GetFree(void) const { return free; };
	inline void			SetTotalSize(int s) { totalSize = s; };
	inline int			GetTotalSize(void) const { return totalSize; };
	inline void			SetAttributes(int a) { attributes = a; };
	inline int			GetAttributes(void) const { return attributes; };
	inline const		BDate& GetDate(void) const { return date; };
	inline void			SetDate(const BDate& d) { date = d; };
	inline void			SetDate(int d, int m, int y) { date.SetDate(d, m, y); };
	inline void			SetTrackSector(int t, int s) { tracks = t; sectors = s; };
	inline void			GetTrackSector(int *t, int *s) const { *t = tracks; *s = sectors; };
	inline const char	*GetName(void) const { return (char *)&name; };
	inline const		BString& GetPath(void) const { return path; };
	inline void			SetType(int t) { type = t; };
	inline int			GetType(void) const {return type; };

};  // class FlexContainerInfo

#endif // __fcinfo_h__

