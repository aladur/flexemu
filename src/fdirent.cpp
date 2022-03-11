/*
    fdirent.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2022  W. Schwotzer

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

#include "misc1.h"
#include <string>
#include <algorithm>
#include <iterator>
#include <locale>
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

FlexDirEntry::~FlexDirEntry()
{
}

FlexDirEntry::FlexDirEntry(const FlexDirEntry &src)
{
    if (&src != this)
    {
        fileName = src.fileName;
        CopyFrom(src);
    }
}

FlexDirEntry::FlexDirEntry(FlexDirEntry &&src)
{
    if (&src != this)
    {
        fileName = std::move(src.fileName);
        CopyFrom(src);
    }
}

FlexDirEntry &FlexDirEntry::operator=(const FlexDirEntry &src)
{
    if (&src != this)
    {
        fileName = src.fileName;
        CopyFrom(src);
    }

    return *this;
}

FlexDirEntry &FlexDirEntry::operator=(FlexDirEntry &&src)
{
    if (&src != this)
    {
        fileName = std::move(src.fileName);
        CopyFrom(src);
    }

    return *this;
}

void FlexDirEntry::CopyFrom(const FlexDirEntry &src)
{
    date = src.date;
    time = src.time;
    size = src.size;
    attributes = src.attributes;
    sectorMap = src.sectorMap;
    startTrk = src.startTrk;
    startSec = src.startSec;
    endTrk = src.endTrk;
    endSec = src.endSec;
    status = src.status;
}

const BDate &FlexDirEntry::GetDate() const
{
    return date;
}

void FlexDirEntry::SetDate(const BDate &d)
{
    date = d;
}

const BTime &FlexDirEntry::GetTime() const
{
    return time;
}

void FlexDirEntry::SetTime(const BTime &t)
{
    time = t;
}

void FlexDirEntry::SetTotalFileName(const char *s)
{
    fileName = s;
}

std::string FlexDirEntry::GetFileName() const
{
    // return substring until but not including last occurence of '.'
    std::string::const_reverse_iterator it =
        std::find(fileName.rbegin(), fileName.rend(), '.');

    if (it == fileName.rend())
    {
        // If '.' not found return whole string
        return fileName;
    }
    else
    {
        std::string result;

        it++;
        if (it != fileName.rend())
        {
            std::copy(fileName.begin(), it.base(), std::back_inserter(result));
        }

        return result;
    }
}

std::string FlexDirEntry::GetFileExt() const
{
    const char *p;
    std::string ext;

    p = strchr(fileName.c_str(), '.');

    if (p != nullptr)
    {
        ext = ++p;
    }

    return ext;
}

const std::string &FlexDirEntry::GetTotalFileName() const
{
    return fileName;
}

void FlexDirEntry::SetStartTrkSec(int t, int s)
{
    startTrk = t;
    startSec = s;
}

void FlexDirEntry::GetStartTrkSec(int &track, int &sector) const
{
    track = startTrk;
    sector = startSec;
}

void FlexDirEntry::SetEndTrkSec(int t, int s)
{
    endTrk = t;
    endSec = s;
}

void FlexDirEntry::GetEndTrkSec(int &track, int &sector) const
{
    track = endTrk;
    sector = endSec;
}

const std::string FlexDirEntry::GetAttributesString() const
{
    std::string str;

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

void FlexDirEntry::SetAttributes(Byte setMask, Byte clearMask)
{
    attributes = (attributes & ~clearMask) | setMask;
}
