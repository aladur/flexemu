/*
    fcinfo.h


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2021  W. Schwotzer

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

#ifndef FCINFO_INCLUDED
#define FCINFO_INCLUDED

#include <stdlib.h>
#include "misc1.h"
#include <string>
#include "bdate.h"


const int   FLEX_DISKNAME_LENGTH = 12;


class FlexContainerInfo
{

private:

    BDate   date;
    std::string path;   // path of container file
    int     sectors;    // Number of sectors per track
    int     tracks;     // Number of tracks
    std::string name;   // name of disk
    unsigned int number;// disk number
    int     type;       // container type
    int     free;       // Number of bytes free
    int     totalSize;  // Number of total bytes writable
    Byte attributes; // Disk attributes
    bool    is_flex_format;// This container contains a FLEX file system.
    bool    is_write_protected;// This container is write protected.

public:
    FlexContainerInfo();        // public constructor
    virtual ~FlexContainerInfo();   // public destructor

    void                SetName(const std::string &n)
    {
        name = n;
    }
    void                SetNumber(unsigned int n)
    {
        number = n;
    }
    const std::string   GetTypeString() const;

    inline void         SetPath(const std::string &p)
    {
        path = p;
    };
    inline void         SetFree(int f)
    {
        free = f;
    };
    inline int          GetFree() const
    {
        return free;
    };
    inline void         SetTotalSize(int s)
    {
        totalSize = s;
    };
    inline int          GetTotalSize() const
    {
        return totalSize;
    };
    inline void SetAttributes(Byte x_attributes)
    {
        attributes = x_attributes;
    };
    inline Byte GetAttributes() const
    {
        return attributes;
    };
    inline const        BDate &GetDate() const
    {
        return date;
    };
    inline void         SetDate(const BDate &d)
    {
        date = d;
    };
    inline void         SetDate(int d, int m, int y)
    {
        date.SetDate(d, m, y);
    };
    inline void         SetTrackSector(int t, int s)
    {
        tracks = t;
        sectors = s;
    };
    inline void         GetTrackSector(int &t, int &s) const
    {
        t = tracks;
        s = sectors;
    };
    inline const std::string GetName() const
    {
        return name;
    };
    inline unsigned int GetNumber() const
    {
        return number;
    };
    inline const std::string GetPath() const
    {
        return path;
    };
    inline void         SetType(int t)
    {
        type = t;
    };
    inline int          GetType() const
    {
        return type;
    };
    inline void SetIsFlexFormat(bool f)
    {
        is_flex_format = f;
    };
    inline bool GetIsFlexFormat() const
    {
        return is_flex_format;
    };

    inline void SetIsWriteProtected(bool f)
    {
        is_write_protected = f;
    };
    inline bool GetIsWriteProtected() const
    {
        return is_write_protected;
    };
    inline bool IsValid() const
    {
        return tracks != 0 && sectors != 0;
    };
};  // class FlexContainerInfo

#endif // FCINFO_INCLUDED

