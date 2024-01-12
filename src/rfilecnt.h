/*
    rfilecnt.h


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2023  W. Schwotzer

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

#include "efiletim.h"
#include "ffilecnt.h"
#include <memory>

class FlexRamFileContainer : public FlexFileContainer
{

private:

    std::unique_ptr<Byte[]> file_buffer;
    bool is_dirty;

public:

    FlexRamFileContainer() = delete;
    FlexRamFileContainer(const FlexRamFileContainer &) = delete;
    FlexRamFileContainer(FlexRamFileContainer &&) noexcept;
    FlexRamFileContainer(const char *path, const char *mode,
                         const FileTimeAccess &fileTimeAccess);
    virtual ~FlexRamFileContainer();

    FlexRamFileContainer &operator= (const FlexRamFileContainer &) = delete;
    FlexRamFileContainer &operator= (FlexRamFileContainer &&) noexcept;

    virtual bool ReadSector(Byte *buffer, int trk, int sec,
                            int side = -1) const override;
    virtual bool WriteSector(const Byte *buffer, int trk, int sec,
                            int side = -1) override;

private:
    bool close();
};

#endif // RFILECNT_INCLUDED 

