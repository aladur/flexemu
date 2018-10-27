/*
    bmembuf.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 2003-2018  W. Schwotzer

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


BMemoryBuffer::BMemoryBuffer(size_t aSize, size_t aBase /* = 0 */) :
    baseAddress(aBase),
    size(aSize),
    startAddress(std::numeric_limits<size_t>::max()),
    endAddress(std::numeric_limits<size_t>::min()),
    currentAddress(aBase)
{
    buffer.resize(size, 0);
}

BMemoryBuffer::BMemoryBuffer(const BMemoryBuffer &src)
{
    baseAddress = src.baseAddress;
    size = src.size;
    std::copy(src.buffer.cbegin(), src.buffer.cend(),
              std::back_inserter(buffer));
    startAddress = src.startAddress;
    endAddress = src.endAddress;
    currentAddress = src.currentAddress;
}

BMemoryBuffer::BMemoryBuffer(BMemoryBuffer &&src)
{
    baseAddress = src.baseAddress;
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
        baseAddress = src.baseAddress;
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
    baseAddress = src.baseAddress;
    size = src.size;
    buffer = std::move(src.buffer);
    startAddress = src.startAddress;
    endAddress = src.endAddress;
    currentAddress = src.currentAddress;

    return *this;
}

void BMemoryBuffer::FillWith(const Byte pattern /* = 0 */)
{
    std::fill(buffer.begin(), buffer.end(), pattern);
}

Byte &BMemoryBuffer::operator[](size_t address)
{
    // if out of range throw an exception (fail early).
    if (address < baseAddress || address > baseAddress + size - 1)
    {
        throw std::out_of_range(
                "BMemoryBuffer::operator[] address out of range");
    }

    return *&buffer[address - baseAddress];
}

const Byte &BMemoryBuffer::operator[](size_t address) const
{
    // if out of range throw an exception (fail early).
    if (address < baseAddress || address > baseAddress + size - 1)
    {
        throw std::out_of_range(
                "BMemoryBuffer::operator[] address out of range");
    }

    return *&buffer[address - baseAddress];
}

bool BMemoryBuffer::CopyFrom(const Byte *from, size_t aSize, size_t address)
{
    size_t secureSize = aSize;
    size_t secureAddress = address;

    // Check if total address range is out of buffer address range.
    if ((address >= baseAddress + size) || (address + aSize <= baseAddress))
    {
        return false;
    }

    if (address < baseAddress)
    {
        secureAddress = baseAddress;
        secureSize -= baseAddress - address;
    }

    if (secureAddress + secureSize > baseAddress + size)
    {
        secureSize -= secureAddress + aSize - (baseAddress + size);
    }

    memcpy(&buffer[secureAddress - baseAddress], from, secureSize);

    return true;
}

size_t BMemoryBuffer::reset_src_addr()
{
    sourceAddress = startAddress;
    return sourceAddress;
}

bool BMemoryBuffer::src_at_end() const
{
    return (sourceAddress >= baseAddress + size ||
            sourceAddress > endAddress);
}

MemorySource<size_t> &BMemoryBuffer::operator>>(Byte &value)
{
    if (sourceAddress >= baseAddress)
    {
        if (sourceAddress >= baseAddress + size ||
            sourceAddress > endAddress)
        {
            return *this;
        }

        value = buffer[sourceAddress - baseAddress];
    }

    sourceAddress++;

    return *this;
}

void BMemoryBuffer::set_tgt_addr(size_t x_address)
{
    currentAddress = x_address;
    if (currentAddress >= baseAddress &&
        (currentAddress < baseAddress + size) &&
        (currentAddress < startAddress))
    {
        startAddress = currentAddress;
    }
}

MemoryTarget<size_t> &BMemoryBuffer::operator<<(Byte value)
{
    if (currentAddress >= baseAddress)
    {
        if (currentAddress >= baseAddress + size)
        {
            return *this;
        }

        buffer[currentAddress - baseAddress] = value;

        if (currentAddress == baseAddress && (currentAddress < startAddress))
        {
            startAddress = currentAddress;
        }

        if (currentAddress > endAddress)
        {
            endAddress = currentAddress;
        }
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

    auto startIter = buffer.cbegin() + startAddress - baseAddress;
    auto endIter = buffer.cbegin() + endAddress + 1 - baseAddress;

    std::copy(startIter, endIter, std::back_inserter(targetBuffer));

    return true;
}

void BMemoryBuffer::ResetStartEndAddress()
{
    startAddress = std::numeric_limits<size_t>::max();
    endAddress = std::numeric_limits<size_t>::min();
}

