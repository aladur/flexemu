/*
    ccopymem.h


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

#ifndef CCOPYMEM_INCLUDED
#define CCOPYMEM_INCLUDED

#include "typedefs.h"
#include "bcommand.h"
#include "bintervl.h"
#include <atomic>
#include <mutex>
#include <vector>


class Memory;

class CReadMemory : public BCommand
{

public:
    CReadMemory(Memory &p_memory, const BInterval<DWord> &p_addressRange);
    ~CReadMemory() override = default;
    CReadMemory(const CReadMemory &src) = delete;
    CReadMemory(CReadMemory &&src) = delete;
    CReadMemory &operator=(const CReadMemory &src) = delete;
    CReadMemory &operator=(CReadMemory &&src) = delete;
    void Execute() override;

    BInterval<DWord> GetAddressRange() const;
    std::vector<Byte> GetData() const;
    bool HasUpdate();

protected:
    Memory &memory;
    BInterval<DWord> addressRange;
    std::vector<Byte> data;
    std::atomic<bool> hasUpdate;
    mutable std::mutex mutex;
};

using CReadMemorySPtr = std::shared_ptr<CReadMemory>;

#endif
