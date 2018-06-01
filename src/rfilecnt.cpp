/*
    rfilecnt.cpp


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

#include <string.h>     // needed for memcpy
#include "rfilecnt.h"
#include "flexerr.h"


FlexRamFileContainer::FlexRamFileContainer(const char *path, const char *mode) :
    FlexFileContainer(path, mode), pfb(NULL)
{
    unsigned int sectors;

    sectors = ByteOffset(param.max_track + 1, 1) / param.byte_p_sector ;
    pfb = new Byte[sectors * param.byte_p_sector];

    if (fseek(fp, 0, SEEK_SET))
    {
        throw FlexException(FERR_READING_FROM, fp.GetPath());
    }

    // read total disk into memory
    if (fread(&pfb[0], param.byte_p_sector, sectors, fp) != sectors)
    {
        throw FlexException(FERR_READING_FROM, fp.GetPath());
    }
}

FlexRamFileContainer::~FlexRamFileContainer()
{
    // final cleanup: close if not already done
    try
    {
        Close();
    }
    catch (...)
    {
        // ignore exceptions
        // usually the file should be closed already
    }
}

int FlexRamFileContainer::Close()
{
    bool throwException = false;

    if (fp != NULL && (pfb != NULL))
    {
        unsigned int sectors;

        sectors = ByteOffset(param.max_track + 1, 1) / param.byte_p_sector;

        if (fseek(fp, 0, SEEK_SET))
        {
            throwException = true;
        }

        if (fwrite(&pfb[0], param.byte_p_sector, sectors, fp) != sectors)
        {
            throwException = true;
        }

        fp.Close();
    }

    delete [] pfb;
    pfb = NULL;

    if (throwException)
    {
        throw FlexException(FERR_WRITING_TO, fp.GetPath());
    }

    return 1;
}

bool FlexRamFileContainer::ReadSector(Byte *pbuffer, int trk, int sec) const
{
    int pos;

    if (pfb == NULL)
    {
        return false;
    }

    pos = ByteOffset(trk, sec);

    if (pos < 0)
    {
        return false;
    }

    memcpy(pbuffer, pfb + pos, param.byte_p_sector);
    return true;
}

bool FlexRamFileContainer::WriteSector(const Byte *pbuffer, int trk, int sec)
{
    int pos;

    if (pfb == NULL)
    {
        return false;
    }

    pos = ByteOffset(trk, sec);

    if (pos < 0)
    {
        return false;
    }

    memcpy(pfb + pos, pbuffer, param.byte_p_sector);
    return true;
}

void FlexRamFileContainer::Initialize_for_flx_format(
    s_floppy        *pfloppy,
    s_flex_header   *pheader,
    bool            wp)
{
    FlexFileContainer::Initialize_for_flx_format(pfloppy, pheader, wp);
    pfloppy->type |= TYPE_RAM_CONTAINER;
}

void FlexRamFileContainer::Initialize_for_dsk_format(
    s_floppy    *pfloppy,
    s_formats   *pformat,
    bool        wp)
{
    FlexFileContainer::Initialize_for_dsk_format(pfloppy, pformat, wp);
    pfloppy->type |= TYPE_RAM_CONTAINER;
}
