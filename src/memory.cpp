/*
    memory.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2021  W. Schwotzer

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
#include "fcnffile.h"
#include "soptions.h"

Byte Memory::initial_content[8] =
{ 0x23, 0x54, 0xF1, 0xAA, 0x78, 0xD3, 0xF2, 0x0 };

Memory::Memory(const struct sOptions &options) :
    isRamExtension(options.isRamExtension),
    isHiMem(options.isHiMem),
    isFlexibleMmu(options.isFlexibleMmu),
    isEurocom2V5(options.isEurocom2V5),
    memory_size(0x10000),
    video_ram_size(0),
    ramBank(0),
    video_ram_active_bits(0)
{
    memory = std::unique_ptr<Byte[]>(new Byte[memory_size]);
    if (isRamExtension)
    {
        video_ram_size = VIDEORAM_SIZE *
                     (isHiMem ? MAXVIDEORAM_BANKS : (MAXVIDEORAM_BANKS >> 2));
        video_ram = std::unique_ptr<Byte[]>(new Byte[video_ram_size]);
    }

    init_memory();
    init_blocks_to_update();
}

Memory::~Memory()
{
    ioDevices.clear();
}

// memory must be initialized AFTER all memory mapped I/O is created
void Memory::init_memory()
{
    DWord i;
    Byte j;
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
    if (isRamExtension)
    {
        i = 0;

        for (j = 0; j < 0x40; j += 0x10)
        {
            if (!isHiMem)
            {
                i = 0;
            }

            init_vram_ptr(j | 0x0c, &video_ram[VIDEORAM_SIZE * i++]);
            init_vram_ptr(j | 0x0d, &video_ram[VIDEORAM_SIZE * i++]);
            init_vram_ptr(j | 0x0e, &video_ram[VIDEORAM_SIZE * i++]);
            init_vram_ptr(j | 0x0f, &memory[VIDEORAM_SIZE * (j >> 4)]);
            init_vram_ptr(j | 0x04, &video_ram[VIDEORAM_SIZE * i++]);
            init_vram_ptr(j | 0x05, &video_ram[VIDEORAM_SIZE * i++]);
            init_vram_ptr(j | 0x06, &video_ram[VIDEORAM_SIZE * i++]);
            init_vram_ptr(j | 0x07, &memory[VIDEORAM_SIZE * (j >> 4)]);
            init_vram_ptr(j | 0x08, &video_ram[VIDEORAM_SIZE * i++]);
            init_vram_ptr(j | 0x09, &video_ram[VIDEORAM_SIZE * i++]);
            init_vram_ptr(j | 0x0a, &video_ram[VIDEORAM_SIZE * i++]);
            init_vram_ptr(j | 0x0b, &memory[VIDEORAM_SIZE * (j >> 4)]);
            init_vram_ptr(j | 0x00, &video_ram[VIDEORAM_SIZE * i++]);
            init_vram_ptr(j | 0x01, &video_ram[VIDEORAM_SIZE * i++]);
            init_vram_ptr(j | 0x02, &video_ram[VIDEORAM_SIZE * i++]);
            init_vram_ptr(j | 0x03, &memory[VIDEORAM_SIZE * (j >> 4)]);
        }

        if (i != (isHiMem ? MAXVIDEORAM_BANKS : MAXVIDEORAM_BANKS >> 2))
        {
            fprintf(stderr,
                    "Memory management initialization failure (i=%u)\n", i);
        }
    }

    // initialize mmu pointers
    for (i = 0; i < 16; i++)
    {
        ppage[i] = &memory[VIDEORAM_SIZE * (i >> 2)];
    }

    if (isEurocom2V5)
    {
        for (i = 12; i < 15; i++)
        {
            ppage[i] = &memory[VIDEORAM_SIZE * ((i - 4) >> 2)];
        }
    }

    video_ram_active_bits = 0;
} // init_memory

void Memory::init_vram_ptr(Byte vram_ptr_index, Byte *ram_ptr)
{
    static int do_preset_ram = -1;

    vram_ptrs[vram_ptr_index] = ram_ptr;

    // Check for debug option 'presetRAM'. If set preset RAM with value.
    // This may be helpfull when debugging the memory management unit (MMU).
    // Please remember that most monitor programs initialize the base and
    // extended RAM to 0x00.
    if (do_preset_ram < 0)
    {
        FlexemuConfigFile configFile(getFlexemuSystemConfigFile().c_str());
        const auto value = configFile.GetDebugSupportOption("presetRAM");

        do_preset_ram = (value == "1") ? 1 : 0;
    }

    if (do_preset_ram == 1)
    {
        memset(ram_ptr, vram_ptr_index, VIDEORAM_SIZE);
    }
}

// init_blocks_to_update
//
// Mark all display block as dirty. This refreshes the whole
// video display.
// This may happen if e.g. vico1 or vico2 has been changed.

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
    Word sizeOfIo = device.sizeOfIo();

    if (size < 0)
    {
        size = sizeOfIo;
    }

    if (base_address < GENIO_BASE || ((int)base_address + size > 0xffff))
    {
        return false;
    }

    ioDevices.push_back(std::ref(device));

    for (offset = 0; offset < size; ++offset)
    {
        ioAccessForAddressMap.emplace(
            std::piecewise_construct,
            std::forward_as_tuple<Word>(base_address + offset),
            std::forward_as_tuple<IoDevice &, Word>(device, offset % sizeOfIo));
    }

    return true;
}

void Memory::reset_io()
{
    for (auto deviceRef : ioDevices)
    {
        deviceRef.get().resetIo();
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
    int ppage_index = 0;

    if ((val & 0x03) != 0x03)
    {
        if (isHiMem && isFlexibleMmu)
        {
            ppage_index = val & 0x3f;
        }
        else
        {
            ppage_index = ((offset << 2) & 0x30) | (val & 0x0f);
        }
        video_ram_active_bits |= (1 << offset);
    }
    else
    {
        ppage_index = ((offset << 2) & 0x30) | (val & 0x0f);
        video_ram_active_bits &= ~(1 << offset);
    }

    ppage[offset] = vram_ptrs[ppage_index];
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

void Memory::UpdateFrom(NotifyId id, void *param)
{
    if (id == NotifyId::RequestScreenUpdate)
    {
        init_blocks_to_update();
    }
    else if (id == NotifyId::VideoRamBankChanged)
    {
        ramBank = *static_cast<Byte *>(param);
        init_blocks_to_update();
    }
}

void Memory::CopyFrom(const Byte *buffer, size_t address, size_t aSize)
{
    size_t secureSize = aSize;

    if (address >= memory_size)
    {
        throw std::out_of_range("address is out of valid range");
    }

    if (address + secureSize >= memory_size)
    {
        secureSize -= address + aSize - memory_size;
    }

    memcpy(memory.get() + address, buffer, secureSize);
}

