/*
    rfilecnt.h


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2018  W. Schwotzer

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

#include "ffilecnt.h"
#include <memory>

class FlexRamFileContainer : public FlexFileContainer
{

private:

    std::unique_ptr<Byte[]> file_buffer;

public:

    FlexRamFileContainer() = delete;
    FlexRamFileContainer(const FlexRamFileContainer &) = delete;
    FlexRamFileContainer(FlexRamFileContainer &&);
    FlexRamFileContainer(const char *path, const char *mode);
    virtual ~FlexRamFileContainer();

    FlexRamFileContainer &operator= (const FlexRamFileContainer &) = delete;
    FlexRamFileContainer &operator= (FlexRamFileContainer &&);

    virtual bool ReadSector(Byte *buffer, int trk, int sec) const;
    virtual bool WriteSector(const Byte *buffer, int trk, int sec);
    virtual int Close();

private:
    virtual void Initialize_for_flx_format(
        s_floppy        *pfloppy,
        s_flex_header   *pheader,
        bool        wp);
    virtual void Initialize_for_dsk_format(
        s_floppy        *pfloppy,
        s_formats       *pformat,
        bool        wp);
};

#endif // RFILECNT_INCLUDED 

