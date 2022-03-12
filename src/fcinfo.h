/*
    fcinfo.h


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

#ifndef FCINFO_INCLUDED
#define FCINFO_INCLUDED

#include <stdlib.h>
#include "misc1.h"
#include <string>
#include "bdate.h"


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
    uint64_t free; // Number of bytes free
    uint64_t totalSize; // Number of total bytes writable
    Byte attributes; // Disk attributes
    bool    is_flex_format;// This container contains a FLEX file system.
    bool    is_write_protected;// This container is write protected.
    bool    is_valid;   // This container info is valid.

public:
    FlexContainerInfo();        // public constructor
    virtual ~FlexContainerInfo();   // public destructor

    void                SetName(const std::string &n)
    {
        name = n;
        is_valid = true;
    }
    void                SetNumber(unsigned int n)
    {
        number = n;
        is_valid = true;
    }
    const std::string   GetTypeString() const;

    inline void         SetPath(const std::string &p)
    {
        path = p;
        is_valid = true;
    };
    inline void SetFree(uint64_t f)
    {
        free = f;
        is_valid = true;
    };
    inline uint64_t GetFree() const
    {
        return free;
    };
    inline void SetTotalSize(uint64_t s)
    {
        totalSize = s;
        is_valid = true;
    };
    inline uint64_t GetTotalSize() const
    {
        return totalSize;
    };
    inline void SetAttributes(Byte x_attributes)
    {
        attributes = x_attributes;
        is_valid = true;
    };
    inline Byte GetAttributes() const
    {
        return attributes;
    };
    /* Property only valid if GetIsFlexFormat() == true */
    inline const        BDate &GetDate() const
    {
        return date;
    };
    inline void         SetDate(const BDate &d)
    {
        date = d;
        is_valid = true;
    };
    inline void         SetTrackSector(int t, int s)
    {
        tracks = t;
        sectors = s;
        is_valid = true;
    };
    inline void         GetTrackSector(int &t, int &s) const
    {
        t = tracks;
        s = sectors;
    };
    /* Property only valid if GetIsFlexFormat() == true */
    inline const std::string GetName() const
    {
        return name;
    };
    /* Property only valid if GetIsFlexFormat() == true */
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
        is_valid = true;
    };
    inline int          GetType() const
    {
        return type;
    };
    inline void SetIsFlexFormat(bool f)
    {
        is_flex_format = f;
        is_valid = true;
    };
    inline bool GetIsFlexFormat() const
    {
        return is_flex_format;
    };

    inline void SetIsWriteProtected(bool f)
    {
        is_write_protected = f;
        is_valid = true;
    };
    inline bool GetIsWriteProtected() const
    {
        return is_write_protected;
    };
    inline bool IsValid() const
    {
        return is_valid;
    };
};  // class FlexContainerInfo

#endif // FCINFO_INCLUDED

