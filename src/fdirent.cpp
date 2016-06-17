/*
    fdirent.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2005  W. Schwotzer

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
#include "bdate.h"
#include "fdirent.h"
#include <stdio.h>


FlexDirEntry::FlexDirEntry() :
    size(0),
    attributes(0),
    sectorMap(0),
    startTrk(-1),
    startSec(0),
    endTrk(0),
    endSec(0),
    status(0)
{
} // FlexDirEntry

FlexDirEntry::~FlexDirEntry(void)
{
}

FlexDirEntry::FlexDirEntry(const FlexDirEntry &de)
{
    CopyFrom(de);
}

void FlexDirEntry::CopyFrom(const FlexDirEntry &de)
{
    date        = de.date;
    fileName    = de.fileName;
    size        = de.size;
    attributes  = de.attributes;
    sectorMap   = de.sectorMap;
    startTrk    = de.startTrk;
    startSec    = de.startSec;
    endTrk      = de.endTrk;
    endSec      = de.endSec;
    status      = de.status;
}


const BDate &FlexDirEntry::GetDate(void) const
{
    return date;
}

void FlexDirEntry::SetDate(const BDate &d)
{
    date = d;
}

void FlexDirEntry::SetDate(int d, int m, int y)
{
    date.SetDate(d, m, y);
}


void FlexDirEntry::SetTotalFileName(const char *s)
{
    fileName = s;
    fileName.upcase();
}

BString FlexDirEntry::GetFileName(void) const
{
    BString name(fileName);

    if (strchr(fileName.c_str(), '.') == NULL)
    {
        return name;
    }
    else
    {
        return name.beforeLast('.');
    }
}

BString FlexDirEntry::GetFileExt(void) const
{
    const char *p;
    BString ext;

    p = strchr(fileName.c_str(), '.');

    if (p != NULL)
    {
        ext = ++p;
    }

    return ext;
}

const BString &FlexDirEntry::GetTotalFileName(void) const
{
    return fileName;
}

void FlexDirEntry::SetStartTrkSec(int t, int s)
{
    startTrk = t;
    startSec = s;
}

void FlexDirEntry::GetStartTrkSec(int *t, int *s)
{
    *t = startTrk;
    *s = startSec;
}

void FlexDirEntry::SetEndTrkSec(int t, int s)
{
    endTrk = t;
    endSec = s;
}

void FlexDirEntry::GetEndTrkSec(int *t, int *s)
{
    *t = endTrk;
    *s = endSec;
}

const BString FlexDirEntry::GetAttributesString(void)
{
    BString str;

    if (attributes & FLX_READONLY)
    {
        str += "W";
    }

    if (attributes & FLX_NODELETE)
    {
        str += "D";
    }

    if (attributes & FLX_NOREAD)
    {
        str += "R";
    }

    if (attributes & FLX_NOCAT)
    {
        str += "C";
    }

    return str;
}

void FlexDirEntry::SetAttributes(int setMask, int clearMask)
{
    attributes = (attributes & ~clearMask) | setMask;
}
