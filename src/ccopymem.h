/*
    ccopymem.h


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

#ifndef CCOPYMEM_INCLUDED
#define CCOPYMEM_INCLUDED

#include "bcommand.h"
#include "bintervl.h"
#include "typedefs.h"
#include <vector>


class Memory;

class CCopyMemory : public BCommand
{

public:
    CCopyMemory(Memory &p_memory, const BInterval<DWord> &p_addressRange);
    ~CCopyMemory() override = default;
    CCopyMemory(const CCopyMemory &src) = delete;
    CCopyMemory(CCopyMemory &&src) = delete;
    CCopyMemory &operator=(const CCopyMemory &src) = delete;
    CCopyMemory &operator=(CCopyMemory &&src) = delete;
    void Execute() override;

    Word GetStartAddress() const;
    Word GetSize() const;
    const std::vector<Byte> &GetData() const;

protected:
    Memory &memory;
    Word start;
    Word size;
    std::vector<Byte> data;
};

using CCopyMemorySPtr = std::shared_ptr<CCopyMemory>;

#endif
