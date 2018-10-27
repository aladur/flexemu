/*
    fileread.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2018  W. Schwotzer

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
#include "fileread.h"
#include "bfileptr.h"


static Byte fread_byte(FILE *fp)
{
    char str[3];
    DWord value;

    str[0] = static_cast<char>(fgetc(fp));
    str[1] = static_cast<char>(fgetc(fp));
    str[2] = '\0';

    value = strtol(str, nullptr, 16);
    return (Byte)(value & 0xff);
}

static Word fread_word(FILE *fp)
{
    Word ret;

    ret = fread_byte(fp);
    ret <<= 8;
    ret |= fread_byte(fp);

    return ret;
}

static void load_intelhex(FILE *fp, MemoryTarget<size_t> &memtgt)
{
    bool done = false;
    Byte count, type, value;
    Word address;

    while (!done)
    {
        (void)fgetc(fp);
        count = fread_byte(fp);
        address = fread_word(fp);
        type = fread_byte(fp);
        memtgt.set_tgt_addr(address);

        if (type == 0x00)
        {
            while (count--)
            {
                value = fread_byte(fp);
                memtgt << value;
            }
        }
        else if (type == 0x01)
        {
            done = true;
        }

        // Read and discard checksum byte
        (void)fread_byte(fp);

        if (fgetc(fp) == '\r')
        {
            (void)fgetc(fp);
        }
    }
}

static void load_motorola_srec(FILE *fp, MemoryTarget<size_t> &memtgt)
{
    Byte done = 0;
    Byte count, type, value;
    Word address;

    while (!done)
    {
        (void)fgetc(fp); /* read 'S' */
        type = static_cast<char>(fgetc(fp));   /* read type of line */
        count = fread_byte(fp);

        switch (type)
        {
            case '0':
                count -= 1;

                while (count--)
                {
                    value = fread_byte(fp);
                };

                break;

            case '1':
                count -= 3;
                address = fread_word(fp);
                memtgt.set_tgt_addr(address);

                while (count--)
                {
                    value = fread_byte(fp);
                    memtgt << value;
                };

                break;

            case '9':
                address = fread_word(fp);
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

void load_flex_binary(FILE *fp, MemoryTarget<size_t> &memtgt)
{
    int value;
    Word address = 0;
    size_t count = 0;

    while ((value = fgetc(fp)) != EOF)
    {
        if (count == 0)
        {
            if (value == 0x02)
            {
                // Read address and byte count.
                address = (fgetc(fp) & 0xff) << 8;
                address |= fgetc(fp) & 0xff;
                count = fgetc(fp) & 0xff;
                memtgt.set_tgt_addr(address);
            } else if (value == 0x16)
            {
                // Read start address and ignore it.
                Word temp = fread_word(fp);
                (void)temp;
                return;
            } else
            {
                return;
            }
            // Read next byte.
            value = fgetc(fp);
        }
        memtgt << (Byte)value;
        --count;
    }
}

int load_hexfile(const char *filename, MemoryTarget<size_t> &memtgt)
{
    Word ch;
    BFilePtr fp(filename, "rb");

    if (fp == nullptr)
    {
        return -1; // File not found
    }

    ch = static_cast<Word>(fgetc(fp));
    ungetc(ch, fp);

    if (ch == ':')
    {
        load_intelhex(fp, memtgt);
    }
    else if (toupper(ch) == 'S')
    {
        load_motorola_srec(fp, memtgt);
    }
    else if (ch == 0x02)
    {
        load_flex_binary(fp, memtgt);
    }
    else
    {
        return -2; // Unknown file format
    }

    return 0;
}

int write_flex_binary(const char *filename, MemorySource<size_t> &memsrc)
{
    BFilePtr fp(filename, "wb");

    if (fp == nullptr)
    {
        return -1; // Could not open file for writing
    }

    const size_t buffer_size = 255;
    Byte header[4] = { 0x02, 0x00, 0x00, 0x00 };
    Byte buffer[buffer_size];
    Word address = memsrc.reset_src_addr();
    size_t index = 0;
    bool last_junk_written = false;
    bool at_end;

    while (!(at_end = memsrc.src_at_end()) || !last_junk_written)
    {
        if (!at_end)
        {
            memsrc >> buffer[index++];
        }

        if (index == buffer_size || (at_end && !last_junk_written))
        {
            header[1] = (address >> 8) & 0xff;
            header[2] = address & 0xff;
            header[3] = index;
            if (fwrite(&header, sizeof(Byte), sizeof(header), fp) !=
                sizeof(header))
            {
                return -2; // write error
            }

            if (fwrite(&buffer, sizeof(Byte), index, fp) != index)
            {
                return -2; // write error
            }

            address += index;
            index = 0;

            if (at_end)
            {
                last_junk_written = true;
            }
        }
    }

    return 0;
}

