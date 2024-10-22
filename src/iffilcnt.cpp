/*
    iffilcnt.cpp

    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2024  W. Schwotzer

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
#include "flexerr.h"
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <array>
#include <cstring>

FlexDiskIteratorImp::FlexDiskIteratorImp(FlexDisk *p_base)
    : base(p_base), dirIndex(-1), dirTrackSector{0, 0}
{
    dirSector.next = first_dir_trk_sec;
}

bool FlexDiskIteratorImp::operator==(const IFlexDiskByFile *rhs) const
{
    return (base == nullptr && rhs == nullptr) ||
           ((base == rhs) && (dirIndex == -1));
}

void FlexDiskIteratorImp::AtEnd()
{
    base = nullptr;
}

bool FlexDiskIteratorImp::NextDirEntry(const std::string &wildcard)
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

            if (!base->ReadSector(reinterpret_cast<Byte *>(&dirSector),
                                  dirTrackSector.trk, dirTrackSector.sec))
            {
                std::stringstream stream;

                stream << dirTrackSector;
                throw FlexException(FERR_READING_TRKSEC,
                                    stream.str(),
                                    base->GetPath());
            }
        }

        const auto &dir_entry = dirSector.dir_entries[dirIndex % DIRENTRIES];

        // an empty entry aborts the search
        if (dir_entry.filename[0] == DE_EMPTY)
        {
            return false;
        }

        // look for the next used directory entry
        if (dir_entry.filename[0] != DE_DELETED)
        {
            // ok, found a valid directory entry
            std::string fileName(flx::getstr<>(dir_entry.filename));
            std::string fileExtension(flx::getstr<>(dir_entry.file_ext));
            fileName += '.' + fileExtension;

            if (flx::multimatches(fileName, wildcard, ';', true))
            {
                dirEntry = FlexDisk::CreateDirEntryFrom(dir_entry, fileName);
            }
        }
    }

    return true;
}

// deletes the file on which the iterator currently
// is pointing on
// Only valid if the iterator has a valid directory entry
bool FlexDiskIteratorImp::DeleteCurrent()
{
    if (base == nullptr)
    {
        return false;
    }

    // reread directory sector to get current content
    if (!base->ReadSector(reinterpret_cast<Byte *>(&dirSector),
                          dirTrackSector.trk, dirTrackSector.sec))
    {
        std::stringstream stream;

        stream << dirTrackSector;
        throw FlexException(FERR_READING_TRKSEC,
                            stream.str(),
                            base->GetPath());
    }

    auto &dir_entry = dirSector.dir_entries[dirIndex % DIRENTRIES];
    auto start = dir_entry.start;
    auto end = dir_entry.end;
    auto records = flx::getValueBigEndian<Word>(&dir_entry.records[0]);

    // deleted file is signed by 0xFF as first byte of filename
    dir_entry.filename[0] = DE_DELETED;

    if (!base->WriteSector(reinterpret_cast<const Byte *>(&dirSector),
                           dirTrackSector.trk, dirTrackSector.sec))
    {
        std::stringstream stream;

        stream << dirTrackSector;
        throw FlexException(FERR_WRITING_TRKSEC,
                            stream.str(),
                            base->GetPath());
    }

    base->SetNextDirectoryPosition(st_t{});
    base->DeleteFromFilenames(dirEntry.GetTotalFileName());

    /* read system info sector (SIS) */
    s_sys_info_sector sis{};
    if (!base->ReadSector(reinterpret_cast<Byte *>(&sis),
                          sis_trk_sec.trk, sis_trk_sec.sec))
    {
        std::stringstream stream;

        stream << sis_trk_sec;
        throw FlexException(FERR_READING_TRKSEC,
                            stream.str(),
                            base->GetPath());
    }

    auto fc_end = sis.sir.fc_end;

    if (fc_end.sec != 0 || fc_end.trk != 0)
    {
        // add deleted file to free chain if free chain not empty
        SectorBuffer_t sectorBuffer{};
        if (!base->ReadSector(sectorBuffer.data(), fc_end.trk, fc_end.sec))
        {
            std::stringstream stream;

            stream << fc_end;
            throw FlexException(FERR_READING_TRKSEC,
                                stream.str(),
                                base->GetPath());
        }

        sectorBuffer[0] = start.trk;
        sectorBuffer[1] = start.sec;

        if (!base->WriteSector(sectorBuffer.data(), fc_end.trk, fc_end.sec))
        {
            std::stringstream stream;

            stream << fc_end;
            throw FlexException(FERR_WRITING_TRKSEC,
                                stream.str(),
                                base->GetPath());
        }

        if (!base->ReadSector(reinterpret_cast<Byte *>(&sis),
                              sis_trk_sec.trk, sis_trk_sec.sec))
        {
            std::stringstream stream;

            stream << sis_trk_sec;
            throw FlexException(FERR_READING_TRKSEC,
                                stream.str(),
                                base->GetPath());
        }

        sis.sir.fc_end = end;
    }
    else
    {
        // create a new free chain if free chain is empty
        if (!base->ReadSector(reinterpret_cast<Byte *>(&sis),
                              sis_trk_sec.trk, sis_trk_sec.sec))
         {
            std::stringstream stream;

            stream << sis_trk_sec;
            throw FlexException(FERR_READING_TRKSEC,
                                stream.str(),
                                base->GetPath());
        }

        sis.sir.fc_start = start;
        sis.sir.fc_end = end;
    }

    // update sys info sector
    // update number of free sectors
    // and end of free chain trk/sec
    Word free = flx::getValueBigEndian<Word>(&sis.sir.free[0]);
    free += records;
    flx::setValueBigEndian<Word>(&sis.sir.free[0], free);

    if (!base->WriteSector(reinterpret_cast<const Byte *>(&sis),
                           sis_trk_sec.trk, sis_trk_sec.sec))
    {
        std::stringstream stream;

        stream << sis_trk_sec;
        throw FlexException(FERR_WRITING_TRKSEC,
                            stream.str(),
                            base->GetPath());
    }

    return true;
}

// Renames the file on which the iterator currently
// is pointing on
// Only valid if the iterator has a valid directory entry
bool FlexDiskIteratorImp::RenameCurrent(const std::string &newName)
{
    std::string name;
    std::string ext;

    if (base == nullptr)
    {
        return false;
    }

    // reread directory sector to get current content
    if (!base->ReadSector(reinterpret_cast<Byte *>(&dirSector),
                          dirTrackSector.trk, dirTrackSector.sec))
    {
        std::stringstream stream;

        stream << dirTrackSector;
        throw FlexException(FERR_READING_TRKSEC,
                            stream.str(),
                            base->GetPath());
    }

    const auto idx = static_cast<Byte>(dirIndex % DIRENTRIES);
    auto &dir_entry = dirSector.dir_entries[idx];

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
    std::memset(dir_entry.filename, 0, FLEX_BASEFILENAME_LENGTH);
    strncpy(dir_entry.filename, name.c_str(), FLEX_BASEFILENAME_LENGTH);
    std::memset(dir_entry.file_ext, 0, FLEX_FILEEXT_LENGTH);
    strncpy(dir_entry.file_ext, ext.c_str(), FLEX_FILEEXT_LENGTH);

    if (!base->WriteSector(reinterpret_cast<const Byte *>(&dirSector),
                           dirTrackSector.trk, dirTrackSector.sec))
    {
        std::stringstream stream;

        stream << dirTrackSector;
        throw FlexException(FERR_WRITING_TRKSEC,
                            stream.str(),
                            base->GetPath());
    }
    base->DeleteFromFilenames(dirEntry.GetTotalFileName());
    base->AddToFilenames(newName, FlexDisk::s_dir_pos{dirTrackSector, idx});

    return true;
}

// Set date for the file on which the iterator currently
// is pointing on
// Only valid if the iterator has a valid directory entry
bool FlexDiskIteratorImp::SetDateCurrent(const BDate &date)
{
    if (base == nullptr)
    {
        return false;
    }

    // reread directory sector to get current content
    if (!base->ReadSector(reinterpret_cast<Byte *>(&dirSector),
                          dirTrackSector.trk, dirTrackSector.sec))
    {
        std::stringstream stream;

        stream << dirTrackSector;
        throw FlexException(FERR_READING_TRKSEC,
                            stream.str(),
                            base->GetPath());
    }

    auto &dir_entry = dirSector.dir_entries[dirIndex % DIRENTRIES];
    dir_entry.day = static_cast<Byte>(date.GetDay());
    dir_entry.month = static_cast<Byte>(date.GetMonth());
    dir_entry.year = static_cast<Byte>(date.GetYear() % 100);

    if (!base->WriteSector(reinterpret_cast<const Byte *>(&dirSector),
                           dirTrackSector.trk, dirTrackSector.sec))
    {
        std::stringstream stream;

        stream << dirTrackSector;
        throw FlexException(FERR_WRITING_TRKSEC,
                            stream.str(),
                            base->GetPath());
    }

    return true;
}

// set the date in the actual selected directory entry
// should only be used after an successful
// Only valid if the iterator has a valid directory entry
bool FlexDiskIteratorImp::SetAttributesCurrent(Byte attributes)
{
    if (base == nullptr)
    {
        return false;
    }

    // reread directory sector to get current content
    if (!base->ReadSector(reinterpret_cast<Byte *>(&dirSector),
                          dirTrackSector.trk, dirTrackSector.sec))
    {
        std::stringstream stream;

        stream << dirTrackSector;
        throw FlexException(FERR_READING_TRKSEC,
                            stream.str(),
                            base->GetPath());
    }

    dirSector.dir_entries[dirIndex % DIRENTRIES].file_attr = attributes;

    if (!base->WriteSector(reinterpret_cast<const Byte *>(&dirSector),
                           dirTrackSector.trk, dirTrackSector.sec))
    {
        std::stringstream stream;

        stream << dirTrackSector;
        throw FlexException(FERR_WRITING_TRKSEC,
                            stream.str(),
                            base->GetPath());
    }

    return true;
}

