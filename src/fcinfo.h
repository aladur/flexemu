/*
    fcinfo.h


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2024  W. Schwotzer

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
#include <vector>
#include "bdate.h"


class FlexContainerInfo
{

private:

    BDate date;
    std::string path; // path of container file
    int sectors{0}; // Number of sectors per track
    int tracks{0}; // Number of tracks
    std::string name; // name of disk
    unsigned int number{0U};// disk number
    int type{0}; // container type
    uint64_t free{0}; // Number of bytes free
    uint64_t totalSize{0}; // Number of total bytes writable
    Byte attributes{0}; // Disk attributes
    bool is_flex_format{false};// This container contains a FLEX file system.
    bool is_write_protected{false};// This container is write protected.
    bool is_valid{false}; // This container info is valid.
    std::vector<Byte> jvc_header; // JVC header Bytes, size==0 if not present.

public:
    FlexContainerInfo() = default;
    virtual ~FlexContainerInfo() = default;

    void SetName(const std::string &n)
    {
        name = n;
        is_valid = true;
    }
    void SetNumber(unsigned int n)
    {
        number = n;
        is_valid = true;
    }
    std::string GetTypeString() const;

    inline void SetPath(const std::string &p)
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
    inline const BDate &GetDate() const
    {
        return date;
    };
    inline void SetDate(const BDate &d)
    {
        date = d;
        is_valid = true;
    };
    inline void SetTrackSector(int t, int s)
    {
        tracks = t;
        sectors = s;
        is_valid = true;
    };
    inline void GetTrackSector(int &t, int &s) const
    {
        t = tracks;
        s = sectors;
    };
    /* Property only valid if GetIsFlexFormat() == true */
    inline std::string GetName() const
    {
        return name;
    };
    /* Property only valid if GetIsFlexFormat() == true */
    inline unsigned int GetNumber() const
    {
        return number;
    };
    inline std::string GetPath() const
    {
        return path;
    };
    inline void SetType(int t)
    {
        type = t;
        is_valid = true;
    };
    inline int GetType() const
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
    inline void SetJvcFileHeader(const std::vector<Byte> &header)
    {
        jvc_header = header;
        is_valid = true;
    };
    inline std::vector<Byte> GetJvcFileHeader() const
    {
        return jvc_header;
    };
    inline bool IsValid() const
    {
        return is_valid;
    };
}; // class FlexContainerInfo

#endif // FCINFO_INCLUDED

