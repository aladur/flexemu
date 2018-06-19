/*
    memory.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2018  W. Schwotzer

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



#ifndef MEMORY_INCLUDED
#define MEMORY_INCLUDED

#include "misc1.h"
#include <stdio.h>
#include <tsl/robin_map.h>
#include <vector>
#include <functional>
#include "iodevice.h"
#include "ioaccess.h"
#include "memtgt.h"
#include "e2.h"

// maximum number of I/O devices which can be handled

#define MAX_VRAM        (4 * 16)

class XAbstractGui;
class Win32Gui;

class Memory : public MemoryTarget
{
    friend class XAbstractGui;
    friend class Win32Gui;

public:
    Memory(bool himem);
    virtual ~Memory();

private:
    Byte *ppage[16];
    bool isHiMem;
    int memory_size;
    int video_ram_size;
    Byte *memory;
    Byte *video_ram;
    Word target_address;

    // I/O device access
    std::vector<std::reference_wrapper<IoDevice> > ioDevices;
    tsl::robin_map<
        Word,
        IoAccess,
        std::hash<Word>,
        std::equal_to<Word>,
        std::allocator<std::pair<Word, IoAccess>>,
        false,
        tsl::rh::power_of_two_growth_policy<16>> ioAccessForAddressMap;

    // interface to video display
    Byte *vram_ptrs[MAX_VRAM];
    Word video_ram_active_bits; // 16-bit, one for each video memory page
    bool changed[YBLOCKS];

private:
    void init_memory(bool himem);
    void uninit_memory();
    static Byte initial_content[8];

    // Initialisation functions

public:

    bool add_io_device(IoDevice &device, Word base_address, int size = -1);

    // memory interface
public:
    void reset_io();
    void switch_mmu(Word offset, Byte val);
    void init_blocks_to_update();

    // memory target interface
public:
    MemoryTarget& operator<< (Byte value)
    {
        write_ram_rom(target_address++, value);
        return *this;
    }

    void set_address(Word newAddress)
    {
        target_address = newAddress;
    }

public:
    void write_ram_rom(Word address, Byte value);
    Byte read_ram_rom(Word address);
    void dump_ram_rom(Word min = 0, Word max = 0xffff);

    // The following memory Byte/Word access methods are
    // inlined for optimized performance.
    inline void write_byte(Word address, Byte value)
    {
        if ((address & GENIO_MASK) == GENIO_MASK)
        {
            auto iterator = ioAccessForAddressMap.find(address);

            if (iterator != ioAccessForAddressMap.end())
            {
                iterator.value().write(value);
                return;
            }
        }

        if (video_ram_active_bits & (1 << (address >> 12)))
        {
            changed[(address & 0x3fff) / YBLOCK_SIZE] = true;
            *(ppage[address >> 12] + (address & 0x3fff)) = value;
        }
        else
        {
            if (address < ROM_BASE)
            {
                *(ppage[address >> 12] + (address & 0x3fff)) = value;
            }
        }
    }

    inline Byte read_byte(Word address)
    {
        if ((address & GENIO_MASK) == GENIO_MASK)
        {
            auto iterator = ioAccessForAddressMap.find(address);

            if (iterator != ioAccessForAddressMap.end())
            {
                // read one Byte from memory mapped I/O device
                return iterator.value().read();
            }
        }

        if (video_ram_active_bits != 0)
        {
            // If at least one video memory page is active
            // access memory through video page memory mapping
            return *(ppage[address >> 12] + (address & 0x3fff));
        }
        else
        {
            // Otherwise directly access RAM
            return memory[address];
        }
    } // read_byte

    inline void write_word(Word address, Word value)
    {
        write_byte(address, static_cast<Byte>(value >> 8));
        write_byte(address + 1, static_cast<Byte>(value));
    }

    inline Word read_word(Word address)
    {
        Word value;

        value = static_cast<Word>(read_byte(address)) << 8;
        value |= static_cast<Word>(read_byte(address + 1));

        return value;
    }
};
#endif // MEMORY_INCLUDED

