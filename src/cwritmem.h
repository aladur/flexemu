/*
    cwritmem.h


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

#ifndef CWRITEMEMORY_INCLUDED
#define CWRITEMEMORY_INCLUDED

#include "bcommand.h"
#include "typedefs.h"
#include <vector>
#include <mutex>


class Memory;

class CWriteMemory : public BCommand
{
public:
    CWriteMemory(Memory &p_memory, Word p_address,
            const std::vector<Byte> &p_data);
    ~CWriteMemory() override = default;
    CWriteMemory(const CWriteMemory &src) = delete;
    CWriteMemory(CWriteMemory &&src) = delete;
    CWriteMemory &operator=(const CWriteMemory &src) = delete;
    CWriteMemory &operator=(CWriteMemory &&src) = delete;

    void Execute() override;

private:
    Memory &memory;
    Word address;
    std::vector<Byte> data;
    mutable std::mutex mutex;
};

using CWriteMemorySPtr = std::shared_ptr<CWriteMemory>;
#endif
