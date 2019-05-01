/*
    fileread.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2018-2019  W. Schwotzer

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
#include <limits>
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
    Byte type;
    DWord index;
    DWord count;
    DWord address;
    Byte buffer[256];

    while (!done)
    {
        (void)fgetc(fp);
        count = fread_byte(fp);
        address = fread_word(fp);
        type = fread_byte(fp);

        if (type == 0x00)
        {
            index = 0;
            while (index < count)
            {
                if (address + index <= std::numeric_limits<Word>::max())
                {
                    *(buffer + index) = fread_byte(fp);
                }
                index++;
            }
            memtgt.CopyFrom(buffer, address, count);
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
    Byte type;
    DWord index;
    DWord count;
    DWord address;
    Byte buffer[256];

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
                    (void)fread_byte(fp);
                };

                break;

            case '1':
                count -= 3;
                address = fread_word(fp);
                index = 0;

                while (index < count)
                {
                    if (address + index <= std::numeric_limits<Word>::max())
                    {
                        *(buffer + index) = fread_byte(fp);
                    }
                    index++;
                }
                memtgt.CopyFrom(buffer, address, count);

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
    DWord address = 0;
    DWord count = 0;
    DWord index = 0;
    Byte buffer[256];

    while ((value = fgetc(fp)) != EOF)
    {
        if (index == count)
        {
            if (value == 0x02)
            {
                // Read address and byte count.
                address = (fgetc(fp) & 0xff) << 8;
                address |= fgetc(fp) & 0xff;
                count = fgetc(fp) & 0xff;
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
        if (address + index <= std::numeric_limits<Word>::max())
        {
            *(buffer + index) = fread_byte(fp);
        }
        if (++index == count)
        {
            memtgt.CopyFrom(buffer, address, count);
        }
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

static int write_buffer(FILE *fp, const Byte *buffer, size_t address,
                        size_t size)
{
    Byte header[4];

    header[0] = 0x02;
    header[1] = (address >> 8) & 0xff;
    header[2] = address & 0xff;
    header[3] = static_cast<Byte>(size);

    if (fwrite(header, sizeof(Byte), sizeof(header), fp) != sizeof(header))
    {
        return -2; // write error
    }

    if (fwrite(buffer, sizeof(Byte), size, fp) != size)
    {
        return -2; // write error
    }

    return 0;
}

int write_flex_binary(const char *filename, const MemorySource<size_t> &memsrc)
{
    BFilePtr fp(filename, "wb");

    if (fp == nullptr)
    {
        return -1; // Could not open file for writing
    }

    const auto& addressRanges = memsrc.GetAddressRanges();

    for (const auto& addressRange : addressRanges)
    {
        Byte buffer[255];
        int result;
        size_t address;
        size_t remainder = (1 + width(addressRange)) % sizeof(buffer);

        for (address = addressRange.lower();
                address <= (addressRange.upper() - sizeof(buffer) + 1);
                address += sizeof(buffer))
        {
            memsrc.CopyTo(buffer, address, sizeof(buffer));
            result = write_buffer(fp, buffer, address, sizeof(buffer));
            if (result != 0)
            {
                return result;
            }
        }

        if (remainder)
        {
            memsrc.CopyTo(buffer, address, remainder);
            result = write_buffer(fp, buffer, address, remainder);
            if (result != 0)
            {
                return result;
            }
        }
    }

    return 0;
}

