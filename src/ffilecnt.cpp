/*
    ffilecnt.cpp


    FLEXplorer, An explorer for FLEX disk image files and directory disks.
    Copyright (C) 1998-2025  W. Schwotzer

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
#include <string>
#include <sstream>
#include <array>
#include <vector>
#include <cctype>
#include <ctime>
#include <cassert>
#include "filecntb.h"
#include "fcinfo.h"
#include "fattrib.h"
#include "flexerr.h"
#include "cistring.h"
#include "ffilecnt.h"
#include "fdirent.h"
#include "bdate.h"
#include "fcopyman.h"
#include "ffilebuf.h"
#include "ifilecnt.h"
#include "ifilcnti.h"
#include "iffilcnt.h"
#include <cstring>
#include <utility>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;


static_assert(sizeof(s_flex_header) == 16, "Wrong alignment");

static const std::string &getDefaultBootSectorFile()
{
    static std::string defaultBootSectorFile;

    if (defaultBootSectorFile.empty())
    {
#ifdef _WIN32
        defaultBootSectorFile =
            (flx::getExecutablePath() / BOOT_FILE).u8string();
#else
        defaultBootSectorFile = fs::path(F_DATADIR) / BOOT_FILE;
#endif
    }

    return defaultBootSectorFile;
}

bool FlexDisk::onTrack0OnlyDirSectors = true;

/***********************************************/
/* Initialization of a s_flex_header structure */
/***********************************************/

void s_flex_header::initialize(unsigned sector_size, int p_tracks,
        int p_sectors0, int p_sectors, int p_sides0, int p_sides)
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
        if (sector_size & (1U << i))
        {
            p_sizecode = i - 7;
            break;
        }
    }

    magic_number = flx::toBigEndian(MAGIC_NUMBER);
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

FlexDisk::FlexDisk(
        const std::string &p_path,
        std::ios::openmode mode,
        const FileTimeAccess &fileTimeAccess)
    : path(p_path)
    , fstream(p_path, mode)
    , ft_access(fileTimeAccess)
    , is_flex_format(true)
{
    const auto status = fs::status(path);
    if (!fs::exists(status) || !fs::is_regular_file(status))
    {
        throw FlexException(FERR_UNABLE_TO_OPEN, path);
    }

    if (!fstream.is_open())
    {
        throw FlexException(FERR_UNABLE_TO_OPEN, path);
    }

    param.type = DiskType::DSK;
    file_size = static_cast<DWord>(fs::file_size(path));
    if (file_size == 0U)
    {
        // If file has been created or file size 0 then
        // it is marked as an unformatted file container.
        // No records are available yet but it can be formatted
        // from within the emulation.
        is_flex_format = false;
        Initialize_unformatted_disk();
        return;
    }

    fstream.seekg(0);
    if (fstream.fail())
    {
        throw FlexException(FERR_UNABLE_TO_OPEN, path);
    }

    if ((mode & std::ios::out) == 0)
    {
        attributes |= WRITE_PROTECT;
    }

    // Try to read the FLX header and check the magic number
    // to identify a FLX formatted disk.
    fstream.read(reinterpret_cast<char *>(&flx_header), sizeof(flx_header));
    if (!fstream.fail() &&
        flx::fromBigEndian(flx_header.magic_number) == MAGIC_NUMBER)
    {
        // File is identified as a FLX container format.
        Initialize_for_flx_format(flx_header);

        if (!IsFlexFileFormat(DiskType::FLX))
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
    if (IsFlexFileFormat(DiskType::DSK) &&
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
                throw FlexException(FERR_INVALID_JVC_HEADER, path);
            }
        }
        format.size = static_cast<SDWord>(file_size);
        format.offset = jvcHeaderSize;
        Initialize_for_dsk_format(format);
        EvaluateTrack0SectorCount();
        return;
    }

    throw FlexException(FERR_IS_NO_FILECONTAINER, path);
}

/****************************************/
/* Public interface                     */
/****************************************/

std::string FlexDisk::GetPath() const
{
    return path;
}

unsigned FlexDisk::GetBytesPerSector() const
{
    return param.byte_p_sector;
}

bool FlexDisk::IsWriteProtected() const
{
    return (attributes & WRITE_PROTECT) != 0;
}

bool FlexDisk::IsTrackValid(int track) const
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

bool FlexDisk::IsSectorValid(int track, int sector) const
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

bool FlexDisk::IsFlexFormat() const
{
    return is_flex_format;
}

FlexDisk *FlexDisk::Create(
        const std::string &path,
        const FileTimeAccess &fileTimeAccess,
        int tracks,
        int sectors,
        DiskType disk_type,
        const char *bsFile /* = nullptr */)
{
    if (disk_type != DiskType::DSK && disk_type != DiskType::FLX)
    {
        using T = std::underlying_type_t<DiskType>;
        auto id = static_cast<T>(disk_type);

        throw FlexException(FERR_INVALID_FORMAT, id);
    }

    Format_disk(path, tracks, sectors, disk_type, bsFile);

    auto mode = std::ios::in | std::ios::out | std::ios::binary;
    return new FlexDisk(path, mode, fileTimeAccess);
}

void FlexDisk::SetBootSectorFile(const std::string &p_bootSectorFile)
{
    GetBootSectorFile() = p_bootSectorFile;
}

std::string &FlexDisk::GetBootSectorFile()
{
    static std::string bootSectorFile;

    return bootSectorFile;
}

// return true if file found
// if file found can also be checked by
// !entry.isEmpty
bool FlexDisk::FindFile(const std::string &wildcard, FlexDirEntry &dirEntry)
{
    if (is_flex_format)
    {
        if (wildcard.find_first_of("*?[]") == std::string::npos)
        {
            if (!is_filenames_initialized)
            {
                InitializeFilenames();
            }

            const auto iter = filenames.find(flx::tolower(wildcard));
            if (iter != filenames.end())
            {
                s_dir_sector sectorBuffer{};
                const auto &dir_pos = iter->second;

                if (!ReadSector(reinterpret_cast<Byte *>(&sectorBuffer),
                                    dir_pos.trk_sec.trk, dir_pos.trk_sec.sec))
                {
                    std::stringstream stream;

                    stream << dir_pos.trk_sec;
                    throw FlexException(FERR_READING_TRKSEC,
                                      stream.str(), GetPath());
                }
                const auto &dir_entry = sectorBuffer.dir_entries[dir_pos.idx];
                dirEntry = CreateDirEntryFrom(dir_entry, wildcard);
                return true;
            }

            dirEntry.SetEmpty();
            return false;
        }

        FlexDiskIterator it(wildcard);

        it = this->begin();

        if (it != this->end())
        {
            dirEntry = *it;
            return true;
        }
    }

    dirEntry.SetEmpty();
    return false;
}

void FlexDisk::InitializeClass()
{
    SetBootSectorFile(getDefaultBootSectorFile());
}

bool FlexDisk::DeleteFile(const std::string &wildcard)
{
    if (!is_flex_format)
    {
        return false;
    }

    CHECK_CONTAINER_WRITEPROTECTED;

    FlexDiskIterator it(wildcard);

    for (it = this->begin(); it != this->end(); ++it)
    {
        it.DeleteCurrent();
    }

    return true;
}

bool FlexDisk::RenameFile(const std::string &oldName,
                          const std::string &newName)
{
    if (!is_flex_format || (oldName.compare(newName) == 0))
    {
        return false;
    }

    CHECK_CONTAINER_WRITEPROTECTED;

    if (oldName.find_first_of("*?[]") != std::string::npos)
    {
        throw FlexException(FERR_WILDCARD_NOT_SUPPORTED, oldName);
    }

    if (newName.find_first_of("*?[]") != std::string::npos)
    {
        throw FlexException(FERR_WILDCARD_NOT_SUPPORTED, newName);
    }

    // prevent overwriting of an existing file
    // except for changing lower to uppercase.
    // std::string with different type traits can not be copy-constructed.
    // A conversion to const char * is needed. False-positive to be ignored.
    // NOLINTBEGIN(readability-redundant-string-cstr)
    ci_string ci_oldName(oldName.c_str());
    if (ci_oldName.compare(newName.c_str()) != 0 && FindInFilenames(newName))
    // NOLINTEND(readability-redundant-string-cstr)
    {
        throw FlexException(FERR_FILE_ALREADY_EXISTS, newName);
    }

    FlexDiskIterator it(oldName);

    it = this->begin();

    if (it == this->end())
    {
        throw FlexException(FERR_NO_FILE_IN_CONTAINER, oldName, path);
    }

    it.RenameCurrent(newName);

    return true;
}

bool FlexDisk::FileCopy(const std::string &sourceName,
                                 const std::string &destName,
                                 IFlexDiskByFile &destination)
{
    if (!is_flex_format)
    {
        return false;
    }

    if (sourceName.find_first_of("*?[]") != std::string::npos)
    {
        throw FlexException(FERR_WILDCARD_NOT_SUPPORTED, sourceName);
    }

    if (destName.find_first_of("*?[]") != std::string::npos)
    {
        throw FlexException(FERR_WILDCARD_NOT_SUPPORTED, destName);
    }

    return FlexCopyManager::FileCopy(sourceName, destName, *this, destination);
}

bool FlexDisk::GetDiskAttributes(FlexDiskAttributes &diskAttributes) const
{
    if (is_flex_format)
    {
        s_sys_info_sector sis{};
        int year;

        if (!ReadSector(reinterpret_cast<Byte *>(&sis), sis_trk_sec.trk,
                        sis_trk_sec.sec))
        {
            std::stringstream stream;

            stream << sis_trk_sec;
            throw FlexException(FERR_READING_TRKSEC, stream.str(), path);
        }

        if (sis.sir.year < 75)
        {
            year = sis.sir.year + 2000;
        }
        else
        {
            year = sis.sir.year + 1900;
        }

        auto size = 0U;
        while (size < sizeof(sis.sir.disk_name) && sis.sir.disk_name[size])
        {
            ++size;
        }
        std::string disk_name(sis.sir.disk_name, size);
        disk_name.append("");
        bool is_valid = true;
        size = 0U;
        while (size < sizeof(sis.sir.disk_ext) && sis.sir.disk_ext[size])
        {
            if (sis.sir.disk_ext[size] < ' ' || sis.sir.disk_ext[size] > '~')
            {
                is_valid = false;
                break;
            }
            ++size;
        }
        if (size > 0U && is_valid)
        {
            std::string disk_ext(sis.sir.disk_ext, size);
            disk_ext.append("");
            disk_name.append(".");
            disk_name.append(disk_ext);
        }
        diskAttributes.SetDate(BDate(sis.sir.day, sis.sir.month, year));
        diskAttributes.SetFree(flx::getValueBigEndian<Word>(&sis.sir.free[0]) *
                               param.byte_p_sector);
        diskAttributes.SetTotalSize((sis.sir.last.sec *
                                    (sis.sir.last.trk + 1)) *
                                    param.byte_p_sector);
        diskAttributes.SetName(disk_name);
        diskAttributes.SetNumber(
                flx::getValueBigEndian<Word>(&sis.sir.disk_number[0]));
    }
    else
    {
        if (param.type == DiskType::FLX)
        {
            const auto size = param.byte_p_track0 +
                    param.byte_p_track * param.max_track;
            diskAttributes.SetTotalSize(size);
        }
    }

    diskAttributes.SetTrackSector(
            param.max_track ? param.max_track + 1 : 0,
            param.max_sector);
    diskAttributes.SetIsFlexFormat(is_flex_format);
    diskAttributes.SetPath(path);
    diskAttributes.SetType(param.type);
    diskAttributes.SetOptions(param.options);
    diskAttributes.SetAttributes(attributes);
    diskAttributes.SetSectorSize(param.byte_p_sector);
    diskAttributes.SetIsWriteProtected(IsWriteProtected());
    if (param.type == DiskType::DSK)
    {
        diskAttributes.SetJvcFileHeader(GetJvcFileHeader());
    }

    return true;
}

DiskType FlexDisk::GetFlexDiskType() const
{
    return param.type;
}

DiskOptions FlexDisk::GetFlexDiskOptions() const
{
    return param.options;
}

std::string FlexDisk::GetSupportedAttributes() const
{
    return "WDRC";
}

/******************************/
/* Nonpublic interface        */
/******************************/

IFlexDiskIteratorImpPtr FlexDisk::IteratorFactory()
{
    return IFlexDiskIteratorImpPtr(new FlexDiskIteratorImp(this));
}

// if successfull return true. If error return false
bool FlexDisk::WriteFromBuffer(const FlexFileBuffer &buffer,
                               const char *p_fileName /* = nullptr */)
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
    s_sys_info_sector sis{};
    std::string fileName;
    // sectorBuffer[2] and [1] are used for the Sector Map of random files.
    std::array<SectorBuffer_t, 3> sectorBuffer{};

    fileName = (p_fileName == nullptr) ? buffer.GetFilename() : p_fileName;

    if (FindInFilenames(fileName))
    {
        throw FlexException(FERR_FILE_ALREADY_EXISTS, fileName);
    }

    if (buffer.GetFileSize() == 0U)
    {
        throw FlexException(FERR_COPY_EMPTY_FILE, fileName);
    }

    // read sys info sector
    if (!ReadSector(reinterpret_cast<Byte *>(&sis), sis_trk_sec.trk,
                    sis_trk_sec.sec))
    {
        std::stringstream stream;

        stream << sis_trk_sec;
        throw FlexException(FERR_READING_TRKSEC, stream.str(), path);
    }

    next = start = sis.sir.fc_start;

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

    while (static_cast<DWord>(recordNr * (SECTOR_SIZE - 4)) <
            buffer.GetFileSize())
    {
        for (count = repeat; count >= 0; count--)
        {
            trk = next.trk;
            sec = next.sec;

            if (trk == 0 && sec == 0)
            {
                throw FlexException(FERR_DISK_FULL_WRITING, path, fileName);
            }

            if (!ReadSector(sectorBuffer[count].data(), trk, sec))
            {
                std::stringstream stream;

                stream << next;
                throw FlexException(FERR_READING_TRKSEC, stream.str(), path);
            }

            if (count)
            {
                // For random files the two sector map sectors are
                // skipped. They are newly generated in sectorBuffer[2]
                // and sectorBuffer[1].
                // Here the buffer is initialized to zero.
                std::memset(&sectorBuffer[count][2], 0, SECTOR_SIZE - 2);
                ++recordNr;
            }

            next.trk = sectorBuffer[count][0];
            next.sec = sectorBuffer[count][1];
        }
        repeat = 0; // Finished preparing random file sector map.

        if (!buffer.CopyTo(&sectorBuffer[0][4], SECTOR_SIZE - 4,
                           recordNr * (SECTOR_SIZE - 4), '\0'))
        {
            std::stringstream stream;

            stream << next;
            throw FlexException(FERR_WRITING_TRKSEC, stream.str(), path);
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
                                            fileName, path);
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
        flx::setValueBigEndian<Word>(&sectorBuffer[0][2],
                                recordNr - (buffer.IsRandom() ? 2U : 0U));

        if (static_cast<DWord>(recordNr * (SECTOR_SIZE - 4)) >=
                buffer.GetFileSize())
        {
            sectorBuffer[0][0] = sectorBuffer[0][1] = 0;
        }

        if (!WriteSector(sectorBuffer[0].data(), trk, sec))
        {
            std::stringstream stream;

            stream << next;
            throw FlexException(FERR_WRITING_TRKSEC, stream.str(), path);
        }
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
            if (!WriteSector(sectorBuffer[count].data(), next.trk, next.sec))
            {
                std::stringstream stream;

                stream << next;
                throw FlexException(FERR_WRITING_TRKSEC, stream.str(), path);
            }

            next.trk = sectorBuffer[count][0];
            next.sec = sectorBuffer[count][1];
        }
    }

    // Update the system info sector.
    auto free = flx::getValueBigEndian<Word>(&sis.sir.free[0]);
    free -= recordNr;
    flx::setValueBigEndian<Word>(&sis.sir.free[0], free);

    if (!WriteSector(reinterpret_cast<const Byte *>(&sis), sis_trk_sec.trk,
                     sis_trk_sec.sec))
    {
        std::stringstream stream;

        stream << sis_trk_sec;
        throw FlexException(FERR_WRITING_TRKSEC, stream.str(), path);
    }

    // Create a new directory entry.
    auto dirEntry = buffer.GetDirEntry();
    dirEntry.SetTotalFileName(fileName);
    if ((ft_access & FileTimeAccess::Set) != FileTimeAccess::Set)
    {
        dirEntry.SetTime(BTime());
    }
    dirEntry.SetStartTrkSec(start.trk, start.sec);
    dirEntry.SetEndTrkSec(trk, sec);
    CreateDirEntry(dirEntry);

    return true;
}

FlexFileBuffer FlexDisk::ReadToBuffer(const std::string &fileName)
{
    FlexFileBuffer buffer;
    FlexDirEntry dirEntry;
    int trk;
    int sec;
    int recordNr;
    SectorBuffer_t sectorBuffer{};
    uint32_t size;

    if (!is_flex_format)
    {
        throw FlexException(FERR_CONTAINER_UNFORMATTED, path);
    }

    if (fileName.find_first_of("*?[]") != std::string::npos)
    {
        throw FlexException(FERR_WILDCARD_NOT_SUPPORTED, fileName);
    }

    if (!FindFile(fileName, dirEntry))
    {
        throw FlexException(FERR_UNABLE_TO_OPEN, fileName);
    }

    buffer.SetAttributes(dirEntry.GetAttributes());
    buffer.SetSectorMap(dirEntry.GetSectorMap());
    buffer.SetFilename(fileName);
    BTime time;
    if ((ft_access & FileTimeAccess::Get) == FileTimeAccess::Get)
    {
        time = dirEntry.GetTime();
    }
    buffer.SetDateTime(dirEntry.GetDate(), time);
    size = dirEntry.GetFileSize();

    size = size * DBPS / static_cast<int>(SECTOR_SIZE);
    buffer.Realloc(size);
    recordNr = 0;

    if (size > 0)
    {
        dirEntry.GetStartTrkSec(trk, sec);

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
bool FlexDisk::SetAttributes(const std::string &wildcard,
        unsigned setMask, unsigned clearMask /* = ~0 */)
{
    if (!is_flex_format)
    {
        return false;
    }

    CHECK_CONTAINER_WRITEPROTECTED;

    FlexDiskIterator it(wildcard);

    for (it = this->begin(); it != this->end(); ++it)
    {
        Byte p_attributes =
            static_cast<Byte>((it->GetAttributes() & ~clearMask) | setMask);
        it.SetAttributesCurrent(p_attributes);
    }

    return true;
}

bool FlexDisk::CreateDirEntry(FlexDirEntry &dirEntry)
{
    if (!is_flex_format)
    {
        return false;
    }

    s_dir_sector dir_sector{};
    st_t next(next_dir_trk_sec);
    int tmp1;
    int tmp2;
    BDate date;

    if (next == st_t())
    {
        next = first_dir_trk_sec;
    }

    // loop until all directory sectors read
    while (next.sec != 0 || next.trk != 0)
    {
        // read next directory sector
        if (!ReadSector(reinterpret_cast<Byte *>(&dir_sector), next.trk,
                        next.sec))
        {
            std::stringstream stream;

            stream << next;
            throw FlexException(FERR_READING_TRKSEC, stream.str(), path);
        }

        for (Byte idx = 0U; idx < DIRENTRIES; ++idx)
        {
            // look for the next free directory entry
            auto &dir_entry = dir_sector.dir_entries[idx];

            if (dir_entry.filename[0] == DE_EMPTY ||
                dir_entry.filename[0] == DE_DELETED)
            {
                BTime time;

                if ((ft_access & FileTimeAccess::Set) == FileTimeAccess::Set)
                {
                   time = dirEntry.GetTime();
                }
                auto records = dirEntry.GetFileSize() / param.byte_p_sector;
                std::memset(dir_entry.filename, 0, FLEX_BASEFILENAME_LENGTH);
                std::strncpy(dir_entry.filename, dirEntry.GetFileName().c_str(),
                        FLEX_BASEFILENAME_LENGTH);
                std::memset(dir_entry.file_ext, 0, FLEX_FILEEXT_LENGTH);
                std::strncpy(dir_entry.file_ext, dirEntry.GetFileExt().c_str(),
                        FLEX_FILEEXT_LENGTH);
                dir_entry.file_attr = dirEntry.GetAttributes();
                dir_entry.hour = static_cast<Byte>(time.GetHour());
                dirEntry.GetStartTrkSec(tmp1, tmp2);
                dir_entry.start.trk = static_cast<Byte>(tmp1);
                dir_entry.start.sec = static_cast<Byte>(tmp2);
                dirEntry.GetEndTrkSec(tmp1, tmp2);
                dir_entry.end.trk = static_cast<Byte>(tmp1);
                dir_entry.end.sec = static_cast<Byte>(tmp2);
                flx::setValueBigEndian<Word>(&dir_entry.records[0],
                        static_cast<Word>(records));
                dir_entry.sector_map =
                    (dirEntry.IsRandom() ? IS_RANDOM_FILE : 0x00);
                dir_entry.minute = static_cast<Byte>(time.GetMinute());
                date = dirEntry.GetDate();
                dir_entry.day = static_cast<Byte>(date.GetDay());
                dir_entry.month = static_cast<Byte>(date.GetMonth());
                dir_entry.year = static_cast<Byte>(date.GetYear() % 100);

                if (!WriteSector(reinterpret_cast<const Byte *>(&dir_sector),
                                 next.trk, next.sec))
                {
                    std::stringstream stream;

                    stream << next;
                    throw FlexException(FERR_WRITING_TRKSEC,
                                        stream.str(), path);
                }

                SetNextDirectoryPosition(next);
                AddToFilenames(
                        dirEntry.GetTotalFileName(), s_dir_pos{next, idx});

                return true;
            }
        }

        next = (dir_sector.next == st_t{}) ?
            ExtendDirectory(dir_sector, next) : dir_sector.next;
    }

    throw FlexException(FERR_DIRECTORY_FULL);
}

// Evaluate the sector count on track 0.
// Follow the directory chain until end of chain reached or link is
// pointing to a directory extend on a track != 0.
// There are DSK files with the same track-sector count but different
// number of sectors on track 0:
//   35-18: Has 10 or 18 sectors on track 0.
//   40-18: Has 10 or 18 sectors on track 0.
// This default behaviour can be overwritten by the global flag:
//   FlexDisk::onTrack0OnlyDirSectors = false;
void FlexDisk::EvaluateTrack0SectorCount()
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
        fstream.seekg(param.offset + (i * param.byte_p_sector));
        if (fstream.fail())
        {
            throw FlexException(FERR_UNABLE_TO_OPEN, path);
        }

        fstream.read(reinterpret_cast<char *>(&link), sizeof(link));
        if (fstream.fail())
        {
            throw FlexException(FERR_UNABLE_TO_OPEN, path);
        }

        if (link == st_t{0, 0} || link.trk != 0)
        {
            break;
        }
    }

    param.max_sector0 = std::min(param.max_sector, static_cast<Word>(i + 1));
}


/****************************************/
/* low level routines                   */
/****************************************/

int FlexDisk::ByteOffset(int trk, int sec, int side) const
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
bool FlexDisk::ReadSector(Byte *pbuffer, int trk, int sec,
                          int side /* = -1 */) const
{
    if (!fstream.is_open())
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

    fstream.seekg(pos);
    if (fstream.fail())
    {
        return false;
    }

    fstream.read(reinterpret_cast<char *>(pbuffer), param.byte_p_sector);
    return !fstream.fail();
}

// low level routine to write a single sector
// should be used with care
// Does not throw any exception !
// returns false on failure
bool FlexDisk::WriteSector(const Byte *pbuffer, int trk, int sec,
                           int side /* = -1 */)
{
    if (!fstream.is_open())
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

    fstream.seekg(pos);
    if (fstream.fail())
    {
        return false;
    }

    if (IsWriteProtected())
    {
        return false;
    }

    fstream.write(reinterpret_cast<const char *>(pbuffer), param.byte_p_sector);
    if (fstream.fail())
    {
        return false;
    }

    if (!is_flex_format &&
        trk == 0 && sec == 3 && IsFlexFileFormat(DiskType::FLX))
    {
        is_flex_format = true;
    }

    return true;
}

bool FlexDisk::FormatSector(const Byte *target, int track, int sector,
                            int side, unsigned sizecode)
{
    if (is_flex_format ||
        track < 0 || track > 255 ||
        sector < 1 || sector > 255 ||
        side < 0 || side > 1 ||
        sizecode > 3U)
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

    fstream.seekg(0);
    if (fstream.fail())
    {
        result = false;
    }

    fstream.write(reinterpret_cast<const char *>(&flx_header),
                  sizeof(flx_header));
    if (fstream.fail())
    {
        result = false;
    }

    result &= WriteSector(target, track, sector, side);

    if (result && !is_flex_format && (file_size == getFileSize(flx_header)) &&
        IsFlexFileFormat(DiskType::FLX))
    {
        is_flex_format = true;
    }

    return result;
}

void FlexDisk::Initialize_for_flx_format(const s_flex_header &header)
{
    if (header.write_protect != 0)
    {
        attributes |= WRITE_PROTECT;
    }

    param.offset = sizeof(struct s_flex_header);
    param.write_protect = ((attributes & WRITE_PROTECT) != 0) ? 1U : 0U;
    param.max_sector = header.sectors * header.sides;
    param.max_sector0 = header.sectors0 * header.sides0;
    param.max_track = header.tracks - 1;
    param.byte_p_sector = getBytesPerSector(header.sizecode);
    param.byte_p_track0 = param.max_sector0 * param.byte_p_sector;
    param.byte_p_track = param.max_sector * param.byte_p_sector;
    param.sides0 = header.sides0;
    param.sides = header.sides;
    param.type = DiskType::FLX;
    param.options = DiskOptions::HasSectorIF;
}

void FlexDisk::Initialize_for_dsk_format(const s_formats &format)
{
    auto sides = format.sides ? format.sides :
                     getSides(format.tracks, format.sectors);
    auto sector0 = (format.offset != 0U) ? format.sectors :
                     getTrack0SectorCount(format.tracks, format.sectors);

    file_size = format.size;
    param.offset = format.offset;
    param.write_protect = ((attributes & WRITE_PROTECT) != 0) ? 1U : 0U;
    param.max_sector = format.sectors;
    param.max_sector0 = sector0;
    param.max_track = format.tracks - 1;
    param.byte_p_sector = SECTOR_SIZE;
    param.byte_p_track0 = param.max_sector * SECTOR_SIZE;
    param.byte_p_track = param.max_sector * SECTOR_SIZE;
    param.sides0 = sides;
    param.sides = sides;
    param.type = DiskType::DSK;
    param.options = DiskOptions::HasSectorIF;
    if (format.offset != 0U)
    {
        param.options |= DiskOptions::JvcHeader;
    }
}

void FlexDisk::Initialize_unformatted_disk()
{
    file_size = sizeof(struct s_flex_header);
    param = { };
    param.offset = sizeof(struct s_flex_header);
    param.type = DiskType::FLX;
    param.options = DiskOptions::NONE;
}

void FlexDisk::Create_boot_sectors(BootSectorBuffer_t &bootSectors,
                                   const char *bsFile)
{
    // Read boot sector(s) if present from file.
    if (bsFile == nullptr)
    {
        bsFile = getDefaultBootSectorFile().c_str();
    }
    std::memset(bootSectors.data(), '\0', bootSectors.size());
    std::fstream boot(bsFile, std::ios::in | std::ios::binary);

    if (boot.is_open())
    {
        boot.read(reinterpret_cast<char *>(bootSectors.data()),
                  static_cast<std::streamsize>(bootSectors.size()));
    }

    if (!boot.is_open() || (boot.fail() &&
        (boot.gcount() != SECTOR_SIZE && boot.gcount() != 2 * SECTOR_SIZE)))
    {
        // No boot sector or read error.
        // Instead jump to monitor program warm start entry point.
        std::memset(bootSectors.data(), '\0', bootSectors.size());
        bootSectors[0] = 0x7E; // JMP $F02D
        bootSectors[1] = 0xF0;
        bootSectors[2] = 0x2D;

        return;
    }
}

void FlexDisk::Create_sys_info_sector(s_sys_info_sector &sis,
        const std::string &name,
        struct s_formats &format)
{
    int start;
    int free;

    memset(&sis, 0, sizeof(sis));

    auto diskname = getDiskName(name);
    diskname.resize(FLEX_DISKNAME_LENGTH);
    std::copy_n(diskname.cbegin(), FLEX_DISKNAME_LENGTH,
            std::begin(sis.sir.disk_name));
    start = format.sectors;
    free = (format.sectors * format.tracks) - start;
    const auto time_now = time(nullptr);
    const struct tm *lt = localtime(&time_now);
    auto year = lt->tm_year >= 100 ? lt->tm_year - 100 : lt-> tm_year;
    flx::setValueBigEndian<Word>(&sis.sir.disk_number[0], 0U);
    sis.sir.fc_start.trk = static_cast<Byte>(start / format.sectors);
    sis.sir.fc_start.sec = static_cast<Byte>((start % format.sectors) + 1);
    sis.sir.fc_end.trk = static_cast<Byte>(format.tracks - 1);
    sis.sir.fc_end.sec = static_cast<Byte>(format.sectors);
    flx::setValueBigEndian<Word>(&sis.sir.free[0], static_cast<Word>(free));
    sis.sir.month = static_cast<Byte>(lt->tm_mon + 1);
    sis.sir.day = static_cast<Byte>(lt->tm_mday);
    sis.sir.year = static_cast<Byte>(year);
    sis.sir.last.trk = static_cast<Byte>(format.tracks - 1);
    sis.sir.last.sec = static_cast<Byte>(format.sectors);
}

// on success return true
bool FlexDisk::Write_dir_sectors(std::fstream &ofs, struct s_formats &format)
{
    SectorBuffer_t sectorBuffer{};
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

        ofs.write(reinterpret_cast<const char *>(sectorBuffer.data()),
                  sectorBuffer.size());
        if (ofs.fail())
        {
            return false;
        }
    }

    return true;
}

// on success return true
bool FlexDisk::Write_sectors(std::fstream &ofs, struct s_formats &format)
{
    SectorBuffer_t sectorBuffer{};
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

        ofs.write(reinterpret_cast<const char *>(sectorBuffer.data()),
                  sectorBuffer.size());
        if (ofs.fail())
        {
            return false;
        }
    }

    return true;
}

void FlexDisk::Create_format_table(DiskType p_disk_type, int trk, int sec,
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
    format.sectors0 = (p_disk_type == DiskType::FLX) ?
                          getTrack0SectorCount(format.tracks, format.sectors) :
                          format.sectors;

    format.size = format.tracks * format.sectors *
        static_cast<int>(SECTOR_SIZE);
    // calculate number of directory sectors.
    // track 0 only contains directory sectors.
    format.dir_sectors = getTrack0SectorCount(trk, sec) -
                          first_dir_trk_sec.sec + 1;
}

// return != 0 on success
// format FLX or DSK format. FLX format always with sector_size 256
// number of sectors on track 0 is calculated by method
// getTrack0SectorCount().

void FlexDisk::Format_disk(
    const std::string &path,
    int tracks,
    int sectors,
    DiskType p_disk_type,
    const char *bsFile /* = nullptr */)
{
    struct s_formats format{};
    int err = 0;

    if (path.empty() ||
        tracks < 2 || sectors < 6 || tracks > 256 || sectors > 255)
    {
        throw FlexException(FERR_WRONG_PARAMETER);
    }

    Create_format_table(p_disk_type, tracks, sectors, format);

    std::fstream fstream(path, std::ios::out | std::ios::binary |
                         std::ios::trunc);

    if (fstream.is_open())
    {
        if (p_disk_type == DiskType::FLX)
        {
            int sides = getSides(format.tracks, format.sectors);
            struct s_flex_header header{};

            header.initialize(SECTOR_SIZE, format.tracks, format.sectors0,
                              format.sectors, sides, sides);

            fstream.write(reinterpret_cast<const char *>(&header),
                          sizeof(header));
            if (fstream.fail())
            {
                err = 1;
            }
        }

        {
            BootSectorBuffer_t bootSectors{};

            Create_boot_sectors(bootSectors, bsFile);

            fstream.write(reinterpret_cast<const char *>(bootSectors.data()),
                          bootSectors.size());
            if (fstream.fail())
            {
                err = 1;
            }
        }

        s_sys_info_sector sis{};
        Create_sys_info_sector(sis, flx::getFileName(path), format);

        fstream.write(reinterpret_cast<const char *>(&sis), sizeof(sis));
        if (fstream.fail())
        {
            err = 1;
        }

        {
            SectorBuffer_t sectorBuffer{};

            // Sector 00-04 seems to be unused. Format with all zeros.
            std::memset(sectorBuffer.data(), '\0', sectorBuffer.size());
            fstream.write(reinterpret_cast<const char *>(sectorBuffer.data()),
                          sectorBuffer.size());
            if (fstream.fail())
            {
                err = 1;
            }
        }

        if (!Write_dir_sectors(fstream, format))
        {
            err = 1;
        }

        if (!Write_sectors(fstream, format))
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
        throw FlexException(FERR_UNABLE_TO_FORMAT, path);
    }
}

// Read the number of tracks and sectors for a FLEX file container.
bool FlexDisk::GetFlexTracksSectors(Word &tracks, Word &sectors,
                                    Word header_offset) const
{
    s_sys_info_sector sis{};

    // Read system info sector.
    long file_offset = header_offset + (sis_trk_sec.sec - 1) * SECTOR_SIZE;
    fstream.seekg(file_offset);
    if (fstream.fail())
    {
        fstream.clear();
        return false;
    }

    fstream.read(reinterpret_cast<char *>(&sis), sizeof(sis));
    if (fstream.fail())
    {
        fstream.clear();
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
bool FlexDisk::IsFlexFileFormat(DiskType p_disk_type) const
{
    Word tracks = 35;
    Word sectors = 10;

    if (!fs::exists(path))
    {
        return false;
    }

    const auto fileSize = fs::file_size(path);
    if (p_disk_type == DiskType::FLX)
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
            if (getFileSize(flx_header) == fileSize)
            {
                return true;
            }
        }
    }

    if (p_disk_type == DiskType::DSK)
    {
        auto jvcHeader = GetJvcFileHeader();
        auto jvcHeaderSize = static_cast<Word>(jvcHeader.size());

        if (GetFlexTracksSectors(tracks, sectors, jvcHeaderSize))
        {
            size_t size_min = jvcHeaderSize + ((tracks - 1) * sectors + 1) *
                             SECTOR_SIZE;
            size_t size_max = jvcHeaderSize + tracks * sectors * SECTOR_SIZE;

            // do a plausibility check with the size of the DSK file
            if (fileSize % SECTOR_SIZE == jvcHeaderSize &&
                fileSize >= size_min && fileSize <= size_max)
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
st_t FlexDisk::ExtendDirectory(s_dir_sector last_dir_sector,
                               const st_t &st_last)
{
    std::stringstream stream;
    s_sys_info_sector sis{};

    if (!ReadSector(reinterpret_cast<Byte *>(&sis), sis_trk_sec.trk,
                    sis_trk_sec.sec))
    {
        stream << sis_trk_sec;
        throw FlexException(FERR_READING_TRKSEC, stream.str(), path);
    }

    auto next = sis.sir.fc_start; // Get next trk-sec from start of free chain
    if (next == st_t{})
    {
        return next; // Free chain is empty, directory extend failed.
    }
    last_dir_sector.next = next;

    if (!WriteSector(reinterpret_cast<const Byte *>(&last_dir_sector),
                     st_last.trk, st_last.sec))
    {
        stream << st_last;
        throw FlexException(FERR_WRITING_TRKSEC, stream.str(), path);
    }

    s_dir_sector dir_sector{};
    if (!ReadSector(reinterpret_cast<Byte *>(&dir_sector), next.trk, next.sec))
    {
        stream << next;
        throw FlexException(FERR_READING_TRKSEC, stream.str(), path);
    }

    auto new_fc_start = dir_sector.next;
    std::memset(&dir_sector, '\0', sizeof(dir_sector));
    dir_sector.record_nr[0] = 0x00;
    dir_sector.record_nr[1] = 0x01;

    if (!WriteSector(reinterpret_cast<const Byte *>(&dir_sector), next.trk,
                     next.sec))
    {
        stream << next;
        throw FlexException(FERR_WRITING_TRKSEC, stream.str(), path);
    }

    sis.sir.fc_start = new_fc_start;
    if (--sis.sir.free[1] == 0xff)
    {
        --sis.sir.free[0];
    }
    // Check if free chain is at the end. If so update sir.
    if (new_fc_start == st_t{})
    {
        sis.sir.fc_end = st_t{};
        sis.sir.free[0] = sis.sir.free[1] = 0x00;
    }

    if (!WriteSector(reinterpret_cast<const Byte *>(&sis), sis_trk_sec.trk,
                     sis_trk_sec.sec))
    {
        stream << sis_trk_sec;
        throw FlexException(FERR_WRITING_TRKSEC, stream.str(), path);
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

std::vector<Byte> FlexDisk::GetJvcFileHeader() const
{
    std::vector<Byte> header;
    Word headerSize = file_size % SECTOR_SIZE;
    Byte temp;

    if (headerSize > MAX_JVC_HEADERSIZE)
    {
        throw FlexException(FERR_IS_NO_FILECONTAINER, path);
    }

    if (!headerSize)
    {
        return {};
    }

    fstream.seekg(0);
    if (fstream.fail())
    {
        throw FlexException(FERR_READING_FROM, path);
    }

    header.resize(headerSize);
    fstream.read(reinterpret_cast<char *>(header.data()), headerSize);
    if (fstream.fail())
    {
        throw FlexException(FERR_READING_FROM, path);
    }

    switch(headerSize)
    {
        case 5:
            temp = header[4]; // sector attribute flag
            if (temp != 0U)
            {
                throw FlexException(FERR_INVALID_JVC_HEADER, path);
            }
            FALLTHROUGH;

        case 4:
            temp = header[3]; // first sector ID
            if (temp != 1U)
            {
                throw FlexException(FERR_INVALID_JVC_HEADER, path);
            }
            FALLTHROUGH;

        case 3:
            temp = header[2]; // sector size count
            if (temp != 1U)
            {
                throw FlexException(FERR_INVALID_JVC_HEADER, path);
            }
            FALLTHROUGH;

        case 2:
            temp = header[1]; // side count
            if (temp < 1U || temp > 2U)
            {
                throw FlexException(FERR_INVALID_JVC_HEADER, path);
            }
            FALLTHROUGH;

        case 1:
            temp = header[0]; // sectors per track
            if (temp < 5U)
            {
                throw FlexException(FERR_INVALID_JVC_HEADER, path);
            }
    }

    return header;
}

FlexDirEntry FlexDisk::CreateDirEntryFrom(const s_dir_entry &dir_entry,
        const std::string &filename)
{
    FlexDirEntry dirEntry;

    dirEntry.SetDate(BDate(dir_entry.day, dir_entry.month, dir_entry.year));
    auto hour = static_cast<int>(dir_entry.hour & 0x7FU);
    dirEntry.SetTime(BTime(hour, dir_entry.minute, 0U));
    dirEntry.SetTotalFileName(flx::toupper(filename));
    dirEntry.SetAttributes(dir_entry.file_attr);
    dirEntry.SetSectorMap(dir_entry.sector_map);
    dirEntry.SetStartTrkSec(dir_entry.start.trk, dir_entry.start.sec);
    dirEntry.SetEndTrkSec(dir_entry.end.trk, dir_entry.end.sec);
    dirEntry.SetFileSize(
        flx::getValueBigEndian<Word>(&dir_entry.records[0]) * SECTOR_SIZE);
    dirEntry.SetSectorMap(dir_entry.sector_map);
    dirEntry.ClearEmpty();

    return dirEntry;
}

void FlexDisk::InitializeFilenames()
{
    if (!is_filenames_initialized)
    {
        s_dir_sector sectorBuffer{};
        auto trk_sec = first_dir_trk_sec;

        while (trk_sec != st_t{})
        {
            Byte idx = 0U;

            if (!ReadSector(reinterpret_cast<Byte *>(&sectorBuffer),
                        trk_sec.trk, trk_sec.sec))
            {
                std::stringstream stream;

                stream << trk_sec;
                throw FlexException(FERR_READING_TRKSEC, stream.str(),
                        GetPath());
            }

            for (const auto &dir_entry : sectorBuffer.dir_entries)
            {
                if (dir_entry.filename[0] == DE_EMPTY)
                {
                    break;
                }

                if (dir_entry.filename[0] != DE_DELETED)
                {
                    auto filename(flx::getstr<>(dir_entry.filename));
                    auto fileExtension(flx::getstr<>(dir_entry.file_ext));

                    filenames.emplace(
                                flx::tolower(filename += '.' + fileExtension),
                                s_dir_pos{trk_sec, idx});
                }

                ++idx;
            }

            trk_sec = sectorBuffer.next;
        }

        is_filenames_initialized = true;
    }
}

void FlexDisk::SetNextDirectoryPosition(const st_t &dirTS)
{
    next_dir_trk_sec = dirTS;
}

void FlexDisk::DeleteFromFilenames(const std::string &filename)
{
    if (!is_filenames_initialized)
    {
        InitializeFilenames();
    }

    auto iter = filenames.find(flx::tolower(filename));
    if (iter != filenames.end())
    {
        filenames.erase(iter->first);
    }
}

void FlexDisk::AddToFilenames(const std::string &filename,
        const s_dir_pos &dir_pos)
{
    if (!is_filenames_initialized)
    {
        InitializeFilenames();
    }

    filenames.insert(std::make_pair(flx::tolower(filename), dir_pos));
}

bool FlexDisk::FindInFilenames(const std::string &filename)
{
    if (!is_filenames_initialized)
    {
        InitializeFilenames();
    }

    return filenames.find(flx::tolower(filename)) != filenames.end();
}
