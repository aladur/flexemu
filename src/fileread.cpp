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
#include "memtgt.h"
#include "bfileptr.h"


static Byte fread_byte(FILE *fp)
{
    char str[3];
    DWord value;

    str[0] = fgetc(fp);
    str[1] = fgetc(fp);
    str[2] = '\0';

    value = strtol(str, NULL, 16);
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

static void load_intelhex(FILE *fp, MemoryTarget &memtgt)
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
        memtgt.set_address(address);

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

static void load_motorola_srec(FILE *fp, MemoryTarget &memtgt)
{
    Byte done = 0;
    Byte count, type, value;
    Word address;

    while (!done)
    {
        (void)fgetc(fp); /* read 'S' */
        type = fgetc(fp);   /* read type of line */
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
                memtgt.set_address(address);

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

void load_flex_binary(FILE *fp, MemoryTarget &memtgt)
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
                memtgt.set_address(address);
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

int load_hexfile(const char *filename, MemoryTarget &memtgt)
{
    Word ch;
    BFilePtr fp(filename, "rb");

    if (fp == NULL)
    {
        return -1; // File not found
    }

    ch = fgetc(fp);
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

