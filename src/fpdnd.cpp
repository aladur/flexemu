/*
    fpdnd.cpp


    FLEXplorer, An explorer for FLEX disk image files and directory disks.
    Copyright (C) 1998-2025  W. Schwotzer

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

#include "typedefs.h"
#include "misc1.h"
#include "fpdnd.h"
#include "flexerr.h"
#include "ffilebuf.h"
#include <cstring>
#include <utility>
#include <string>


FlexDnDFiles::FlexDnDFiles(std::string p_Path, std::string p_DnsHostName) :
      path(std::move(p_Path))
    , dnsHostName(std::move(p_DnsHostName))
{
}

void FlexDnDFiles::ReadDataFrom(const Byte *buffer)
{
    const Byte *ptr = buffer;

    fileBuffers.clear();

    if (buffer == nullptr)
    {
        return;
    }

    auto size = *reinterpret_cast<const DWord *>(ptr);
    // Independent of the cpu architecture the
    // clipboard format uses big endian.
    size = flx::fromBigEndian<DWord>(size);
    ptr += sizeof(size);
    dnsHostName = reinterpret_cast<const char *>(ptr);
    ptr += size;

    size = *reinterpret_cast<const DWord *>(ptr);
    size = flx::fromBigEndian<DWord>(size);
    ptr += sizeof(size);
    path = reinterpret_cast<const char *>(ptr);
    ptr += size;

    auto count = *reinterpret_cast<const DWord *>(ptr);
    count = flx::fromBigEndian<DWord>(count);
    ptr += sizeof(count);

    for (DWord index = 0; index < count; ++index)
    {
        FlexFileBuffer fileBuffer;
        const auto &fileHeader =
            *reinterpret_cast<const tFlexFileHeader *>(ptr);
        fileBuffer.CopyHeaderBigEndianFrom(fileHeader);
        ptr += sizeof(tFlexFileHeader);
        fileBuffer.CopyFrom(ptr, fileBuffer.GetFileSize());
        ptr += fileBuffer.GetFileSize();
        fileBuffers.push_back(fileBuffer);
    }
}

FlexFileBuffer &FlexDnDFiles::GetBufferAt(unsigned int index)
{
    if (index >= fileBuffers.size())
    {
        throw FlexException(FERR_WRONG_PARAMETER);
    }

    return fileBuffers[index];
}

DWord FlexDnDFiles::GetFileSize() const
{
    // Reserve memory for path size, dnsHostName size and file count
    DWord fileSize = 3 * sizeof(DWord);
    fileSize += static_cast<DWord>(path.size()) + 1U;
    fileSize += static_cast<DWord>(dnsHostName.size()) + 1U;

    for (const auto &fileBuffer : fileBuffers)
    {
        fileSize += fileBuffer.GetFileSize(); // Add byte size of each file
    }

    // Add size of header of each file
    fileSize +=
        static_cast<DWord>(fileBuffers.size() * sizeof(tFlexFileHeader));

    return fileSize;
}

void FlexDnDFiles::WriteDataTo(Byte *buffer) const
{
    Byte *ptr = buffer;

    if (buffer == nullptr)
    {
        return;
    }

    auto size = static_cast<DWord>(dnsHostName.size() + 1);
    // Independent of the cpu architecture the
    // clipboard format uses big endian byte order.
    auto reversed = flx::toBigEndian<DWord>(size);
    std::memcpy(ptr, &reversed, sizeof(reversed));
    ptr += sizeof(reversed);
    std::memcpy(ptr, dnsHostName.c_str(), size);
    ptr += size;

    size = static_cast<DWord>(path.size() + 1);
    reversed = flx::toBigEndian<DWord>(size);
    std::memcpy(ptr, &reversed, sizeof(reversed));
    ptr += sizeof(reversed);
    std::memcpy(ptr, path.c_str(), size);
    ptr += size;

    auto count = static_cast<DWord>(fileBuffers.size());
    count = flx::toBigEndian<DWord>(count);
    std::memcpy(ptr, &count, sizeof(count));
    ptr += sizeof(count);

    for (const auto &fileBuffer : fileBuffers)
    {
        auto header = fileBuffer.GetHeaderBigEndian();

        std::memcpy(ptr, &header, sizeof(tFlexFileHeader));
        ptr += sizeof(tFlexFileHeader);
        fileBuffer.CopyTo(ptr, fileBuffer.GetFileSize());
        ptr += fileBuffer.GetFileSize();
    }
}

// ATTENTION: fileBuffer is an rvalue and moved into
// the fileBuffers.
void FlexDnDFiles::Add(FlexFileBuffer &&fileBuffer)
{
    fileBuffers.push_back(std::move(fileBuffer));
}

std::string FlexDnDFiles::GetPath() const
{
    return path;
}

std::string FlexDnDFiles::GetDnsHostName() const
{
    return dnsHostName;
}

