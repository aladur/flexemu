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


FlexRamFileContainer::FlexRamFileContainer(const char *path, const char *mode,
                                           const FileTimeAccess
                                           &p_fileTimeAccess) :
    FlexFileContainer(path, mode, p_fileTimeAccess), is_dirty(false)
{
    unsigned int sectors;

    if (!is_flex_format)
    {
        // This file container only supports compatible FLEX file formats.
        throw FlexException(FERR_CONTAINER_UNFORMATTED, fp.GetPath());
    }

    param.type |= TYPE_RAM_CONTAINER;
    sectors = (file_size - param.offset) / param.byte_p_sector;
    file_buffer =
        std::unique_ptr<Byte[]>(new Byte[sectors * param.byte_p_sector]);

    // For FLX file format skip the header, it will never be changed.
    if (fseek(fp, param.offset, SEEK_SET))
    {
        throw FlexException(FERR_READING_FROM, fp.GetPath());
    }

    // read total disk into memory
    if (fread(file_buffer.get(), param.byte_p_sector, sectors, fp) != sectors)
    {
        throw FlexException(FERR_READING_FROM, fp.GetPath());
    }
}

FlexRamFileContainer::~FlexRamFileContainer()
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

FlexRamFileContainer::FlexRamFileContainer(FlexRamFileContainer &&src) noexcept
    : FlexFileContainer(std::move(src))
    , file_buffer(std::move(src.file_buffer))
{
}

FlexRamFileContainer &FlexRamFileContainer::operator=
                                         (FlexRamFileContainer &&src) noexcept
{
    file_buffer = std::move(src.file_buffer);

    FlexFileContainer::operator=(std::move(src));

    return *this;
}

bool FlexRamFileContainer::close()
{
    bool throwException = false;
    std::string path = fp.GetPath();

    if (fp != nullptr)
    {
        // Only if the buffer contents has been changed it
        // will be written to file.
        if (is_dirty && (file_buffer != nullptr))
        {
            unsigned int sectors;

            sectors = (file_size - param.offset) / param.byte_p_sector;

            if (fseek(fp, param.offset, SEEK_SET))
            {
                throwException = true;
            }

            if (fwrite(file_buffer.get(), param.byte_p_sector, sectors, fp)
                    != sectors)
            {
                throwException = true;
            }
        }

        fp.Close();
    }

    file_buffer.reset(nullptr);

    if (throwException)
    {
        throw FlexException(FERR_WRITING_TO, path);
    }

    return true;
}

bool FlexRamFileContainer::ReadSector(Byte *pbuffer, int trk, int sec,
                                      int side /* = -1 */) const
{
    if (file_buffer == nullptr)
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

    memcpy(pbuffer, &file_buffer[pos], param.byte_p_sector);
    return true;
}

bool FlexRamFileContainer::WriteSector(const Byte *pbuffer, int trk, int sec,
                                       int side /* = -1 */)
{
    if (file_buffer == nullptr)
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

