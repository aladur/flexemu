/*
    bmembuf.cpp


    FLEXplorer, An explorer for FLEX disk image files and directory disks.
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

#include "bmembuf.h"
#include <utility>
#include <algorithm>
#include <stdexcept>
#include <cstring>


BMemoryBuffer::BMemoryBuffer(DWord aSize)
{
    buffer.resize(aSize, 0);
}

BMemoryBuffer::BMemoryBuffer(const BMemoryBuffer &src)
{
    buffer.clear();
    std::copy(src.buffer.cbegin(), src.buffer.cend(),
              std::back_inserter(buffer));
    std::copy(src.addressRanges.cbegin(), src.addressRanges.cend(),
              std::back_inserter(addressRanges));
}

BMemoryBuffer::BMemoryBuffer(BMemoryBuffer &&src) noexcept
    : buffer(std::move(src.buffer))
    , addressRanges(std::move(src.addressRanges))
{
}

BMemoryBuffer &BMemoryBuffer::operator= (const BMemoryBuffer &src)
{
    if (this != &src)
    {
        buffer.clear();
        std::copy(src.buffer.cbegin(), src.buffer.cend(),
                  std::back_inserter(buffer));
        std::copy(src.addressRanges.cbegin(), src.addressRanges.cend(),
                  std::back_inserter(addressRanges));
    }

    return *this;
}

BMemoryBuffer &BMemoryBuffer::operator= (BMemoryBuffer &&src) noexcept
{
    buffer = std::move(src.buffer);
    addressRanges = std::move(src.addressRanges);

    return *this;
}

void BMemoryBuffer::CopyTo(Byte *target, DWord address, DWord size) const
{
    auto secureSize = size;

    if (address >= buffer.size())
    {
        throw std::out_of_range("address is out of valid range");
    }

    if (address + secureSize >= buffer.size())
    {
        secureSize -= static_cast<DWord>(address + size - buffer.size());
    }

    std::memcpy(target, buffer.data() + address, secureSize);
    if (secureSize < size)
    {
        // Fill up buffer not represented in BMemoryBuffer by 0.
        std::memset(target + secureSize, 0, size - secureSize);
    }
}

void BMemoryBuffer::CopyFrom(const Byte *source, DWord address, DWord size)
{
    auto secureSize = size;

    if (address >= buffer.size())
    {
        throw std::out_of_range("address is out of valid range");
    }

    if (address + secureSize >= buffer.size())
    {
        secureSize -= static_cast<DWord>(address + size - buffer.size());
    }

    std::memcpy(buffer.data() + address, source, secureSize);

    DWord endAddress = address + secureSize - 1;
    addressRanges.emplace_back(address, endAddress);
    join(addressRanges);
}

bool BMemoryBuffer::CopyTo(std::vector<Byte> &targetBuffer,
           const MemorySource<DWord>::AddressRange &addressRange) const
{
    auto startIter = buffer.cbegin() + addressRange.lower();
    auto endIter = buffer.cbegin() + addressRange.upper() + 1;

    std::copy(startIter, endIter, std::back_inserter(targetBuffer));

    return true;
}

void BMemoryBuffer::Reset()
{
    std::memset(buffer.data(), 0, buffer.size());
    addressRanges.clear();
}

const MemorySource<DWord>::AddressRanges& BMemoryBuffer::GetAddressRanges() const
{
    return addressRanges;
}

