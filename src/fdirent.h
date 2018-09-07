/*
    fdirent.h


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2018  W. Schwotzer

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

#ifndef FDIRENT_INCLUDED
#define FDIRENT_INCLUDED


#include "misc1.h"
#include <string>
#include "bdate.h"

enum flexFileAttributes
{
    FLX_READONLY    = 0x80,
    FLX_NODELETE    = 0x40,
    FLX_NOREAD  = 0x20,
    FLX_NOCAT   = 0x10,
    FLX_RANDOM  = 0x02
};

enum flexFileStatus
{
    FLX_EMPTY   = 0x1
};


#ifdef _MSC_VER
const SIZE_T FLEX_BASEFILENAME_LENGTH = 8;
const SIZE_T FLEX_FILEEXT_LENGTH = 3;
#else
const unsigned long FLEX_BASEFILENAME_LENGTH = 8;
const unsigned long FLEX_FILEEXT_LENGTH = 3;
#endif


class FlexDirEntry
{
private:
    int     size;
    Byte attributes;
    int     sectorMap;
    int     startTrk, startSec;
    int     endTrk, endSec;
    int     status;
    BDate   date;
    std::string fileName;

public:
    FlexDirEntry();
    ~FlexDirEntry();
    FlexDirEntry(const FlexDirEntry &src);
    FlexDirEntry(FlexDirEntry &&src);

    FlexDirEntry &operator= (const FlexDirEntry &src);
    FlexDirEntry &operator= (FlexDirEntry &&src);

    const BDate &GetDate() const;
    void    SetDate(const BDate &date);
    void    SetDate(int d, int m, int y);
    void    SetStartTrkSec(int t, int s);
    void    GetStartTrkSec(int *t, int *s);
    void    SetEndTrkSec(int t, int s);
    void    GetEndTrkSec(int *t, int *s);
    void    SetTotalFileName(const char *fileName);
    const std::string &GetTotalFileName() const;
    std::string GetFileName() const;
    std::string GetFileExt() const;
    void    SetSize(int size);
    int     GetSize();
    void SetAttributes(Byte attributes);
    void SetAttributes(Byte setMask, Byte clearMask);
    Byte GetAttributes();
    const std::string GetAttributesString();
    int     GetSectorMap() const;
    int     IsRandom() const;
    void    SetSectorMap(int aSectorMap);
    int     IsEmpty();
    void    SetEmpty();
    void    ClearEmpty();

private:
    void CopyFrom(const FlexDirEntry &src);
};  // class FlexDirEntry

inline void FlexDirEntry::SetSize(int s)
{
    size = s;
}
inline int FlexDirEntry::GetSize()
{
    return size;
}

inline void FlexDirEntry::SetAttributes(Byte x_attributes)
{
    attributes = x_attributes;
}
inline Byte FlexDirEntry::GetAttributes()
{
    return attributes;
}

inline void FlexDirEntry::SetEmpty()
{
    status |= FLX_EMPTY;
}
inline void FlexDirEntry::ClearEmpty()
{
    status &= ~FLX_EMPTY;
}
inline int FlexDirEntry::IsEmpty()
{
    return status & FLX_EMPTY;
}

inline int FlexDirEntry::IsRandom() const
{
    return (sectorMap != 0);
}
inline void FlexDirEntry::SetSectorMap(int aSectorMap)
{
    sectorMap = aSectorMap;
}
inline int FlexDirEntry::GetSectorMap() const
{
    return sectorMap;
}

#endif // FDIRENT_INCLUDED

