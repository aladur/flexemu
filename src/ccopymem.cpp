/*
    ccopymem.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2025-2026  W. Schwotzer

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


#include "typedefs.h"
#include "ccopymem.h"
#include "bintervl.h"
#include "memory.h"
#include <atomic>
#include <mutex>


CReadMemory::CReadMemory(Memory &p_memory,
        const BInterval<DWord> &p_addressRange)
    : memory(p_memory)
    , addressRange(p_addressRange)
    , hasUpdate{}
{
}

void CReadMemory::Execute()
{
    std::lock_guard<std::mutex> guard(mutex);
    auto size = width(addressRange) + 1U;

    data.resize(size);
    memory.CopyTo(data.data(), addressRange.lower(), size);
    hasUpdate.store(true, std::memory_order_relaxed);
}

BInterval<DWord> CReadMemory::GetAddressRange() const
{
    return addressRange;
}

std::vector<Byte> CReadMemory::GetData() const
{
    std::lock_guard<std::mutex> guard(mutex);

    return data;
}

bool CReadMemory::HasUpdate()
{
    bool result = hasUpdate.load(std::memory_order_relaxed);

    hasUpdate.store(false, std::memory_order_relaxed);
    return result;
}
