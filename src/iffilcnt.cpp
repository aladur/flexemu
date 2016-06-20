/*
    iffilcnt.cpp

    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2004  W. Schwotzer

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

#ifndef __iffilcnt_cpp__
#define __iffilcnt_cpp__

#include "iffilcnt.h"
#include "ffilecnt.h"
#include <string>
#include <algorithm>
#include <iterator>

FlexFileContainerIteratorImp::FlexFileContainerIteratorImp(
    FlexFileContainer *aBase)
    : base(aBase), dirIndex(-1), dirSectorTrk(0), dirSectorSec(0)
{
    dirSector.next_trk = 0;
    dirSector.next_sec = 5;
}

FlexFileContainerIteratorImp::~FlexFileContainerIteratorImp()
{
}

bool FlexFileContainerIteratorImp::operator==(const FileContainerIf *src) const
{
    return (base == NULL && src == NULL) ||
           (((const FileContainerIf *)base == src) && (dirIndex == -1));
}

void FlexFileContainerIteratorImp::AtEnd()
{
    base = NULL;
}

bool FlexFileContainerIteratorImp::NextDirEntry(const char *filePattern)
{
    std::string fileName;

    dirEntry.SetEmpty();

    while (dirEntry.IsEmpty())
    {
        dirIndex++;

        if ((dirIndex % 10) == 0)
        {
            if (dirSector.next_trk == 0 &&
                dirSector.next_sec == 0)
            {
                return false;
            }

            dirSectorTrk = dirSector.next_trk;
            dirSectorSec = dirSector.next_sec;

            if (!base->ReadSector((Byte *)&dirSector,
                                  dirSectorTrk, dirSectorSec))
            {
                throw FlexException(FERR_READING_TRKSEC, dirSectorTrk,
                                    dirSectorSec, base->GetPath().c_str());
            }
        }

        s_dir_entry *pd = &dirSector.dir_entry[dirIndex % 10];

        // an empty entry aborts the search
        if (pd->filename[0] == '\0')
        {
            return false;
        }

        // look for the next used directory entry
        if (pd->filename[0] != -1)
        {
            // ok, found a valid directory entry
            std::string::size_type length;

            length = std::min(strlen(pd->filename), FLEX_BASEFILENAME_LENGTH);
            fileName = std::string(pd->filename, length);
            fileName += ".";
            length = std::min(strlen(pd->file_ext), FLEX_FILEEXT_LENGTH);
            fileName += std::string(pd->file_ext, length);

            if (multimatches(fileName.c_str(), filePattern, ';', true))
            {
                dirEntry.SetDate(pd->day, pd->month, pd->year);
                dirEntry.SetTotalFileName(fileName.c_str());
                dirEntry.SetAttributes(pd->file_attr);
                dirEntry.SetSectorMap(pd->sector_map);
                dirEntry.SetStartTrkSec(pd->start_trk,
                                        pd->start_sec);
                dirEntry.SetEndTrkSec(pd->end_trk, pd->end_sec)
                ;
                dirEntry.SetSize(
                    (pd->records[0] << 8 |
                     pd->records[1]) * base->GetBytesPerSector());
                dirEntry.SetSectorMap(pd->sector_map);
                dirEntry.ClearEmpty();
            }
        }
    }

    return true;
}

// deletes the file on which the iterator currently
// is pointing on
// Only valid if the iterator has a valid directory entry
bool FlexFileContainerIteratorImp::DeleteCurrent(void)
{
    int start_trk, start_sec, end_trk, end_sec;
    int fc_end_trk, fc_end_sec;
    int records, free;
    Byte buffer[SECTOR_SIZE];
    s_sys_info_sector *psis;
    s_dir_entry *pd;

    if (base == NULL)
    {
        return false;
    }

    // reread directory sector to get current content
    if (!base->ReadSector((Byte *)&dirSector, dirSectorTrk, dirSectorSec))
    {
        throw FlexException(FERR_READING_TRKSEC, dirSectorTrk,
                            dirSectorSec, base->GetPath().c_str());
    }

    pd = &dirSector.dir_entry[dirIndex % 10];
    start_trk = pd->start_trk;
    start_sec = pd->start_sec;
    end_sec = pd->end_sec;
    end_trk = pd->end_trk;
    records = (pd->records[0] << 8) | pd->records[1];
    psis = (s_sys_info_sector *)buffer;

    // deleted file is signed by 0xFF as first byte of filename
    pd->filename[0] = (char)0xFF;

    if (!base->WriteSector((const Byte *)&dirSector, dirSectorTrk,
                           dirSectorSec))
    {
        throw FlexException(FERR_WRITING_TRKSEC, dirSectorTrk,
                            dirSectorSec, base->GetPath().c_str());
    }

    if (!base->ReadSector(&buffer[0], 0, 3))   /* read sys info sector */
    {
        throw FlexException(FERR_READING_TRKSEC, 0, 3, base->GetPath().c_str());
    }

    fc_end_trk = psis->fc_end_trk;
    fc_end_sec = psis->fc_end_sec;

    if (fc_end_trk || fc_end_sec)
    {
        // add deleted file to free chain if free chain not empty
        if (!base->ReadSector(&buffer[0], fc_end_trk, fc_end_sec))
        {
            throw FlexException(FERR_READING_TRKSEC, fc_end_trk, fc_end_sec,
                                base->GetPath().c_str());
        }

        buffer[0] = start_trk;
        buffer[1] = start_sec;

        if (!base->WriteSector(&buffer[0], fc_end_trk, fc_end_sec))
        {
            throw FlexException(FERR_WRITING_TRKSEC, fc_end_trk, fc_end_sec,
                                base->GetPath().c_str());
        }

        if (!base->ReadSector(&buffer[0], 0, 3))
        {
            throw FlexException(FERR_READING_TRKSEC, 0, 3,
                                base->GetPath().c_str());
        }

        psis->fc_end_trk = end_trk;
        psis->fc_end_sec = end_sec;
    }
    else
    {
        // create a new free chain if free chain is empty
        if (!base->ReadSector(&buffer[0], 0, 3))
        {
            throw FlexException(FERR_READING_TRKSEC, 0, 3,
                                base->GetPath().c_str());
        }

        psis->fc_start_trk = start_trk;
        psis->fc_start_sec = start_sec;
        psis->fc_end_trk = end_trk;
        psis->fc_end_sec = end_sec;
    }

    // update sys info sector
    // update number of free sectors
    // and end of free chain trk/sec
    free = psis->free[0] << 8 | psis->free[1];
    free += records;
    psis->free[0] = free >> 8;
    psis->free[1] = free & 0xff;

    if (!base->WriteSector(&buffer[0], 0, 3))
    {
        throw FlexException(FERR_WRITING_TRKSEC, 0, 3, base->GetPath().c_str());
    }

    return true;
}

// Renames the file on which the iterator currently
// is pointing on
// Only valid if the iterator has a valid directory entry
bool FlexFileContainerIteratorImp::RenameCurrent(const char *newName)
{
    s_dir_entry *pd;
    std::string name, ext;

    if (base == NULL)
    {
        return false;
    }

    // reread directory sector to get current content
    if (!base->ReadSector((Byte *)&dirSector, dirSectorTrk, dirSectorSec))
    {
        throw FlexException(FERR_READING_TRKSEC, dirSectorTrk,
                            dirSectorSec, base->GetPath().c_str());
    }

    pd = &dirSector.dir_entry[dirIndex % 10];

    std::string totalName(newName);

    std::transform(totalName.begin(), totalName.end(), totalName.begin(),
         ::tolower);

    std::string::iterator it =
        std::find(totalName.begin(), totalName.end(), '.');

    if (it != totalName.end())
    {
        // copy the file name only
        std::copy(totalName.begin(), it, std::back_inserter(name));

        // copy the file extension
        it++;
        if (it != totalName.end())
        {
            std::string::iterator itend = it;
            int size = FLEX_FILEEXT_LENGTH;

            while (itend != totalName.end() && (size >= 0))
            {
                ++itend;
                --size;
            }

            std::copy(it, itend, std::back_inserter(ext));
        }
    }
    else
    {
        name = totalName;
    }

    /* update directory entry */
    memset(&pd->filename[0], 0, FLEX_BASEFILENAME_LENGTH);
    strncpy(&pd->filename[0], name.c_str(), FLEX_BASEFILENAME_LENGTH);
    memset(&pd->file_ext[0], 0, FLEX_FILEEXT_LENGTH);
    strncpy(&pd->file_ext[0], ext.c_str(), FLEX_FILEEXT_LENGTH);

    if (!base->WriteSector((const Byte *)&dirSector,
                           dirSectorTrk, dirSectorSec))
    {
        throw FlexException(FERR_WRITING_TRKSEC, dirSectorTrk, dirSectorSec,
                            base->GetPath().c_str());
    }

    return true;
}

// Set date for the file on which the iterator currently
// is pointing on
// Only valid if the iterator has a valid directory entry
bool FlexFileContainerIteratorImp::SetDateCurrent(const BDate &date)
{
    s_dir_entry *pd;

    if (base == NULL)
    {
        return false;
    }

    // reread directory sector to get current content
    if (!base->ReadSector((Byte *)&dirSector, dirSectorTrk, dirSectorSec))
    {
        throw FlexException(FERR_READING_TRKSEC, dirSectorTrk, dirSectorSec,
                            base->GetPath().c_str());
    }

    pd = &dirSector.dir_entry[dirIndex % 10];
    pd->day = date.GetDay();
    pd->month = date.GetMonth();
    pd->year = date.GetYear() % 100;

    if (!base->WriteSector((const Byte *)&dirSector,
                           dirSectorTrk, dirSectorSec))
    {
        throw FlexException(FERR_WRITING_TRKSEC, dirSectorTrk, dirSectorSec,
                            base->GetPath().c_str());
    }

    return true;
}

// set the date in the actual selected directory entry
// should only be used after an successful
// Only valid if the iterator has a valid directory entry
bool FlexFileContainerIteratorImp::SetAttributesCurrent(int attributes)
{
    s_dir_entry *pd;

    if (base == NULL)
    {
        return false;
    }

    // reread directory sector to get current content
    if (!base->ReadSector((Byte *)&dirSector, dirSectorTrk, dirSectorSec))
    {
        throw FlexException(FERR_READING_TRKSEC, dirSectorTrk,
                            dirSectorSec, base->GetPath().c_str());
    }

    pd = &dirSector.dir_entry[dirIndex % 10];
    pd->file_attr = attributes;

    if (!base->WriteSector((const Byte *)&dirSector,
                           dirSectorTrk, dirSectorSec))
    {
        throw FlexException(FERR_WRITING_TRKSEC, dirSectorTrk, dirSectorSec,
                            base->GetPath().c_str());
    }

    return true;
}

#endif // __iffilcnt_h__

