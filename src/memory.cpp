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


#include <sstream>
#include <stdio.h>
#include <ctype.h>
#include "memory.h"
#include "bfileptr.h"

Byte Memory::initial_content[8] =
{ 0x23, 0x54, 0xF1, 0xAA, 0x78, 0xD3, 0xF2, 0x0 };

Memory::Memory(bool himem) :
    isHiMem(himem),
    memory_size(0x10000),
    memory(nullptr),
    video_ram(nullptr),
    video_ram_active_bits(0)
{
    memory = (Byte *) new Byte[memory_size];
    video_ram_size = VIDEORAM_SIZE *
                     (isHiMem ? MAXVIDEORAM_BANKS : (MAXVIDEORAM_BANKS >> 2));
    video_ram = (Byte *)new Byte[video_ram_size];

    init_memory(isHiMem);
}

Memory::~Memory()
{
    uninit_memory();
    delete [] memory;
    delete [] video_ram;
    memory    = nullptr;
    video_ram = nullptr;
}

// memory must be initialized AFTER all memory mapped I/O is created
void Memory::init_memory(bool himem)
{
    int i, j;
    Byte *p;

    p = initial_content;

    for (i = 0; i < video_ram_size; i++)
    {
        if (*p == 0x00)
        {
            p = initial_content;
        }

        video_ram[i] = *(p++);
    }

    p = initial_content;

    for (i = 0; i < memory_size; i++)
    {
        if (*p == 0x00)
        {
            p = initial_content;
        }

        memory[i] = *(p++);
    }

    for (i = 0; i < YBLOCKS; i++)
    {
        changed[i] = false;
    }

    // initialize default pointer for mmu configuration
    // following table indices correspond to following RAM ranges
    // 0x0C:    green low,  Bank 0
    // 0x0D:    blue  low,  Bank 0
    // 0x0E:    red   low,  Bank 0
    // 0x04:    green high, Bank 0
    // 0x05:    blue  high, Bank 0
    // 0x06:    red   high, Bank 0
    // 0x08:    green low,  Bank 1
    // 0x09:    blue  low,  Bank 1
    // 0x0A:    red   low,  Bank 1
    // 0x00:    green high, Bank 1
    // 0x01:    blue  high, Bank 1
    // 0x02:    red   high, Bank 1
    // 0x0F, 0x07, 0x0B, 0x03 switch back to non paged memory
    i = 0;

    for (j = 0; j < 0x40; j += 0x10)
    {
        if (!himem)
        {
            i = 0;
        }

        vram_ptrs[j | 0x0c] = &video_ram[VIDEORAM_SIZE * i++];
        vram_ptrs[j | 0x0d] = &video_ram[VIDEORAM_SIZE * i++];
        vram_ptrs[j | 0x0e] = &video_ram[VIDEORAM_SIZE * i++];
        vram_ptrs[j | 0x0f] = &memory[VIDEORAM_SIZE * (j >> 4)];
        vram_ptrs[j | 0x04] = &video_ram[VIDEORAM_SIZE * i++];
        vram_ptrs[j | 0x05] = &video_ram[VIDEORAM_SIZE * i++];
        vram_ptrs[j | 0x06] = &video_ram[VIDEORAM_SIZE * i++];
        vram_ptrs[j | 0x07] = &memory[VIDEORAM_SIZE * (j >> 4)];
        vram_ptrs[j | 0x08] = &video_ram[VIDEORAM_SIZE * i++];
        vram_ptrs[j | 0x09] = &video_ram[VIDEORAM_SIZE * i++];
        vram_ptrs[j | 0x0a] = &video_ram[VIDEORAM_SIZE * i++];
        vram_ptrs[j | 0x0b] = &memory[VIDEORAM_SIZE * (j >> 4)];
        vram_ptrs[j | 0x00] = &video_ram[VIDEORAM_SIZE * i++];
        vram_ptrs[j | 0x01] = &video_ram[VIDEORAM_SIZE * i++];
        vram_ptrs[j | 0x02] = &video_ram[VIDEORAM_SIZE * i++];
        vram_ptrs[j | 0x03] = &memory[VIDEORAM_SIZE * (j >> 4)];
    }

    if (i != (himem ? MAXVIDEORAM_BANKS : MAXVIDEORAM_BANKS >> 2))
    {
        fprintf(stderr, "Memory management initialization failure (i=%d)\n", i);
    }

    // initialize mmu pointers
    for (i = 0; i < 16; i++)
    {
        ppage[i] = &memory[VIDEORAM_SIZE * (i >> 2)];
    }
    video_ram_active_bits = 0;
} // init_memory

void Memory::uninit_memory()
{
    ioDevices.clear();
}

void Memory::init_blocks_to_update()
{
    short display_block;

    for (display_block = 0; display_block < YBLOCKS; display_block++)
    {
        changed[display_block] = true;
    }
}

// Add an I/O device to the address space
// device       The device to be added
// base_address The base address of the device
// size         The I/O address size. Default value: -1.
//              If -1 the size of the I/O device is used (method sizeOfIo() )
//
// return false if not successful
bool Memory::add_io_device(
        IoDevice &device,
        Word base_address,
        int size /* = -1 */)
{
    Word offset;

    if (size < 0)
    {
        size = device.sizeOfIo();
    }

    if (base_address < GENIO_BASE || ((int)base_address + size > 0xffff))
    {
        return false;
    }

    ioDevices.push_back(std::ref(device));

    for (offset = 0; offset < size; ++offset)
    {
        ioAccessForAddressMap.emplace(base_address + offset,
                                    IoAccess(device, offset));
    }

    return true;
}

void Memory::reset_io()
{
    for (auto &iter : ioDevices)
    {
        iter.get().resetIo();
    }
}

// Write Byte into RAM or ROM independent of MMU.
void Memory::write_ram_rom(Word offset, Byte value)
{
    memory[offset] = value;
} // write_ram_rom

// Read Byte from RAM or ROM independent of MMU.
Byte Memory::read_ram_rom(Word offset)
{
    return memory[offset];
} // read_ram_rom

void Memory::switch_mmu(Word offset, Byte val)
{
    ppage[offset] = vram_ptrs[((offset << 2) & 0x30) | (val & 0x0f)];
    if ((val & 0x03) != 0x03)
    {
        video_ram_active_bits |= (1 << offset);
    }
    else
    {
        video_ram_active_bits &= ~(1 << offset);
    }
} // switch_mmu

void Memory::dump_ram_rom(Word min, Word max)
{
    Word address = min;
    Byte value;
    Byte padding;

    padding = 3 * (address % 16);
    printf("%04X ", address);
    if (padding)
    {
        printf("%*c", padding, ' ');
    }

    while (true)
    {
        value = read_ram_rom(address);
        printf(" %02X", value);
        if (address == max)
        {
            printf("\n");
            return;
        }
        if (address % 16 == 15)
        {
            printf("\n%04X ", address + 1);
        }
        ++address;
    }
}

