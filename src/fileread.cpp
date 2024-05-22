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


#include "misc1.h"
#include "fileread.h"
#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <limits>
#include <array>
#include <vector>
#include <functional>


static Word read_word(std::istream &istream)
{
    Word result = (istream.get() & 0xFF) << 8;
    result |= istream.get() & 0xFF;

    return result;
}

static bool read_hex_byte(std::istream &istream, Byte &value, Byte &checksum)
{
    char byte;

    value = 0U;
    for (int i = 0; i < 2; ++i)
    {
        if (istream.eof())
        {
            return false;
        }

        istream.get(byte);
        byte = static_cast<char>(toupper(static_cast<Byte>(byte)));
        if (byte >= '0' && byte <= '9')
        {
            value <<= 4;
            value |= (byte - '0');
        }
        else if (byte >= 'A' && byte <= 'F')
        {
            value <<= 4;
            value |= (byte - 'A' + 10);
        }
        else
        {
            return false;
        }
    }

    checksum += value;

    return true;
}

static bool read_hex_word(std::istream &istream, Word &value, Byte &checksum)
{
    Byte byte;

    value = 0U;
    for (int i = 0; i < 2; ++i)
    {
        auto ok = read_hex_byte(istream, byte, checksum);
        if (!ok)
        {
            return false;
        }
        value <<= 8;
        value |= byte;
    }

    return true;
}

static int load_intel_hex(std::istream &istream, MemoryTarget<DWord> &memtgt,
                          DWord &startAddress)
{
    bool done = false;
    std::istream::int_type value;
    Byte type;
    Byte checksum;
    Byte expected_checksum;
    Byte dummy;
    Byte index;
    Byte count;
    Word address;
    std::array<Byte, 255> buffer{};
    // MSVC does not allow auto *ibuffer.
    // NOLINTNEXTLINE(readability-qualified-auto).
    auto ibuffer = buffer.begin();

    while (!done)
    {
        checksum = 0;
        value = istream.get();
        if (value != ':')
        {
            return -3; // format error
        }
        auto ok = read_hex_byte(istream, count, checksum);
        ok &= read_hex_word(istream, address, checksum);
        ok &= read_hex_byte(istream, type, checksum);
        if (!ok)
        {
            return -3;
        }

        if (type == 0x00) // Data
        {
            // Read a maximum of 255 hex-byte per line.
            for (index = 0; index < count; ++index)
            {
                if (address + index <= std::numeric_limits<Word>::max())
                {
                    Byte bval;
                    ok = read_hex_byte(istream, bval, checksum);
                    if (!ok)
                    {
                        return -3;
                    }
                    *(ibuffer++) = bval;
                }
            }
            memtgt.CopyFrom(buffer.data(), address, count);
            ibuffer = buffer.begin();
            if (count == 0)
            {
                done = true;
            }
        }
        else if (type == 0x01) // End of file
        {
            done = true;
        }
        else if (type == 0x05) // Start linear address
        {
            if (count != 4)
            {
                return -3; // Start address has 4 byte.
            }
            startAddress = 0U;
            for (index = 0; index < count; ++index)
            {
                Byte bval;
                ok = read_hex_byte(istream, bval, checksum);
                if (!ok)
                {
                    return -3;
                }
                startAddress = (startAddress << 8) | bval;
            }
        }
        else
        {
            return -3; // format error
        }

        checksum = ~checksum + 1;
        ok = read_hex_byte(istream, expected_checksum, dummy);
        if (!ok)
        {
            return -3;
        }
        if (checksum != expected_checksum)
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

static int load_motorola_srecord(std::istream &istream,
                                 MemoryTarget<DWord> &memtgt,
                                 DWord &startAddress)
{
    bool done = false;
    std::istream::int_type value;
    Byte type;
    Byte checksum;
    Byte expected_checksum;
    Byte dummy;
    DWord index;
    Byte count;
    Word address;
    std::array<Byte, 255> buffer{};
    // MSVC does not allow auto *ibuffer.
    // NOLINTNEXTLINE(readability-qualified-auto).
    auto ibuffer = buffer.begin();

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

        type = static_cast<char>(istream.get()); /* read type of line */
        auto ok = read_hex_byte(istream, count, checksum);
        ok &= read_hex_word(istream, address, checksum);
        if (!ok)
        {
            return -3;
        }

        switch (type)
        {
            case '0': // Header
                count -= 3;

                for (index = 0; index < count; ++index)
                {
                    read_hex_byte(istream, dummy, checksum);
                }
                break;

            case '1': // Data
                count -= 3;

                // Read a maximum of 252 hex-byte per line.
                for (index = 0; index < count; ++index)
                {
                    if (address + index <= std::numeric_limits<Word>::max())
                    {
                        Byte bval;
                        ok = read_hex_byte(istream, bval, checksum);
                        if (!ok)
                        {
                            return -3;
                        }
                        *(ibuffer++) = bval;
                    }
                }
                if (!ok)
                {
                    return -3;
                }
                memtgt.CopyFrom(buffer.data(), address, count);
                ibuffer = buffer.begin();

                break;

            case '9': // End of file / start address
                startAddress = address;
                done = true;
                break;

            case '5': // Record count
                break;

            default:
                return -3; // format error
        }

        checksum = ~checksum;
        ok = read_hex_byte(istream, expected_checksum, dummy);
        if (!ok)
        {
            return -3;
        }
        if (checksum != expected_checksum)
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

static int load_flex_binary(std::istream &istream, MemoryTarget<DWord> &memtgt,
                            DWord &startAddress)
{
    Byte value;
    DWord address = 0;
    DWord count = 0;
    DWord index = 0;
    std::array<Byte, 256> buffer{};
    // MSVC does not allow auto *ibuffer.
    // NOLINTNEXTLINE(readability-qualified-auto).
    auto ibuffer = buffer.begin();
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

            if (count != 0U)
            {
                // Read next byte.
                value = static_cast<char>(istream.get());
            }
        }
        if (count != 0U && address + index <= std::numeric_limits<Word>::max())
        {
            *(ibuffer++) = value;
        }
        if (count == 0 || ++index == count)
        {
            memtgt.CopyFrom(buffer.data(), address, count);
            ibuffer = buffer.begin();
        }
    }

    return 0;
}

int load_hexfile(const std::string &filename, MemoryTarget<DWord> &memtgt,
                 DWord &startAddress)
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
        return load_intel_hex(istream, memtgt, startAddress);
    }

    if (toupper(ch) == 'S')
    {
        return load_motorola_srecord(istream, memtgt, startAddress);
    }

    if (ch == 0x02)
    {
        return load_flex_binary(istream, memtgt, startAddress);
    }

    return -3; // Unknown or invalid file format
}

int load_flex_binary(const std::string &filename, MemoryTarget<DWord> &memtgt,
                     DWord &startAddress)
{
    std::ifstream istream(filename, std::ios_base::in | std::ios_base::binary);

    if (!istream.is_open())
    {
        return -1; // Could not open file for reading
    }

    return load_flex_binary(istream, memtgt, startAddress);
}

enum class WBType : uint8_t
{
    Header,
    Data,
    StartAddress,
    EndOfFile,
};

static int write_buffer_flex_binary(WBType wbType, std::ostream &ostream,
                                    const Byte *buffer,
                                    DWord address, DWord size)
{
    std::array<char, 4> header{};

    header[1] = static_cast<char>((address >> 8) & 0xff);
    header[2] = static_cast<char>(address & 0xff);

    switch (wbType)
    {
        case WBType::Header:
        case WBType::EndOfFile:
            break;

        case WBType::Data:
            header[0] = '\x2';
            header[3] = static_cast<char>(size & 0xff);

            ostream.write(header.data(), header.size());
            if (ostream.fail())
            {
                return -5; // write error
            }

            ostream.write(reinterpret_cast<const char *>(buffer), size);
            if (ostream.fail())
            {
                return -5; // write error
            }
            break;

        case WBType::StartAddress:
            if (address != std::numeric_limits<DWord>::max())
            {
                // Write start address if available
                header[0] = 0x16;

                ostream.write(header.data(), header.size() - 1);
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
                                 DWord address, DWord size)
{
    Byte checksum = 0;
    Byte type = 0;
    DWord index;

    if (wbType == WBType::Header)
    {
        return 0;
    }

    if (wbType == WBType::StartAddress)
    {
        // Write the start-linear-address record.
        type = 5;
        size = 4;
        if (address == std::numeric_limits<DWord>::max())
        {
            return 0;
        }
        address = toBigEndian(address);
        buffer = reinterpret_cast<Byte *>(&address);
    }
    else if (wbType == WBType::EndOfFile)
    {
        // Write the end-of-file record
        type = 1;
        address = 0;
    }

    ostream
        << ":"
        << std::hex << std::setw(2) << std::setfill('0') << (size & 0xFF)
        << std::hex << std::setw(4) << std::setfill('0') << (address & 0xFFFF)
        << std::hex << std::setw(2) << std::setfill('0')
        << static_cast<Word>(type);

    checksum += size & 0xFF;
    checksum += ((address >> 8) & 0xFF) + (address & 0xFF);
    checksum += type;

    for (index = 0; index < size; ++index)
    {
        ostream << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<Word>(buffer[index]);
        checksum += buffer[index];
    }

    checksum = ~checksum + 1;
    ostream << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<Word>(checksum) << '\n';

    if (ostream.fail())
    {
        return -5; // write error
    }

    return 0;
}

static int write_buffer_motorola_srec(WBType wbType, std::ostream &ostream,
                                      const Byte *buffer,
                                      DWord address, DWord size)
{
    Byte checksum = 0;
    Word type = 1;
    DWord index;

    switch (wbType)
    {
        case WBType::Header: type = 0; break;
        case WBType::Data: type = 1; break;
        case WBType::StartAddress: type = 9; break;
        case WBType::EndOfFile: return 0;
    }

    if (wbType == WBType::StartAddress &&
        address == std::numeric_limits<DWord>::max())
    {
        // Write the end-of-file record with start address
        address = 0;
    }

    ostream
        << "S" << type
        << std::hex << std::setw(2) << std::setfill('0') << ((size + 3) & 0xFF)
        << std::hex << std::setw(4) << std::setfill('0') << (address & 0xFFFF);

    checksum += (size + 3) & 0xFF;
    checksum += ((address >> 8) & 0xFF) + (address & 0xFF);

    for (index = 0; index < size; ++index)
    {
        ostream << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<Word>(buffer[index]);
        checksum += buffer[index];
    }

    checksum = ~checksum;
    ostream << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<Word>(checksum) << '\n';

    if (ostream.fail())
    {
        return -5; // write error
    }

    return 0;
}

static int write_buffer_raw_binary(WBType wbType, std::ostream &ostream,
                                   const Byte *buffer,
                                   DWord address, DWord size)
{
    static DWord previous_address = std::numeric_limits<DWord>::max();

    switch (wbType)
    {
        case WBType::Header:
        case WBType::StartAddress:
        case WBType::EndOfFile:
            return 0;

        case WBType::Data:
            break;
    }

    if (previous_address != std::numeric_limits<DWord>::max() &&
        previous_address < address &&
        previous_address != address)
    {
        // Fill address gap with 0.
        for (DWord i = 0; i < (address - previous_address); ++i)
        {
            ostream.put('\0');
        }
    }

    const auto *pBuffer = reinterpret_cast<const char *>(buffer);
    ostream.write(pBuffer, size);

    previous_address = address + size;

    if (ostream.fail())
    {
        return -5; // write error
    }

    return 0;
}

static int write_hexfile(
    const std::string &filename,
    const MemorySource<DWord> &memsrc,
    const std::function<int(WBType, std::ostream&, const Byte *, DWord,
        DWord)>& write_buffer,
    Byte buffer_size, DWord startAddress,
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

    std::vector<Byte> buffer;
    const auto& addressRanges = memsrc.GetAddressRanges();
    const std::array<Byte, 22> header{"Created with flex2hex"};

    result = write_buffer(WBType::Header, ostream, header.data(), 0,
                          static_cast<DWord>(header.size() - 1));
    if (result != 0)
    {
        return result;
    }

    buffer.resize(buffer_size);
    for (const auto& addressRange : addressRanges)
    {
        DWord address;
        DWord remainder = (1 + width(addressRange)) % buffer_size;

        for (address = addressRange.lower();
                address <= addressRange.upper() - buffer_size + 1;
                address += buffer_size)
        {
            memsrc.CopyTo(buffer.data(), address, buffer_size);
            result = write_buffer(WBType::Data, ostream, buffer.data(),
                                  address, buffer_size);
            if (result != 0)
            {
                return result;
            }
        }

        if (remainder)
        {
            memsrc.CopyTo(buffer.data(), address, remainder);
            result = write_buffer(WBType::Data, ostream, buffer.data(),
                                  address, remainder);
            if (result != 0)
            {
                return result;
            }
        }
    }

    result = write_buffer(WBType::StartAddress, ostream, buffer.data(),
                          startAddress, 0);
    if (result != 0)
    {
        return result;
    }
    result = write_buffer(WBType::EndOfFile, ostream, buffer.data(),
                          startAddress, 0);
    if (result != 0)
    {
        return result;
    }

    return result;
}

int write_intel_hex(const std::string &filename,
                    const MemorySource<DWord> &memsrc,
                    DWord startAddress)
{
    return write_hexfile(filename, memsrc, write_buffer_intelhex, 32,
                         startAddress, false);
}

int write_motorola_srecord(const std::string &filename,
                           const MemorySource<DWord> &memsrc,
                           DWord startAddress)
{
    return write_hexfile(filename, memsrc, write_buffer_motorola_srec, 32,
                         startAddress, false);
}

int write_raw_binary(const std::string &filename,
                     const MemorySource<DWord> &memsrc,
                     DWord startAddress)
{
    return write_hexfile(filename, memsrc, write_buffer_raw_binary, 32,
                         startAddress, true);
}

int write_flex_binary(const std::string &filename,
                      const MemorySource<DWord> &memsrc,
                      DWord startAddress)
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

