/*
    bmembuf.h


    FLEXplorer, An explorer for FLEX disk image files and directory disks.
    Copyright (C) 2003-2025  W. Schwotzer

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

#ifndef BMEMBUF_INCLUDED
#define BMEMBUF_INCLUDED

#include "typedefs.h"
#include "memsrc.h"
#include "memtgt.h"
#include <vector>


class BMemoryBuffer : public MemorySource<DWord>, public MemoryTarget<DWord>
{
public:
    BMemoryBuffer() = delete;
    explicit BMemoryBuffer(DWord size);
    BMemoryBuffer(const BMemoryBuffer &src);
    BMemoryBuffer(BMemoryBuffer &&src) noexcept;
    ~BMemoryBuffer() override = default;

    BMemoryBuffer &operator=(const BMemoryBuffer &src);
    BMemoryBuffer &operator=(BMemoryBuffer &&src) noexcept;

    inline DWord GetSize() const
    {
        return static_cast<DWord>(buffer.size());
    };

    // MemorySource interface
    const MemorySource<DWord>::AddressRanges& GetAddressRanges() const override;
    void CopyTo(Byte *target, DWord address, DWord size) const override;

    // MemoryTarget interface
    void CopyFrom(const Byte *source, DWord address, DWord size) override;

public:
    bool CopyTo(std::vector<Byte> &buffer,
                const MemorySource<DWord>::AddressRange &addressRange) const;
    void Reset();

private:
    std::vector<Byte> buffer;
    MemorySource<DWord>::AddressRanges addressRanges;
};

#endif
