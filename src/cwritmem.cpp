/*
    cwritmem.cpp


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


#include "cwritmem.h"
#include "memory.h"
#include <vector>


CWriteMemory::CWriteMemory(Memory &p_memory, Word p_address,
        const std::vector<Byte> &p_data)
    : memory(p_memory)
    , address(p_address)
    , data(p_data)
{
}

void CWriteMemory::Execute()
{
    std::lock_guard<std::mutex> guard(mutex);

    if (!data.empty())
    {
        memory.CopyFrom(data.data(), address, static_cast<DWord>(data.size()));
    }
}
