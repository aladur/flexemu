/*
    rfilecnt.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2024  W. Schwotzer

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

#include <string.h>     // needed for memcpy
#include "rfilecnt.h"
#include "flexerr.h"


FlexRamDisk::FlexRamDisk(const char *p_path, std::ios::openmode mode,
                         const FileTimeAccess &p_fileTimeAccess)
    : FlexDisk(p_path, mode, p_fileTimeAccess)
{
    unsigned int sectors;

    if (!is_flex_format)
    {
        // This file container only supports compatible FLEX file formats.
        throw FlexException(FERR_CONTAINER_UNFORMATTED, path);
    }

    param.type |= TYPE_RAM_DISKFILE;
    sectors = (file_size - param.offset) / param.byte_p_sector;
    file_buffer.resize(sectors * param.byte_p_sector);

    // For FLX file format skip the header, it will never be changed.
    fstream.seekg(param.offset);
    if (fstream.fail())
    {
        throw FlexException(FERR_READING_FROM, path);
    }

    // read whole disk content into memory.
    fstream.read(reinterpret_cast<char *>(file_buffer.data()),
                 param.byte_p_sector * sectors);
    if (fstream.fail())
    {
        throw FlexException(FERR_READING_FROM, path);
    }
}

FlexRamDisk::~FlexRamDisk()
{
    // final cleanup: close if not already done
    try
    {
        close();
    }
    catch (...)
    {
        // ignore exceptions
        // usually the file should be closed already
    }
}

FlexRamDisk::FlexRamDisk(FlexRamDisk &&src) noexcept
    : FlexDisk(std::move(src))
    , file_buffer(std::move(src.file_buffer))
{
}

FlexRamDisk &FlexRamDisk::operator= (FlexRamDisk &&src) noexcept
{
    file_buffer = std::move(src.file_buffer);

    FlexDisk::operator=(std::move(src));

    return *this;
}

bool FlexRamDisk::close()
{
    bool throwException = false;

    if (fstream.is_open())
    {
        // Only if the buffer contents has been changed it
        // will be written to file.
        if (is_dirty && !file_buffer.empty())
        {
            unsigned int sectors;

            sectors = (file_size - param.offset) / param.byte_p_sector;

            fstream.seekg(param.offset);
            if (fstream.fail())
            {
                throwException = true;
            }

            fstream.write(reinterpret_cast<const char *>(file_buffer.data()),
                          param.byte_p_sector * sectors);
            if (fstream.fail())
            {
                throwException = true;
            }
        }

        fstream.close();
    }

    file_buffer.clear();

    if (throwException)
    {
        throw FlexException(FERR_WRITING_TO, path);
    }

    return true;
}

bool FlexRamDisk::ReadSector(Byte *pbuffer, int trk, int sec,
                             int side /* = -1 */) const
{
    if (file_buffer.empty())
    {
        return false;
    }

    if (!IsTrackValid(trk) || !IsSectorValid(trk, sec))
    {
        return false;
    }

    int pos = ByteOffset(trk, sec, side) - param.offset;

    if (pos < 0)
    {
        return false;
    }

    memcpy(pbuffer, file_buffer.data() + pos, param.byte_p_sector);
    return true;
}

bool FlexRamDisk::WriteSector(const Byte *pbuffer, int trk, int sec,
                              int side /* = -1 */)
{
    if (file_buffer.empty())
    {
        return false;
    }

    if (!IsTrackValid(trk) || !IsSectorValid(trk, sec))
    {
        return false;
    }

    int pos = ByteOffset(trk, sec, side) - param.offset;

    if (pos < 0)
    {
        return false;
    }

    if (param.write_protect)
    {
        return false;
    }

    is_dirty = true;
    memcpy(&file_buffer[pos], pbuffer, param.byte_p_sector);

    return true;
}

