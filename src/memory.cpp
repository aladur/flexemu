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

Memory::Memory(bool himem) : memory(NULL), video_ram(NULL),
    ppIo(NULL), isHiMem(himem)
{
    video_ram_size = VIDEORAM_SIZE *
                     (isHiMem ? MAXVIDEORAM_BANKS : (MAXVIDEORAM_BANKS >> 2));
    video_ram = (Byte *)new Byte[video_ram_size];
    memory_size = 0x10000;
    memory = (Byte *) new Byte[memory_size];

    init_memory(himem);
}

Memory::~Memory()
{
    uninit_memory();
    delete [] memory;
    delete [] video_ram;
    memory    = NULL;
    video_ram = NULL;
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

    for (i = 0; i < MAX_IO_DEVICES; i++)
    {
        ioDevices[i] = NULL;
    }

    io_initialized  = 0;
    io_base_addr    = 10000L;

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
        video_ram_active[i] = false;
    }
} // init_memory

void Memory::uninit_memory(void)
{
    Word i = MAX_IO_DEVICES;

    delete [] ppIo;
    ppIo = NULL;

    do
    {
        --i;

        if (ioDevices[i] != NULL && ioDevices[i] != this)
        {
            delete ioDevices[i];
        }

        ioDevices[i] = NULL;
    }
    while (i != 0);
} // uninit_memory

void Memory::init_blocks_to_update(void)
{
    short display_block;

    for (display_block = 0; display_block < YBLOCKS; display_block++)
    {
        changed[display_block] = true;
    }
}

void Memory::initialize_io_page(Word base_addr)
{
    size_t  i, range;

    io_base_addr   = base_addr & GENIO_MASK;
    range = (Word)(~GENIO_MASK) + 1;
    ppIo  = new class IoAccess[range];

    for (i = 0; i < range; i++)
    {
        (ppIo + i)->device = (IoDevice *)this;
        (ppIo + i)->addressOffset = static_cast<Word>(io_base_addr + i);
    }

    io_initialized = 1;
}

// return 0 if not successful
Byte Memory::add_io_device(IoDevice *device,
                           Word base_addr1, Byte range1,
                           Word base_addr2, Byte range2)
{
    Word i = 0;
    Word offset;

    if (!io_initialized)
    {
        return 0;    // first initialize I/O
    }

    while (i < MAX_IO_DEVICES && ioDevices[i] != NULL)
    {
        i++;
    }

    if (i == MAX_IO_DEVICES)
    {
        return 0;    // all io-devices already in use
    }

    if (base_addr1 < io_base_addr ||
        (range2 > 0 && base_addr2 < io_base_addr))
    {
        return 0;    // base addr out of io address space
    }

    if ((base_addr1 + range1 - 1 - io_base_addr) > (Word)(~GENIO_MASK))
    {
        return 0;    // base addr out of io address space
    }

    if (range2 > 0 &&
        ((base_addr2 + range2 - 1 - io_base_addr) > (Word)(~GENIO_MASK)))
    {
        return 0;    // base addr out of io address space
    }

    ioDevices[i]    = device;
    offset = base_addr1 - io_base_addr;

    for (i = 0; i < range1; i++)
    {
        (ppIo + offset + i)->device = device;
        (ppIo + offset + i)->addressOffset = i;
    }

    offset = base_addr2 - io_base_addr;

    for (i = 0; i < range2; i++)
    {
        (ppIo + offset + i)->device = device;
        (ppIo + offset + i)->addressOffset = base_addr2 - base_addr1 + i;
    }

    return 1;
}


void Memory::reset_io(void)
{
    Word i;

    for (i = 0; i < MAX_IO_DEVICES; i++)
        if (ioDevices[i] != NULL)
        {
            ioDevices[i]->resetIo();
        }
} // reset

// write Byte into ROM
void Memory::write_rom(Word offset, Byte val)
{
    memory[offset] = val;
} // write_rom

Byte Memory::readIo(Word addr)
{
    // this part is implemented twice for performance reasons
    return (Byte) * (ppage[addr >> 12] + (addr & 0x3fff));
} // readIo

void Memory::writeIo(Word addr, Byte val)
{
    Word offset;  // offset into video ram (modulo 0x4000)

    // this part is implemented twice for performance reasons
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
} // writeIo


void Memory::switch_mmu(Word offset, Byte val)
{
    //if (offset < 12)
    ppage[offset] = vram_ptrs[((offset << 2) & 0x30) | (val & 0x0f)];
    video_ram_active[offset] = ((val & 0x03) != 0x03);
} // switch_mmu

//----------------------------------------------------------------------------
// Processor loading routines
//----------------------------------------------------------------------------
Byte Memory::fread_byte(FILE *fp)
{
    char                    str[3];
    DWord                   l;

    str[0] = fgetc(fp);
    str[1] = fgetc(fp);
    str[2] = '\0';

    l = strtol(str, NULL, 16);
    return (Byte)(l & 0xff);
}

Word Memory::fread_word(FILE *fp)
{
    Word            ret;

    ret = fread_byte(fp);
    ret <<= 8;
    ret |= fread_byte(fp);

    return ret;
}

void Memory::load_intelhex(FILE *fp)
{
    Byte    done = 0;
    Byte    n, t, b;
    Word    addr;

    while (!done)
    {
        (void)fgetc(fp);
        n = fread_byte(fp);
        addr = fread_word(fp);
        t = fread_byte(fp);

        if (t == 0x00)
        {
            while (n--)
            {
                b = fread_byte(fp);
                write_rom(addr++, b);
            }
        }
        else if (t == 0x01)
        {
            //PC = addr; // to be ignored
            done = 1;
        }

        // Read and discard checksum byte
        (void)fread_byte(fp);

        if (fgetc(fp) == '\r')
        {
            (void)fgetc(fp);
        }
    }
}

void Memory::load_motorola_srec(FILE *fp)
{
    Byte    done = 0;
    Byte    n, t, b;
    Word    addr;


    while (!done)
    {
        (void)fgetc(fp); /* read 'S' */
        t = fgetc(fp);   /* read type of line */
        n = fread_byte(fp);

        switch (t)
        {
            case '0':
                n -= 1;

                while (n--)
                {
                    b = fread_byte(fp);
                };

                break;

            case '1':
                n -= 3;
                addr = fread_word(fp);

                while (n--)
                {
                    b = fread_byte(fp);
                    write_rom(addr++, b);
                };

                break;

            case '9':
                addr = fread_word(fp);
                //PC = addr; // to be ignored
                done = 1;
                break;

            default:
                done = 1;
        }

        // Read and discard checksum byte
        (void)fread_byte(fp);

        if (fgetc(fp) == '\r')
        {
            (void)fgetc(fp);
        }
    }
}

bool Memory::load_hexfile(const char *filename, bool ignore_errors)
{
    Word    ch;
    BFilePtr fp(filename, "r");

    if (fp == NULL)
    {
        if (!ignore_errors)
        {
            std::stringstream pmsg;

            pmsg << "Unable to locate or read \"" << filename
                 << "\"" << std::endl;
#ifdef _WIN32
            MessageBox(NULL, pmsg.str().c_str(),
                       PROGRAMNAME " error",
                       MB_OK | MB_ICONERROR);
#endif
#ifdef UNIX
            fprintf(stderr, "%s", pmsg.str().c_str());
#endif
        }

        return false;
    } // if

    ch = fgetc(fp);
    ungetc(ch, fp);

    if (ch == ':')
    {
        load_intelhex(fp);
    }
    else if (toupper(ch) == 'S')
    {
        load_motorola_srec(fp);
    }
    else
    {
        std::stringstream pmsg;

        pmsg << "File \"" << filename << "\" has unknown fileformat"
             << std::endl;
#ifdef _WIN32
        MessageBox(NULL, pmsg.str().c_str(), PROGRAMNAME " error",
                   MB_OK | MB_ICONERROR);
#endif
#ifdef UNIX
        fprintf(stderr, "%s", pmsg.str().c_str());
#endif
        return false;
    }

    return true;
}  // load_hexfile

