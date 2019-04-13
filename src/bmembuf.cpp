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
    endAddress(std::numeric_limits<size_t>::min()),
    currentAddress(0)
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
    currentAddress = src.currentAddress;
}

BMemoryBuffer::BMemoryBuffer(BMemoryBuffer &&src)
{
    size = src.size;
    buffer = std::move(src.buffer);
    startAddress = src.startAddress;
    endAddress = src.endAddress;
    currentAddress = src.currentAddress;
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
        currentAddress = src.currentAddress;
    }

    return *this;
}

BMemoryBuffer &BMemoryBuffer::operator= (BMemoryBuffer &&src)
{
    size = src.size;
    buffer = std::move(src.buffer);
    startAddress = src.startAddress;
    endAddress = src.endAddress;
    currentAddress = src.currentAddress;

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

void BMemoryBuffer::set_tgt_addr(size_t x_address)
{
    currentAddress = x_address;
    if ((currentAddress < size) && (currentAddress < startAddress))
    {
        startAddress = currentAddress;
    }
}

MemoryTarget<size_t> &BMemoryBuffer::operator<<(Byte value)
{
    if (currentAddress >= size)
    {
        return *this;
    }

    buffer[currentAddress] = value;

    if (currentAddress == 0 && (currentAddress < startAddress))
    {
        startAddress = currentAddress;
    }

    if (currentAddress > endAddress)
    {
        endAddress = currentAddress;
    }

    currentAddress++;

    return *this;
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

