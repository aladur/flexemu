/*
    iffilcnt.cpp

    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2023  W. Schwotzer

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

#include "iffilcnt.h"
#include "ffilecnt.h"
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>

FlexFileContainerIteratorImp::FlexFileContainerIteratorImp(
    FlexFileContainer *aBase)
    : base(aBase), dirIndex(-1), dirTrackSector{0, 0}
{
    dirSector.next = first_dir_trk_sec;
}

FlexFileContainerIteratorImp::~FlexFileContainerIteratorImp()
{
}

bool FlexFileContainerIteratorImp::operator==(const FileContainerIf *src) const
{
    return (base == nullptr && src == nullptr) ||
           (((const FileContainerIf *)base == src) && (dirIndex == -1));
}

void FlexFileContainerIteratorImp::AtEnd()
{
    base = nullptr;
}

bool FlexFileContainerIteratorImp::NextDirEntry(const char *filePattern)
{
    dirEntry.SetEmpty();

    while (dirEntry.IsEmpty())
    {
        dirIndex++;

        if ((dirIndex % DIRENTRIES) == 0)
        {
            if (!dirSector.next.sec && !dirSector.next.trk)
            {
                return false;
            }

            dirTrackSector = dirSector.next;

            if (!base->ReadSector((Byte *)&dirSector,
                                  dirTrackSector.trk, dirTrackSector.sec))
            {
                std::stringstream stream;

                stream << dirTrackSector;
                throw FlexException(FERR_READING_TRKSEC,
                                    stream.str(),
                                    base->GetPath().c_str());
            }
        }

        s_dir_entry *pd = &dirSector.dir_entry[dirIndex % DIRENTRIES];

        // an empty entry aborts the search
        if (pd->filename[0] == DE_EMPTY)
        {
            return false;
        }

        // look for the next used directory entry
        if (pd->filename[0] != DE_DELETED)
        {
            // ok, found a valid directory entry
            std::string fileName(pd->filename, 0U, FLEX_BASEFILENAME_LENGTH);
            std::string fileExtension(pd->file_ext, 0U, FLEX_FILEEXT_LENGTH);
            fileName += '.' + fileExtension;

            if (multimatches(fileName.c_str(), filePattern, ';', true))
            {
                dirEntry.SetDate(BDate(pd->day, pd->month, pd->year));
                dirEntry.SetTime(BTime(pd->hour & 0x7F, pd->minute, 0U));
                dirEntry.SetTotalFileName(fileName.c_str());
                dirEntry.SetAttributes(pd->file_attr);
                dirEntry.SetSectorMap(pd->sector_map);
                dirEntry.SetStartTrkSec(pd->start.trk, pd->start.sec);
                dirEntry.SetEndTrkSec(pd->end.trk, pd->end.sec);
                dirEntry.SetFileSize(
                    getValueBigEndian<Word>(&pd->records[0]) * base->GetBytesPerSector());
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
bool FlexFileContainerIteratorImp::DeleteCurrent()
{
    st_t start, end;
    st_t fc_end;
    Byte buffer[SECTOR_SIZE];
    s_dir_entry *pd;

    if (base == nullptr)
    {
        return false;
    }

    // reread directory sector to get current content
    if (!base->ReadSector((Byte *)&dirSector,
                          dirTrackSector.trk, dirTrackSector.sec))
    {
        std::stringstream stream;

        stream << dirTrackSector;
        throw FlexException(FERR_READING_TRKSEC,
                            stream.str(),
                            base->GetPath().c_str());
    }

    pd = &dirSector.dir_entry[dirIndex % DIRENTRIES];
    start = pd->start;
    end = pd->end;
    auto records = getValueBigEndian<Word>(&pd->records[0]);
    auto &sis = *reinterpret_cast<s_sys_info_sector *>(buffer);

    // deleted file is signed by 0xFF as first byte of filename
    pd->filename[0] = DE_DELETED;

    if (!base->WriteSector((const Byte *)&dirSector,
                           dirTrackSector.trk, dirTrackSector.sec))
    {
        std::stringstream stream;

        stream << dirTrackSector;
        throw FlexException(FERR_WRITING_TRKSEC,
                            stream.str(),
                            base->GetPath().c_str());
    }

    /* read sysstem info sector (SIS) */
    if (!base->ReadSector(&buffer[0], sis_trk_sec.trk, sis_trk_sec.sec))
    {
        std::stringstream stream;

        stream << sis_trk_sec;
        throw FlexException(FERR_READING_TRKSEC,
                            stream.str(),
                            base->GetPath().c_str());
    }

    fc_end = sis.sir.fc_end;

    if (fc_end.sec != 0 || fc_end.trk != 0)
    {
        // add deleted file to free chain if free chain not empty
        if (!base->ReadSector(&buffer[0], fc_end.trk, fc_end.sec))
        {
            std::stringstream stream;

            stream << fc_end;
            throw FlexException(FERR_READING_TRKSEC,
                                stream.str(),
                                base->GetPath().c_str());
        }

        buffer[0] = start.trk;
        buffer[1] = start.sec;

        if (!base->WriteSector(&buffer[0], fc_end.trk, fc_end.sec))
        {
            std::stringstream stream;

            stream << fc_end;
            throw FlexException(FERR_WRITING_TRKSEC,
                                stream.str(),
                                base->GetPath().c_str());
        }

        if (!base->ReadSector(&buffer[0], sis_trk_sec.trk,
                              sis_trk_sec.sec))
        {
            std::stringstream stream;

            stream << sis_trk_sec;
            throw FlexException(FERR_READING_TRKSEC,
                                stream.str(),
                                base->GetPath().c_str());
        }

        sis.sir.fc_end = end;
    }
    else
    {
        // create a new free chain if free chain is empty
        if (!base->ReadSector(&buffer[0], sis_trk_sec.trk,
                              sis_trk_sec.sec))
        {
            std::stringstream stream;

            stream << sis_trk_sec;
            throw FlexException(FERR_READING_TRKSEC,
                                stream.str(),
                                base->GetPath().c_str());
        }

        sis.sir.fc_start = start;
        sis.sir.fc_end = end;
    }

    // update sys info sector
    // update number of free sectors
    // and end of free chain trk/sec
    Word free = getValueBigEndian<Word>(&sis.sir.free[0]);
    free += records;
    setValueBigEndian<Word>(&sis.sir.free[0], free);

    if (!base->WriteSector(&buffer[0], sis_trk_sec.trk,
                          sis_trk_sec.sec))
    {
        std::stringstream stream;

        stream << sis_trk_sec;
        throw FlexException(FERR_WRITING_TRKSEC,
                            stream.str(),
                            base->GetPath().c_str());
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

    if (base == nullptr)
    {
        return false;
    }

    // reread directory sector to get current content
    if (!base->ReadSector((Byte *)&dirSector,
                          dirTrackSector.trk, dirTrackSector.sec))
    {
        std::stringstream stream;

        stream << dirTrackSector;
        throw FlexException(FERR_READING_TRKSEC,
                            stream.str(),
                            base->GetPath().c_str());
    }

    pd = &dirSector.dir_entry[dirIndex % DIRENTRIES];

    std::string totalName(newName);

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
                           dirTrackSector.trk, dirTrackSector.sec))
    {
        std::stringstream stream;

        stream << dirTrackSector;
        throw FlexException(FERR_WRITING_TRKSEC,
                            stream.str(),
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

    if (base == nullptr)
    {
        return false;
    }

    // reread directory sector to get current content
    if (!base->ReadSector((Byte *)&dirSector,
                          dirTrackSector.trk, dirTrackSector.sec))
    {
        std::stringstream stream;

        stream << dirTrackSector;
        throw FlexException(FERR_READING_TRKSEC,
                            stream.str(),
                            base->GetPath().c_str());
    }

    pd = &dirSector.dir_entry[dirIndex % DIRENTRIES];
    pd->day = static_cast<Byte>(date.GetDay());
    pd->month = static_cast<Byte>(date.GetMonth());
    pd->year = static_cast<Byte>(date.GetYear() % 100);

    if (!base->WriteSector((const Byte *)&dirSector,
                           dirTrackSector.trk, dirTrackSector.sec))
    {
        std::stringstream stream;

        stream << dirTrackSector;
        throw FlexException(FERR_WRITING_TRKSEC,
                            stream.str(),
                            base->GetPath().c_str());
    }

    return true;
}

// set the date in the actual selected directory entry
// should only be used after an successful
// Only valid if the iterator has a valid directory entry
bool FlexFileContainerIteratorImp::SetAttributesCurrent(Byte attributes)
{
    s_dir_entry *pd;

    if (base == nullptr)
    {
        return false;
    }

    // reread directory sector to get current content
    if (!base->ReadSector((Byte *)&dirSector,
                          dirTrackSector.trk, dirTrackSector.sec))
    {
        std::stringstream stream;

        stream << dirTrackSector;
        throw FlexException(FERR_READING_TRKSEC,
                            stream.str(),
                            base->GetPath().c_str());
    }

    pd = &dirSector.dir_entry[dirIndex % DIRENTRIES];
    pd->file_attr = attributes;

    if (!base->WriteSector((const Byte *)&dirSector,
                           dirTrackSector.trk, dirTrackSector.sec))
    {
        std::stringstream stream;

        stream << dirTrackSector;
        throw FlexException(FERR_WRITING_TRKSEC,
                            stream.str(),
                            base->GetPath().c_str());
    }

    return true;
}

