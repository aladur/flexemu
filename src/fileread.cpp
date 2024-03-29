/*
    fileread.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2018-2024  W. Schwotzer

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
#include <iomanip>
#include <fstream>
#include <limits>
#include <memory>
#include <functional>
#include "fileread.h"


static Word read_word(std::istream &istream)
{
    Word result = (istream.get() & 0xFF) << 8;
    result |= istream.get() & 0xFF;

    return result;
}

static Byte read_hex_byte(std::istream &istream, Byte *checksum = nullptr)
{
    std::stringstream stream;
    char str[3];
    unsigned int value;

    str[0] = static_cast<char>(istream.get());
    str[1] = static_cast<char>(istream.get());
    str[2] = '\0';

    stream << std::hex << str;
    stream >> value;
    if (checksum != nullptr)
    {
        *checksum += static_cast<Byte>(value);
    }
    return static_cast<Byte>(value & 0xff);
}

static Word read_hex_word(std::istream &istream, Byte &checksum)
{
    Byte value;
    Word ret;

    value = read_hex_byte(istream);
    checksum += value;
    ret = (Word)value << 8;
    value = read_hex_byte(istream);
    checksum += value;
    ret |= value;

    return ret;
}

static int load_intelhex(std::istream &istream, MemoryTarget<size_t> &memtgt,
                         size_t &startAddress)
{
    bool done = false;
    std::istream::int_type value;
    Byte type;
    Byte checksum;
    DWord index;
    DWord count;
    DWord address;
    Byte buffer[256];

    while (!done)
    {
        checksum = 0;
        value = istream.get();
        if (value != ':')
        {
            return -3; // format error
        }
        count = read_hex_byte(istream, &checksum);
        address = read_hex_word(istream, checksum);
        type = read_hex_byte(istream, &checksum);

        if (type == 0x00) // Data
        {
            for (index = 0; index < count; ++index)
            {
                if (address + index <= std::numeric_limits<Word>::max())
                {
                    *(buffer + index) = read_hex_byte(istream, &checksum);
                }
            }
            memtgt.CopyFrom(buffer, address, count);
        }
        else if (type == 0x01) // End of file / start address
        {
            startAddress = address;
            done = true;
        } else
        {
            return -3; // format error
        }

        checksum = ~checksum + 1;

        if (checksum != read_hex_byte(istream))
        {
            return -4; // checksum error
        }

        if (istream.get() == '\r')
        {
            (void)istream.get();
        }

        if (istream.fail())
        {
            return -2; // read error
        }
    }

    return 0;
}

static int load_motorola_srec(std::istream &istream,
                              MemoryTarget<size_t> &memtgt,
                              size_t &startAddress)
{
    bool done = false;
    std::istream::int_type value;
    Byte type;
    Byte checksum;
    DWord index;
    DWord count;
    DWord address;
    Byte buffer[256];

    while (!done)
    {
        checksum = 0;
        value = istream.get(); /* read 'S' */
        if (tolower(value) != 's')
        {
            return -3; // format error
        }

        if (istream.eof())
        {
            break;
        }

        type = static_cast<char>(istream.get());   /* read type of line */
        count = read_hex_byte(istream, &checksum);
        address = read_hex_word(istream, checksum);

        switch (type)
        {
            case '0': // Header
                count -= 3;

                for (index = 0; index < count; ++index)
                {
                    (void)read_hex_byte(istream, &checksum);
                }
                break;

            case '1': // Data
                count -= 3;

                for (index = 0; index < count; ++index)
                {
                    if (address + index <= std::numeric_limits<Word>::max())
                    {
                        *(buffer + index) = read_hex_byte(istream, &checksum);
                    }
                }
                memtgt.CopyFrom(buffer, address, count);

                break;

            case '9': // End of file / start address
                startAddress = address;
                done = 1;
                break;

            case '5': // Record count
                break;

            default:
                return -3; // format error
        }

        checksum = ~checksum;

        if (checksum != read_hex_byte(istream))
        {
            return -4; // checksum error
        }

        if (istream.get() == '\r')
        {
            (void)istream.get();
        }

        if (istream.fail())
        {
            return -2; // read error
        }
    }

    return 0;
}

static int load_flex_binary(std::istream &istream, MemoryTarget<size_t> &memtgt,
                            size_t &startAddress)
{
    Byte value;
    DWord address = 0;
    DWord count = 0;
    DWord index = 0;
    Byte buffer[256];
    bool isFirst = true;

    while (true)
    {
        value = static_cast<char>(istream.get());
        if (isFirst && value != 0x02)
        {
            return -3; // format error
        }

        if (istream.eof())
        {
            break;
        }
        if (index == count)
        {
            index = 0;
            if (value == 0x02)
            {
                isFirst = false;
                // Read address and byte count.
                address = read_word(istream);
                count = istream.get() & 0xff;
            }
            else if (value == 0x16)
            {
                startAddress = read_word(istream);
                return 0;
            }
            else if (value == 0x00)
            {
                return 0;
            } else
            {
                return -3;
            }

            // Read next byte.
            value = static_cast<char>(istream.get());
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

    return 0;
}

int load_hexfile(const char *filename, MemoryTarget<size_t> &memtgt,
                 size_t &startAddress)
{
    Word ch;
    std::ifstream istream(filename, std::ios_base::in | std::ios_base::binary);

    if (!istream.is_open())
    {
        return -1; // Could not open file for reading
    }

    ch = static_cast<Word>(istream.get());
    istream.unget();

    if (ch != 0x02)
    {
        istream.close();
        istream.open(filename, std::ios_base::in);
        if (!istream.is_open())
        {
            return -1; // Could not open file for reading
        }
    }

    if (ch == ':')
    {
        return load_intelhex(istream, memtgt, startAddress);
    }

    if (toupper(ch) == 'S')
    {
        return load_motorola_srec(istream, memtgt, startAddress);
    }

    if (ch == 0x02)
    {
        return load_flex_binary(istream, memtgt, startAddress);
    }

    return -3; // Unknown or invalid file format
}

int load_flex_binary(const char *filename, MemoryTarget<size_t> &memtgt,
                     size_t &startAddress)
{
    std::ifstream istream(filename, std::ios_base::in | std::ios_base::binary);

    if (!istream.is_open())
    {
        return -1; // Could not open file for reading
    }

    return load_flex_binary(istream, memtgt, startAddress);
}

enum class WBType
{
    Header,
    Data,
    StartAddress,
};

static int write_buffer_flex_binary(WBType wbType, std::ostream &ostream,
                                    const Byte *buffer,
                                    size_t address, size_t size)
{
    Byte header[4];

    header[1] = (address >> 8) & 0xff;
    header[2] = address & 0xff;

    switch (wbType)
    {
        case WBType::Header:
            break;

        case WBType::Data:
            header[0] = 0x02;
            header[3] = static_cast<Byte>(size);

            ostream.write(
                    reinterpret_cast<const char *>(&header[0]), sizeof(header));
            if (ostream.fail())
            {
                return -5; // write error
            }

            ostream.write(reinterpret_cast<const char *>(&buffer[0]), size);
            if (ostream.fail())
            {
                return -5; // write error
            }
            break;

        case WBType::StartAddress:
            if (address != std::numeric_limits<size_t>::max())
            {
                // Write start address if available
                header[0] = 0x16;

                ostream.write(
                        reinterpret_cast<const char *>(&header[0]), sizeof(header) - 1);
                if (ostream.fail())
                {
                    return -5; // write error
                }
            }
            break;
    }

    return 0;
}

static int write_buffer_intelhex(WBType wbType, std::ostream &ostream,
                                 const Byte *buffer,
                                 size_t address, size_t size)
{
    Byte checksum = 0;
    Byte type = 0;
    size_t index;

    if (wbType == WBType::Header)
    {
        return 0;
    }

    if (wbType == WBType::StartAddress)
    {
        // Write the end-of-file record with start address
        type = 1;
        if (address == std::numeric_limits<size_t>::max())
        {
            address = 0;
        }
    }

    ostream
        << ":"
        << std::hex << std::setw(2) << std::setfill('0') << (size & 0xFF)
        << std::hex << std::setw(4) << std::setfill('0') << (address & 0xFFFF)
        << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)type;

    checksum += size & 0xFF;
    checksum += ((address >> 8) & 0xFF) + (address & 0xFF);
    checksum += type;

    for (index = 0; index < size; ++index)
    {
        ostream << std::hex << std::setw(2) << std::setfill('0')
                << (unsigned int)buffer[index];
        checksum += buffer[index];
    }

    checksum = ~checksum + 1;
    ostream << std::hex << std::setw(2) << std::setfill('0')
            << (unsigned int)checksum << std::endl;

    if (ostream.fail())
    {
        return -5; // write error
    }

    return 0;
}

static int write_buffer_motorola_srec(WBType wbType, std::ostream &ostream,
                                      const Byte *buffer,
                                      size_t address, size_t size)
{
    Byte checksum = 0;
    Byte type = 1;
    size_t index;

    switch (wbType)
    {
        case WBType::Header: type = 0; break;
        case WBType::Data: type = 1; break;
        case WBType::StartAddress: type = 9; break;
    }

    if (wbType == WBType::StartAddress &&
        address == std::numeric_limits<size_t>::max())
    {
        // Write the end-of-file record with start address
        address = 0;
    }

    ostream
        << "S" << (unsigned int)type
        << std::hex << std::setw(2) << std::setfill('0') << ((size + 3) & 0xFF)
        << std::hex << std::setw(4) << std::setfill('0') << (address & 0xFFFF);

    checksum += (size + 3) & 0xFF;
    checksum += ((address >> 8) & 0xFF) + (address & 0xFF);

    for (index = 0; index < size; ++index)
    {
        ostream << std::hex << std::setw(2) << std::setfill('0')
                << (unsigned int)buffer[index];
        checksum += buffer[index];
    }

    checksum = ~checksum;
    ostream << std::hex << std::setw(2) << std::setfill('0')
            << (unsigned int)checksum << std::endl;

    if (ostream.fail())
    {
        return -5; // write error
    }

    return 0;
}

static int write_buffer_raw_binary(WBType wbType, std::ostream &ostream,
                                   const Byte *buffer,
                                   size_t address, size_t size)
{
    static size_t previous_address = std::numeric_limits<size_t>::max();

    switch (wbType)
    {
        case WBType::Header:
        case WBType::StartAddress:
            return 0;

        case WBType::Data:
            break;
    }

    if (previous_address != std::numeric_limits<size_t>::max() &&
        previous_address < address &&
        previous_address != address)
    {
        // Fill address gap with 0.
        for (size_t i = 0; i < (address - previous_address); ++i)
        {
            ostream.put('\0');
        }
    }

    const char *pBuffer = reinterpret_cast<char *>(const_cast<Byte *>(buffer));
    ostream.write(pBuffer, size);

    previous_address = address + size;

    if (ostream.fail())
    {
        return -5; // write error
    }

    return 0;
}

static int write_hexfile(
    const char *filename,
    const MemorySource<size_t> &memsrc,
    const std::function<int(WBType, std::ostream&, const Byte *, size_t,
        size_t)>& write_buffer,
    Byte buffer_size, size_t startAddress,
    bool isBinary)
{
    int result;
    const auto mode = isBinary ?
                         std::ios_base::out |
                         std::ios_base::trunc |
                         std::ios_base::binary :
                         std::ios_base::out |
                         std::ios_base::trunc;
    std::ofstream ostream(filename, mode);

    if (!ostream.is_open())
    {
        return -6; // Could not open file for writing
    }

    auto buffer = std::unique_ptr<Byte []>(new Byte[buffer_size]);
    const auto& addressRanges = memsrc.GetAddressRanges();
    const std::string header{"Created with flex2hex"};

    result = write_buffer(WBType::Header, ostream,
                          reinterpret_cast<const Byte *>(header.c_str()), 0,
                          header.size() + 1);
    if (result != 0)
    {
        return result;
    }

    for (const auto& addressRange : addressRanges)
    {
        size_t address;
        size_t remainder = (1 + width(addressRange)) % buffer_size;

        for (address = addressRange.lower();
                address <= addressRange.upper() - buffer_size + 1;
                address += buffer_size)
        {
            memsrc.CopyTo(buffer.get(), address, buffer_size);
            result = write_buffer(WBType::Data, ostream, buffer.get(), address,
                                  buffer_size);
            if (result != 0)
            {
                return result;
            }
        }

        if (remainder)
        {
            memsrc.CopyTo(buffer.get(), address, remainder);
            result = write_buffer(WBType::Data, ostream, buffer.get(), address,
                                  remainder);
            if (result != 0)
            {
                return result;
            }
        }
    }

    result = write_buffer(WBType::StartAddress, ostream, buffer.get(),
                          startAddress, 0);

    return result;
}

int write_intelhex(const char *filename, const MemorySource<size_t> &memsrc,
                   size_t startAddress)
{
    return write_hexfile(filename, memsrc, write_buffer_intelhex, 32,
                         startAddress, false);
}

int write_motorola_srec(const char *filename,
                        const MemorySource<size_t> &memsrc,
                        size_t startAddress)
{
    return write_hexfile(filename, memsrc, write_buffer_motorola_srec, 32,
                         startAddress, false);
}

int write_raw_binary(const char *filename,
                     const MemorySource<size_t> &memsrc,
                     size_t startAddress)
{
    return write_hexfile(filename, memsrc, write_buffer_raw_binary, 32,
                         startAddress, true);
}

int write_flex_binary(const char *filename, const MemorySource<size_t> &memsrc,
                      size_t startAddress)
{
    return write_hexfile(filename, memsrc, write_buffer_flex_binary, 255,
                         startAddress, true);
}

void print_hexfile_error(std::ostream &ostream, int error_id)
{
    if (error_id < 0)
    {
        switch (error_id)
        {
            case -1: ostream << "File does not exist or "
                                "can not be opened for reading.";
                     break;

            case -2: ostream << "Error reading from file.";
                     break;

            case -3: ostream << "Unknown or invalid file format.";
                     break;

            case -4: ostream << "Wrong checksum.";
                     break;

            case -5: ostream << "Error writing to file.";
                     break;

            case -6: ostream << "File can not be opened for writing.";
                     break;

            default: ostream << "Unspecified error.";
                     break;
        }
    }
}

