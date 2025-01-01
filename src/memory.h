/*
    memory.h


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



#ifndef MEMORY_INCLUDED
#define MEMORY_INCLUDED

#include "misc1.h"
#include "warnoff.h"
#include "warnon.h"
#include <functional>
#include <memory>
#include <vector>
#include "iodevice.h"
#include "memtgt.h"
#include "e2.h"
#include "bobserv.h"
#include <array>
#include <iostream>

// Maximum number of video RAM pointers supported.
// Each video RAM page has as size of 16KByte.

enum : uint8_t {
MAX_VRAM = (4 * 16),
};

struct sOptions;

struct ioDeviceAccess
{
    Byte deviceIndex{0};
    Byte addressOffset{0};
};

class Memory : public MemoryTarget<DWord>, public BObserver
{
public:
    explicit Memory(const struct sOptions &options);
    ~Memory() override;

private:
    std::array<Byte *, 16> ppage{};
    bool isRamExtension{false};
    bool isHiMem{false};
    bool isFlexibleMmu{false};
    bool isEurocom2V5{false}; // Emulate an Eurocom II/V5
                              // (instead of Eurocom II/V7)
    DWord memory_size{0x10000};
    DWord video_ram_size{0};
    Byte ramBank{0};
    std::vector<Byte> memory;
    std::vector<Byte> video_ram;

    // I/O device access
    std::vector<std::reference_wrapper<IoDevice> > ioDevices;
    std::vector<ioDeviceAccess> deviceAccess;
    static const Byte NO_DEVICE = 0xFF;

    // interface to video display
    std::array<Byte *, MAX_VRAM> vram_ptrs{};
    Word video_ram_active_bits{0}; // 16-bit, one for each video memory page
    std::array<bool, YBLOCKS> changed{};
    static std::array<Byte, 8> initial_content;

private:
    void init_memory();
    void init_vram_ptr(Byte vram_ptr_index, Byte *ram_ptr);

    // Initialisation functions

public:

    bool add_io_device(IoDevice &device, Word base_address, int size = -1);

    // memory interface
public:
    void reset_io();
    void switch_mmu(Word offset, Byte val);
    void init_blocks_to_update();

    // BObserver interface
public:
    void UpdateFrom(NotifyId id, void *param = nullptr) override;

    // memory target interface
public:
    void CopyFrom(const Byte *source, DWord address, DWord size) override;

public:
    void write_ram_rom(Word address, Byte value);
    Byte read_ram_rom(Word address);
    void dump_ram_rom(std::ostream &os, Word min, Word max);

    // The following memory Byte/Word access methods are
    // inlined for optimized performance.
    inline void write_byte(Word address, Byte value)
    {
        if (address >= GENIO_BASE)
        {
            auto access = deviceAccess[address - GENIO_BASE];

            if (access.deviceIndex != NO_DEVICE)
            {
                auto offset = access.addressOffset;

                // Write one Byte to memory mapped I/O device.
                ioDevices[access.deviceIndex].get().writeIo(offset, value);
                return;
            }
        }

        if (video_ram_active_bits &
                (1U << (static_cast<unsigned>(address) >> 12U)))
        {
            changed[(address & 0x3FFFU) / YBLOCK_SIZE] = true;
            *(ppage[address >> 12U] + (address & 0x3FFFU)) = value;
        }
        else
        {
            if (address < ROM_BASE)
            {
                // Use paged memory access to be able to mirror
                // RAM banks (e.g. for Eurocom V5).
                *(ppage[address >> 12U] + (address & 0x3FFFU)) = value;
                if (!isRamExtension && ((ramBank & 0x03U) != 3U) &&
                    ((address / 16384U) == (ramBank & 0x03U)))
                {
                    changed[(address & 0x3FFFU) / YBLOCK_SIZE] = true;
                }
            }
        }
    }

    inline Byte read_byte(Word address)
    {
        if (address >= GENIO_BASE)
        {
            auto access = deviceAccess[address - GENIO_BASE];

            if (access.deviceIndex != NO_DEVICE)
            {
                auto offset = access.addressOffset;

                // Read one Byte from memory mapped I/O device.
                return ioDevices[access.deviceIndex].get().readIo(offset);
            }
        }

        return *(ppage[address >> 12U] + (address & 0x3FFFU));
    }

    inline void write_word(Word address, Word value)
    {
        write_byte(address, static_cast<Byte>(value >> 8U));
        write_byte(address + 1, static_cast<Byte>(value));
    }

    inline Word read_word(Word address)
    {
        Word value;

        value = static_cast<Word>(read_byte(address)) << 8U;
        value |= static_cast<Word>(read_byte(address + 1));

        return value;
    }

    inline bool has_changed(int block_number) const
    {
        return changed[block_number];
    }

    inline void reset_changed(int block_number)
    {
        changed[block_number] = false;
    }

    // Get read-only access to video RAM.
    // This can be used by the GUI to update the video display.
    inline Byte const *get_video_ram(Byte bank, int block_number) const
    {
        if (isRamExtension)
        {
            if ((bank & 0x01U) == 1U)
            {
                return vram_ptrs[0x08] + block_number * YBLOCK_SIZE;
            }

            return vram_ptrs[0x0C] + block_number * YBLOCK_SIZE;
        }

        auto offset = (bank & 0x03U) * VIDEORAM_SIZE;

        return &memory[offset] + block_number * YBLOCK_SIZE;
    }

    inline bool is_video_bank_valid(Byte bank) const
    {
        if (isRamExtension)
        {
            return (bank & 0x02U) == 0U;
        }

        return (bank & 0x3U) != 3U;
    }
};
#endif // MEMORY_INCLUDED

