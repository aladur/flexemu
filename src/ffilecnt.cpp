/*
    ffilecnt.cpp


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
    std::string FlexFileContainer::bootSectorFile =
        getExecutablePath() + PATHSEPARATORSTRING BOOT_FILE;
#endif
    bool FlexFileContainer::onTrack0OnlyDirSectors = true;

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
    p_tracks = std::min(p_tracks, 256);
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

    magic_number = toBigEndian(MAGIC_NUMBER);
    write_protect = 0;
    sizecode = p_sizecode;
    sides0 = static_cast<Byte>(p_sides0);
    sectors0 = static_cast<Byte>(p_sectors0 / p_sides0);
    sides = static_cast<Byte>(p_sides);
    sectors = static_cast<Byte>(p_sectors / p_sides);
    tracks = static_cast<Byte>(p_tracks);
    dummy1 = 0;
    dummy2 = 0;
    dummy3 = 0;
    dummy4 = 0;
    dummy5 = 0;
}


/****************************************/
/* Constructor                          */
/****************************************/

FlexFileContainer::FlexFileContainer(
        const std::string &path,
        const std::string &mode,
        const FileTimeAccess &fileTimeAccess)
    : fp(path, mode)
    , ft_access(fileTimeAccess)
    , is_flex_format(true)
{
    struct stat sbuf{};

    if (fp == nullptr)
    {
        throw FlexException(FERR_UNABLE_TO_OPEN, fp.GetPath());
    }

    if (stat(fp.GetPath().c_str(), &sbuf) != 0 || !S_ISREG(sbuf.st_mode))
    {
        throw FlexException(FERR_UNABLE_TO_OPEN, fp.GetPath());
    }

    param.type = 0U;
    if (sbuf.st_size == 0)
    {
        // If file has been created or file size 0 then
        // it is marked as an unformatted file container.
        // No records are available yet but it can be formatted
        // from within the emulation.
        is_flex_format = false;
        Initialize_unformatted_disk();
        return;
    }

    if (fseek(fp, 0, SEEK_SET))
    {
        throw FlexException(FERR_UNABLE_TO_OPEN, fp.GetPath());
    }
    file_size = static_cast<DWord>(sbuf.st_size);

    if (mode.find_first_of('+') == std::string::npos)
    {
        attributes |= FLX_READONLY;
    }

    if (stat(fp.GetPath().c_str(), &sbuf) == 0)
    {
        bool write_protected = ((attributes & FLX_READONLY) != 0);

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
                is_flex_format = false;
            }
            return;
        }

        s_formats format { };
        auto jvcHeader = GetJvcFileHeader();
        auto jvcHeaderSize = static_cast<Word>(jvcHeader.size());
        Word tracks;
        Word sectors;

        // check if it is a DSK formated disk
        // read system info sector
        if (IsFlexFileFormat(TYPE_DSK_CONTAINER) &&
            GetFlexTracksSectors(tracks, sectors, jvcHeaderSize))
        {
            // File is identified as a FLEX DSK container format
            if (jvcHeaderSize == 0U)
            {
                format.tracks = tracks;
                format.sectors = sectors;
                format.sides = 0U;
            }
            else
            {
                Word jvcSectors = jvcHeader[0];
                Word jvcSides = (jvcHeaderSize >= 2) ? jvcHeader[1] : 1U;

                // FLEX DSK container with JVC file header
                format.tracks = static_cast<Word>(
                                (file_size - jvcHeaderSize) /
                                (jvcSectors * SECTOR_SIZE) / jvcSides);
                format.sectors = jvcSectors * jvcSides;
                format.sides = jvcSides;

                if (tracks != format.tracks || sectors != format.sectors)
                {
                    throw FlexException(FERR_INVALID_JVC_HEADER,
                                        fp.GetPath());
                }
            }
            format.size = static_cast<SDWord>(file_size);
            format.offset = jvcHeaderSize;
            Initialize_for_dsk_format(format, write_protected);
            EvaluateTrack0SectorCount();
            return;
        }
    }

    throw FlexException(FERR_IS_NO_FILECONTAINER, fp.GetPath());
}

FlexFileContainer::FlexFileContainer(FlexFileContainer &&src) noexcept :
    fp(std::move(src.fp)), param(src.param), file_size(src.file_size),
    ft_access(src.ft_access),
    is_flex_format(src.is_flex_format),
    sectors0_side0_max(src.sectors0_side0_max),
    sectors_side0_max(src.sectors_side0_max),
    attributes(src.attributes)
{
}

FlexFileContainer &FlexFileContainer::operator= (FlexFileContainer &&src)
noexcept
{
    fp = std::move(src.fp);
    param = src.param;
    file_size = src.file_size;
    is_flex_format = src.is_flex_format;
    sectors0_side0_max = src.sectors0_side0_max;
    sectors_side0_max = src.sectors_side0_max;
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
    return param.write_protect != 0;
}

bool FlexFileContainer::IsTrackValid(int track) const
{
    if (!is_flex_format)
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

    return (sector > 0 && sector <= param.max_sector0);
}

bool FlexFileContainer::IsFlexFormat() const
{
    return is_flex_format;
}

FlexFileContainer *FlexFileContainer::Create(
        const std::string &directory,
        const std::string &name,
        const FileTimeAccess &fileTimeAccess,
        int tracks,
        int sectors,
        int fmt /* = TYPE_DSK_CONTAINER */,
        const char *bsFile /* = nullptr */)
{
    std::string path;

    if (fmt != TYPE_DSK_CONTAINER && fmt != TYPE_FLX_CONTAINER)
    {
        throw FlexException(FERR_INVALID_FORMAT, fmt);
    }

    Format_disk(directory, name, tracks, sectors, fmt, bsFile);

    path = directory;

    if (!path.empty() && !endsWithPathSeparator(path))
    {
        path += PATHSEPARATORSTRING;
    }

    path += name;

    return new FlexFileContainer(path, "rb+", fileTimeAccess);
}

// return true if file found
// if file found can also be checked by
// !entry.isEmpty
bool FlexFileContainer::FindFile(const char *fileName, FlexDirEntry &entry)
{
    if (is_flex_format)
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

bool FlexFileContainer::DeleteFile(const char *wildcard)
{
    if (!is_flex_format)
    {
        return false;
    }

    CHECK_CONTAINER_WRITEPROTECTED;

    FileContainerIterator it(wildcard);

    for (it = this->begin(); it != this->end(); ++it)
    {
        it.DeleteCurrent();
    }

    return true;
}

bool FlexFileContainer::RenameFile(const char *oldName, const char *newName)
{
    if (!is_flex_format || (strcmp(oldName, newName) == 0))
    {
        return false;
    }

    CHECK_CONTAINER_WRITEPROTECTED;

    FlexDirEntry de;

    // prevent overwriting of an existing file
    // except for changing lower to uppercase.
    if (stricmp(oldName, newName) != 0 && FindFile(newName, de))
    {
        throw FlexException(FERR_FILE_ALREADY_EXISTS, newName);
    }

    FileContainerIterator it(oldName);

    it = this->begin();

    if (it == this->end())
    {
        throw FlexException(FERR_NO_FILE_IN_CONTAINER, oldName, fp.GetPath());
    }

    it.RenameCurrent(newName);

    return true;
}

bool FlexFileContainer::FileCopy(const char *sourceName, const char *destName,
                                 FileContainerIf &destination)
{
    if (!is_flex_format)
    {
        return false;
    }

    return FlexCopyManager::FileCopy(sourceName, destName, *this, destination);
}

bool FlexFileContainer::GetInfo(FlexContainerInfo &info) const
{
    if (is_flex_format)
    {
        u_sys_info_sector sis{};
        int year;

        if (!ReadSector(sis.raw, sis_trk_sec.trk, sis_trk_sec.sec))
        {
            std::stringstream stream;

            stream << sis_trk_sec;
            throw FlexException(FERR_READING_TRKSEC, stream.str(),
                                fp.GetPath());
        }

        if (sis.s.sir.year < 75)
        {
            year = sis.s.sir.year + 2000;
        }
        else
        {
            year = sis.s.sir.year + 1900;
        }

        auto size = 0U;
        while (size < sizeof(sis.s.sir.disk_name) && sis.s.sir.disk_name[size])
        {
            ++size;
        }
        std::string disk_name(sis.s.sir.disk_name, size);
        disk_name.append("");
        bool is_valid = true;
        size = 0U;
        while (size < sizeof(sis.s.sir.disk_ext) && sis.s.sir.disk_ext[size])
        {
            if (sis.s.sir.disk_ext[size] < ' ' ||
                sis.s.sir.disk_ext[size] > '~')
            {
                is_valid = false;
                break;
            }
            ++size;
        }
        if (size > 0U && is_valid)
        {
            std::string disk_ext(sis.s.sir.disk_ext, size);
            disk_ext.append("");
            disk_name.append(".");
            disk_name.append(disk_ext);
        }
        info.SetDate(BDate(sis.s.sir.day, sis.s.sir.month, year));
        info.SetFree(getValueBigEndian<Word>(&sis.s.sir.free[0]) *
                     param.byte_p_sector);
        info.SetTotalSize((sis.s.sir.last.sec * (sis.s.sir.last.trk + 1)) *
                           param.byte_p_sector);
        info.SetName(disk_name);
        info.SetNumber(getValueBigEndian<Word>(&sis.s.sir.disk_number[0]));
    }

    info.SetTrackSector(
            param.max_track ? param.max_track + 1 : 0,
            param.max_sector);
    info.SetIsFlexFormat(is_flex_format);
    info.SetPath(fp.GetPath());
    info.SetType(param.type);
    info.SetAttributes(attributes);
    info.SetIsWriteProtected(IsWriteProtected());
    if (param.type & TYPE_DSK_CONTAINER)
    {
        info.SetJvcFileHeader(GetJvcFileHeader());
    }

    return true;
}

int FlexFileContainer::GetContainerType() const
{
    return param.type;
}

std::string FlexFileContainer::GetSupportedAttributes() const
{
    return "WDRC";
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
    if (!is_flex_format)
    {
        return false;
    }

    Byte trk = 0;
    Byte sec = 0;
    st_t start;
    st_t next;
    Word recordNr = 0; // Current record number. For random files
                       // the two sector map sectors are counted too.
    int count;
    FlexDirEntry de;
    u_sys_info_sector sis{};
    const char *pFileName = fileName;
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

    if (buffer.GetFileSize() == 0U)
    {
        throw FlexException(FERR_COPY_EMPTY_FILE, pFileName);
    }

    // read sys info sector
    if (!ReadSector(sis.raw, sis_trk_sec.trk, sis_trk_sec.sec))
    {
        std::stringstream stream;

        stream << sis_trk_sec;
        throw FlexException(FERR_READING_TRKSEC, stream.str(), fp.GetPath());
    } // get start trk/sec of free chain

    next = start = sis.s.sir.fc_start;

    // write each sector to buffer
    Byte repeat = 0;
    DWord smSector = 2; // Index of current sector map sector (2..1)
    DWord smIndex = 1; // Byte index within current sector map sector
    Word nextTrk = 0; // Contains next subsequent track
    Word nextSec = 0; // Contains next subsequent sector

    // at the begin of a random file reserve two sectors for the sector map
    if (recordNr == 0U && buffer.IsRandom())
    {
        repeat = 2;
    }

    while (recordNr * (SECTOR_SIZE - 4) < buffer.GetFileSize())
    {
        for (count = repeat; count >= 0; count--)
        {
            trk = next.trk;
            sec = next.sec;

            if (trk == 0 && sec == 0)
            {
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

            if (count)
            {
                // For random files the two sector map sectors are
                // skipped. They are newly generated in sectorBuffer[2]
                // and sectorBuffer[1].
                // Here the buffer is initialized to zero.
                memset(&sectorBuffer[count][2], 0, SECTOR_SIZE - 2);
                ++recordNr;
            }

            next.trk = sectorBuffer[count][0];
            next.sec = sectorBuffer[count][1];
        }
        repeat = 0; // Finished preparing random file sector map.

        if (!buffer.CopyTo(&sectorBuffer[0][4], SECTOR_SIZE - 4,
                           recordNr * (SECTOR_SIZE - 4), 0x00))
        {
            std::stringstream stream;

            stream << next;
            throw FlexException(FERR_WRITING_TRKSEC,
                                stream.str(), fp.GetPath());
        }

        recordNr++;

        if (buffer.IsRandom())
        {
            // For random files update the sector map.
            // For each non continuous sector or if sector count is 255
            // a new entry in sector map is created.
            if (trk != nextTrk || sec != nextSec ||
                sectorBuffer[smSector][smIndex + 2U] == 255U)
            {
                smIndex += 3U;

                if (smIndex >= SECTOR_SIZE)
                {
                    if (--smSector == 0U)
                    {
                        throw FlexException(FERR_RECORDMAP_FULL,
                                            pFileName, fp.GetPath());
                    }
                    smIndex = 4U;
                }

                sectorBuffer[smSector][smIndex] = trk;
                sectorBuffer[smSector][smIndex + 1U] = sec;
            }

            ++sectorBuffer[smSector][smIndex + 2U];

            // Calculate the next subsequent track/sector for given trk/sec.
            nextTrk = trk;
            nextSec = sec + 1;
            if (nextSec > (param.byte_p_track / param.byte_p_sector))
            {
                ++nextTrk;
                nextSec = 1;
            }
        }

        // Set record number. If last sector of file set link to 0.
        // Write the sector to disk.
        setValueBigEndian<Word>(&sectorBuffer[0][2],
                                recordNr - (buffer.IsRandom() ? 2U : 0U));

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
    }

    sis.s.sir.fc_start = next;

    // if free chain full, set end trk/sec of free chain also to 0
    if (!next.sec && !next.trk)
    {
        sis.s.sir.fc_end = next;
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

    // Update the system info sector.
    auto free = getValueBigEndian<Word>(&sis.s.sir.free[0]);
    free -= recordNr;
    setValueBigEndian<Word>(&sis.s.sir.free[0], free);

    if (!WriteSector(sis.raw, sis_trk_sec.trk, sis_trk_sec.sec))
    {
        std::stringstream stream;

        stream << sis_trk_sec;
        throw FlexException(FERR_WRITING_TRKSEC, stream.str(), fp.GetPath());
    }

    // Create a new directory entry.
    de.SetDate(buffer.GetDate());
    if ((ft_access & FileTimeAccess::Set) == FileTimeAccess::Set)
    {
        de.SetTime(buffer.GetTime());
    }
    de.SetStartTrkSec(start.trk, start.sec);
    de.SetEndTrkSec(trk, sec);
    de.SetTotalFileName(pFileName);
    de.SetFileSize(recordNr * static_cast<int>(SECTOR_SIZE));
    de.SetAttributes(buffer.GetAttributes());
    de.SetSectorMap(buffer.GetSectorMap());
    CreateDirEntry(de);

    return true;
}

FlexFileBuffer FlexFileContainer::ReadToBuffer(const char *fileName)
{
    FlexFileBuffer buffer;
    FlexDirEntry de;
    int trk;
    int sec;
    int recordNr;
    std::array<Byte, SECTOR_SIZE> sectorBuffer{};
    int size;

    if (!is_flex_format)
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
    BTime time;
    if ((ft_access & FileTimeAccess::Get) == FileTimeAccess::Get)
    {
        time = de.GetTime();
    }
    buffer.SetDateTime(de.GetDate(), time);
    size = de.GetFileSize();

    if (size < 0)
    {
        throw FlexException(FERR_FILE_UNEXPECTED_SEC, fileName,
                            std::to_string(size / SECTOR_SIZE));
    }

    size = size * DBPS / static_cast<int>(SECTOR_SIZE);
    buffer.Realloc(size);
    recordNr = 0;

    if (size > 0)
    {
        de.GetStartTrkSec(trk, sec);

        while (true)
        {
            if (trk == 0 && sec == 0)
            {
                return buffer;
            }
            if (!ReadSector(sectorBuffer.data(), trk, sec))
            {
                st_t st{static_cast<Byte>(trk), static_cast<Byte>(sec)};
                std::stringstream stream;

                stream << st;
                throw FlexException(FERR_READING_TRKSEC,
                                    stream.str(), fileName);
            }

            trk = sectorBuffer[0];
            sec = sectorBuffer[1];

            if (!buffer.CopyFrom(sectorBuffer.data() + 4, SECTOR_SIZE - 4,
                                 recordNr * (SECTOR_SIZE - 4)))
            {
                size = recordNr + 1;
                throw FlexException(FERR_FILE_UNEXPECTED_SEC, fileName,
                                    std::to_string(size));
            }

            recordNr++;
        }
    }

    return buffer;
}


// set the file attributes of one or multiple files
bool FlexFileContainer::SetAttributes(const char *wildcard,
        Byte setMask, Byte clearMask)
{
    if (!is_flex_format)
    {
        return false;
    }

    FlexDirEntry de;

    CHECK_CONTAINER_WRITEPROTECTED;

    FileContainerIterator it(wildcard);

    for (it = this->begin(); it != this->end(); ++it)
    {
        Byte p_attributes = (it->GetAttributes() & ~clearMask) | setMask;
        it.SetAttributesCurrent(p_attributes);
    }

    return true;
}

bool FlexFileContainer::CreateDirEntry(FlexDirEntry &entry)
{
    if (!is_flex_format)
    {
        return false;
    }

    int i;
    u_dir_sector dir_sector{};
    s_dir_entry *pde;
    st_t next(first_dir_trk_sec);
    int tmp1;
    int tmp2;
    BDate date;

    // loop until all directory sectors read
    while (next.sec != 0 || next.trk != 0)
    {
        // read next directory sector
        if (!ReadSector(dir_sector.raw, next.trk, next.sec))
        {
            std::stringstream stream;

            stream << next;
            throw FlexException(FERR_READING_TRKSEC,
                                stream.str(), fp.GetPath());
        }

        for (i = 0; i < 10; i++)
        {
            // look for the next free directory entry
            pde = &dir_sector.s.dir_entries[i];

            if (pde->filename[0] == DE_EMPTY || pde->filename[0] == DE_DELETED)
            {
                BTime time;

                if ((ft_access & FileTimeAccess::Set) == FileTimeAccess::Set)
                {
                   time = entry.GetTime();
                }
                int records = entry.GetFileSize() / param.byte_p_sector;
                memset(pde->filename, 0, FLEX_BASEFILENAME_LENGTH);
                strncpy(pde->filename, entry.GetFileName().c_str(),
                        FLEX_BASEFILENAME_LENGTH);
                memset(pde->file_ext, 0, FLEX_FILEEXT_LENGTH);
                strncpy(pde->file_ext, entry.GetFileExt().c_str(),
                        FLEX_FILEEXT_LENGTH);
                pde->file_attr = entry.GetAttributes();
                pde->hour = static_cast<Byte>(time.GetHour());
                entry.GetStartTrkSec(tmp1, tmp2);
                pde->start.trk = static_cast<Byte>(tmp1);
                pde->start.sec = static_cast<Byte>(tmp2);
                entry.GetEndTrkSec(tmp1, tmp2);
                pde->end.trk = static_cast<Byte>(tmp1);
                pde->end.sec = static_cast<Byte>(tmp2);
                setValueBigEndian<Word>(&pde->records[0], static_cast<Word>(records));
                pde->sector_map = (entry.IsRandom() ? IS_RANDOM_FILE : 0x00);
                pde->minute = static_cast<Byte>(time.GetMinute());
                date = entry.GetDate();
                pde->day = static_cast<Byte>(date.GetDay());
                pde->month = static_cast<Byte>(date.GetMonth());
                pde->year = static_cast<Byte>(date.GetYear() % 100);

                if (!WriteSector(dir_sector.raw, next.trk, next.sec))
                {
                    std::stringstream stream;

                    stream << next;
                    throw FlexException(FERR_WRITING_TRKSEC,
                                        stream.str(), fp.GetPath());
                }

                return true;
            }
        }

        next = (dir_sector.s.next == st_t{}) ?
            ExtendDirectory(dir_sector, next) : dir_sector.s.next;
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
// This default behaviour can be overwritten by the global flag:
//   FlexFileContainer::onTrack0OnlyDirSectors = false;
void FlexFileContainer::EvaluateTrack0SectorCount()
{
    st_t link;
    Word i;

    if (!onTrack0OnlyDirSectors)
    {
        param.max_sector0 = param.max_sector;
        return;
    }

    for (i = first_dir_trk_sec.sec - 1; i < param.max_sector; ++i)
    {
        if (fseek(fp, param.offset + (i * param.byte_p_sector), SEEK_SET))
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

    param.max_sector0 = std::min(param.max_sector, static_cast<Word>(i + 1));
}


/****************************************/
/* low level routines                   */
/****************************************/

int FlexFileContainer::ByteOffset(int trk, int sec, int side) const
{
    int byteOffs = param.offset;
    Word side0_offset = 0;

    if (!is_flex_format && side < 0)
    {
        throw FlexException(FERR_UNEXPECTED_SIDE, side);
    }

    if (trk > 0)
    {
        byteOffs += static_cast<int>(param.byte_p_track0);
        byteOffs += static_cast<int>(param.byte_p_track * (trk - 1));
    }

    if (!is_flex_format && side == 1)
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

    if (!is_flex_format &&
        trk == 0 && sec == 3 && IsFlexFileFormat(TYPE_FLX_CONTAINER))
    {
        is_flex_format = true;
    }

    return true;
}

bool FlexFileContainer::FormatSector(const Byte *target, int track, int sector,
                                     int side, int sizecode)
{
    if (is_flex_format ||
        track < 0 || track > 255 ||
        sector < 1 || sector > 255 ||
        side < 0 || side > 1 ||
        sizecode < 0 || sizecode > 3)
    {
        return false;
    }

    auto byte_p_sector = getBytesPerSector(sizecode);

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
        if (side == 0)
        {
            sectors0_side0_max = std::max(sectors0_side0_max, sector);
        }
        else if (side == 1)
        {
            // If side 1 is formatted it immediately is assumed that
            // the total number of sectors is 2 * sectors0_side0_max.
            param.max_sector0 =
                static_cast<Word>(2 * sectors0_side0_max);
            param.byte_p_track0 = param.max_sector0 * param.byte_p_sector;
        }

        param.sides0 = std::max(param.sides0, static_cast<Word>(side + 1));

        if (sector > param.max_sector0)
        {
            param.max_sector0 = static_cast<Word>(sector);
            param.byte_p_track0 = param.max_sector0 * param.byte_p_sector;
        }

        file_size = std::max(file_size, param.offset + param.byte_p_track0);
    }
    else
    {
        if (side == 0)
        {
            sectors_side0_max = std::max(sectors_side0_max, sector);
        }
        else if (side == 1)
        {
            // If side 1 is formatted it immediately is assumed that
            // the total number of sectors is 2 * sectors_side0_max.
            param.max_sector = static_cast<Word>(2 * sectors_side0_max);
            param.byte_p_track = param.max_sector * param.byte_p_sector;
        }

        param.sides = std::max(param.sides, static_cast<Word>(side + 1));

        if (sector > param.max_sector)
        {
            param.max_sector = static_cast<Word>(sector);
            param.byte_p_track = param.max_sector * param.byte_p_sector;
        }

        file_size = std::max(file_size,
                            param.offset + param.byte_p_track0 +
                            (track * param.byte_p_track));
    }

    bool result = true;

    flx_header.initialize(byte_p_sector, param.max_track + 1,
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

    result &= WriteSector(target, track, sector, side);

    if (!is_flex_format && (file_size == getFileSize(flx_header)) &&
        IsFlexFileFormat(TYPE_FLX_CONTAINER))
    {
        is_flex_format = true;
    }

    return result;
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
    auto sides = format.sides ? format.sides :
                     getSides(format.tracks, format.sectors);
    auto sector0 = (format.offset != 0U) ? format.sectors :
                     getTrack0SectorCount(format.tracks, format.sectors);

    file_size = format.size;
    param.offset = format.offset;
    param.write_protect = write_protected ? 1 : 0;
    param.max_sector = format.sectors;
    param.max_sector0 = sector0;
    param.max_track = format.tracks - 1;
    param.byte_p_sector = SECTOR_SIZE;
    param.byte_p_track0 = param.max_sector * SECTOR_SIZE;
    param.byte_p_track = param.max_sector * SECTOR_SIZE;
    param.sides0 = sides;
    param.sides = sides;
    param.type = TYPE_CONTAINER | TYPE_DSK_CONTAINER;
    if (format.offset != 0U)
    {
        param.type |= TYPE_JVC_HEADER;
    }
}

void FlexFileContainer::Initialize_unformatted_disk()
{
    file_size = sizeof(struct s_flex_header);
    param = { };
    param.offset = sizeof(struct s_flex_header);
    param.type = TYPE_CONTAINER | TYPE_FLX_CONTAINER;
}

void FlexFileContainer::Create_boot_sectors(Byte sectorBuffer1[],
                                            Byte sectorBuffer2[],
                                            const char *bsFile)
{
    // Read boot sector(s) if present from file.
    if (bsFile == nullptr)
    {
        bsFile = bootSectorFile.c_str();
    }
    BFilePtr boot(bsFile, "rb");

    if (boot == nullptr || fread(sectorBuffer1, SECTOR_SIZE, 1, boot) != 1)
    {
        // No boot sector or read error.
        // Instead jump to monitor program warm start entry point.
        memset(sectorBuffer1, 0, SECTOR_SIZE);
        sectorBuffer1[0] = 0x7E; // JMP $F02D
        sectorBuffer1[1] = 0xF0;
        sectorBuffer1[2] = 0x2D;
        memset(sectorBuffer2, 0, SECTOR_SIZE);
    }
    if (boot != nullptr && fread(sectorBuffer2, SECTOR_SIZE, 1, boot) != 1)
    {
        memset(sectorBuffer2, 0, SECTOR_SIZE);
    }
} // Create_boot_sectors

void FlexFileContainer::Create_sys_info_sector(u_sys_info_sector &sis,
        const std::string &name,
        struct s_formats &format)
{
    int start;
    int free;
    time_t time_now;
    struct tm *lt;

    memset(sis.raw, 0, SECTOR_SIZE);

    int i = 0;

    for (const char ch : name)
    {
        if (i == FLEX_DISKNAME_LENGTH || ch == '.' || ch == '\0')
        {
            break;
        }

        sis.s.sir.disk_name[i] = static_cast<char>(std::toupper(ch));
        ++i;
    }

    start = format.sectors;
    free = (format.sectors * format.tracks) - start;
    time_now = time(nullptr);
    lt = localtime(&time_now);
    auto year = lt->tm_year >= 100 ? lt->tm_year - 100 : lt-> tm_year;
    setValueBigEndian<Word>(&sis.s.sir.disk_number[0], 1U);
    sis.s.sir.fc_start.trk = static_cast<Byte>(start / format.sectors);
    sis.s.sir.fc_start.sec = static_cast<Byte>((start % format.sectors) + 1);
    sis.s.sir.fc_end.trk = static_cast<Byte>(format.tracks - 1);
    sis.s.sir.fc_end.sec = static_cast<Byte>(format.sectors);
    setValueBigEndian<Word>(&sis.s.sir.free[0], static_cast<Word>(free));
    sis.s.sir.month = static_cast<Byte>(lt->tm_mon + 1);
    sis.s.sir.day = static_cast<Byte>(lt->tm_mday);
    sis.s.sir.year = static_cast<Byte>(year);
    sis.s.sir.last.trk = static_cast<Byte>(format.tracks - 1);
    sis.s.sir.last.sec = static_cast<Byte>(format.sectors);
} // create_sys_info_sector

// on success return true
bool FlexFileContainer::Write_dir_sectors(FILE *fp, struct s_formats &format)
{
    std::array<Byte, SECTOR_SIZE> sectorBuffer{};
    int i;

    for (i = 0; i < format.sectors0 - first_dir_trk_sec.sec + 1; i++)
    {
        sectorBuffer[0] = 0;
        sectorBuffer[1] = 0;

        if (i < format.dir_sectors - 1)
        {
            auto sector = i + first_dir_trk_sec.sec;
            sectorBuffer[0] = static_cast<Byte>(sector / format.sectors);
            sectorBuffer[1] = static_cast<Byte>((sector % format.sectors) + 1);
        }

        if (fwrite(sectorBuffer.data(), sectorBuffer.size(), 1, fp) != 1)
        {
            return false;
        }
    }

    return true;
} // write_dir_sectors

// on success return true
bool FlexFileContainer::Write_sectors(FILE *fp, struct s_formats &format)
{
    std::array<Byte, SECTOR_SIZE> sectorBuffer{};
    int i;

    for (i = format.sectors + 1; i <= format.sectors * format.tracks; ++i)
    {
        sectorBuffer[0] = static_cast<Byte>(i / format.sectors);
        sectorBuffer[1] = static_cast<Byte>((i % format.sectors) + 1);

        // use for tests to correctly save random files:
        // (the link always jumps over one sector)
        //      sectorBuffer[0] = (i+1) / format.sectors;
        //      sectorBuffer[1] = ((i+1) % format.sectors) + 1;
        if (i == format.sectors * format.tracks)
        {
            sectorBuffer[0] = sectorBuffer[1] = 0;
        }

        if (fwrite(sectorBuffer.data(), sectorBuffer.size(), 1, fp) != 1)
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
    else if (trk > 256)
    {
        trk = 256;
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

    format.size = format.tracks * format.sectors *
        static_cast<int>(SECTOR_SIZE);
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
    const std::string &directory,
    const std::string &name,
    int tracks,
    int sectors,
    int fmt /* = TYPE_DSK_CONTAINER */,
    const char *bsFile /* = nullptr */)
{
    std::string path;
    struct s_formats format{};
    int err = 0;

    if (name.empty() ||
        tracks < 2 || sectors < 6 || tracks > 256 || sectors > 255)
    {
        throw FlexException(FERR_WRONG_PARAMETER);
    }

    Create_format_table(fmt, tracks, sectors, format);

    path = directory;

    if (!path.empty() && !endsWithPathSeparator(path))
    {
        path +=PATHSEPARATORSTRING;
    }

    path += name;

    BFilePtr fp(path, "wb");

    if (fp != nullptr)
    {
        Byte sector_buffer[SECTOR_SIZE];

        if (fmt == TYPE_FLX_CONTAINER)
        {
            int sides = getSides(format.tracks, format.sectors);
            struct s_flex_header header{};

            header.initialize(SECTOR_SIZE, format.tracks, format.sectors0,
                              format.sectors, sides, sides);

            if (fwrite(&header, sizeof(header), 1, fp) != 1)
            {
                err = 1;
            }
        }

        {
            Byte sector_buffer2[SECTOR_SIZE];

            Create_boot_sectors(sector_buffer, sector_buffer2, bsFile);

            if (fwrite(sector_buffer, sizeof(sector_buffer), 1, fp) != 1)
            {
                err = 1;
            }

            if (fwrite(sector_buffer2, sizeof(sector_buffer2), 1, fp) != 1)
            {
                err = 1;
            }
        }

        u_sys_info_sector sis{};
        Create_sys_info_sector(sis, name, format);

        if (fwrite(sis.raw, sizeof(sis), 1, fp) != 1)
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
    s_sys_info_sector sis{};

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
    struct stat sbuf{};
    Word tracks = 35;
    Word sectors = 10;

    if (stat(fp.GetPath().c_str(), &sbuf) != 0)
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
            auto size = static_cast<off_t>(getFileSize(flx_header));
            if (size == sbuf.st_size)
            {
                return true;
            }
        }
    }

    if ((type & TYPE_DSK_CONTAINER) != 0)
    {
        auto jvcHeader = GetJvcFileHeader();
        auto jvcHeaderSize = static_cast<Word>(jvcHeader.size());

        if (GetFlexTracksSectors(tracks, sectors, jvcHeaderSize))
        {
            off_t size_min = jvcHeaderSize + ((tracks - 1) * sectors + 1) *
                             SECTOR_SIZE;
            off_t size_max = jvcHeaderSize + tracks * sectors * SECTOR_SIZE;

            // do a plausibility check with the size of the DSK file
            if (sbuf.st_size % SECTOR_SIZE == jvcHeaderSize &&
                sbuf.st_size >= size_min && sbuf.st_size <= size_max)
            {
                return true;
            }
        }
    }

    return false;
}

// Extend the directory by one sector. On success return trk-sec of it.
// If disk is full return 00-00.
// parameters:
//    last_dir_sector    sector buffer of last directory sector
//    st_last            trk-sec of last directory sector
st_t FlexFileContainer::ExtendDirectory(u_dir_sector last_dir_sector,
                                        const st_t &st_last)
{
    std::stringstream stream;
    u_sys_info_sector sis{};

    if (!ReadSector(sis.raw, sis_trk_sec.trk, sis_trk_sec.sec))
    {
        stream << sis_trk_sec;
        throw FlexException(FERR_READING_TRKSEC, stream.str(), fp.GetPath());
    }

    auto next = sis.s.sir.fc_start; // Get next trk-sec from start of free chain
    if (next == st_t{})
    {
        return next; // Free chain is empty, directory extend failed.
    }
    last_dir_sector.s.next = next;

    if (!WriteSector(last_dir_sector.raw, st_last.trk, st_last.sec))
    {
        stream << st_last;
        throw FlexException(FERR_WRITING_TRKSEC, stream.str(), fp.GetPath());
    }

    u_dir_sector dir_sector{};
    if (!ReadSector(dir_sector.raw, next.trk, next.sec))
    {
        stream << next;
        throw FlexException(FERR_READING_TRKSEC, stream.str(), fp.GetPath());
    }

    auto new_fc_start = dir_sector.s.next;
    memset(dir_sector.raw, '\0', sizeof(dir_sector));
    dir_sector.s.record_nr[0] = 0x00;
    dir_sector.s.record_nr[1] = 0x01;

    if (!WriteSector(dir_sector.raw, next.trk, next.sec))
    {
        stream << next;
        throw FlexException(FERR_WRITING_TRKSEC, stream.str(), fp.GetPath());
    }

    sis.s.sir.fc_start = new_fc_start;
    if (--sis.s.sir.free[1] == 0xff)
    {
        --sis.s.sir.free[0];
    }
    // Check if free chain is at the end. If so update sir.
    if (new_fc_start == st_t{})
    {
        sis.s.sir.fc_end = st_t{};
        sis.s.sir.free[0] = sis.s.sir.free[1] = 0x00;
    }

    if (!WriteSector(sis.raw, sis_trk_sec.trk, sis_trk_sec.sec))
    {
        stream << sis_trk_sec;
        throw FlexException(FERR_WRITING_TRKSEC, stream.str(), fp.GetPath());
    }

    return next;
}

// JVC file header support functions
// For details see:
// https://sites.google.com/site/dabarnstudio/coco-utilities/jvc-disk-format
//
// The JVC file header has 1 up to 5 bytes:
//
// Byte Offset | Description           | Default | Values supported
//             |                       |         | by flexemu
// ------------+-----------------------+---------+-----------------
//      0      | Sectors per track     |    - 1) | 5-255
//      1      | Side count            |    1    | 1,2
//      2      | Sector Size code      |    1 2) | 1
//      3      | First sector ID       |    1 3) | 1
//      4      | Sector attribute flag |    0 4) | 0
//
// 1) If JVC file header size is 0 then flexemu automatically tries to detect
//    the right disk geometry
// 2) A sector size code of 1 means a sector size of 256 bytes
// 3) On FLEX the first sector ID always should be 1
// 4) Sector attribute flag should be 0, an additional sector attribute byte
//    is not supported by flexemu

std::vector<Byte> FlexFileContainer::GetJvcFileHeader() const
{
    std::vector<Byte> header;
    Word headerSize = file_size % SECTOR_SIZE;
    Byte temp;

    if (headerSize > MAX_JVC_HEADERSIZE)
    {
        throw FlexException(FERR_INVALID_FORMAT, fp.GetPath());
    }

    if (!headerSize)
    {
        return {};
    }

    if (fseek(fp, 0, SEEK_SET))
    {
        throw FlexException(FERR_READING_FROM, fp.GetPath());
    }

    header.resize(headerSize);
    if (fread(header.data(), headerSize, 1, fp) != 1)
    {
        throw FlexException(FERR_READING_FROM, fp.GetPath());
    }

    switch(headerSize)
    {
        case 5:
            temp = header[4]; // sector attribute flag
            if (temp != 0U)
            {
                throw FlexException(FERR_INVALID_JVC_HEADER, fp.GetPath());
            }
            FALLTHROUGH;

        case 4:
            temp = header[3]; // first sector ID
            if (temp != 1U)
            {
                throw FlexException(FERR_INVALID_JVC_HEADER, fp.GetPath());
            }
            FALLTHROUGH;

        case 3:
            temp = header[2]; // sector size count
            if (temp != 1U)
            {
                throw FlexException(FERR_INVALID_JVC_HEADER, fp.GetPath());
            }
            FALLTHROUGH;

        case 2:
            temp = header[1]; // side count
            if (temp < 1U || temp > 2U)
            {
                throw FlexException(FERR_INVALID_JVC_HEADER, fp.GetPath());
            }
            FALLTHROUGH;

        case 1:
            temp = header[0]; // sectors per track
            if (temp < 5U)
            {
                throw FlexException(FERR_INVALID_JVC_HEADER, fp.GetPath());
            }
    }

    return header;
}

