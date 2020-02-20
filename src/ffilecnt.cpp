/*
    ffilecnt.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2019  W. Schwotzer

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


#include "misc1.h"
#include <sys/stat.h>
#include <string>
#include <sstream>
#include <cctype>

#include "fcinfo.h"
#include "flexerr.h"
#include "ffilecnt.h"
#include "fdirent.h"
#include "bdate.h"
#include "fcopyman.h"
#include "ffilebuf.h"
#include "ifilecnt.h"
#include "ifilcnti.h"
#include "iffilcnt.h"

#ifdef UNIX
    std::string FlexFileContainer::bootSectorFile =
        F_DATADIR PATHSEPARATORSTRING BOOT_FILE;
#endif
#ifdef _WIN32
    std::string FlexFileContainer::bootSectorFile = BOOT_FILE;
#endif

/***********************************************/
/* Initialization of a s_flex_header structure */
/***********************************************/

void s_flex_header::initialize(int sector_size, int p_tracks, int p_sectors0,
                               int p_sectors, int p_sides0, int p_sides)
{
    Byte p_sizecode = 1; /* default */
    Byte i;

    p_sides0 = std::max(p_sides0, 1);
    p_sides0 = std::min(p_sides0, 2);
    p_sides = std::max(p_sides, 1);
    p_sides = std::min(p_sides, 2);
    p_tracks = std::min(p_tracks, 255);
    p_sectors0 = std::min(p_sectors0, 255);
    p_sectors = std::min(p_sectors, 255);

    for (i = 15; i >= 7; i--)
    {
        if (sector_size & (1 << i))
        {
            p_sizecode = i - 7;
            break;
        }
    }

    magic_number    = toBigEndian(MAGIC_NUMBER);
    write_protect   = 0;
    sizecode        = p_sizecode;
    sides0          = static_cast<Byte>(p_sides0);
    sectors0        = static_cast<Byte>(p_sectors0 / p_sides0);
    sides           = static_cast<Byte>(p_sides);
    sectors         = static_cast<Byte>(p_sectors / p_sides);
    tracks          = static_cast<Byte>(p_tracks);
    dummy1          = 0;
    dummy2          = 0;
    dummy3          = 0;
    dummy4          = 0;
    dummy5          = 0;
}


/****************************************/
/* Constructor                          */
/****************************************/

FlexFileContainer::FlexFileContainer(const char *path, const char *mode) :
    fp(path, mode), param { },
    file_size(0), is_formatted(true),
    sectors0_side1_max(0), sectors_side1_max(0),
    flx_header { },
    attributes(0)
{
    struct  stat sbuf;

    if (fp == nullptr)
    {
        throw FlexException(FERR_UNABLE_TO_OPEN, fp.GetPath());
    }

    if (stat(fp.GetPath(), &sbuf) || !S_ISREG(sbuf.st_mode))
    {
        throw FlexException(FERR_UNABLE_TO_OPEN, fp.GetPath());
    }

    if (sbuf.st_size == 0)
    {
        // If file has been created or file size 0 then
        // it is marked as an unformatted file container.
        // No records are available yet but it can be formatted
        // from within the emulation.
        is_formatted = false;
        Initialize_unformatted_disk();
        return;
    }

    if (fseek(fp, 0, SEEK_SET))
    {
        throw FlexException(FERR_UNABLE_TO_OPEN, fp.GetPath());
    }
    file_size = static_cast<DWord>(sbuf.st_size);

    if (strchr(fp.GetMode(), '+') == nullptr)
    {
        attributes |= FLX_READONLY;
    }

    if (!stat(fp.GetPath(), &sbuf))
    {
        bool write_protected = ((attributes & FLX_READONLY) != 0);
        Word tracks;
        Word sectors;

        // Try to read the FLX header and check the magic number
        // to identify a FLX formatted disk.
        if (fread(&flx_header, sizeof(flx_header), 1, fp) == 1 &&
            fromBigEndian(flx_header.magic_number) == MAGIC_NUMBER)
        {
            // File is identified as a FLX container format.
            Initialize_for_flx_format(flx_header, write_protected);

            if (!IsFlexFileFormat(TYPE_FLX_CONTAINER))
            {
                // This is a FLX file container but it is not compatible
                // to FLEX.
                is_formatted = false;
            }
            return;
        }
        else
        {
            s_formats format { };

            // check if it is a DSK formated disk
            // read system info sector
            if (IsFlexFileFormat(TYPE_DSK_CONTAINER) &&
                GetFlexTracksSectors(tracks, sectors, 0U))
            {
                // File is identified as a FLEX DSK container format
                format.tracks = tracks;
                format.sectors = sectors;
                format.size = sbuf.st_size;
                Initialize_for_dsk_format(format, write_protected);
                EvaluateTrack0SectorCount();
                return;
            }
        }
    }

    throw FlexException(FERR_IS_NO_FILECONTAINER, fp.GetPath());
}

/****************************************/
/* Destructor                           */
/****************************************/

FlexFileContainer::~FlexFileContainer()
{
}

FlexFileContainer::FlexFileContainer(FlexFileContainer &&src) :
    fp(std::move(src.fp)), param(src.param), file_size(src.file_size),
    is_formatted(src.is_formatted),
    sectors0_side1_max(src.sectors0_side1_max),
    sectors_side1_max(src.sectors_side1_max),
    attributes(src.attributes)
{
}

FlexFileContainer &FlexFileContainer::operator= (FlexFileContainer &&src)
{
    fp = std::move(src.fp);
    param = src.param;
    file_size = src.file_size;
    is_formatted = src.is_formatted;
    sectors0_side1_max = sectors0_side1_max;
    sectors_side1_max = sectors_side1_max;
    attributes = src.attributes;

    return *this;
}

/****************************************/
/* Public interface                     */
/****************************************/

std::string FlexFileContainer::GetPath() const
{
    return fp.GetPath();
}

int FlexFileContainer::GetBytesPerSector() const
{
    return param.byte_p_sector;
}

bool FlexFileContainer::IsWriteProtected() const
{
    return (param.write_protect ? true : false);
}

bool FlexFileContainer::IsTrackValid(int track) const
{
    if (!is_formatted)
    {
        // All tracks have to be seekable because it is unknown
        // how many tracks the disk will have finally
        // and in which order they are formatted.
        return (track >= 0 && track <= 255);
    }

    return (track >= 0 && track <= param.max_track);
}

bool FlexFileContainer::IsSectorValid(int track, int sector) const
{
    if (track)
    {
        return (sector > 0 && sector <= param.max_sector &&
                (param.offset + param.byte_p_track0 +
                 param.byte_p_track * (track - 1) +
                 sector * param.byte_p_sector <= file_size));
    }
    else
    {
        return (sector > 0 && sector <= param.max_sector0);
    }
}

bool FlexFileContainer::IsFormatted() const
{
    return is_formatted;
}

FlexFileContainer *FlexFileContainer::Create(const char *dir, const char *name,
        int t, int s, int fmt)
{
    std::string path;

    if (fmt != TYPE_DSK_CONTAINER && fmt != TYPE_FLX_CONTAINER)
    {
        throw FlexException(FERR_INVALID_FORMAT, fmt);
    }

    Format_disk(t, s, dir, name, fmt);

    path = dir;

    if (!path.empty() && path[path.length()-1] != PATHSEPARATOR)
    {
        path += PATHSEPARATORSTRING;
    }

    path += name;
    return new FlexFileContainer(path.c_str(), "rb+");
}

// return true if file found
// if file found can also be checked by
// !entry.isEmpty
bool FlexFileContainer::FindFile(const char *fileName, FlexDirEntry &entry)
{
    if (is_formatted)
    {
        FileContainerIterator it(fileName);

        it = this->begin();

        if (it != this->end())
        {
            entry = *it;
            return true;
        }
    }

    entry.SetEmpty();
    return false;
}

bool    FlexFileContainer::DeleteFile(const char *filePattern)
{
    if (!is_formatted)
    {
        return false;
    }

    CHECK_CONTAINER_WRITEPROTECTED;

    FileContainerIterator it(filePattern);

    for (it = this->begin(); it != this->end(); ++it)
    {
        it.DeleteCurrent();
    }

    return true;
}

bool    FlexFileContainer::RenameFile(const char *oldName, const char *newName)
{
    if (!is_formatted)
    {
        return false;
    }

    CHECK_CONTAINER_WRITEPROTECTED;

    FlexDirEntry de;

    // prevent overwriting of an existing file
    if (FindFile(newName, de))
    {
        throw FlexException(FERR_FILE_ALREADY_EXISTS, newName);
    }

    FileContainerIterator it(oldName);

    it = this->begin();

    if (it == this->end())
    {
        throw FlexException(FERR_NO_FILE_IN_CONTAINER, oldName, fp.GetPath());
    }
    else
    {
        it.RenameCurrent(newName);
    }

    return true;
}

bool FlexFileContainer::FileCopy(const char *sourceName, const char *destName,
                                 FileContainerIf &destination)
{
    if (!is_formatted)
    {
        return false;
    }

    FlexCopyManager copyMan;

    return copyMan.FileCopy(sourceName, destName, *this, destination);
}

bool    FlexFileContainer::GetInfo(FlexContainerInfo &info) const
{
    if (is_formatted)
    {
        s_sys_info_sector sis;
        char disk_name[13];
        int year;

        if (!ReadSector(reinterpret_cast<Byte *>(&sis), sis_trk_sec.trk,
                        sis_trk_sec.sec))
        {
            std::stringstream stream;

            stream << sis_trk_sec;
            throw FlexException(FERR_READING_TRKSEC, stream.str(),
                                fp.GetPath());
        }

        if (sis.sir.year < 75)
        {
            year = sis.sir.year + 2000;
        }
        else
        {
            year = sis.sir.year + 1900;
        }

        strncpy(disk_name, sis.sir.disk_name, sizeof(sis.sir.disk_name));
        disk_name[sizeof(sis.sir.disk_name)] = '\0';
        if (sis.sir.disk_ext[0] != '\0')
        {
            strcat(disk_name, ".");
            size_t index = strlen(disk_name);
            for (size_t i = 0; i < sizeof(sis.sir.disk_ext); ++i)
            {
                char ch = sis.sir.disk_ext[i];
                if (ch >= ' ' && ch <= '~')
                {
                    disk_name[index++] = ch;
                }
                else
                {
                    break;
                }
            }
            disk_name[index++] = '\0';
        }
        info.SetDate(sis.sir.day, sis.sir.month, year);
        info.SetFree(getValueBigEndian<Word>(&sis.sir.free[0]) *
                     param.byte_p_sector);
        info.SetTotalSize((sis.sir.last.sec * (sis.sir.last.trk + 1)) *
                           param.byte_p_sector);
        info.SetName(disk_name);
        info.SetNumber(getValueBigEndian<Word>(&sis.sir.disk_number[0]));
    }

    info.SetTrackSector(param.max_track + 1U, param.max_sector);
    info.SetIsFormatted(is_formatted);
    info.SetPath(fp.GetPath());
    info.SetType(param.type);
    info.SetAttributes(attributes);

    return true;
}

int FlexFileContainer::GetContainerType() const
{
    return param.type;
}

bool FlexFileContainer::CheckFilename(const char *fileName) const
{
    int     result; // result from sscanf should be int
    char    dot;
    char    name[9];
    char    ext[4];
    const char    *format;

    dot    = '\0';
    format = "%1[A-Za-z]%7[A-Za-z0-9_-]";
    result = sscanf(fileName, format, name, &name[1]);

    if (!result || result == EOF)
    {
        return false;
    }

    if (result == 1)
    {
        format = "%*1[A-Za-z]%c%1[A-Za-z]%2[A-Za-z0-9_-]";
    }
    else
    {
        format = "%*1[A-Za-z]%*7[A-Za-z0-9_-]%c%1[A-Za-z]%2[A-Za-z0-9_-]";
    }

    result = sscanf(fileName, format, &dot, ext, &ext[1]);

    if (!result || result == 1 || result == EOF)
    {
        return false;
    }

    if (strlen(name) + strlen(ext) + (dot == '.' ? 1 : 0) != strlen(fileName))
    {
        return false;
    }

    return true;
}

/******************************/
/* Nonpublic interface        */
/******************************/

FileContainerIteratorImpPtr FlexFileContainer::IteratorFactory()
{
    return FileContainerIteratorImpPtr(new FlexFileContainerIteratorImp(this));
}

// if successfull return true. If error return false
bool FlexFileContainer::WriteFromBuffer(const FlexFileBuffer &buffer,
                                        const char *fileName /* = nullptr */)
{
    if (!is_formatted)
    {
        return false;
    }

    Byte trk = 0, sec = 0;
    st_t start;
    st_t next;
    Word recordNr; // Number of record. For random files it does not contain
                   // the two records for the sector map.
    int count;
    FlexDirEntry    de;
    s_sys_info_sector sis;
    const char      *pFileName = fileName;
    // sectorBuffer[2] and [1] are used for the Sector Map
    Byte sectorBuffer[3][SECTOR_SIZE];

    if (fileName == nullptr)
    {
        pFileName = buffer.GetFilename();
    }

    if (FindFile(pFileName, de))
    {
        throw FlexException(FERR_FILE_ALREADY_EXISTS, pFileName);
    }

    // read sys info sector
    if (!ReadSector(reinterpret_cast<Byte *>(&sis), sis_trk_sec.trk,
                    sis_trk_sec.sec))
    {
        std::stringstream stream;

        stream << sis_trk_sec;
        throw FlexException(FERR_READING_TRKSEC, stream.str(), fp.GetPath());
    } // get start trk/sec of free chain

    next = start = sis.sir.fc_start;
    {
        // write each sector to buffer
        Byte repeat = 0;
        unsigned int smIndex,  smSector;
        Word nextPTrk, nextPSec; // contains next physical trk/sec
        smIndex = 1;
        smSector = 2;
        recordNr = nextPTrk = nextPSec = 0;

        // at the begin of a random file reserve two sectors for the sector map
        if (recordNr == 0 && buffer.IsRandom())
        {
            repeat = 2;
        }

        do
        {
            for (count = repeat; count >= 0; count--)
            {
                trk = next.trk;
                sec = next.sec;

                if (trk == 0 && sec == 0)
                {
                    // disk full
                    throw FlexException(FERR_DISK_FULL_WRITING,
                                        fp.GetPath(), pFileName);
                }

                if (!ReadSector(&sectorBuffer[count][0], trk, sec))
                {
                    std::stringstream stream;

                    stream << next;
                    throw FlexException(FERR_READING_TRKSEC,
                                        stream.str(), fp.GetPath());
                }
                else if (count)
                {
                    memset(&sectorBuffer[count][2], 0, SECTOR_SIZE - 2);
                }

                next.trk = sectorBuffer[count][0];
                next.sec = sectorBuffer[count][1];
            }

            if (!buffer.CopyTo(&sectorBuffer[0][4], SECTOR_SIZE - 4,
                               recordNr * (SECTOR_SIZE - 4), 0x00))
            {
                std::stringstream stream;

                stream << next;
                throw FlexException(FERR_WRITING_TRKSEC,
                                    stream.str(), fp.GetPath());
            }

            recordNr++;

            // if random file update sector map
            if (buffer.IsRandom())
            {
                if (trk != nextPTrk || sec != nextPSec ||
                    sectorBuffer[smSector][smIndex + 2] == 255)
                {
                    smIndex += 3;

                    if (smIndex >= SECTOR_SIZE)
                    {
                        smSector--;

                        if (smSector == 0)
                        {
                            throw FlexException(FERR_RECORDMAP_FULL,
                                                pFileName, fp.GetPath());
                        }

                        smIndex = 4;
                    }

                    sectorBuffer[smSector][smIndex] = trk;
                    sectorBuffer[smSector][smIndex + 1] = sec;
                }

                sectorBuffer[smSector][smIndex + 2]++;
                nextPTrk = trk;

                if ((nextPSec = sec + 1) > (param.byte_p_track /
                                            param.byte_p_sector))
                {
                    nextPTrk++;
                    nextPSec = 1;
                }
            }

            // set record nr and if last sector set link to 0. Write sector
            setValueBigEndian<Word>(&sectorBuffer[0][2], recordNr);

            if (recordNr * (SECTOR_SIZE - 4) >= buffer.GetFileSize())
            {
                sectorBuffer[0][0] = sectorBuffer[0][1] = 0;
            }

            if (!WriteSector(&sectorBuffer[0][0], trk, sec))
            {
                std::stringstream stream;

                stream << next;
                throw FlexException(FERR_WRITING_TRKSEC,
                                    stream.str(), fp.GetPath());
            }

            repeat = 0;
        }
        while (recordNr * (SECTOR_SIZE - 4) < buffer.GetFileSize());
    }
    sis.sir.fc_start = next;

    // if free chain full, set end trk/sec of free chain also to 0
    if (!next.sec && !next.trk)
    {
        sis.sir.fc_end = next;
    }

    // if random file, write the sector map buffers back
    next = start;

    if (buffer.IsRandom())
    {
        for (count = 2; count >= 1; count--)
        {
            if (!WriteSector(&sectorBuffer[count][0], next.trk, next.sec))
            {
                std::stringstream stream;

                stream << next;
                throw FlexException(FERR_WRITING_TRKSEC,
                                    stream.str(), fp.GetPath());
            }

            next.trk = sectorBuffer[count][0];
            next.sec = sectorBuffer[count][1];
        }
    }

    // update sys info sector
    auto free = getValueBigEndian<Word>(&sis.sir.free[0]);
    free -= (recordNr + (buffer.IsRandom() ? 2 : 0));
    setValueBigEndian<Word>(&sis.sir.free[0], free);

    if (!WriteSector(reinterpret_cast<Byte *>(&sis), sis_trk_sec.trk,
                     sis_trk_sec.sec))
    {
        std::stringstream stream;

        stream << sis_trk_sec;
        throw FlexException(FERR_WRITING_TRKSEC, stream.str(), fp.GetPath());
    }

    // make new directory entry
    de.SetDate(buffer.GetDate());
    de.SetStartTrkSec(start.trk, start.sec);
    de.SetEndTrkSec(trk, sec);
    de.SetTotalFileName(pFileName);
    de.SetSize(recordNr * SECTOR_SIZE);
    de.SetAttributes(buffer.GetAttributes());
    de.SetSectorMap(buffer.GetSectorMap());
    CreateDirEntry(de);
    return true;
}

FlexFileBuffer FlexFileContainer::ReadToBuffer(const char *fileName)
{
    FlexFileBuffer  buffer;
    FlexDirEntry    de;
    int             trk, sec;
    int             recordNr, repeat;
    Byte            sectorBuffer[SECTOR_SIZE];
    int             size;

    if (!is_formatted)
    {
        throw FlexException(FERR_CONTAINER_UNFORMATTED, GetPath());
    }

    if (!FindFile(fileName, de))
    {
        throw FlexException(FERR_UNABLE_TO_OPEN, fileName);
    }

    buffer.SetAttributes(de.GetAttributes());
    buffer.SetSectorMap(de.GetSectorMap());
    buffer.SetFilename(fileName);
    buffer.SetDate(de.GetDate());
    size = de.GetSize();

    if (de.IsRandom())
    {
        size -= 2 * SECTOR_SIZE;
    }

    if (size < 0)
    {
        throw FlexException(FERR_FILE_UNEXPECTED_SEC, fileName,
                            std::to_string(size / SECTOR_SIZE));
    }

    size = size * DBPS / SECTOR_SIZE;
    buffer.Realloc(size);
    recordNr = 0;
    repeat = 1;

    if (size > 0)
    {
        de.GetStartTrkSec(trk, sec);

        while (true)
        {
            // if random file skip the two sector map sectors
            if (recordNr == 0 && de.IsRandom())
            {
                repeat = 3;
            }

            for (int i = 0; i < repeat; i++)
            {
                if (trk == 0 && sec == 0)
                {
                    return buffer;
                }
                if (!ReadSector(&sectorBuffer[0], trk, sec))
                {
                    st_t st{static_cast<Byte>(trk), static_cast<Byte>(sec)};
                    std::stringstream stream;

                    stream << st;
                    throw FlexException(FERR_READING_TRKSEC,
                                        stream.str(), fileName);
                }

                trk = sectorBuffer[0];
                sec = sectorBuffer[1];
            } // for

            if (!buffer.CopyFrom(&sectorBuffer[4], SECTOR_SIZE - 4,
                                 recordNr * (SECTOR_SIZE - 4)))
            {
                size = recordNr + 1;
                throw FlexException(FERR_FILE_UNEXPECTED_SEC, fileName,
                                    std::to_string(size));
            }

            recordNr++;
            repeat = 1;
        }
    }

    return buffer;
}


// set the file attributes of one or multiple files
bool    FlexFileContainer::SetAttributes(const char *filePattern,
        Byte setMask, Byte clearMask)
{
    if (!is_formatted)
    {
        return false;
    }

    FlexDirEntry de;

    CHECK_CONTAINER_WRITEPROTECTED;

    FileContainerIterator it(filePattern);

    for (it = this->begin(); it != this->end(); ++it)
    {
        Byte x_attributes = (it->GetAttributes() & ~clearMask) | setMask;
        it.SetAttributesCurrent(x_attributes);
    }

    return true;
}

bool FlexFileContainer::CreateDirEntry(FlexDirEntry &entry)
{
    if (!is_formatted)
    {
        return false;
    }

    int     i;
    s_dir_sector    ds;
    s_dir_entry *pde;
    st_t next(first_dir_trk_sec);
    int     tmp1, tmp2;
    BDate       date;

    // loop until all directory sectors read
    while (next.sec != 0 || next.trk != 0)
    {
        // read next directory sector
        if (!ReadSector((Byte *)&ds, next.trk, next.sec))
        {
            std::stringstream stream;

            stream << next;
            throw FlexException(FERR_READING_TRKSEC,
                                stream.str(), fp.GetPath());
        }

        for (i = 0; i < 10; i++)
        {
            // look for the next free directory entry
            pde = &ds.dir_entry[i];

            if (pde->filename[0] == DE_EMPTY || pde->filename[0] == DE_DELETED)
            {
                int records;

                records = (entry.GetSize() / param.byte_p_sector) +
                          (entry.IsRandom() ? 2 : 0);
                memset(pde->filename, 0, FLEX_BASEFILENAME_LENGTH);
                strncpy(pde->filename, entry.GetFileName().c_str(),
                        FLEX_BASEFILENAME_LENGTH);
                memset(pde->file_ext, 0, FLEX_FILEEXT_LENGTH);
                strncpy(pde->file_ext, entry.GetFileExt().c_str(),
                        FLEX_FILEEXT_LENGTH);
                pde->file_attr = entry.GetAttributes();
                pde->reserved1 = 0;
                entry.GetStartTrkSec(tmp1, tmp2);
                pde->start.trk = static_cast<Byte>(tmp1);
                pde->start.sec = static_cast<Byte>(tmp2);
                entry.GetEndTrkSec(tmp1, tmp2);
                pde->end.trk = static_cast<Byte>(tmp1);
                pde->end.sec = static_cast<Byte>(tmp2);
                setValueBigEndian<Word>(&pde->records[0], static_cast<Word>(records));
                pde->sector_map = (entry.IsRandom() ? IS_RANDOM_FILE : 0x00);
                pde->reserved2 = 0;
                date = entry.GetDate();
                pde->day = static_cast<Byte>(date.GetDay());
                pde->month = static_cast<Byte>(date.GetMonth());
                pde->year = static_cast<Byte>(date.GetYear() % 100);

                if (!WriteSector((Byte *)&ds, next.trk, next.sec))
                {
                    std::stringstream stream;

                    stream << next;
                    throw FlexException(FERR_WRITING_TRKSEC,
                                        stream.str(), fp.GetPath());
                }

                return true;
            }
        }

        next = ds.next;
    }

    throw FlexException(FERR_DIRECTORY_FULL);
    return true; // satisfy compiler
}

// Evaluate the sector count on track 0.
// Follow the directory chain until end of chain reached or link is
// pointing to a directory extend on a track != 0.
// There are DSK files with the same track-sector count but different
// number of sectors on track 0:
//   35-18: Has 10 or 18 sectors on track 0.
//   40-18: Has 10 or 18 sectors on track 0.
void FlexFileContainer::EvaluateTrack0SectorCount()
{
    st_t link;
    Word i;

    for (i = first_dir_trk_sec.sec - 1; i < param.max_sector; ++i)
    {
        if (fseek(fp, i * param.byte_p_sector, SEEK_SET))
        {
            throw FlexException(FERR_UNABLE_TO_OPEN, fp.GetPath());
        }

        if (fread(&link, sizeof(link), 1, fp) == 1)
        {
            if (link == st_t{0, 0} || link.trk != 0)
            {
                break;
            }
        }
        else
        {
            throw FlexException(FERR_UNABLE_TO_OPEN, fp.GetPath());
        }
    }

    param.max_sector0 = i + 1;
}


/****************************************/
/* low level routines                   */
/****************************************/

int FlexFileContainer::ByteOffset(int trk, int sec, int side) const
{
    int byteOffs = param.offset;
    Word side0_offset = 0;

    if (!is_formatted && side < 0)
    {
        throw FlexException(FERR_UNEXPECTED_SIDE, side);
    }

    if (trk > 0)
    {
        byteOffs += param.byte_p_track0;
        byteOffs += param.byte_p_track * (trk - 1);
    }

    if (!is_formatted && side > 0)
    {
        // This case handles non FLEX file formats on side 1.
        // In this case evtl. the sector count from side 0 has to be added,
        // to get the right byte offset.
        const Word sectors_side0 =
                       (trk ? param.max_sector : param.max_sector0) / 2;

        if (sec <= sectors_side0)
        {
            side0_offset = sectors_side0;
        }
    }

    byteOffs += param.byte_p_sector * (sec + side0_offset - 1);

    return byteOffs;
}

// low level routine to read a single sector
// should be used with care
// Does not throw any exception !
// returns false on failure
bool FlexFileContainer::ReadSector(Byte *pbuffer, int trk, int sec,
                                   int side /* = -1 */) const
{
    if (fp == nullptr)
    {
        return false;
    }

    if (!IsTrackValid(trk) || !IsSectorValid(trk, sec))
    {
        return false;
    }

    int pos = ByteOffset(trk, sec, side);

    if (pos < 0)
    {
        return false;
    }

    if (fseek(fp, pos, SEEK_SET))
    {
        return false;
    }

    if (fread(pbuffer, param.byte_p_sector, 1, fp) != 1)
    {
        return false;
    }

    return true;
}

// low level routine to write a single sector
// should be used with care
// Does not throw any exception !
// returns false on failure
bool FlexFileContainer::WriteSector(const Byte *pbuffer, int trk, int sec,
                                    int side /* = -1 */)
{
    if (fp == nullptr)
    {
        return false;
    }

    if (!IsTrackValid(trk) || !IsSectorValid(trk, sec))
    {
        return false;
    }

    int pos = ByteOffset(trk, sec, side);

    if (pos < 0)
    {
        return false;
    }

    if (fseek(fp, pos, SEEK_SET))
    {
        return false;
    }

    if (fwrite(pbuffer, param.byte_p_sector, 1, fp) != 1)
    {
        return false;
    }

    if (!is_formatted &&
        trk == 0 && sec == 3 && IsFlexFileFormat(TYPE_FLX_CONTAINER))
    {
        is_formatted = true;
    }

    return true;
}

bool FlexFileContainer::FormatSector(const Byte *pbuffer, int track, int sector,
                                     int side, int sizecode)
{
    if (is_formatted ||
        track < 0 || track > 255 ||
        sector < 1 || sector > 255 ||
        side < 1 || side > 2 ||
        sizecode < 0 || sizecode > 3)
    {
        return false;
    }

    int byte_p_sector = getBytesPerSector(sizecode);

    // All sectors have to have same sector size.
    if (param.byte_p_sector != 0 && param.byte_p_sector != byte_p_sector)
    {
        return false;
    }

    // Dynamically update all disk parameters so that ReadSector or WriteSector
    // is possible immediately after FormatSector.
    if (param.byte_p_sector == 0)
    {
        param.byte_p_sector = byte_p_sector;
    }

    param.max_track = std::max(param.max_track, static_cast<Word>(track));

    if (track == 0)
    {
        sector = CorrectSector(side, sector, sectors0_side1_max);

        param.sides0 = std::max(param.sides0, static_cast<Word>(side));

        if (sector > param.max_sector0)
        {
            param.max_sector0 = sector;
            param.byte_p_track0 = param.max_sector0 * param.byte_p_sector;
        }

        file_size = std::max(file_size, param.offset + param.byte_p_track0);
    }
    else
    {
        sector = CorrectSector(side, sector, sectors_side1_max);

        param.sides = std::max(param.sides, static_cast<Word>(side));

        if (sector > param.max_sector)
        {
            param.max_sector = sector;
            param.byte_p_track = param.max_sector * param.byte_p_sector;
        }

        file_size = std::max(file_size,
                            param.offset + param.byte_p_track0 +
                            (track * param.byte_p_track));
    }

    bool result = true;

    flx_header.initialize(byte_p_sector, param.max_track + 1U,
                          param.max_sector0, param.max_sector, param.sides0,
                          param.sides);

    if (fseek(fp, 0, SEEK_SET))
    {
        result = false;
    }

    if (fwrite(&flx_header, sizeof(flx_header), 1, fp) != 1)
    {
        result = false;
    }

    result &= WriteSector(pbuffer, track, sector, side);

    if (!is_formatted && (file_size == getFileSize(flx_header)) &&
        IsFlexFileFormat(TYPE_FLX_CONTAINER))
    {
        is_formatted = true;
    }

    return result;
}

int FlexFileContainer::CorrectSector(int side, int sector,
                                     int &sectorsX_side1_max)
{
    if (side == 1)
    {
        sectorsX_side1_max = std::max(sectorsX_side1_max, sector);
    }

    if (side == 2 && sector <= sectorsX_side1_max)
    {
        sector += sectorsX_side1_max;
    }

    return sector;
}

void FlexFileContainer::Initialize_for_flx_format(const s_flex_header &header,
                                                  bool write_protected)
{
    param.offset = sizeof(struct s_flex_header);
    param.write_protect = (write_protected || header.write_protect) ? 0x40 : 0;
    param.max_sector = header.sectors * header.sides;
    param.max_sector0 = header.sectors0 * header.sides0;
    param.max_track = header.tracks - 1;
    param.byte_p_sector = getBytesPerSector(header.sizecode);
    param.byte_p_track0 = param.max_sector0 * param.byte_p_sector;
    param.byte_p_track = param.max_sector * param.byte_p_sector;
    param.sides0 = header.sides0;
    param.sides = header.sides;
    param.type = TYPE_CONTAINER | TYPE_FLX_CONTAINER;
}

void FlexFileContainer::Initialize_for_dsk_format(const s_formats &format,
                                                  bool write_protected)
{
    file_size = format.size;
    param.offset = 0;
    param.write_protect = write_protected ? 1 : 0;
    param.max_sector = format.sectors;
    param.max_sector0 = getTrack0SectorCount(format.tracks, format.sectors);
    param.max_track = format.tracks - 1;
    param.byte_p_sector = SECTOR_SIZE;
    param.byte_p_track0 = param.max_sector * SECTOR_SIZE;
    param.byte_p_track = param.max_sector * SECTOR_SIZE;
    param.sides0 = getSides(format.tracks, format.sectors);
    param.sides = getSides(format.tracks, format.sectors);
    param.type = TYPE_CONTAINER | TYPE_DSK_CONTAINER;
}

void FlexFileContainer::Initialize_unformatted_disk()
{
    file_size = sizeof(struct s_flex_header);
    param = { };
    param.offset = sizeof(struct s_flex_header);
    param.type = TYPE_CONTAINER | TYPE_FLX_CONTAINER;
}

void FlexFileContainer::Create_boot_sectors(Byte sec_buf[], Byte sec_buf2[])
{
    // Read boot sector(s) if present from file.
    BFilePtr boot(bootSectorFile.c_str(), "rb");

    if (boot == nullptr || fread(sec_buf, SECTOR_SIZE, 1, boot) != 1)
    {
        // No boot sector or read error.
        // Instead jump to monitor program warm start entry point.
        memset(sec_buf, 0, SECTOR_SIZE);
        sec_buf[0] = 0x7E; // JMP $F02D
        sec_buf[1] = 0xF0;
        sec_buf[2] = 0x2D;
    }
    if (boot != nullptr && fread(sec_buf2, SECTOR_SIZE, 1, boot) != 1)
    {
        memset(sec_buf2, 0, SECTOR_SIZE);
    }
} // Create_boot_sectors

void FlexFileContainer::Create_sys_info_sector(Byte sec_buf[], const char *name,
        struct s_formats &format)
{
    int     i, start, free;
    time_t      time_now;
    struct tm   *lt;

    memset(sec_buf, 0, SECTOR_SIZE);

    auto &sis = *reinterpret_cast<s_sys_info_sector *>(sec_buf);

    for (i = 0; i < 8; i++)
    {
        if (*(name + i) == '.' || *(name + i) == '\0')
        {
            break;
        }

        sis.sir.disk_name[i] = static_cast<char>(std::toupper(*(name + i)));
    } // for

    start           = format.sectors;
    free            = (format.sectors * format.tracks) - start;
    time_now        = time(nullptr);
    lt          = localtime(&time_now);
    auto year = lt->tm_year >= 100 ? lt->tm_year - 100 : lt-> tm_year;
    setValueBigEndian<Word>(&sis.sir.disk_number[0], 1U);
    sis.sir.fc_start.trk = static_cast<Byte>(start / format.sectors);
    sis.sir.fc_start.sec = static_cast<Byte>((start % format.sectors) + 1);
    sis.sir.fc_end.trk = static_cast<Byte>(format.tracks - 1);
    sis.sir.fc_end.sec = static_cast<Byte>(format.sectors);
    setValueBigEndian<Word>(&sis.sir.free[0], static_cast<Word>(free));
    sis.sir.month = static_cast<Byte>(lt->tm_mon + 1);
    sis.sir.day = static_cast<Byte>(lt->tm_mday);
    sis.sir.year = static_cast<Byte>(year);
    sis.sir.last.trk = static_cast<Byte>(format.tracks - 1);
    sis.sir.last.sec = static_cast<Byte>(format.sectors);
} // create_sys_info_sector

// on success return true
bool FlexFileContainer::Write_dir_sectors(FILE *fp, struct s_formats &format)
{
    Byte    sec_buf[SECTOR_SIZE];
    int     i;

    memset(sec_buf, 0, sizeof(sec_buf));

    for (i = 0; i < format.sectors0 - first_dir_trk_sec.sec + 1; i++)
    {
        sec_buf[0] = 0;
        sec_buf[1] = 0;

        if (i < format.dir_sectors - 1)
        {
            auto sector = i + first_dir_trk_sec.sec;
            sec_buf[0] = static_cast<Byte>(sector / format.sectors);
            sec_buf[1] = static_cast<Byte>((sector % format.sectors) + 1);
        }

        if (fwrite(sec_buf, sizeof(sec_buf), 1, fp) != 1)
        {
            return false;
        }
    }

    return true;
} // write_dir_sectors

// on success return true
bool FlexFileContainer::Write_sectors(FILE *fp, struct s_formats &format)
{
    Byte    sec_buf[SECTOR_SIZE];
    int     i;

    memset(sec_buf, 0, sizeof(sec_buf));

    for (i = format.sectors + 1; i <= format.sectors * format.tracks; ++i)
    {
        sec_buf[0] = static_cast<Byte>(i / format.sectors);
        sec_buf[1] = static_cast<Byte>((i % format.sectors) + 1);

        // use for tests to correctly save random files:
        // (the link always jumps over one sector)
        //      sec_buf[0] = (i+1) / format.sectors;
        //      sec_buf[1] = ((i+1) % format.sectors) + 1;
        if (i == format.sectors * format.tracks)
        {
            sec_buf[0] = sec_buf[1] = 0;
        }

        if (fwrite(sec_buf, sizeof(sec_buf), 1, fp) != 1)
        {
            return false;
        }
    }

    return true;
} // write_sectors

void FlexFileContainer::Create_format_table(int type, int trk, int sec,
        struct s_formats &format)
{
    if (trk < 2)
    {
        trk = 2;
    }
    else if (trk > 255)
    {
        trk = 255;
    }

    if (sec < 5)
    {
        sec = 5;
    }
    else if (sec > 255)
    {
        sec = 255;
    }

    format.tracks = static_cast<Word>(trk);
    format.sectors = static_cast<Word>(sec);
    format.sectors0 = (type == TYPE_FLX_CONTAINER) ?
                          getTrack0SectorCount(format.tracks, format.sectors) :
                          format.sectors;

    format.size = format.tracks * format.sectors * SECTOR_SIZE;
    // calculate number of directory sectors.
    // track 0 only contains directory sectors.
    format.dir_sectors = getTrack0SectorCount(trk, sec) -
                          first_dir_trk_sec.sec + 1;
} // create_format_table

// return != 0 on success
// format FLX or DSK format. FLX format always with sector_size 256
// number of sectors on track 0 is calculated by method
// getTrack0SectorCount().
// type:
//  use TYPE_DSK_CONTAINER for DSK format
//  use TYPE_FLX_CONTAINER for FLX format

void FlexFileContainer::Format_disk(
    int trk,
    int sec,
    const char *disk_dir,
    const char *name,
    int  type /* = TYPE_DSK_CONTAINER */)
{
    std::string path;
    struct s_formats format;
    int     err = 0;

    if (disk_dir == nullptr ||
        name == nullptr || strlen(name) == 0 ||
        trk < 2 || sec < 6 || trk > 255 || sec > 255)
    {
        throw FlexException(FERR_WRONG_PARAMETER);
    }

    Create_format_table(type, trk, sec, format);

    path = disk_dir;

    if (path.size() > 0 && path[path.size() - 1] != PATHSEPARATOR)
    {
        path +=PATHSEPARATORSTRING;
    }

    path += name;

    BFilePtr fp(path.c_str(), "wb");

    if (fp != nullptr)
    {
        Byte sector_buffer[SECTOR_SIZE];

        if (type == TYPE_FLX_CONTAINER)
        {
            int sides = getSides(format.tracks, format.sectors);
            struct s_flex_header header;

            header.initialize(SECTOR_SIZE, format.tracks, format.sectors0,
                              format.sectors, sides, sides);

            if (fwrite(&header, sizeof(header), 1, fp) != 1)
            {
                err = 1;
            }
        }

        {
            Byte sector_buffer2[SECTOR_SIZE];

            Create_boot_sectors(sector_buffer, sector_buffer2);

            if (fwrite(sector_buffer, sizeof(sector_buffer), 1, fp) != 1)
            {
                err = 1;
            }

            if (fwrite(sector_buffer2, sizeof(sector_buffer2), 1, fp) != 1)
            {
                err = 1;
            }
        }

        Create_sys_info_sector(sector_buffer, name, format);

        if (fwrite(sector_buffer, sizeof(sector_buffer), 1, fp) != 1)
        {
            err = 1;
        }

        // Sector 00-04 seems to be unused. Write all zeros.
        memset(sector_buffer, 0, sizeof(sector_buffer));
        if (fwrite(sector_buffer, sizeof(sector_buffer), 1, fp) != 1)
        {
            err = 1;
        }

        if (!Write_dir_sectors(fp, format))
        {
            err = 1;
        }

        if (!Write_sectors(fp, format))
        {
            err = 1;
        }
    }
    else
    {
        err = 1;
    }

    if (err)
    {
        throw FlexException(FERR_UNABLE_TO_FORMAT, name);
    }
} // format_disk

// Read the number of tracks and sectors for a FLEX file container.
bool FlexFileContainer::GetFlexTracksSectors(Word &tracks, Word &sectors,
                                             Word header_offset) const
{
    s_sys_info_sector sis;

    // Read system info sector.
    long file_offset = header_offset + (sis_trk_sec.sec - 1) * SECTOR_SIZE;
    if (fseek(fp, file_offset, SEEK_SET))
    {
        return false;
    }

    if (fread(&sis, sizeof(sis), 1, fp) != 1)
    {
        return false;
    }

    tracks = sis.sir.last.trk;
    sectors = sis.sir.last.sec;
    if (tracks)
    {
        ++tracks;
    }

    return true;
}

// Check if the file container contains a FLEX compatible file system.
bool FlexFileContainer::IsFlexFileFormat(int type) const
{
    struct stat sbuf;
    Word tracks = 35;
    Word sectors = 10;

    if (stat(fp.GetPath(), &sbuf))
    {
        return false;
    }

    if ((type & TYPE_FLX_CONTAINER) != 0)
    {
        if (GetFlexTracksSectors(tracks, sectors, sizeof(s_flex_header)) &&
            tracks != 0U &&
            sectors != 0U &&
            flx_header.sizecode == 1U &&
            flx_header.tracks == tracks,
            flx_header.sectors0 != 0U &&
            flx_header.sectors0 * flx_header.sides0 <= sectors &&
            flx_header.sectors * flx_header.sides == sectors &&
            flx_header.sides0 >= 1U && flx_header.sides0 <= 2U &&
            flx_header.sides >= 1U && flx_header.sides <= 2U)
        {
            // Plausibility check with the file size.
            off_t size = getFileSize(flx_header);
            if (size == sbuf.st_size)
            {
                return true;
            }
        }
    }

    if ((type & TYPE_DSK_CONTAINER) != 0)
    {
        if (GetFlexTracksSectors(tracks, sectors, 0U))
        {
            off_t size_min = ((tracks - 1) * sectors + 1) * SECTOR_SIZE;
            off_t size_max = tracks * sectors * SECTOR_SIZE;

            // do a plausibility check with the size of the DSK file
            if (sbuf.st_size % SECTOR_SIZE == 0 &&
                sbuf.st_size >= size_min && sbuf.st_size <= size_max)
            {
                return true;
            }
        }
    }

    return false;
}

