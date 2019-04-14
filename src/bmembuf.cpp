/*
    bmembuf.cpp


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

#include "misc1.h"
#include "bmembuf.h"
#include <algorithm>


BMemoryBuffer::BMemoryBuffer(size_t aSize) :
    size(aSize),
    startAddress(std::numeric_limits<size_t>::max()),
    endAddress(std::numeric_limits<size_t>::min())
{
    buffer.resize(size, 0);
}

BMemoryBuffer::BMemoryBuffer(const BMemoryBuffer &src)
{
    size = src.size;
    std::copy(src.buffer.cbegin(), src.buffer.cend(),
              std::back_inserter(buffer));
    startAddress = src.startAddress;
    endAddress = src.endAddress;
}

BMemoryBuffer::BMemoryBuffer(BMemoryBuffer &&src)
{
    size = src.size;
    buffer = std::move(src.buffer);
    startAddress = src.startAddress;
    endAddress = src.endAddress;
}

BMemoryBuffer::~BMemoryBuffer()
{
}

BMemoryBuffer &BMemoryBuffer::operator= (const BMemoryBuffer &src)
{
    if (this != &src)
    {
        size = src.size;
        std::copy(src.buffer.cbegin(), src.buffer.cend(),
                  std::back_inserter(buffer));
        startAddress = src.startAddress;
        endAddress = src.endAddress;
    }

    return *this;
}

BMemoryBuffer &BMemoryBuffer::operator= (BMemoryBuffer &&src)
{
    size = src.size;
    buffer = std::move(src.buffer);
    startAddress = src.startAddress;
    endAddress = src.endAddress;

    return *this;
}

size_t BMemoryBuffer::reset_src_addr()
{
    sourceAddress = startAddress;
    return sourceAddress;
}

bool BMemoryBuffer::src_at_end() const
{
    return (sourceAddress >= size || sourceAddress > endAddress);
}

MemorySource<size_t> &BMemoryBuffer::operator>>(Byte &value)
{
    if (sourceAddress >= size || sourceAddress > endAddress)
    {
        return *this;
    }

    value = buffer[sourceAddress];

    sourceAddress++;

    return *this;
}

void BMemoryBuffer::CopyFrom(const Byte *src, size_t address, size_t aSize)
{
    size_t secureSize = aSize;

    if (address >= size)
    {
        throw std::out_of_range("address is out of valid range");
    }

    if (address + secureSize >= size)
    {
        secureSize -= address + aSize - size;
    }

    memcpy(buffer.data() + address, src, secureSize);

    if (address < startAddress)
    {
        startAddress = address;
    }

    if (address + secureSize - 1 > endAddress)
    {
        endAddress = address + secureSize - 1;
    }
}

bool BMemoryBuffer::IsStartEndAddressValid() const
{
    return (startAddress != std::numeric_limits<size_t>::max() &&
            endAddress != std::numeric_limits<size_t>::min());
}

std::pair<size_t, size_t> BMemoryBuffer::GetStartEndAddress() const
{
    return std::pair<size_t, size_t>(startAddress, endAddress);
}

bool BMemoryBuffer::CopyStartEndRangeTo(std::vector<Byte> &targetBuffer) const
{
    targetBuffer.clear();

    if (!IsStartEndAddressValid())
    {
        return false;
    }

    auto startIter = buffer.cbegin() + startAddress;
    auto endIter = buffer.cbegin() + endAddress + 1;

    std::copy(startIter, endIter, std::back_inserter(targetBuffer));

    return true;
}

void BMemoryBuffer::ResetStartEndAddress()
{
    startAddress = std::numeric_limits<size_t>::max();
    endAddress = std::numeric_limits<size_t>::min();
}

