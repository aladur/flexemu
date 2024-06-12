/*
    ndircont.cpp


    FLEXplorer, An explorer for FLEX disk image files and directory disks.
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

#include <iterator>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <locale>
#include <cstring>
#include <vector>
#include <set>
#include <unordered_set>
#include <fstream>
#include <limits>
#include <cassert>
#include <stdio.h>
#include <sys/stat.h>
#include <ctype.h>
#include "bdir.h"
#include "filecntb.h"
#include "fattrib.h"
#include "ndircont.h"
#include "fdirent.h"
#include "fcinfo.h"
#include "flexerr.h"
#include "cvtwchar.h"
#include "fdoptman.h"
#include <fmt/format.h>


// A debug log can be written to a file
// by uncommenting the DEBUG_FILE macro definition.
// DEBUG_VERBOSE defines the verbosity:
// 1: Log any read/write access to a sector.
// 2: Same as 1, additionally log sector dump.
//#define DEBUG_FILE get_unique_filename("log").c_str()
#ifdef DEBUG_FILE
    #define DEBUG_VERBOSE 2
    #include "debug.h"
#endif

// nafs means NAtive File System. It allows to emulate a FLEX disk
// by simply using a directory on the host file system.
// The emulation fully supports read, modify, write, delete access for normal
// and random files. Only files, which are identified as FLEX compatible file
// names are emulated. All other files in the directory are ignored.
// See isFlexFilename for details. If the files in the emulated directory
// exceed the size of the emulated disk only part of the files are emulated.
// The size of the emulated disk is read from a file .flexdiskrc. If it does
// not exist the parameters tracks and sectors are used.
// IMPORTANT HINT: As long as a host file system is mounted as a FLEX
// disk any file in this directory should not be renamed, modified or deleted
// Care should also be taken when using low level disk access tools
// like EXAMINE.CMD which can be used to read, modify and write single sectors.
// It is possible to read or write sectors of existing files, the system info
// sector or boot sector.
// But writing sectors which are part of the free chain or a directory sector
// can corrupt the emulation.

FlexDirectoryDiskBySector::FlexDirectoryDiskBySector(
        const std::string &path,
        const FileTimeAccess &fileTimeAccess,
        int tracks,
        int sectors)
    : ft_access(fileTimeAccess)
{
    struct stat sbuf{};
    static Word number = 0U;

    static_assert(sizeof(s_sys_info_sector) == SECTOR_SIZE, "Wrong alignment");
    static_assert(sizeof(s_dir_sector) == SECTOR_SIZE, "Wrong alignment");

    if (stat(path.c_str(), &sbuf) != 0 || !S_ISDIR(sbuf.st_mode))
    {
        throw FlexException(FERR_UNABLE_TO_OPEN, path);
    }

    directory = path;
    if (directory.size() > 1 && endsWithPathSeparator(directory))
    {
        // Remove trailing PATHSEPARATOR character.
        directory.resize(directory.size() - 1);
    }

    if (access(directory.c_str(), W_OK) != 0)
    {
        attributes |= WRITE_PROTECT;
    }

    FlexDirectoryDiskOptions opts(directory);

    if (opts.Read())
    {
        tracks = opts.GetTracks();
        sectors = opts.GetSectors();
    }
    else
    {
        opts.SetTracks(tracks);
        opts.SetSectors(sectors);
        opts.Write(true);
    }

    mount(number, tracks, sectors);
    number++;
}

FlexDirectoryDiskBySector::~FlexDirectoryDiskBySector()
{
    // final cleanup: close if not already done
    try
    {
        close_new_files();
        directory.clear();
    }
    catch (...)
    {
        // ignore exceptions
        // exceptions in destructors cause much problems
        // usually the file should be closed already
    }
}

// Create a new directory disk in path directory.
// format parameter is ignored.
FlexDirectoryDiskBySector *FlexDirectoryDiskBySector::Create(
        const std::string &directory,
        const std::string &name,
        const FileTimeAccess &fileTimeAccess,
        int tracks,
        int sectors,
        int /* fmt = TYPE_DSK_DISKFILE */)
{
    struct stat sbuf{};

    if (stat(directory.c_str(), &sbuf) != 0 || !S_ISDIR(sbuf.st_mode))
    {
        throw FlexException(FERR_UNABLE_TO_CREATE, name);
    }

    auto path = directory;
    path += PATHSEPARATORSTRING;
    path += name;

    if (!BDirectory::Create(path, 0755))
    {
        throw FlexException(FERR_UNABLE_TO_CREATE, name);
    }

    return new FlexDirectoryDiskBySector(path, fileTimeAccess, tracks, sectors);
}

std::string FlexDirectoryDiskBySector::GetPath() const
{
    return directory;
}

bool FlexDirectoryDiskBySector::GetAttributes(
        FlexDiskAttributes &diskAttributes) const
{

    const auto &sis = flex_sys_info[0];
    std::string disk_name(getstr<>(sis.sir.disk_name));

    diskAttributes.SetDate(BDate(sis.sir.day, sis.sir.month, sis.sir.year));
    diskAttributes.SetTrackSector(sis.sir.last.trk + 1, sis.sir.last.sec);
    diskAttributes.SetFree(getValueBigEndian<Word>(&sis.sir.free[0]) *
                           param.byte_p_sector);
    diskAttributes.SetTotalSize((sis.sir.last.sec * (sis.sir.last.trk + 1)) *
                                param.byte_p_sector);
    diskAttributes.SetName(disk_name);
    diskAttributes.SetNumber(getValueBigEndian<Word>(&sis.sir.disk_number[0]));
    diskAttributes.SetPath(directory);
    diskAttributes.SetType(param.type);
    diskAttributes.SetAttributes(attributes);
    diskAttributes.SetIsWriteProtected(IsWriteProtected());
    diskAttributes.SetIsFlexFormat(true);

    return true;
}

int FlexDirectoryDiskBySector::GetBytesPerSector() const
{
    return param.byte_p_sector;
}

bool FlexDirectoryDiskBySector::IsWriteProtected() const
{
    return param.write_protect != 0;
}

bool FlexDirectoryDiskBySector::IsTrackValid(int track) const
{
    return (track >= 0 && track <= param.max_track);
}

bool FlexDirectoryDiskBySector::IsSectorValid(int track, int sector) const
{
    if (track > 0)
    {
        return (sector > 0 && sector <= param.max_sector);
    }

    if (track == 0)
    {
        return (sector > 0 && sector <= param.max_sector0);
    }

    return false;
}

bool FlexDirectoryDiskBySector::IsFlexFormat() const
{
    return true;
}

int FlexDirectoryDiskBySector::GetFlexDiskType() const
{
    return param.type;
}

///////////////////////////////////////////////////////
// private interface
///////////////////////////////////////////////////////

// Initialize the internal data structures.
void FlexDirectoryDiskBySector::initialize_header(bool is_write_protected,
                                                  int tracks, int sectors)
{
    param.offset = 0;
    param.write_protect = is_write_protected ? 1U : 0U;
    param.max_sector = static_cast<Word>(sectors);
    param.max_sector0 = getTrack0SectorCount(tracks, sectors);
    param.max_track = static_cast<Word>(tracks - 1);
    param.byte_p_sector = SECTOR_SIZE;
    param.byte_p_track0 = param.max_sector0 * SECTOR_SIZE;
    param.byte_p_track = param.max_sector * SECTOR_SIZE;
    param.type = TYPE_DIRECTORY | TYPE_DIRECTORY_BY_SECTOR;
}

// Initialize the FLEX directory sectors.
void FlexDirectoryDiskBySector::initialize_flex_directory()
{
    init_dir_sectors = static_cast<Word>(
                           param.max_sector0 + 1 - first_dir_trk_sec.sec);
    next_dir_idx = -1;

    flex_directory.clear();
    for (Word i = 0; i < init_dir_sectors; ++i)
    {
        flex_directory.emplace_back();
    }

    dir_extend = st_t{0, 0};
    Word i = first_dir_trk_sec.sec;

    for (auto &dir_sector : flex_directory)
    {
        bool is_last = (i == init_dir_sectors - 1 + first_dir_trk_sec.sec);

        dir_sector.next.trk = '\0';
        dir_sector.next.sec = is_last ? '\0' : static_cast<Byte>(i + 1);
        setValueBigEndian<Word>(dir_sector.record_nr, 0U);

        std::fill(std::begin(dir_sector.unused),
                  std::end(dir_sector.unused), '\0');

        for (auto &dir_entry : dir_sector.dir_entries)
        {
            dir_entry.filename[0] = DE_EMPTY;
        }

        ++i;
    }
}


// Return the unix filename for a given FLEX directory entry s_dir_entry.
// If directory entry is empty or deleted return an empty string.
// pfn is a pointer to the filename property in a FLEX directory entry
// of type s_dir_entry.
std::string FlexDirectoryDiskBySector::get_unix_filename(
        const s_dir_entry &dir_entry)
{
    if (dir_entry.filename[0] != DE_EMPTY &&
        dir_entry.filename[0] != DE_DELETED)
    {
        auto basename(tolower(getstr<>(dir_entry.filename)));
        auto extension(tolower(getstr<>(dir_entry.file_ext)));
        return basename + '.' + extension;
    }

    return {};
}

// Return unix filename for a given file_id.
// - New files: The file_id is < 0. It is named tmpXX where XX
//   is related to the file_id.
// - Existing files: The file_id is >= 0. It is used as an index into
//   the FLEX directory entries of type s_dir_entry.
std::string FlexDirectoryDiskBySector::get_unix_filename(SDWord file_id) const
{
    if (file_id < 0)
    {
        std::stringstream filename;

        filename << fmt::format("tmp{:02}", -1 - file_id);
        return filename.str();
    }

    const auto ds_idx = static_cast<Word>(file_id / DIRENTRIES);

    if (ds_idx < flex_directory.size())
    {
        const auto &directory_entry =
            flex_directory[ds_idx].dir_entries[file_id % DIRENTRIES];
        return get_unix_filename(directory_entry);
    }

    // file_id located beyond valid range of directory entries.
    throw FlexException(FERR_WRONG_PARAMETER);
}

// Return the record number (zero based) of a new file which first
// sector has index 'sec_idx' into FLEX link table.
Word FlexDirectoryDiskBySector::record_nr_of_new_file(SDWord new_file_id,
        SDWord sec_idx) const
{
    Word record_nr = 0;

    auto i = get_sector_index(new_files.at(new_file_id).first);

    while (i != sec_idx && i >= 0)
    {
        record_nr++;
        i = get_sector_index(flex_links[i].next);
    }

    return record_nr;
}

// Return the new_file_id of the already existing new file with given track
// and sector. If not found create another new file and return its new_file_id.
// A new_file_id is always < 0.
// Return 0 if the new file can not be opened.
SDWord FlexDirectoryDiskBySector::id_of_new_file(const st_t &track_sector)
{
    for (const auto &iter : new_files)
    {
        if (track_sector == iter.second.next)
        {
            return iter.first;
        }

        auto current_index = get_sector_index(track_sector);
        auto last_index = get_sector_index(iter.second.next);
        auto index = get_sector_index(iter.second.first);

        while (index != last_index)
        {
            if (index == current_index)
            {
                return iter.first;
            }
            index = get_sector_index(flex_links[index].next);
        }
    }

    // Not found in existing new files.
    // Create another new file.
    s_new_file new_file{ };
    SWord new_file_id;

    // Find a new file index which is not yet used.
    for (new_file_id = -1;
         new_files.find(new_file_id) != new_files.end(); --new_file_id)
    {
    }

    new_file.filename = get_unix_filename(new_file_id);
    new_file.first = track_sector;
    new_file.next = st_t{0, 0};

    new_files.emplace(new_file_id, new_file);

    return new_file_id;
}

std::string FlexDirectoryDiskBySector::get_path_of_file(SDWord file_id) const
{
    if (file_id < 0)
    {
        if (new_files.find(file_id) != new_files.end())
        {
            return directory + PATHSEPARATORSTRING +
                   new_files.at(file_id).filename;
        }
    }
    else
    {
        const auto ds_idx = static_cast<Word>(file_id / DIRENTRIES);

        if (ds_idx < flex_directory.size())
        {
            const auto &directory_entry =
                flex_directory[ds_idx].dir_entries[file_id % DIRENTRIES];
            return directory + PATHSEPARATORSTRING +
                   get_unix_filename(directory_entry);
        }
    }

    return {};
}


// Extend the FLEX directory by one directory sector.
// If it fails return false.
bool FlexDirectoryDiskBySector::extend_directory(SDWord sec_idx,
    const s_dir_sector &dir_sector)
{
    if (sec_idx < 0)
    {
        return false;
    }

    flex_links[sec_idx].f_record = static_cast<Word>(flex_directory.size());
    flex_links[sec_idx].file_id = std::numeric_limits<SDWord>::max();
    flex_links[sec_idx].type = SectorType::Directory;
    flex_directory.push_back(dir_sector);
    dir_extend = st_t{0, 0};// reset directory extend track/sector

    return true;
}


// Return the first sector and track of the file FLEX.SYS.
// In FLEX this is called to LINK the disk. It is needed
// to be able to boot from it.
// If not found return 0/0.
st_t FlexDirectoryDiskBySector::link_address() const
{
    st_t link { };

    for (const auto &dir_sector : flex_directory)
    {
        for (const auto &dir_entry : dir_sector.dir_entries)
        {
            if (!strncmp(dir_entry.filename, "FLEX\0\0\0\0",
                         FLEX_BASEFILENAME_LENGTH) &&
                !strncmp(dir_entry.file_ext, "SYS", FLEX_FILEEXT_LENGTH))
            {
                link = dir_entry.start;
                break;
            }
        }
    }

    return link;
}


// Return the index (zero based) of the first free directory entry.
// If directory is full extend it by one directory sector.
// If directory can't be extended return -1.
SDWord FlexDirectoryDiskBySector::next_free_dir_entry()
{
    ++next_dir_idx;

    if ((next_dir_idx / DIRENTRIES) <
            static_cast<SDWord>(flex_directory.size()))
    {
        return next_dir_idx;
    }

    // Extend directory by one sector.
    auto &sis = flex_sys_info[0];

    s_dir_sector dir_sector { };

    Word record_nr = static_cast<Word>(flex_directory.size()) -
                     init_dir_sectors + 1;
    setValueBigEndian<Word>(dir_sector.record_nr, record_nr);

    const auto track_sector = sis.sir.fc_start;
    const auto sec_idx = get_sector_index(track_sector);

    if (extend_directory(sec_idx, dir_sector))
    {
        auto ds_idx = static_cast<Word>(flex_directory.size());

        flex_directory[ds_idx - 2].next = track_sector;

        if (--sis.sir.free[1] == 0xff)
        {
            --sis.sir.free[0];
        }

        if (sis.sir.free[1] == '\0' && sis.sir.free[0] == '\0')
        {
            // No space left => no more free chain sectors available.
            sis.sir.fc_start = st_t{ };
            sis.sir.fc_end = st_t{ };
        }
        else
        {
            if (sis.sir.fc_start.sec == param.max_sector)
            {
                sis.sir.fc_start.sec = 1;
                sis.sir.fc_start.trk++;
            }
            else
            {
                ++sis.sir.fc_start.sec;
            }
        }

        return next_dir_idx;
    }

    return -1;
}


// Initialize the FLEX link table
void FlexDirectoryDiskBySector::initialize_flex_link_table()
{
    Word i;
    Word fc_start = param.max_sector; // Start index of free chain.
    constexpr Word first_dir_sec = first_dir_trk_sec.sec - 1U; // zero based
    const Word max_dir_sector = first_dir_sec - 1U + init_dir_sectors;

    const auto sum_sectors =
        static_cast<Word>((param.max_track + 1) * param.max_sector);
    struct s_link_table new_link{};
    flex_links.reserve(param.max_sector);

    for (i = 0; i < sum_sectors; ++i)
    {
        flex_links.emplace_back(new_link);
    }

    // On track 0 are all boot, system info and directory sectors
    for (i = 0; i < fc_start; i++)
    {
        auto &link = flex_links[i];

        link.next = st_t{0, 0};
        link.record_nr[0] = 0;
        link.record_nr[1] = 0;
        link.f_record = i < first_dir_sec ? 0 : i - first_dir_sec;
        link.file_id = std::numeric_limits<SDWord>::max();
        link.type =
            (i > max_dir_sector) ? SectorType::Unknown : SectorType::Directory;
        link.type = (i < first_dir_sec) ? SectorType::SystemInfo : link.type;
        link.type = (i < 2) ? SectorType::Boot : link.type;
    }

    // All other tracks are initialized as free chain.
    for (i = fc_start; i < static_cast<Word>(flex_links.size()); i++)
    {
        auto &link = flex_links[i];

        if (i == flex_links.size() - 1)
        {
            link.next = st_t{0, 0};
        }
        else
        {
            link.next.trk = static_cast<Byte>((i + 1) / param.max_sector);
            link.next.sec = static_cast<Byte>(((i + 1) % param.max_sector) + 1);
        }

        setValueBigEndian(&link.record_nr[0], 0U);
        link.f_record = 0;
        link.file_id = std::numeric_limits<SDWord>::max();
        link.type = SectorType::FreeChain;
    }

    Word free = static_cast<Word>(flex_links.size() - fc_start);

    // and now update system info sectors
    for (auto &sis : flex_sys_info)
    {
        sis.sir.fc_start.trk = static_cast<Byte>(fc_start / param.max_sector);
        sis.sir.fc_start.sec =
            static_cast<Byte>((fc_start % param.max_sector) + 1);
        sis.sir.fc_end.trk = static_cast<Byte>(param.max_track);
        sis.sir.fc_end.sec = static_cast<Byte>(param.max_sector);
        setValueBigEndian<Word>(sis.sir.free, free);
    }
}


// Check for any open new files.
// If so print a message or open a message dialog.
void FlexDirectoryDiskBySector::close_new_files()
{
    bool is_first = true;
    std::string msg;

    for (auto &iter : new_files)
    {
        if (is_first)
        {
            msg = "There are still temporary files stored as:\n";
            is_first = false;
        }

        msg += "   ";
        msg += iter.second.filename;
        msg += "\n";
    }
    new_files.clear();

    if (!msg.empty())
    {
#ifdef _WIN32
        std::string title(PROGRAMNAME " warning");
        MessageBox(nullptr, ConvertToUtf16String(msg).c_str(),
            ConvertToUtf16String(title).c_str(),
            MB_OK | MB_ICONEXCLAMATION);
#endif
#ifdef UNIX
        fprintf(stderr, "%s", msg.c_str());
#endif
    }
}


// Add a file with directory index dir_idx to the link table.
// If file won't fit return false otherwise return true.
// On success return its first and last track/sector.
bool FlexDirectoryDiskBySector::add_to_link_table(
    SDWord dir_idx,
    off_t size,
    bool is_random,
    st_t &begin,
    st_t &end)
{
    auto &sis = flex_sys_info[0];

    if (dir_idx < 0 ||
        (dir_idx / DIRENTRIES) >= static_cast<signed>(flex_directory.size()))
    {
        throw FlexException(FERR_WRONG_PARAMETER);
    }

    auto free = getValueBigEndian<Word>(&sis.sir.free[0]);

    if (size == 0U)
    {
        begin = {};
        end = {};
        return true;
    }

    if (size > free * DBPS)
    {
        return false;
    }

    auto records = static_cast<Word>((size + (DBPS - 1)) / DBPS);
    begin = sis.sir.fc_start;
    auto sec_idx_begin = get_sector_index(begin);

    for (Word i = 1; i <= records; ++i)
    {
        auto &link = flex_links[i + sec_idx_begin - 1];

        if (i == records)
        {
            link.next = st_t{0, 0};
        }

        if (is_random)
        {
            Word record_nr = i > 2 ? static_cast<Word>(i - 2) : 0U;
            setValueBigEndian<Word>(&link.record_nr[0], record_nr);
        }
        else
        {
            Word record_nr = i;
            setValueBigEndian<Word>(&link.record_nr[0], record_nr);
        }

        link.f_record = static_cast<Word>(i - 1);
        link.file_id = dir_idx;
        link.type = SectorType::File;
    }

    auto sec_idx_end = static_cast<SDWord>(sec_idx_begin + records - 1);
    end.sec = static_cast<Byte>((sec_idx_end % param.max_sector) + 1);
    end.trk = static_cast<Byte>(sec_idx_end / param.max_sector);
    // update sys info sector
    free -= records;
    setValueBigEndian<Word>(&sis.sir.free[0], free);
    if (free > 0U)
    {
        sis.sir.fc_start.sec =
            static_cast<Byte>(((sec_idx_end + 1) % param.max_sector) + 1);
        sis.sir.fc_start.trk =
            static_cast<Byte>((sec_idx_end + 1) / param.max_sector);
    }
    else
    {
        // No space left => no more free chain sectors available.
        sis.sir.fc_start = st_t{ };
        sis.sir.fc_end = st_t{ };
    }

    return true;
}


// Add file properties to directory entry with index 'dir_idx'.
void FlexDirectoryDiskBySector::add_to_directory(
    std::string name,
    std::string extension,
    SDWord dir_idx,
    bool is_random,
    const struct stat &stat,
    const st_t &begin,
    const st_t &end,
    bool is_write_protected)
{
    struct tm *lt;
    const bool setFileTime =
        (ft_access & FileTimeAccess::Set) == FileTimeAccess::Set;

    if (dir_idx < 0 ||
        (dir_idx / DIRENTRIES) >= static_cast<signed>(flex_directory.size()))
    {
        throw FlexException(FERR_WRONG_PARAMETER);
    }

    lt = localtime(&(stat.st_mtime));
    auto year = lt->tm_year > 100 ? lt->tm_year - 100 : lt->tm_year;
    auto records = static_cast<Word>((stat.st_size + (DBPS - 1)) / DBPS);
    auto &dir_entry =
      flex_directory[dir_idx / DIRENTRIES].dir_entries[dir_idx % DIRENTRIES];
    name.resize(FLEX_BASEFILENAME_LENGTH);
    std::copy_n(name.cbegin(), FLEX_BASEFILENAME_LENGTH,
                std::begin(dir_entry.filename));
    extension.resize(FLEX_FILEEXT_LENGTH);
    std::copy_n(extension.cbegin(), FLEX_FILEEXT_LENGTH,
                std::begin(dir_entry.file_ext));
    // A write protected file is automatically also delete protected.
    dir_entry.file_attr = is_write_protected ? WRITE_PROTECT : 0;
    dir_entry.start = begin;
    dir_entry.end = end;
    setValueBigEndian<Word>(&dir_entry.records[0], records);
    dir_entry.sector_map = is_random ? IS_RANDOM_FILE : 0;
    dir_entry.month = static_cast<Byte>(lt->tm_mon + 1);
    dir_entry.day = static_cast<Byte>(lt->tm_mday);
    dir_entry.year = static_cast<Byte>(year);
    dir_entry.hour = setFileTime ? static_cast<Byte>(lt->tm_hour) : 0U;
    dir_entry.minute = setFileTime ? static_cast<Byte>(lt->tm_min) : 0U;
}

// Update random file sector map.
void FlexDirectoryDiskBySector::modify_random_file(const char *path,
        const struct stat &stat, const st_t &begin)
{
    Byte file_sector_map[DBPS * 2];
    DWord data_size;
    Word i;
    Word n;

    data_size = stat.st_size - (DBPS * 2);

    if (data_size >= DBPS * 2)
    {
        auto sec_idx = get_sector_index(begin) + 2;

        std::fill(std::begin(file_sector_map), std::end(file_sector_map), '\0');

        for (n = 0; n < static_cast<Word>(data_size / (DBPS * 255)) ; n++)
        {
            file_sector_map[3 * n] =
                static_cast<Byte>(sec_idx / param.max_sector);
            file_sector_map[3 * n + 1] =
                static_cast<Byte>((sec_idx % param.max_sector) + 1);
            file_sector_map[3 * n + 2] = 255;
            sec_idx += 255;
        }

        i = static_cast<Word>(data_size % (DBPS * 255));

        if (i != 0)
        {
            file_sector_map[3 * n] =
                static_cast<Byte>(sec_idx / param.max_sector);
            file_sector_map[3 * n + 1] =
                static_cast<Byte>((sec_idx % param.max_sector) + 1);
            file_sector_map[3 * n + 2] =
                static_cast<Byte>((i + (DBPS - 1)) / DBPS);
        }

        std::fstream fs(path, std::ios::in | std::ios::out | std::ios::binary);
        if (fs.is_open())
        {
            fs.write(reinterpret_cast<const char *>(file_sector_map), 2 * DBPS);
        }
    }
}


// Create a directory entry for each file for which the file name
// is identified as a FLEX file name. See isFlexFilename for details.
// If is_write_protected is true the drive is write protected.
// The following criterion have to be met to add a file to the FLEX directory:
// - It is identified as a FLEX file name.
// - It is not empty.
// - If files just differ in case sensitivity only the first one is used.
//   (This can happen for case sensitive file systems only).
// - There is space left for the file itself and it's directory entry.
// - It's name is neither "random" nor "boot".
// Any other files are just ignored. The files "random" and "boot" have
// a special meaning in a directory container.
void FlexDirectoryDiskBySector::fill_flex_directory(bool is_write_protected)
{
    std::vector<std::string> filenames; // List of to be added files
    std::unordered_set<std::string> lc_filenames; // Compare lower case filen.
    std::unordered_set<std::string> random_filenames; // random files.
    struct stat sbuf{};

    auto add_file = [&](const std::string &filename, bool is_random)
    {
        auto lc_filename(tolower(filename));

        // CDFS-Support: look for file name in file 'random'
        if (is_write_protected)
        {
            is_random = isListedInFileRandom(directory, filename);
        }

        if (isFlexFilename(filename) &&
            strcmp(filename.c_str(), RANDOM_FILE_LIST) != 0 &&
            strcmp(filename.c_str(), BOOT_FILE) != 0 &&
            lc_filenames.find(lc_filename) == lc_filenames.end())
        {

            if (is_random)
            {
                random_filenames.emplace(filename);
            }

            filenames.emplace_back(filename);
            lc_filenames.emplace(lc_filename);
        }
    };

    memset(&sbuf, 0, sizeof(sbuf));
    initialize_flex_directory();
    initialize_flex_link_table();

#ifdef _WIN32
    WIN32_FIND_DATA pentry;
    const auto wWildcard(
        ConvertToUtf16String(directory + PATHSEPARATORSTRING "*.*"));

    auto hdl = FindFirstFile(wWildcard.c_str(), &pentry);

    if (hdl != INVALID_HANDLE_VALUE)
    {
        do
        {
            auto filename(tolower(ConvertToUtf8String(pentry.cFileName)));
            auto path = directory + PATHSEPARATORSTRING + filename.c_str();
            if (stat(path.c_str(), &sbuf) || !S_ISREG(sbuf.st_mode))
            {
                continue;
            }
            bool is_random = (pentry.dwFileAttributes &
                              FILE_ATTRIBUTE_HIDDEN) ? true : false;

            add_file(filename, is_random);
        }
        while (FindNextFile(hdl, &pentry) != 0);

        FindClose(hdl);
    }
#endif

#ifdef UNIX
    auto *pd = opendir(directory.c_str());
    if (pd != nullptr)
    {
        struct dirent *pentry;

        while ((pentry = readdir(pd)) != nullptr)
        {
            std::string path;

            std::string filename = pentry->d_name;
            path = directory + PATHSEPARATORSTRING + filename;
            if (stat(path.c_str(), &sbuf) || !S_ISREG(sbuf.st_mode))
            {
                continue;
            }
            bool is_random = ((sbuf.st_mode & S_IXUSR) != 0);

            add_file(filename, is_random);
        }

        closedir(pd);
    }
#endif

    // Sort all filenames before adding them to the container.
    std::sort(filenames.begin(), filenames.end());

    for (const auto &filename : filenames)
    {
        const auto dir_idx = next_free_dir_entry();
        if (dir_idx >= 0)
        {
            st_t begin;
            st_t end;
            auto path = directory + PATHSEPARATORSTRING + filename;
            bool is_random = (random_filenames.find(filename) !=
                              random_filenames.end());

            if (!stat(path.c_str(), &sbuf) &&
                add_to_link_table(dir_idx, sbuf.st_size, is_random, begin, end))
            {
                std::string name(getFileStem(filename));
                std::string extension(getFileExtension(filename).c_str() + 1);

                strupper(name);
                strupper(extension);
                add_to_directory(name, extension,
                                 dir_idx, is_random, sbuf, begin,
                                 end, (access(path.c_str(), W_OK) != 0));

                // Unfinished: don't write sector map if write
                // protected.
                if (is_random && !is_write_protected)
                {
                    modify_random_file(path.c_str(), sbuf, begin);
                }
            }
        }
        else
        {
            break;
        }
    }
}


// Initialize the FLEX system info sector.
void FlexDirectoryDiskBySector::initialize_flex_sys_info_sectors(Word number)
{
    struct stat sbuf{};

    if (!stat(directory.c_str(), &sbuf))
    {
        auto &sis = flex_sys_info[0];
        struct tm *lt = localtime(&(sbuf.st_mtime));

        std::string diskname = getFileName(directory);

        if (diskname.size() > FLEX_DISKNAME_LENGTH)
        {
            diskname.resize(FLEX_DISKNAME_LENGTH);
        }
        auto pos = diskname.find('.');
        std::string name{"FLEXDISK"};
        if (pos != std::string::npos && pos > 0)
        {
            name = diskname.substr(0, std::min(pos, FLEX_DISKNAME_LENGTH));
            strupper(name);
        }

        std::fill(std::begin(sis.unused1), std::end(sis.unused1), '\0');
        name.resize(FLEX_DISKNAME_LENGTH);
        std::copy_n(name.cbegin(), FLEX_DISKNAME_LENGTH,
                    std::begin(sis.sir.disk_name));
        std::fill(std::begin(sis.sir.disk_ext), std::end(sis.sir.disk_ext),
                  '\0');
        setValueBigEndian<Word>(&sis.sir.disk_number[0], number);
        sis.sir.fc_start = st_t{0, 0};
        sis.sir.fc_end = st_t{0, 0};
        setValueBigEndian<Word>(&sis.sir.free[0], 0U);
        sis.sir.month = static_cast<Byte>(lt->tm_mon + 1);
        sis.sir.day = static_cast<Byte>(lt->tm_mday);
        sis.sir.year = static_cast<Byte>(lt->tm_year);
        sis.sir.last = st_t{static_cast<Byte>(param.max_track),
                            static_cast<Byte>(param.max_sector)};
        std::fill(std::begin(sis.unused2), std::end(sis.unused2), '\0');

        flex_sys_info[1] = flex_sys_info[0];
    }
}


// Change the file id and type in the FLEX link table.
// The following transitions occur:
// SectorType::FreeChain -> SectorType::NewFile   Writing to a new file.
// SectorType::NewFile   -> SectorType::File      A directory entry for a new
//                                                file has been created.
// SectorType::File      -> SectorType::FreeChain A file has been deleted.
void FlexDirectoryDiskBySector::change_file_id_and_type(SDWord sec_idx,
        SDWord old_file_id, SDWord new_file_id, SectorType new_type)
{
    std::set<decltype(sec_idx)> usedIndices;

    while (sec_idx >= 0 && flex_links[sec_idx].file_id == old_file_id)
    {
        flex_links[sec_idx].file_id = new_file_id;
        flex_links[sec_idx].type = new_type;
        sec_idx = get_sector_index(flex_links[sec_idx].next);
        // Remember all indices already used and break loop if an index
        // appears a second time. This protects from endless loops based on
        // wrong sector links.
        if (usedIndices.find(sec_idx) != usedIndices.end())
        {
            break;
        }
        usedIndices.insert(sec_idx);
    }
}


void FlexDirectoryDiskBySector::update_sector_buffer_from_link(Byte *buffer,
        const s_link_table &link)
{
    buffer[0] = link.next.trk;
    buffer[1] = link.next.sec;
    buffer[2] = link.record_nr[0];
    buffer[3] = link.record_nr[1];
}


void FlexDirectoryDiskBySector::update_link_from_sector_buffer(s_link_table &link,
        const Byte *buffer)
{
     link.next.trk = buffer[0];
     link.next.sec = buffer[1];
     link.record_nr[0] = buffer[2];
     link.record_nr[1] = buffer[3];
}


// Check if a file has been deleted. A file is marked as deleted
// if the first byte of the file name is set to DE_DELETED.
// For this compare the old directory sector (old_dir_sector) with the new
// directory sector (dir_sector).
void FlexDirectoryDiskBySector::check_for_delete(Word ds_idx,
        const s_dir_sector &dir_sector)
{
    const auto &old_dir_sector = flex_directory[ds_idx];

    const auto dir_idx0 = static_cast<SDWord>(ds_idx * DIRENTRIES);
    for (Word i = 0; i < DIRENTRIES; i++)
    {
        if (dir_sector.dir_entries[i].filename[0] == DE_DELETED &&
            old_dir_sector.dir_entries[i].filename[0] != DE_DELETED)
        {
            const auto dir_idx = dir_idx0 + i;
            auto filename = get_unix_filename(dir_idx);
            auto track_sector = old_dir_sector.dir_entries[i].start;
            auto sec_idx = get_sector_index(track_sector);
            auto path = directory + PATHSEPARATORSTRING + filename;
            unlink(path.c_str());
            change_file_id_and_type(sec_idx, dir_idx, 0, SectorType::FreeChain);
#ifdef DEBUG_FILE
            LOG_X("      delete {}\n", filename);
#endif
            break;
        }
    }
}


// Check if a file has been renamed.
// For this compare the filename of the old directory sector (old_filename)
// with the corresponding filename in new directory sector (new_filename).
// If a file has been renamed then rename it on the host file system too.
void FlexDirectoryDiskBySector::check_for_rename(Word ds_idx,
        const s_dir_sector &dir_sector) const
{
    std::string old_filename;
    std::string new_filename;

    const auto dir_idx0 = static_cast<SDWord>(ds_idx * DIRENTRIES);
    for (Word i = 0; i < DIRENTRIES; i++)
    {
        const auto dir_idx = dir_idx0 + i;
        old_filename = get_unix_filename(dir_idx);
        new_filename = get_unix_filename(dir_sector.dir_entries[i]);

        if (!old_filename.empty() && !new_filename.empty() &&
            strcmp(old_filename.c_str(), new_filename.c_str()) != 0)
        {
            auto old_path = directory + PATHSEPARATORSTRING + old_filename;
            auto new_path = directory + PATHSEPARATORSTRING + new_filename;
            rename(old_path.c_str(), new_path.c_str());
#ifdef DEBUG_FILE
            LOG_XX("      rename {} to {}\n", old_filename, new_filename);
#endif
            break;
        }
    }
}


// Check if the directory has been extended.
// For this compare the old (old_dir_sector) with the new directory sector
// (dir_sector).
// If the directoy has been extended then the old directory sector had no
// link to a next directory sector (next_trk == next_sec == 0) AND the
// new directory sector has a link to the next directory sector. If so
// save track and sector in dir_extend.
void FlexDirectoryDiskBySector::check_for_extend(Word ds_idx,
        const s_dir_sector &dir_sector)
{
    const auto &old_dir_sector = flex_directory[ds_idx];

    if (old_dir_sector.next == st_t{0, 0} && (dir_sector.next != st_t{0, 0}))
    {
        dir_extend = dir_sector.next;
    }
}


// Check if file attributes have been changed. Only write protection is
// supported.
// File attributes is located in struct s_dir_entry in byte file_attr.
void FlexDirectoryDiskBySector::check_for_changed_file_attr(Word ds_idx,
        s_dir_sector &dir_sector)
{
    static const auto unsupported_attr =
        DELETE_PROTECT | CATALOG_PROTECT | READ_PROTECT;
    const auto &old_dir_sector = flex_directory[ds_idx];

    const auto dir_idx0 = static_cast<SDWord>(ds_idx * DIRENTRIES);
    for (Word i = 0; i < DIRENTRIES; i++)
    {
        if ((dir_sector.dir_entries[i].file_attr & unsupported_attr) != 0)
        {
            // Remove all unsupported file attributes.
            dir_sector.dir_entries[i].file_attr &= ~unsupported_attr;
        }

        if (attributes & WRITE_PROTECT)
        {
            // If disk is write protected the file write protection can not
            // be removed.
            dir_sector.dir_entries[i].file_attr |= WRITE_PROTECT;
            continue;
        }

        if ((dir_sector.dir_entries[i].file_attr & WRITE_PROTECT) !=
            (old_dir_sector.dir_entries[i].file_attr & WRITE_PROTECT))
        {
            const auto dir_idx = dir_idx0 + i;
            auto filename = get_unix_filename(dir_idx);
            auto file_attr = dir_sector.dir_entries[i].file_attr;
            const char *set_clear = nullptr;
#ifdef _WIN32
            const auto wFilePath(ConvertToUtf16String(directory +
                PATHSEPARATORSTRING + filename));
            DWORD attrs = GetFileAttributes(wFilePath.c_str());

            if (file_attr & WRITE_PROTECT)
            {
                attrs |= FILE_ATTRIBUTE_READONLY;
                set_clear = "set";
            }
            else
            {
                attrs &= ~FILE_ATTRIBUTE_READONLY;
                set_clear = "clear";
            }

            SetFileAttributes(wFilePath.c_str(), attrs);
#endif
#ifdef UNIX
            const auto path = directory + PATHSEPARATORSTRING + filename;
            struct stat sbuf{};
            if (!stat(path.c_str(), &sbuf))
            {
                if (file_attr & WRITE_PROTECT)
                {
                    chmod(path.c_str(), sbuf.st_mode & ~S_IWUSR);
                    set_clear = "set";
                }
                else
                {
                    chmod(path.c_str(), sbuf.st_mode | S_IWUSR);
                    set_clear = "clear";
                }
            }
#endif
#ifdef DEBUG_FILE
            LOG_XX("      {} write_protect {}\n", set_clear, filename);
#else
            (void)set_clear;
#endif
            break;
        }
    }
}


// Return false if not successful otherwise return true.
// years < 75 will be represented as >= 2000
bool FlexDirectoryDiskBySector::set_file_time(const char *ppath, Byte month,
        Byte day, Byte year, Byte hour, Byte minute) const
{
    struct stat statbuf{};
    struct utimbuf timebuf{};
    struct tm file_time{};
    const bool setFileTime =
        (ft_access & FileTimeAccess::Set) == FileTimeAccess::Set;

    if (stat(ppath, &statbuf) >= 0)
    {
        timebuf.actime = statbuf.st_atime;
        file_time.tm_sec = 0;
        file_time.tm_min = setFileTime ? minute : 0;
        file_time.tm_hour = setFileTime ? hour : 12;
        file_time.tm_mon = month - 1;
        file_time.tm_mday = day;
        file_time.tm_year = year < 75 ? year + 100 : year;
        file_time.tm_isdst = 0;
        timebuf.modtime = mktime(&file_time);

        if (timebuf.modtime >= 0 && utime(ppath, &timebuf) >= 0)
        {
            return true;
        }
    }

    return false;
}

// Set back the file time to the date in the emulated file system.
bool FlexDirectoryDiskBySector::update_file_time(const char *path,
                                                 SDWord file_id) const
{
    if (file_id >= 0)
    {
        const auto ds_idx = static_cast<Word>(file_id / DIRENTRIES);

        if (ds_idx < flex_directory.size())
        {
            const auto &directory_entry =
                flex_directory[ds_idx].dir_entries[file_id % DIRENTRIES];
            return set_file_time(path,
                directory_entry.month,
                directory_entry.day,
                directory_entry.year,
                directory_entry.hour,
                directory_entry.minute);
        }
    }

    return true;
}


// Check for new directory entries. A new directory entry is identified
// by the first byte of the file name set to neither DE_DELETED nor DE_EMPTY.
// For this compare the old (pd) with the new directory sector (dir_sector).
// If found remove the file from the list of new files and rename the
// file to its new file name.
void FlexDirectoryDiskBySector::check_for_new_file(Word ds_idx,
        const s_dir_sector &dir_sector)
{
    std::vector<SDWord> keys;

    const auto dir_idx0 = static_cast<SDWord>(ds_idx * DIRENTRIES);
    for (Word i = 0; i < DIRENTRIES; i++)
    {
        if (
           dir_sector.dir_entries[i].filename[0] != DE_EMPTY &&
           dir_sector.dir_entries[i].filename[0] != DE_DELETED &&
           (dir_sector.dir_entries[i].start != st_t{0, 0}))
        {
            for (const auto &iter : new_files)
            {
#ifdef UNIX
                struct stat sbuf{};
#endif

                if (iter.second.first != dir_sector.dir_entries[i].start)
                {
                    continue;
                }

                keys.push_back(iter.first);
                const auto dir_idx = dir_idx0 + i;
                auto sec_idx = get_sector_index(iter.second.first);
                change_file_id_and_type(sec_idx, iter.first,
                                        dir_idx, SectorType::File);

                auto old_path =
                    directory + PATHSEPARATORSTRING + iter.second.filename;

                // check for random file, if true set user execute bit
                if (dir_sector.dir_entries[i].sector_map & IS_RANDOM_FILE)
                {
#ifdef _WIN32
                    SetFileAttributes(ConvertToUtf16String(old_path).c_str(),
                        FILE_ATTRIBUTE_HIDDEN);
#endif
#ifdef UNIX
                    if (!stat(old_path.c_str(), &sbuf))
                    {
                        chmod(old_path.c_str(), sbuf.st_mode | S_IXUSR);
                    }
#endif
                }
                auto new_path = directory + PATHSEPARATORSTRING +
                                get_unix_filename(flex_links[sec_idx].file_id);
                rename(old_path.c_str(), new_path.c_str());
#ifdef DEBUG_FILE
                LOG_XX("      new file {}, was {}\n",
                       get_unix_filename(flex_links[sec_idx].file_id),
                       iter.second.filename);
#endif
                set_file_time(new_path.c_str(),
                    dir_sector.dir_entries[i].month,
                    dir_sector.dir_entries[i].day,
                    dir_sector.dir_entries[i].year,
                    dir_sector.dir_entries[i].hour,
                    dir_sector.dir_entries[i].minute);
            }
        }
    }

    // Remove all new files for which a directory entry has been created.
    for (auto key : keys)
    {
        new_files.erase(key);
    }
}

// Check if track and sector is the last sector in the free chain.
bool FlexDirectoryDiskBySector::is_last_of_free_chain(
                             const st_t &track_sector) const
{
    return flex_sys_info[0].sir.fc_end == track_sector;
}


// Public interface to read one sector contained in byte stream 'buffer'
// from given track and sector.
// Return true on success.
bool FlexDirectoryDiskBySector::ReadSector(Byte *buffer, int trk, int sec,
        int /* side = -1 */) const
{
    st_t track_sector{static_cast<Byte>(trk), static_cast<Byte>(sec)};
    auto sec_idx = get_sector_index(track_sector);
    const auto &link = flex_links[sec_idx];
    bool result = true;

    if (!IsTrackValid(trk) || !IsSectorValid(trk, sec))
    {
        result = false;
    }

#ifdef DEBUG_FILE
    LOG_XXX("read: {:02X}-{:02X} {}", trk, sec, to_string(link.type));
    if (link.type == SectorType::File || link.type == SectorType::NewFile)
    {
        LOG_X(" {}", get_unix_filename(link.file_id));
    }
    if (!result)
    {
        LOG(". *** Invalid track or sector.");
    }
    LOG("\n");
#endif

    if (!result)
    {
        return result;
    }

    switch (link.type)
    {
        case SectorType::Unknown:
            memset(buffer, '\0', SECTOR_SIZE);
            break;

        case SectorType::SystemInfo:
            {
                memcpy(buffer, &flex_sys_info[sec - 3], SECTOR_SIZE);
            }
            break;

        case SectorType::Boot:
            {
                // According to the FLEX Advanced Programmer's Guide
                // chapter "Diskette Initialization" the first two
                // sectors contain a boot program.
                // Reading the boot sector 00-01 or 00-02
                // from a file which name is defined in BOOT_FILE.
                // If this file has a size of SECTOR_SIZE reading
                // sector 00-02 contains all zeros.
                // The boot code is contained in sector 00-01 and
                // optionally in sector 00-02.
                // The emulated Eurocom II usually only uses sector 00-01.
                // If a boot file is not present or a failure occurs
                // when reading it a default boot code is set which
                // savely jumps back into the monitor program.

                auto path = directory + PATHSEPARATORSTRING BOOT_FILE;
                bool set_default_boot_code = true;
                std::ifstream ifs(path, std::ios::in | std::ios::binary);

                std::memset(buffer, 0, SECTOR_SIZE);
                if (ifs.is_open())
                {
                    set_default_boot_code = false;
                    ifs.seekg(SECTOR_SIZE * (sec - 1));
                    ifs.read(reinterpret_cast<char *>(buffer), SECTOR_SIZE);
                    if (!ifs.fail() && sec == 1)
                    {
                        st_t boot_link = link_address();

                        buffer[3] = boot_link.trk;
                        buffer[4] = boot_link.sec;
                    }
                    if (ifs.fail() && sec == 1)
                    {
                        set_default_boot_code = true;
                    }
                    ifs.close();
                }
                if (set_default_boot_code)
                {
                    // Default buffer content if no boot sector present.
                    // Jump to monitor program warm start entry point.
                    buffer[0] = 0x7E; // JMP $F02D
                    buffer[1] = 0xF0;
                    buffer[2] = 0x2D;
                }
            }
            break;

        case SectorType::Directory:
            {
                Word di = link.f_record;
                memcpy(buffer, &flex_directory[di], SECTOR_SIZE);
            }
            break;

        case SectorType::FreeChain:
            update_sector_buffer_from_link(buffer, link);

            // free chain sector reads always
            // filled with zero
            std::memset(buffer + MDPS, 0, DBPS);
            break;

        case SectorType::NewFile: // new file with temporary name tmpXX
        case SectorType::File: // Read from an existing file.
            {
                auto path = get_path_of_file(link.file_id);

                result = false;
                std::ifstream ifs(path, std::ios::in | std::ios::binary);
                if (ifs.is_open())
                {
                    ifs.seekg(link.f_record * DBPS);
                    if (!ifs.fail())
                    {
                        auto *p = reinterpret_cast<char *>(buffer + MDPS);
                        ifs.read(p, DBPS);
                        auto bytes = ifs.gcount();

                        // Pad remaining bytes of sector of a file with 0.
                        // A number of bytes read of 0 is also valid.
                        if (bytes < DBPS)
                        {
                            std::memset(buffer + MDPS + bytes, 0, DBPS - bytes);
                        }
                        result = true;
                    }
                    ifs.close();

                    if (link.type == SectorType::File)
                    {
                        // The host file system changes the modification time.
                        // Set it back to the time of the emulated file system.
                        update_file_time(path.c_str(), link.file_id);
                    }
                }

                update_sector_buffer_from_link(buffer, link);
            }
            break;
    }

#if (defined DEBUG_FILE && defined DEBUG_VERBOSE && DEBUG_VERBOSE >= 2)
    std::ofstream log_ofs(DEBUG_FILE, std::ios::out | std::ios::app);
    if (log_ofs.is_open())
    {
        dumpSector(log_ofs, 6U, buffer, SECTOR_SIZE);
        log_ofs.close();
    }
#endif

    return result;
}

// Public interface to write one sector contained in byte stream 'buffer'
// to given track and sector.
bool FlexDirectoryDiskBySector::WriteSector(const Byte *buffer, int trk,
        int sec, int /* side = -1 */)
{
    bool result = true;
    st_t track_sector{static_cast<Byte>(trk), static_cast<Byte>(sec)};
    auto sec_idx = get_sector_index(track_sector);
    auto &link = flex_links[sec_idx];

    if (!IsTrackValid(trk) || !IsSectorValid(trk, sec))
    {
        result = false;
    }

#ifdef DEBUG_FILE
    LOG_XXX("write: {:02X}-{:02X} {}", trk, sec, to_string(link.type));
    if (link.type == SectorType::File || link.type == SectorType::NewFile)
    {
        LOG_X(" {}", get_unix_filename(link.file_id));
    }
    if (!result)
    {
        LOG(". *** Invalid track or sector.");
    }
    LOG("\n");
#endif

    if (!result)
    {
        return result;
    }

    switch (link.type)
    {
        case SectorType::Unknown:
            break;

        case SectorType::SystemInfo:
            {
                memcpy(&flex_sys_info[sec - 3], buffer, SECTOR_SIZE);
            }
            break;

        case SectorType::Boot:
            {
                // Write boot sector 00-01 or 00-02.
                // into a file which name is defined in BOOT_FILE.
                std::array<Byte, 2 * SECTOR_SIZE> boot_buffer{};
                struct stat sbuf{};

                auto path = directory + PATHSEPARATORSTRING BOOT_FILE;
                std::fill(boot_buffer.begin(), boot_buffer.end(), '\0');

                if (!stat(path.c_str(), &sbuf) && S_ISREG(sbuf.st_mode))
                {
                    std::ifstream ifs(path, std::ios::in | std::ios::binary);
                    if (ifs.is_open())
                    {
                        auto *p = reinterpret_cast<char *>(boot_buffer.data());
                        ifs.read(p, SECTOR_SIZE * 2);
                        ifs.close();
                    }
                }

                Byte *p = boot_buffer.data() + (SECTOR_SIZE * (sec - 1));
                memcpy(p, buffer, SECTOR_SIZE);
                // Check if 2nd sector contains all zeros.
                bool is_all_zero = std::all_of(
                        boot_buffer.cbegin() + SECTOR_SIZE,
                        boot_buffer.cend(),
                        [](Byte b){ return b == 0; });
                // Remove link address.
                setValueBigEndian<Word>(&boot_buffer[3], 0U);
                // If sector 2 contains all zero bytes only write
                // the first sector otherwise write first and second
                // sector to file.
                auto count = !is_all_zero ? 2 : 1;

                std::ofstream ofs(path, std::ios::out | std::ios::binary);
                if (ofs.is_open())
                {
                    const auto pos = ofs.tellp();
                    const auto *p_out =
                        reinterpret_cast<const char *>(boot_buffer.data());
                    ofs.write(p_out, SECTOR_SIZE * count);
                    auto size = ofs.tellp() - pos;
                    if (ofs.fail() || size != SECTOR_SIZE * count)
                    {
                        result = false;
                    }
                    ofs.close();
                }
            }
            break;

        case SectorType::Directory:
            {
                const auto ds_idx = link.f_record;
                s_dir_sector dir_sector{};

                // Temporarily copy new directory sector.
                memcpy(&dir_sector, buffer, SECTOR_SIZE);
                check_for_delete(ds_idx, dir_sector);
                check_for_new_file(ds_idx, dir_sector);
                check_for_rename(ds_idx, dir_sector);
                check_for_extend(ds_idx, dir_sector);
                check_for_changed_file_attr(ds_idx, dir_sector);
                memcpy(&flex_directory[ds_idx], &dir_sector, SECTOR_SIZE);
            }
            break;

        case SectorType::FreeChain:

            if (dir_extend == track_sector)
            {
                s_dir_sector dir_sector{};

                memcpy(&dir_sector, buffer, SECTOR_SIZE);
                extend_directory(sec_idx, dir_sector);
#ifdef DEBUG_FILE
                LOG("      extend directory\n");
#endif
                break;
            }

            if (is_last_of_free_chain(track_sector) && (buffer[1] || buffer[0]))
            {
                // A file has been deleted. It's sectors are added to the end
                // of the free chain.
                update_link_from_sector_buffer(link, buffer);
#ifdef DEBUG_FILE
                LOG("      file deleted\n");
#endif
                break;
            }

            {
                auto new_file_id = id_of_new_file(track_sector);
                if (new_file_id == 0)
                {
#ifdef DEBUG_FILE
                    LOG("   ** error: unable to create new file\n");
#endif
                    result = false; // no more new files can be created
                    break;
                }

#ifdef DEBUG_FILE
                LOG_X("      file {}\n", new_files.at(new_file_id).filename);
#endif
                link.file_id = new_file_id;
                auto path = get_path_of_file(link.file_id);
                // Create an empty new file.
                std::ofstream ofs(path, std::ios::out | std::ios::binary);
                if (ofs.is_open())
                {
                    ofs.close();
                }
            }
            FALLTHROUGH;

        case SectorType::NewFile:
        case SectorType::File:
            {
                auto path = get_path_of_file(link.file_id);

                update_link_from_sector_buffer(link, buffer);
                if (link.file_id < 0)
                {
                    st_t next;

                    next.trk = buffer[0];
                    next.sec = buffer[1];
                    new_files.at(link.file_id).next.trk = next.trk;
                    new_files.at(link.file_id).next.sec = next.sec;
                    link.type = SectorType::NewFile;
                    link.f_record =
                        record_nr_of_new_file(link.file_id, sec_idx);
                    if (next.sec != 0 || next.trk != 0)
                    {
                        // If there is a link to the next track/sector also
                        // type it as a new file sector.
                        auto next_index = get_sector_index(next);
                        auto &next_link = flex_links[next_index];
                        next_link.type = SectorType::NewFile;
                        next_link.file_id = link.file_id;
                        next_link.f_record =
                           record_nr_of_new_file(next_link.file_id, next_index);
                    }
                }

                result = false;
                // Even if the file is only written to it has to be opened
                // for read/write otherwise random files do not work as
                // expected.
                std::fstream fs(path, std::ios::in | std::ios::out |
                                std::ios::binary);
                if (fs.is_open())
                {
                    fs.seekp(link.f_record * DBPS);
                    if (!fs.fail())
                    {
                        auto pos = fs.tellp();
                        const auto *p =
                            reinterpret_cast<const char *>(buffer + MDPS);
                        fs.write(p, DBPS);
                        if (!fs.fail() && fs.tellp() - pos == DBPS)
                        {
                            result = true;
                        }
                    }

                    fs.close();

                    if (link.type == SectorType::File)
                    {
                        // The host file system changes the modification time.
                        // Set it back to the time of the emulated file system.
                        update_file_time(path.c_str(), link.file_id);
                    }
                }
            }
            break;
    }

#if (defined DEBUG_FILE && defined DEBUG_VERBOSE && DEBUG_VERBOSE >= 2)
    std::ofstream log_ofs(DEBUG_FILE, std::ios::out | std::ios::app);
    if (log_ofs.is_open())
    {
        dumpSector(log_ofs, 6U, buffer, SECTOR_SIZE);
        log_ofs.close();
    }
#endif

    return result;
}


bool FlexDirectoryDiskBySector::FormatSector(
        const Byte * /*buffer*/,
        int /*trk*/,
        int /*sec*/,
        int /*side*/,
        int /*sizecode*/)
{
    // Handling unformated disks is not supported by this container.
    return false;
}

// Mount the directory container. number is the disk number.
void FlexDirectoryDiskBySector::mount(Word number, int tracks, int sectors)
{
    bool is_write_protected = (access(directory.c_str(), W_OK) != 0);
    initialize_header(is_write_protected, tracks, sectors);
    initialize_flex_sys_info_sectors(number);
    fill_flex_directory(is_write_protected);
}

std::string FlexDirectoryDiskBySector::to_string(SectorType type)
{
    switch (type)
    {
        case SectorType::Unknown:
            return "unknown sector";
        case SectorType::Boot:
            return "boot sector";
        case SectorType::SystemInfo:
            return "system info sector";
        case SectorType::Directory:
            return "directory sector";
        case SectorType::FreeChain:
            return "free chain";
        case SectorType::File:
            return "sector of file";
        case SectorType::NewFile:
            return "sector of new file";
    }

    return {};
}

// Return a unique filename for this directory container.
// The file extension is specified as parameter.
std::string FlexDirectoryDiskBySector::get_unique_filename(
                                    const char *extension) const
{
    auto number = getValueBigEndian<Word>(flex_sys_info[0].sir.disk_number);
    std::string diskname = getFileName(directory);
    if (diskname[0] == '.')
    {
        diskname = "flexdisk";
    }
    diskname += '_' + std::to_string(number) + "." + extension;

    return diskname;
}

// Return the sector index of the given track/sector.
// For 00-00 return -1.
SDWord FlexDirectoryDiskBySector::get_sector_index(const st_t &track_sector) const
{
    return track_sector.trk * param.max_sector + track_sector.sec - 1;
}

