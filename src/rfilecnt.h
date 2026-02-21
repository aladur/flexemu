/*
    rfilecnt.h


    FLEXplorer, An explorer for FLEX disk image files and directory disks.
    Copyright (C) 1998-2026  W. Schwotzer

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

#ifndef RFILECNT_INCLUDED
#define RFILECNT_INCLUDED

#include "typedefs.h"
#include "efiletim.h"
#include "ffilecnt.h"
#include <optional>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;


// class FlexRamDisk is a specialization of FlexDisk where the whole disk
// image is stored in RAM for improved performance. The disk image is only
// written back to disk if there are changes (see is_dirty flag).
class FlexRamDisk : public FlexDisk
{

private:

    std::vector<Byte> file_buffer;
    bool is_dirty{};

public:

    FlexRamDisk() = delete;
    FlexRamDisk(const FlexRamDisk &src) = delete;
    FlexRamDisk(FlexRamDisk &&src) = delete;
    FlexRamDisk(const fs::path &p_path, std::ios::openmode mode,
                const FileTimeAccess &fileTimeAccess);
    ~FlexRamDisk() override;

    FlexRamDisk &operator= (const FlexRamDisk &src) = delete;
    FlexRamDisk &operator= (FlexRamDisk &&src) = delete;

    bool ReadSector(Byte *buffer, int trk, int sec,
                    std::optional<int> side = std::nullopt) const override;
    bool WriteSector(const Byte *buffer, int trk, int sec,
                     std::optional<int> side = std::nullopt) override;

private:
    bool close();
};

#endif // RFILECNT_INCLUDED

