/*
    mdcrfs.cpp


    FLEXplorer, An explorer for FLEX disk image files and directory disks.
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


#include "mdcrfs.h"
#include "mdcrtape.h"
#include "bmembuf.h"
#include <limits>
#include <algorithm>
#include <cctype>


const std::string &GetMdcrError(int index)
{
    static const std::vector<std::string> mdcrErrors
    {
        "Success",
        "Invalid input data",
        "Tape is write protected",
        "Wrong checksum",
        "Dublicate name",
        "Error reading a record",
        "Error writing a record",
        "Unknown error",
    };

    if (index >= static_cast<int>(mdcrErrors.size()) || index < 0)
    {
        return mdcrErrors[mdcrErrors.size() - 1U];
    }

    return mdcrErrors[index];
}

// Create a filename which conforms to the MDCR file system:
// - Up to 6 ASCII characters
// - Unused characters filled up with space, ' '
// - If string is empty or starts with a dot return 'MYFILE'.
std::string MdcrFileSystem::CreateMdcrFilename(const char *name,
                                               bool toUppercase)
{
    std::string result = { ' ', ' ', ' ', ' ', ' ', ' ' };
    int index;

    if (name == nullptr || name[0] == '\0' || name[0] == '.')
    {
        return "MYFILE";
    }

    for (index = 0;
         index < 6 && name[index] != '\0' && name[index] != '.';
         ++index)
    {
        result[index] = name[index];
    }

    if (toUppercase)
    {
        flx::strupper(result);
    }

    return result;
}

// Copy the MDCR file name in to the record (up to 6 characters).
void MdcrFileSystem::SetFilename(std::vector<Byte>::iterator &iter,
                                 const char *filename)
{
    // Make shure that the file name always has exactly 6 characters.
    std::string mdcrFilename =
        MdcrFileSystem::CreateMdcrFilename(filename, false);

    std::copy(mdcrFilename.cbegin(), mdcrFilename.cend(), iter);

    iter += static_cast<uint32_t>(mdcrFilename.size());
}

// Copy the MDCR file name from the record (up to 6 characters).
// Spaces are trimmed.
std::string MdcrFileSystem::GetFilename(
        std::vector<Byte>::const_iterator &iter)
{
    size_t index;
    std::string filename;

    for (index = 0; index < 6; ++index)
    {
        Byte value = *(iter++);
        filename.push_back(static_cast<char>(value));
    }

    return filename;
}

Byte MdcrFileSystem::CalculateChecksum(
        std::vector<Byte>::const_iterator &iter, size_t size)
{
    Byte checksum = 0;
    size_t index;

    for (index = 0; index < size; ++index)
    {
        checksum += *(iter++);
    }

    return checksum;
}

MdcrStatus MdcrFileSystem::ReadFile(
              std::string &filename,
              BMemoryBuffer &memory,
              MiniDcrTape &mdcr)
{
    std::vector<Byte> ibuffer;
    Word startAddress = 0;
    Word endAddress = 0;
    size_t size;
    size_t count = 0;
    bool hasFoundFile = false;

    while (true)
    {
        // Read the file header
        if (!mdcr.ReadRecord(ibuffer) ||
            ibuffer.size() != 13 || ibuffer[0] != 0x55 || ibuffer[12] != 0)
        {
            return MdcrStatus::ReadError;
        }

        auto iter = ibuffer.cbegin() + 1;
        if (ibuffer[11] != CalculateChecksum(iter, 10))
        {
            return MdcrStatus::WrongChecksum;
        }

        startAddress = flx::getValueBigEndian<Word>(&ibuffer[7]);
        endAddress = flx::getValueBigEndian<Word>(&ibuffer[9]);
        if (startAddress > endAddress)
        {
            return MdcrStatus::ReadError;
        }
        size = endAddress - startAddress + 1;

        iter = ibuffer.cbegin() + 1;
        auto mdcrFilename = GetFilename(iter);

        if (filename.empty() || mdcrFilename == filename)
        {
            hasFoundFile = true;
        }

        while (count < size)
        {
            if (!mdcr.ReadRecord(ibuffer) ||
                ibuffer[0] != 0x55 || ibuffer[ibuffer.size() - 1] != 0)
            {
                return MdcrStatus::ReadError;
            }

            iter = ibuffer.cbegin() + 1;
            if (ibuffer[ibuffer.size() - 2] !=
                CalculateChecksum(iter, ibuffer.size() - 3))
            {
                return MdcrStatus::WrongChecksum;
            }

            if (hasFoundFile)
            {
                // Copy file contents into memory.
                memory.CopyFrom(&ibuffer[1], startAddress, ibuffer.size() - 3U);
                startAddress += static_cast<Word>(ibuffer.size() - 3U);
            }
            count += ibuffer.size() - 3;
        }

        if (hasFoundFile)
        {
            filename = mdcrFilename;
            break;
        }
    }

    return MdcrStatus::Success;
}

MdcrStatus MdcrFileSystem::WriteFile(
        const char *filepath,
        const BMemoryBuffer &memory,
        MiniDcrTape &mdcr,
        MdcrWriteMode mode,
        bool toUppercase /* = true */)
{
    int32_t index = 0;
    std::vector<Byte> ibuffer;
    std::vector<Byte> obuffer;
    std::string filename = flx::getFileName(filepath);
    std::string mdcrFilename = CreateMdcrFilename(filename.c_str(), toUppercase);

    if (mdcr.IsWriteProtected())
    {
        return MdcrStatus::WriteProtected;
    }

    if (memory.GetAddressRanges().size() != 1)
    {
        return MdcrStatus::InvalidData;
    }

    const auto addressRange = memory.GetAddressRanges()[0];
    if (!memory.CopyTo(ibuffer, addressRange) || ibuffer.empty() ||
            ibuffer.size() > std::numeric_limits<int>::max())
    {
        return MdcrStatus::InvalidData;
    }

    // Rewind tape until begin.
    while(mdcr.GotoPreviousRecord())
    {
    }

    if (mode == MdcrWriteMode::Append)
    {
        // To append the file forward tape until the end.
        // Read each file and check for errors or doube filename.
        auto status = ForEachFile(mdcr,
                [&mdcrFilename](const std::string &tempFilename,
                                BMemoryBuffer &)
        {
            if (tempFilename == mdcrFilename)
            {
                return MdcrStatus::DoubleName;
            }

            return MdcrStatus::Success;
        });

        if (status != MdcrStatus::Success)
        {
            return status;
        }
    }

    obuffer.resize(13);
    auto iter = obuffer.begin();
    auto checksumIter = obuffer.cbegin() + 1;

    // Write the file header (13 Byte).
    // Format:
    // +------+--------+----------+--------+--------+------+
    // |  0   | 1...6  |   7,8    |  9,10  |   11   |  12  |
    // +------+--------+----------+--------+--------+------+
    // | 0x55 |Filename|Startaddr.|Endaddr.|Checksum| 0x00 |
    // +------+--------+----------+--------+--------+------+
    // Remarks:
    // - 0x55: The synchronisation byte
    // - Filename: If less than 6 character fill up with space, ' '
    // - Startaddr.,Endaddr.: 16-bit, most significant byte first (MSB)
    //                        For BASIC programs for start-/endaddr.
    //                        0x2020 is used.
    // - Checksum: Sum of byte 1 ... 10.

    *(iter++) = 0x55;
    SetFilename(iter, mdcrFilename.c_str());
    *(iter++) = (addressRange.lower() >> 8U) & 0xFFU;
    *(iter++) = addressRange.lower() & 0xFFU;
    *(iter++) = (addressRange.upper() >> 8U) & 0xFFU;
    *(iter++) = addressRange.upper() & 0xFFU;
    *(iter++) = CalculateChecksum(checksumIter, 10);
    *(iter++) = 0;

    // Write the file header
    if (!mdcr.WriteRecord(obuffer))
    {
        return MdcrStatus::WriteError;
    }

    for (index = 0;
         index < static_cast<int>(ibuffer.size());
         index += MaxRecordSize)
    {
        size_t size =
          std::min<size_t>(ibuffer.size() - index, MaxRecordSize);
        obuffer.resize(size + 3);

        // Write a data record (size of data record + 3 Byte).
        // Format:
        // +------+-----------+--------+------+
        // |  0   | 1...size  | size+1 |size+2|
        // +------+-----------+-----------+---+
        // | 0x55 |data record|Checksum| 0x00 |
        // +------+-----------+--------+------+
        // Remarks:
        // - 0x55: The synchronisation byte
        // - size: One record has a maximum size of 1024 Bytes.
        //         Each file has one header and one or mutiple data records.
        // - Checksum: Sum of byte 1 ... size.

        obuffer[0] = 0x55;

        std::copy_n(ibuffer.cbegin() + index, size, obuffer.begin() + 1);

        checksumIter = obuffer.cbegin() + 1;
        obuffer[size + 1] = CalculateChecksum(checksumIter, size);
        obuffer[size + 2] = 0;

        if (!mdcr.WriteRecord(obuffer))
        {
            return MdcrStatus::WriteError;
        }
    }

    return MdcrStatus::Success;
}

// Call the iterateFunction for each successfully read file on MDCR tape.
// Implementation may change in future.
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
MdcrStatus MdcrFileSystem::ForEachFile(MiniDcrTape &mdcr,
                  const std::function<MdcrStatus
                            (const std::string&, BMemoryBuffer &memory)>&
                            iterateFunction)
{
    BMemoryBuffer memory(65536);
    std::string filename;

    // Rewind tape until begin.
    while(mdcr.GotoPreviousRecord())
    {
    }

    // To append the file forward tape until the end.
    // Read each file and check for errors or doube filename.
    while (mdcr.HasRecord())
    {
        filename.clear();
        memory.Reset();
        MdcrStatus status = ReadFile(filename, memory, mdcr);

        if (status != MdcrStatus::Success)
        {
            return status;
        }

        status = iterateFunction(filename, memory);
        if (status != MdcrStatus::Success)
        {
            return status;
        }
    }

    return MdcrStatus::Success;
}

