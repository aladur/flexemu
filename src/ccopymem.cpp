/*
    ccopymem.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2025  W. Schwotzer

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


#include "ccopymem.h"
#include "memory.h"
#include <atomic>
#include <mutex>


CCopyMemory::CCopyMemory(Memory &p_memory,
        const BInterval<DWord> &addressRange)
    : memory(p_memory)
    , start(static_cast<Word>(addressRange.lower()))
    , size(static_cast<Word>(width(addressRange) + 1U))
    , hasUpdate{}
{
}

void CCopyMemory::Execute()
{
    std::lock_guard<std::mutex> guard(mutex);

    data.resize(size);
    memory.CopyTo(data.data(), start, static_cast<DWord>(data.size()));
    hasUpdate.store(true, std::memory_order_relaxed);
}

Word CCopyMemory::GetStartAddress() const
{
    return start;
}

Word CCopyMemory::GetSize() const
{
    return size;
}

std::vector<Byte> CCopyMemory::GetData() const
{
    std::lock_guard<std::mutex> guard(mutex);

    return data;
}

bool CCopyMemory::HasUpdate()
{
    bool result = hasUpdate.load(std::memory_order_relaxed);

    hasUpdate.store(false, std::memory_order_relaxed);
    return result;
}
