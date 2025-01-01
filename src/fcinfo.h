/*
    fcinfo.h


    FLEXplorer, An explorer for FLEX disk image files and directory disks.
    Copyright (C) 1998-2025  W. Schwotzer

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

#include "misc1.h"
#include "filecntb.h"
#include <string>
#include <vector>
#include "bdate.h"

// class FlexDiskAttributes is a data class to set and get a number of
// attributes of a FLEX disk image.
//
// Rename: FlexContainerInfo => FlexDiskAttributes.
class FlexDiskAttributes
{

private:

    BDate date;
    std::string path; // path of container file
    int sectors{0}; // Number of sectors per track
    int tracks{0}; // Number of tracks
    std::string name; // name of disk
    unsigned int number{0U};// disk number
    DiskType type{}; // disk type
    DiskOptions options{}; // disk options
    uint64_t free{0}; // Number of bytes free
    uint64_t totalSize{0}; // Number of total bytes writable
    Word sectorSize{0}; // Byte size of one sector
    Byte attributes{0}; // Disk attributes
    bool is_flex_format{false};// This container contains a FLEX file system.
    bool is_write_protected{false};// This container is write protected.
    bool is_valid{false}; // This container info is valid.
    std::vector<Byte> jvc_header; // JVC header Bytes, size==0 if not present.

public:
    FlexDiskAttributes() = default;
    virtual ~FlexDiskAttributes() = default;
    FlexDiskAttributes(const FlexDiskAttributes &src) = default;
    FlexDiskAttributes(FlexDiskAttributes &&src) = default;
    FlexDiskAttributes &operator=(const FlexDiskAttributes &src) = default;
    FlexDiskAttributes &operator=(FlexDiskAttributes &&src) = default;

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
    }

    inline void SetFree(uint64_t f)
    {
        free = f;
        is_valid = true;
    }

    inline uint64_t GetFree() const
    {
        return free;
    }

    inline void SetTotalSize(uint64_t s)
    {
        totalSize = s;
        is_valid = true;
    }

    inline uint64_t GetTotalSize() const
    {
        return totalSize;
    }

    inline void SetSectorSize(Word s)
    {
        sectorSize = s;
        is_valid = true;
    }

    inline Word GetSectorSize() const
    {
        return sectorSize;
    }

    inline void SetAttributes(Byte x_attributes)
    {
        attributes = x_attributes;
        is_valid = true;
    }

    inline Byte GetAttributes() const
    {
        return attributes;
    }

    /* Property only valid if GetIsFlexFormat() == true */
    inline const BDate &GetDate() const
    {
        return date;
    }

    inline void SetDate(const BDate &d)
    {
        date = d;
        is_valid = true;
    }

    inline void SetTrackSector(int t, int s)
    {
        tracks = t;
        sectors = s;
        is_valid = true;
    }

    inline void GetTrackSector(int &t, int &s) const
    {
        t = tracks;
        s = sectors;
    }

    /* Property only valid if GetIsFlexFormat() == true */
    inline std::string GetName() const
    {
        return name;
    }

    /* Property only valid if GetIsFlexFormat() == true */
    inline unsigned int GetNumber() const
    {
        return number;
    }

    inline std::string GetPath() const
    {
        return path;
    }

    inline void SetType(DiskType p_type)
    {
        type = p_type;
        is_valid = true;
    }

    inline DiskType GetType() const
    {
        return type;
    }

    inline void SetOptions(DiskOptions p_options)
    {
        options = p_options;
        is_valid = true;
    }

    inline DiskOptions GetOptions() const
    {
        return options;
    }

    inline void SetIsFlexFormat(bool f)
    {
        is_flex_format = f;
        is_valid = true;
    }

    inline bool GetIsFlexFormat() const
    {
        return is_flex_format;
    }

    inline void SetIsWriteProtected(bool f)
    {
        is_write_protected = f;
        is_valid = true;
    }

    inline bool GetIsWriteProtected() const
    {
        return is_write_protected;
    }

    inline void SetJvcFileHeader(const std::vector<Byte> &header)
    {
        jvc_header = header;
        is_valid = true;
    }

    inline std::vector<Byte> GetJvcFileHeader() const
    {
        return jvc_header;
    }

    inline bool IsValid() const
    {
        return is_valid;
    }
};

#endif // FCINFO_INCLUDED

