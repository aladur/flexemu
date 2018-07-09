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


BMemoryBuffer::BMemoryBuffer(size_t aSize, size_t aBase /* = 0 */) :
    baseAddress(aBase), size(aSize)
{
    buffer = std::unique_ptr<Byte[]>(new Byte[size]);
}

BMemoryBuffer::BMemoryBuffer(const BMemoryBuffer &src)
{
    baseAddress = src.baseAddress;
    size = src.size;
    buffer = std::unique_ptr<Byte[]>(new Byte[size]);
    memcpy(buffer.get(), src.buffer.get(), size);
}

BMemoryBuffer::BMemoryBuffer(BMemoryBuffer &&src)
{
    baseAddress = src.baseAddress;
    size = src.size;
    buffer = std::move(src.buffer);
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
        buffer = std::unique_ptr<Byte[]>(new Byte[size]);
        memcpy(buffer.get(), src.buffer.get(), size);
    }

    return *this;
}

BMemoryBuffer &BMemoryBuffer::operator= (BMemoryBuffer &&src)
{
    baseAddress = src.baseAddress;
    size = src.size;
    buffer = std::move(src.buffer);

    return *this;
}

void BMemoryBuffer::FillWith(const Byte pattern /* = 0 */)
{
    for (size_t i = 0; i < size; i++)
    {
        buffer[i] = pattern;
    }
}

Byte BMemoryBuffer::operator[](size_t address) const
{
    // if out of range always return 0!
    if (address < baseAddress || address > baseAddress + size - 1)
    {
        return 0;
    }

    return buffer[address - baseAddress];
}

bool BMemoryBuffer::CopyFrom(const Byte *from, size_t aSize, size_t address)
{
    size_t secureSize;

    if (address >= baseAddress + size)
    {
        return false;
    }

    secureSize = (address + aSize > baseAddress + size) ? size -
                 (address - baseAddress) : aSize;
    memcpy(buffer.get() + address - baseAddress, from, secureSize);
    return true;
}

const Byte *BMemoryBuffer::GetBuffer(size_t address) const
{
    if (address < baseAddress || address >= baseAddress + size)
    {
        return nullptr;
    }

    return buffer.get() + address - baseAddress;
}

