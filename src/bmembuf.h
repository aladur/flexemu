/*
    bmembuf.h


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 2003-2024  W. Schwotzer

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

#include "misc1.h"
#include "memsrc.h"
#include "memtgt.h"
#include <vector>
#include <utility>


class BMemoryBuffer : public MemorySource<size_t>, public MemoryTarget<size_t>
{
public:
    BMemoryBuffer() = delete;
    BMemoryBuffer(size_t aSize);
    BMemoryBuffer(const BMemoryBuffer &src);
    BMemoryBuffer(BMemoryBuffer &&src) noexcept;
    ~BMemoryBuffer() override = default;

    BMemoryBuffer &operator=(const BMemoryBuffer &src);
    BMemoryBuffer &operator=(BMemoryBuffer &&src) noexcept;

    inline size_t GetSize() const
    {
        return buffer.size();
    };

    // MemorySource interface
    const MemorySource<size_t>::AddressRanges& GetAddressRanges() const override;
    void CopyTo(Byte *buffer, size_t address, size_t aSize) const override;

    // MemoryTarget interface
    void CopyFrom(const Byte *buffer, size_t address, size_t aSize) override;

public:
    bool CopyTo(std::vector<Byte> &buffer,
                const MemorySource<size_t>::AddressRange &addressRange) const;
    void Reset();

private:
    std::vector<Byte> buffer;
    MemorySource<size_t>::AddressRanges addressRanges;
};

#endif // BMEMBUF_INCLUDED
