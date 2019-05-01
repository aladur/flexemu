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
#include <iostream>
#include <fstream>
#include <limits>
#include <memory>
#include "fileread.h"


static Byte fread_byte(std::istream &istream)
{
    std::stringstream stream;
    char str[3];
    unsigned int value;

    str[0] = istream.get();
    str[1] = istream.get();
    str[2] = '\0';

    stream << std::hex << str;
    stream >> value;
    return (Byte)(value & 0xff);
}

static Word fread_word(std::istream &istream)
{
    Word ret;

    ret = fread_byte(istream);
    ret <<= 8;
    ret |= fread_byte(istream);

    return ret;
}

static void load_intelhex(std::istream &istream, MemoryTarget<size_t> &memtgt)
{
    bool done = false;
    Byte type;
    DWord index;
    DWord count;
    DWord address;
    Byte buffer[256];

    while (!done)
    {
        (void)istream.get();
        count = fread_byte(istream);
        address = fread_word(istream);
        type = fread_byte(istream);

        if (type == 0x00)
        {
            index = 0;
            while (index < count)
            {
                if (address + index <= std::numeric_limits<Word>::max())
                {
                    *(buffer + index) = fread_byte(istream);
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
        (void)fread_byte(istream);

        if (istream.get() == '\r')
        {
            (void)istream.get();
        }
    }
}

static void load_motorola_srec(std::istream &istream,
                               MemoryTarget<size_t> &memtgt)
{
    bool done = false;
    Byte type;
    DWord index;
    DWord count;
    DWord address;
    Byte buffer[256];

    while (!done)
    {
        (void)istream.get(); /* read 'S' */
        if (istream.eof())
        {
            break;
        }
        type = static_cast<char>(istream.get());   /* read type of line */
        count = fread_byte(istream);

        switch (type)
        {
            case '0':
                count -= 1;

                while (count--)
                {
                    (void)fread_byte(istream);
                };

                break;

            case '1':
                count -= 3;
                address = fread_word(istream);
                index = 0;

                while (index < count)
                {
                    if (address + index <= std::numeric_limits<Word>::max())
                    {
                        *(buffer + index) = fread_byte(istream);
                    }
                    index++;
                }
                memtgt.CopyFrom(buffer, address, count);

                break;

            case '9':
                address = fread_word(istream);
                done = 1;
                break;

            default:
                done = 1;
        }

        // Read and discard checksum byte
        (void)fread_byte(istream);

        if (istream.get() == '\r')
        {
            (void)istream.get();
        }
    }
}

void load_flex_binary(std::istream &istream, MemoryTarget<size_t> &memtgt)
{
    Byte value;
    DWord address = 0;
    DWord count = 0;
    DWord index = 0;
    Byte buffer[256];

    while (true)
    {
        value = istream.get();
        if (istream.eof())
        {
            break;
        }
        if (index == count)
        {
            index = 0;
            if (value == 0x02)
            {
                // Read address and byte count.
                address = (istream.get() & 0xff) << 8;
                address |= istream.get() & 0xff;
                count = istream.get() & 0xff;
            } else if (value == 0x16)
            {
                // Read start address and ignore it.
                Word temp = fread_word(istream);
                (void)temp;
                return;
            } else
            {
                return;
            }
            // Read next byte.
            value = istream.get();
        }
        if (address + index <= std::numeric_limits<Word>::max())
        {
            *(buffer + index) = value;
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
    std::ifstream istream(filename, std::ios_base::in);

    if (!istream.is_open())
    {
        return -1; // Could not open file for reading
    }

    ch = static_cast<Word>(istream.get());
    istream.unget();

    if (ch == ':')
    {
        load_intelhex(istream, memtgt);
    }
    else if (toupper(ch) == 'S')
    {
        load_motorola_srec(istream, memtgt);
    }
    else if (ch == 0x02)
    {
        load_flex_binary(istream, memtgt);
    }
    else
    {
        return -2; // Unknown file format
    }

    return 0;
}

static int write_buffer(std::ostream &ostream, const Byte *buffer,
                        size_t address, size_t size)
{
    Byte header[4];

    header[0] = 0x02;
    header[1] = (address >> 8) & 0xff;
    header[2] = address & 0xff;
    header[3] = static_cast<Byte>(size);

    ostream.write(reinterpret_cast<const char *>(&header[0]), sizeof(header));
    if (ostream.fail())
    {
        return -2; // write error
    }

    ostream.write(reinterpret_cast<const char *>(&buffer[0]), size);
    if (ostream.fail())
    {
        return -2; // write error
    }

    return 0;
}

static int write_hexfile(const char *filename,
                         const MemorySource<size_t> &memsrc,
                         Byte buffer_size)
{
    const auto mode = std::ios_base::out |
                      std::ios_base::trunc |
                      std::ios_base::binary;
    std::ofstream ostream(filename, mode);

    if (!ostream.is_open())
    {
        return -1; // Could not open file for writing
    }

    auto buffer = std::unique_ptr<Byte []>(new Byte[buffer_size]);
    const auto& addressRanges = memsrc.GetAddressRanges();

    for (const auto& addressRange : addressRanges)
    {
        int result;
        size_t address;
        size_t remainder = (1 + width(addressRange)) % buffer_size;

        for (address = addressRange.lower();
                address <= addressRange.upper() - buffer_size + 1;
                address += buffer_size)
        {
            memsrc.CopyTo(buffer.get(), address, buffer_size);
            result = write_buffer(ostream, buffer.get(), address, buffer_size);
            if (result != 0)
            {
                return result;
            }
        }

        if (remainder)
        {
            memsrc.CopyTo(buffer.get(), address, remainder);
            result = write_buffer(ostream, buffer.get(), address, remainder);
            if (result != 0)
            {
                return result;
            }
        }
    }

    return 0;
}

int write_flex_binary(const char *filename, const MemorySource<size_t> &memsrc)
{
    return write_hexfile(filename, memsrc, 255);
}

