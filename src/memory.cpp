/*
    memory.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2025  W. Schwotzer

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
#include "memory.h"
#include "bintervl.h"
#include "fcnffile.h"
#include "soptions.h"
#include "free.h"
#include <cstring>
#include <cassert>
#include <algorithm>
#include "warnoff.h"
#include <fmt/format.h>
#include "warnon.h"

std::array<Byte, 8> Memory::initial_content =
{ 0x23, 0x54, 0xF1, 0xAA, 0x78, 0xD3, 0xF2, 0x0 };

Memory::Memory(const struct sOptions &options) :
    isRamExtension(options.isRamExtension),
    isHiMem(options.isHiMem),
    isFlexibleMmu(options.isFlexibleMmu),
    isEurocom2V5(options.isEurocom2V5),
    deviceAccess(0x10000U - genio_base, ioDeviceAccess{NO_DEVICE, 0U})

{
    // Eurocom II/V5 has 48 KByte memory on mainboard. Never the less
    // allocate 64 KByte to map the Boot ROM into F000 - FFFF.
    memory.resize(memory_size);
    if (isRamExtension)
    {
        video_ram_size = VIDEORAM_SIZE *
                     (isHiMem ? MAXVIDEORAM_BANKS : (MAXVIDEORAM_BANKS >> 2U));
        video_ram.resize(video_ram_size);
    }

    init_memory();
    init_blocks_to_update();

    if (isEurocom2V5)
    {
        // 48 KByte mainboard memory + 4KByte BOOT rom.
        addressRanges.emplace_back(0x0000U, 0xBFFFU);
        addressRanges.emplace_back(0xF000U, 0xFFFFU);
    }
    else
    {
        // 60 KByte mainboard memory + 4KByte BOOT rom.
        addressRanges.emplace_back(0x0000U, 0xFFFFU);
    }
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

    p = initial_content.data();

    for (i = 0; i < video_ram_size; i++)
    {
        if (*p == 0x00)
        {
            p = initial_content.data();
        }

        video_ram[i] = *(p++);
    }

    p = initial_content.data();

    for (i = 0; i < memory_size; i++)
    {
        if (*p == 0x00)
        {
            p = initial_content.data();
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

            init_vram_ptr(j | 0x0CU, &video_ram[VIDEORAM_SIZE * i++]);
            init_vram_ptr(j | 0x0DU, &video_ram[VIDEORAM_SIZE * i++]);
            init_vram_ptr(j | 0x0EU, &video_ram[VIDEORAM_SIZE * i++]);
            init_vram_ptr(j | 0x0FU, &memory[VIDEORAM_SIZE * (j >> 4U)]);
            init_vram_ptr(j | 0x04U, &video_ram[VIDEORAM_SIZE * i++]);
            init_vram_ptr(j | 0x05U, &video_ram[VIDEORAM_SIZE * i++]);
            init_vram_ptr(j | 0x06U, &video_ram[VIDEORAM_SIZE * i++]);
            init_vram_ptr(j | 0x07U, &memory[VIDEORAM_SIZE * (j >> 4U)]);
            init_vram_ptr(j | 0x08U, &video_ram[VIDEORAM_SIZE * i++]);
            init_vram_ptr(j | 0x09U, &video_ram[VIDEORAM_SIZE * i++]);
            init_vram_ptr(j | 0x0AU, &video_ram[VIDEORAM_SIZE * i++]);
            init_vram_ptr(j | 0x0BU, &memory[VIDEORAM_SIZE * (j >> 4U)]);
            init_vram_ptr(j | 0x00U, &video_ram[VIDEORAM_SIZE * i++]);
            init_vram_ptr(j | 0x01U, &video_ram[VIDEORAM_SIZE * i++]);
            init_vram_ptr(j | 0x02U, &video_ram[VIDEORAM_SIZE * i++]);
            init_vram_ptr(j | 0x03U, &memory[VIDEORAM_SIZE * (j >> 4U)]);
        }

        if (i != (isHiMem ? MAXVIDEORAM_BANKS : MAXVIDEORAM_BANKS / 4U))
        {
            std::cerr << "Memory management initialization failure (i="
                      << i << ")\n";
        }
    }

    // initialize mmu pointers
    for (i = 0; i < 16; i++)
    {
        ppage[i] = &memory[VIDEORAM_SIZE * (i >> 2U)];
    }

    if (isEurocom2V5)
    {
        // Eurocom II V5 only has 48 KByte on mainboard.
        // Accessing memory range C000 - EFFF is a mirror of 8000 - AFFF.
        for (i = 12; i < 15; i++)
        {
            ppage[i] = &memory[VIDEORAM_SIZE * ((i - 4U) >> 2U)];
        }
    }

    video_ram_active_bits = 0;
}

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
        const auto path(flx::getFlexemuConfigFile().u8string());
        FlexemuConfigFile configFile(path);
        const auto value = configFile.GetDebugSupportOption("presetRAM");

        do_preset_ram = (value == "1") ? 1 : 0;
    }

    if (do_preset_ram == 1)
    {
        std::memset(ram_ptr, vram_ptr_index, VIDEORAM_SIZE);
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
        std::optional<Word> size)
{
    Word sizeOfIo = device.sizeOfIo();

    if (!size.has_value())
    {
        size = sizeOfIo;
    }

    if (static_cast<int>(base_address) + size.value() > 0xffff)
    {
        return false;
    }

    struct ioDeviceProperties properties{
            device.getName(),
            base_address,
            size.value()};
    devicesProperties.emplace_back(properties);
    devicesPropertiesSorted = false;

    if (base_address < genio_base)
    {
        // Increase deviceAccess vector for lower addresses down to
        // base_address.
        deviceAccess.insert(deviceAccess.begin(),
                genio_base - base_address,
                ioDeviceAccess{NO_DEVICE, 0U});
        genio_base = base_address;
        assert((0x10000U - genio_base) == deviceAccess.size());
    }

    auto deviceIndex = static_cast<Byte>(ioDevices.size());
    if (deviceIndex == NO_DEVICE)
    {
        return false; // No more I/O devices allowed.
    }
    ioDevices.push_back(std::ref(device));

    // To access a device store the device index and it's byte offset
    // in a vector. The vector index is the address - genio_base.
    // If device index contains NO_DEVICE there is no memory mapped device
    // at this address location.
    for (Word offset = 0; offset < size.value(); ++offset)
    {
        auto byteOffset = static_cast<Word>(offset % sizeOfIo);
        ioDeviceAccess access{ deviceIndex, byteOffset };

        deviceAccess[base_address + offset - genio_base] = access;
    }

    return true;
}

void Memory::reset_io()
{
    for (auto deviceRef : ioDevices)
    {
        deviceRef.get().resetIo();
    }
    sort_devices_properties();
}

// Write Byte into RAM or ROM independent of MMU.
void Memory::write_ram_rom(Word address, Byte value)
{
    memory[address] = value;
}

// Read Byte from RAM or ROM independent of MMU.
Byte Memory::read_ram_rom(Word address)
{
    return memory[address];
}

void Memory::switch_mmu(Word offset, Byte val)
{
    Word ppage_index = 0U;

    if ((val & 0x03U) != 0x03U)
    {
        if (isHiMem && isFlexibleMmu)
        {
            ppage_index = val & 0x3FU;
        }
        else
        {
            ppage_index = (static_cast<DWord>(offset << 2U) & 0x30U) |
                (val & 0x0FU);
        }
        video_ram_active_bits |= (1U << offset);
    }
    else
    {
        ppage_index = (static_cast<DWord>(offset << 2U) & 0x30U) |
            (val & 0x0FU);
        video_ram_active_bits &= ~(1U << offset);
    }

    ppage[offset] = vram_ptrs[ppage_index];
}

void Memory::dump_ram_rom(std::ostream &os, Word min, Word max)
{
    Word address = min;
    Byte value;
    Byte padding;

    padding = 3 * (address % 16);
    os << fmt::format("{:04X} ", address);
    if (padding)
    {
        os << fmt::format("{0:{1}}", ' ', padding);
    }

    while (true)
    {
        value = read_ram_rom(address);
        os << fmt::format(" {:02X}", value);
        if (address == max)
        {
            os << "\n";
            return;
        }
        if (address % 16 == 15)
        {
            os << fmt::format("\n{:04X} ", address + 1);
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

void Memory::CopyFrom(const Byte *source, DWord address, DWord size)
{
    DWord secureSize = size;

    if (address >= memory_size)
    {
        throw std::out_of_range("address is out of valid range");
    }

    if (address + secureSize >= memory_size)
    {
        secureSize -= address + size - memory_size;
    }

    std::memcpy(memory.data() + address, source, secureSize);
}

void Memory::CopyTo(Byte *target, DWord address, DWord size) const
{
    BInterval<DWord> copyRange(address, address + size - 1U);

    if (!flx::is_range_in_ranges(copyRange, addressRanges))
    {
        throw std::out_of_range("address is out of valid range");
    }

    std::memcpy(target, memory.data() + address, size);
}

const MemorySource<DWord>::AddressRanges& Memory::GetAddressRanges() const
{
    return addressRanges;
}

unsigned Memory::get_ram_size() const
{
    return isEurocom2V5 ? 48U : memory_size / 1024U;
}

unsigned Memory::get_ram_extension_boards() const
{
    return isRamExtension ? 2U : 0U;
}

unsigned Memory::get_ram_extension_size() const
{
    return video_ram_size / 1024U;
}

DevicesProperties_t Memory::get_devices_properties() const
{
    return devicesProperties;
}

void Memory::sort_devices_properties()
{
    if (!devicesPropertiesSorted)
    {
        std::sort(devicesProperties.begin(), devicesProperties.end(), [](
                    const ioDeviceProperties &lhs,
                    const ioDeviceProperties &rhs)
                {
                    return lhs.baseAddress < rhs.baseAddress;
                });
        devicesPropertiesSorted = true;
    }
}
