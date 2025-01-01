/*
    ndircont.h


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

#ifndef NDIRCONT_INCLUDED
#define NDIRCONT_INCLUDED


#include "misc1.h"

#include "efiletim.h"
#include "filecntb.h"
#include "filecnts.h"
#include "rndcheck.h"
#include "flexemu.h"
#include <string>
#include <vector>
#include <array>
#include <unordered_map>

// class FlexDirectoryDiskBySector implements a sector oriented access
// to a FLEX disk by mapping a host directory emulating a FLEX disk.
//
// Rename: NafsDirectoryContainer => FlexDirectoryDiskBySector.
class FlexDirectoryDiskBySector : public IFlexDiskBySector
{
    // Common used parameter names and types:
    //
    // SDWord sec_idx     Index into flex_links (if < 0 it is invalid e.g.
    //                    end of free chain).
    // Word   ds_idx      Directory sector index, index into flex_directory.
    // SDWord dir_idx     Index to directory entry (s_dir_entry),
    //                    for existing files can be used as file_id,
    //                    (if < 0 it is invalid, e.g. directory/disk full).
    // SDWord file_id     Unique id of a file. For existing files can be used
    //                    as dir_idx. Ids of new files are < 0.
    // SDWord new_file_id Id of a new file. always < 0. Used as key in
    //                    new_files.

    enum class SectorType : uint8_t
    {
        Unknown, // Unknown sector type.
        Boot, // Boot sectors.
        SystemInfo, // System info sectors.
        Directory, // Directory sectors.
        FreeChain, // Sectors in free chain (unused sectors).
        File, // File sectors.
        NewFile, // New file sectors without directory entry.
    };

    // To emulate a FLEX disk meta data for each sector is stored in
    // the following structure.
    // file_id:
    // - New files: The file_id is < 0. It is named tmpXX where XX
    //   is related to the file_id.
    // - Existing files: The file_id is >= 0. It is used as an index into
    //   the FLEX directory entry of type s_dir_entry.
    // - It is only used for SectorType::File and SectorType::NewFile.
    // Fields next and record_nr are only used for sector type:
    //   SectorType::File, SectorType::NewFile and SectorType::FreeChain.
    // Field f_record is only used for sector type:
    //   SectorType::File, SectorType::NewFile, SectorType::Directory.
    struct s_link_table
    {
        st_t next; // Track and sector number of next sector
        std::array<Byte, 2> record_nr; // FLEX logical record number
        Word f_record; // Relative position in file / 252
        SDWord file_id;
        SectorType type; // The sector type.
    };

    // A new file is a newly created file which not yet has an entry in
    // a directory sector (s_dir_sector), so the name of this file is unknown.
    // As soon as a new directory entry (s_dir_entry) is created for it, it is
    // removed from the list of new files (new_files) and the file is renamed
    // on the host file system.
    struct s_new_file
    {
        std::string filename;
        st_t first; /* track and sector of first first first sector */
        st_t next; /* track and sector of next sector to be written */
    };

public:
    FlexDirectoryDiskBySector() = delete;
    FlexDirectoryDiskBySector(const FlexDirectoryDiskBySector &) = delete;
    FlexDirectoryDiskBySector(FlexDirectoryDiskBySector &&) = delete;
    FlexDirectoryDiskBySector(const std::string &path,
                           const FileTimeAccess &fileTimeAccess,
                           int tracks, int sectors);
    ~FlexDirectoryDiskBySector() override;

    FlexDirectoryDiskBySector &operator=(const FlexDirectoryDiskBySector &) = delete;
    FlexDirectoryDiskBySector &operator=(FlexDirectoryDiskBySector &&) = delete;

private:
    std::string directory;
    RandomFileCheck randomFileCheck;
    Byte attributes{};
    const FileTimeAccess &ft_access{};
    s_floppy param{};

    // Some structures needed for a FLEX file system
    // link table: Each sector has an entry in the link table.
    std::vector<s_link_table> flex_links;
    std::array<s_sys_info_sector, 2> flex_sys_info{}; // system info sectors
    std::vector<s_dir_sector> flex_directory; // directory sectors
    std::unordered_map<SDWord, s_new_file> new_files; // new file table
    std::unordered_map<SDWord, SectorMap_t> sector_maps; // random file
                                                         // sector maps
    st_t dir_extend{0U, 0U}; // track and sector of directory extend sector
    Word init_dir_sectors{}; // initial number of directory sectors
                             // without directory extension.
    SDWord next_dir_idx{-1}; // Next directory index used when filling up
                             // directory with file entries.

public:
    static FlexDirectoryDiskBySector *Create(const std::string &path,
            const FileTimeAccess &fileTimeAccess,
            int tracks, int sectors, DiskType disk_type);

    // IFlexDiskBase interface declaration.
    bool IsWriteProtected() const override;
    bool GetDiskAttributes(FlexDiskAttributes &diskAttributes) const override;
    DiskType GetFlexDiskType() const override;
    DiskOptions GetFlexDiskOptions() const override;
    std::string GetPath() const override;

    // IFlexDiskBySector interface declaration.
    bool ReadSector(Byte *buffer, int trk, int sec,
                    int side = -1) const override;
    bool WriteSector(const Byte *buffer, int trk, int sec,
                     int side = -1) override;
    bool FormatSector(const Byte *buffer, int trk, int sec, int side,
                      unsigned sizecode) override;
    bool IsFlexFormat() const override;
    bool IsTrackValid(int track) const override;
    bool IsSectorValid(int track, int sector) const override;
    unsigned GetBytesPerSector() const override;

private:
    void fill_flex_directory();
    void initialize_header(int tracks, int sectors);
    void initialize_flex_sys_info_sectors(Word number);
    void initialize_flex_directory();
    void initialize_flex_link_table();
    void close_new_files();
    void mount(Word number, int tracks, int sectors);
    SDWord next_free_dir_entry();
    std::string get_unix_filename(SDWord file_id) const;
    static std::string get_unix_filename(const s_dir_entry &dir_entry);
    bool add_to_link_table(
        SDWord dir_idx,
        off_t size,
        bool is_random,
        st_t &begin,
        st_t &end);
    void add_to_directory(
        std::string name,
        std::string ext,
        SDWord dir_idx,
        bool is_random,
        const struct stat &stat,
        const st_t &begin,
        const st_t &end,
        bool is_file_wp);
    SectorMap_t create_sector_map(
                           const std::string &path,
                           const struct stat &sbuf,
                           const st_t &begin);
    void check_for_delete(Word ds_idx, const s_dir_sector &d);
    void check_for_extend(Word ds_idx, const s_dir_sector &d);
    void check_for_rename(Word ds_idx, const s_dir_sector &d);
    void check_for_new_file(Word ds_idx, const s_dir_sector &d);
    void check_for_changed_file_attr(Word ds_idx, s_dir_sector &d);
    bool extend_directory(SDWord sec_idx, const s_dir_sector &d);
    bool set_file_time(
        const char *ppath,
        Byte month,
        Byte day,
        Byte year,
        Byte hour,
        Byte minute) const;
    bool update_file_time(const char *path, SDWord file_id) const;
    st_t link_address() const;
    bool is_last_of_free_chain(const st_t &track_sector) const;
    SDWord id_of_new_file(const st_t &track_sector);
    std::string get_path_of_file(SDWord file_id) const;
    Word record_nr_of_new_file(SDWord new_file_id, SDWord sec_idx) const;
    void change_file_id_and_type(
        SDWord sec_idx,
        SDWord old_file_id,
        SDWord new_file_id,
        SectorType new_type);
    static void update_sector_buffer_from_link(Byte *buffer,
                                               const s_link_table &link);
    static void update_link_from_sector_buffer(s_link_table &link,
                                               const Byte *buffer);
    static std::string to_string(SectorType type);
    std::string get_unique_filename(const char *extension) const;
    SDWord get_sector_index(const st_t &track_sector) const;
};

#endif // NDIRCONT_INCLUDED

