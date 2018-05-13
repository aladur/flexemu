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



#ifndef __memory_h__
#define __memory_h__

#include "misc1.h"
#include <stdio.h>
#include "iodevice.h"
#include "e2.h"

// maximum number of I/O devices which can be handled

#define MAX_IO_DEVICES  (12)
#define MAX_VRAM        (4 * 16)

class XAbstractGui;
class Win32Gui;

struct sIoSelect
{
    IoDevice        *device;
    Word            offset;
};

class Memory : public IoDevice
{
    friend class XAbstractGui;
    friend class Win32Gui;

public:
    Memory(bool himem);
    virtual ~Memory();

    // Internal registers
private:
    Byte            *ppage[16];
    bool             video_ram_active[16];
    Byte            *memory;
    Byte            *video_ram;
    int              memory_size;
    int              video_ram_size;
    int              io_base_addr;
    int              io_initialized;
    IoDevice        *ioDevices[MAX_IO_DEVICES];
    struct sIoSelect *ppIo;

private:
    // interface to video display
    bool             isHiMem;
    Byte            *vram_ptrs[MAX_VRAM];
    bool             changed[YBLOCKS];

private:
    void    init_memory(bool himem);
    void    uninit_memory(void);
    static Byte initial_content[8];

    // Initialisation functions

public:

    void    initialize_io_page(Word base_addr);
    Byte    add_io_device(
        IoDevice *device,
        Word base_addr1, Byte range1,
        Word base_addr2, Byte range2);
    bool load_hexfile(const char *filename, bool ignore_errors = false);
private:
    void load_intelhex(FILE *fp);
    void load_motorola_srec(FILE *fp);
    static Byte fread_byte(FILE *fp);
    static Word fread_word(FILE *fp);

    // memory interface
public:
    void    reset_io(void);
    void    switch_mmu(Word offset, Byte val);
    void    init_blocks_to_update();

public:
    void            write_rom(Word addr, Byte val);
    inline void     write(Word addr, Byte val);
    void            write_word(Word addr, Word val);
    inline Byte     read(Word addr);
    Word            read_word(Word addr);

    // interface for io device protocol
    // The memory also implements the io device protocol
public:

    virtual Byte    readIo(Word addr);
    virtual void    writeIo(Word addr, Byte val);
    virtual void    resetIo() {};
    virtual const char      *getName(void)
    {
        return "memory";
    };
};

inline void Memory::write(Word addr, Byte val)
{

    if ((addr & GENIO_MASK) == io_base_addr)
    {
        struct sIoSelect *pIo = ppIo + (addr & (Word)(~GENIO_MASK));
        (pIo->device)->writeIo(pIo->offset, val);
    }
    else
    {
        Word offset;  // offset into ram (modulo 0x4000)

        offset = addr & 0x3fff;

        if (video_ram_active[addr >> 12])
        {
            changed[offset / YBLOCK_SIZE] = true;
            *(ppage[addr >> 12] + offset) = val;
        }
        else
        {
            if (addr < ROM_BASE)
            {
                *(ppage[addr >> 12] + offset) = val;
            }
        }
    } // else
} // write

inline Byte Memory::read(Word addr)
{
    // read from memory mapped I/O
    if ((addr & GENIO_MASK) == io_base_addr)
    {
        struct sIoSelect *pIo;

        pIo = ppIo + (addr & (Word)(~GENIO_MASK));
        return (pIo->device)->readIo(pIo->offset);
    }

    // otherwise read from memory
    return (Byte) * (ppage[addr >> 12] + (addr & 0x3fff));
} // read
#endif // __memory_h__

