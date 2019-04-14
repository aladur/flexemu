/*
    bmembuf.h


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 2003-2019  W. Schwotzer

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
    BMemoryBuffer(BMemoryBuffer &&src);
    ~BMemoryBuffer();

    BMemoryBuffer &operator=(const BMemoryBuffer &src);
    BMemoryBuffer &operator=(BMemoryBuffer &&src);

    inline size_t GetSize() const
    {
        return size;
    };

    // MemorySource interface
    size_t reset_src_addr() override;
    bool src_at_end() const override;
    MemorySource& operator>>(Byte &b) override;

    // MemoryTarget interface
    void CopyFrom(const Byte *buffer, size_t address, size_t aSize) override;

public:
    bool IsStartEndAddressValid() const;
    std::pair<size_t, size_t> GetStartEndAddress() const;
    bool CopyStartEndRangeTo(std::vector<Byte> &buffer) const;
    void ResetStartEndAddress();

private:
    size_t size;
    std::vector<Byte> buffer;
    size_t startAddress;
    size_t endAddress;
    size_t sourceAddress;
};

#endif // BMEMBUF_INCLUDED
