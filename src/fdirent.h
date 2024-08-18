/*
    fdirent.h


    FLEXplorer, An explorer for FLEX disk image files and directory disks.
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

#ifndef FDIRENT_INCLUDED
#define FDIRENT_INCLUDED


#include "misc1.h"
#include <cstdint>
#include <string>
#include "bdate.h"
#include "btime.h"
#include "filecntb.h"

enum flexFileAttributes : uint8_t
{
    FLX_READONLY = 0x80,
    FLX_NODELETE = 0x40,
    FLX_NOREAD = 0x20,
    FLX_NOCAT = 0x10,
};

const unsigned FLX_NOT_EMPTY{1U};

class FlexDirEntry
{
private:
    uint32_t size{0};
    Byte attributes{0};
    int sectorMap{0};
    int startTrk{-1};
    int startSec{0};
    int endTrk{0};
    int endSec{0};
    Byte status{0};
    BDate date;
    BTime time;
    std::string fileName;

public:
    FlexDirEntry() = default;
    ~FlexDirEntry() = default;
    FlexDirEntry(const FlexDirEntry &src) = default;
    FlexDirEntry(FlexDirEntry &&src) noexcept;

    FlexDirEntry &operator= (const FlexDirEntry &src) = default;
    FlexDirEntry &operator= (FlexDirEntry &&src) noexcept = default;

    const BDate &GetDate() const;
    void SetDate(const BDate &date);
    const BTime &GetTime() const;
    void SetTime(const BTime &time);
    void SetStartTrkSec(int t, int s);
    void GetStartTrkSec(int &t, int &s) const;
    void SetEndTrkSec(int t, int s);
    void GetEndTrkSec(int &t, int &s) const;
    void SetTotalFileName(const std::string &p_fileName);
    const std::string &GetTotalFileName() const;
    std::string GetFileName() const;
    std::string GetFileExt() const;
    void SetFileSize(uint32_t p_size);
    uint32_t GetFileSize() const;
    void SetAttributes(Byte attributes);
    void SetAttributes(Byte setMask, Byte clearMask);
    Byte GetAttributes() const;
    std::string GetAttributesString() const;
    int GetSectorMap() const;
    bool IsRandom() const;
    void SetSectorMap(int p_sectorMap);
    bool IsEmpty() const;
    void SetEmpty();
    void ClearEmpty();

private:
    void CopyFrom(const FlexDirEntry &src);
};

inline void FlexDirEntry::SetFileSize(uint32_t p_size)
{
    size = p_size;
}
inline uint32_t FlexDirEntry::GetFileSize() const
{
    return size;
}

inline void FlexDirEntry::SetAttributes(Byte p_attributes)
{
    attributes = p_attributes;
}
inline Byte FlexDirEntry::GetAttributes() const
{
    return attributes;
}

inline void FlexDirEntry::SetEmpty()
{
    status &= ~FLX_NOT_EMPTY;
}
inline void FlexDirEntry::ClearEmpty()
{
    status |= FLX_NOT_EMPTY;
}
inline bool FlexDirEntry::IsEmpty() const
{
    return (status & FLX_NOT_EMPTY) == 0;
}

inline bool FlexDirEntry::IsRandom() const
{
    return (sectorMap != 0);
}
inline void FlexDirEntry::SetSectorMap(int p_sectorMap)
{
    sectorMap = p_sectorMap;
}
inline int FlexDirEntry::GetSectorMap() const
{
    return sectorMap;
}

#endif // FDIRENT_INCLUDED

