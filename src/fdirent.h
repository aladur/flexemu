/*
    fdirent.h


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998, 1999  W. Schwotzer

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

#ifndef __fdirent_h__
#define __fdirent_h__


#include "bstring.h"
#include "bdate.h"
//#include <new>

enum flexFileAttributes {
	FLX_READONLY	= 0x80,
	FLX_NODELETE	= 0x40,
	FLX_NOREAD	= 0x20,
	FLX_NOCAT	= 0x10,
	FLX_RANDOM	= 0x02
};

enum flexFileStatus {
	FLX_EMPTY	= 0x1
};


const int	FLEX_BASEFILENAME_LENGTH = 8;
const int	FLEX_FILEEXT_LENGTH      = 3;


class FlexDirEntry {

private:

	int		size;
	int		attributes;
	int		sectorMap;
	int		startTrk, startSec;
	int		endTrk, endSec;
	int		status;
	BDate	date;
	BString	fileName;
	static char *fileDescription[];

public:
	FlexDirEntry();			// public constructor
	virtual ~FlexDirEntry();	// public destructor
	FlexDirEntry(const FlexDirEntry& de);

	void	CopyFrom(const FlexDirEntry& de);
	const BDate& GetDate(void) const;
	void	SetDate(const BDate& date);
	void	SetDate(int d, int m, int y);
	void	SetStartTrkSec(int t, int s);
	void	GetStartTrkSec(int *t, int *s);
	void	SetEndTrkSec(int t, int s);
	void	GetEndTrkSec(int *t, int *s);
	void	SetTotalFileName(const char *fileName);
	const BString& GetTotalFileName(void) const;
	const char *GetFileName(void) const;
	const char *GetFileExt(void);
	void	SetSize(int size);
	int		GetSize(void);
	void	SetAttributes(int attr);
	void	SetAttributes(int setMask, int clearMask);
	int		GetAttributes(void);
	const BString GetAttributesString(void);
	const BString GetFileDescription(void);
	int		GetSectorMap(void) const;
	int		IsRandom(void) const;
	void	SetSectorMap(int aSectorMap);
	int		IsEmpty(void);
	void	SetEmpty(void);
	void	ClearEmpty(void);
	FlexDirEntry& operator = (const FlexDirEntry& de);
	
};  // class FlexDirEntry

inline void FlexDirEntry::SetSize(int s) { size = s; }
inline int FlexDirEntry::GetSize(void) { return size; }

inline void FlexDirEntry::SetAttributes(int a) { attributes = a; }
inline int FlexDirEntry::GetAttributes(void) { return attributes; }

inline void FlexDirEntry::SetEmpty(void) { status |= FLX_EMPTY; }
inline void FlexDirEntry::ClearEmpty(void) { status &= ~FLX_EMPTY; }
inline int FlexDirEntry::IsEmpty(void) { return status & FLX_EMPTY; }

inline int FlexDirEntry::IsRandom(void) const { return (sectorMap != 0); }
inline void FlexDirEntry::SetSectorMap(int aSectorMap) { sectorMap = aSectorMap; }
inline int FlexDirEntry::GetSectorMap(void) const { return sectorMap; }

inline FlexDirEntry& FlexDirEntry::operator = (const FlexDirEntry& de) { CopyFrom(de); return *this; }

#endif // __fdirent_h__

