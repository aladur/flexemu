/*
    ndircont.h


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

#ifndef NDIRCONT_INCLUDED
#define NDIRCONT_INCLUDED

#ifdef NAFS

#include "misc1.h"

#include "filecnts.h"
#include "flexemu.h"
#include <string>
#include <vector>
#include <unordered_map>


class NafsDirectoryContainer : public FileContainerIfSector
{

    static const int MAX_TRACK{79}; // maximum track number (zero based).
    static const int MAX_SECTOR{36}; // number of sectors per track,
                                     // side 0 and 1 (one based).
    static const int MAX_SECTOR0{30}; // number of sectors on track 0,
                                      // side 0 and 1 (one based).
    static const int INIT_DIR_SECTORS{MAX_SECTOR0 - 4}; // initial number of
                                                        // directory sectors.
    static const int LINK_TABLE_SIZE{(MAX_TRACK + 1) * MAX_SECTOR}; // Each
                                       // sector has an entry in the link table.
    enum class SectorType : Byte
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
        st_t        next;       // Track and sector number of next sector
        Byte        record_nr[2]; // FLEX logical record number
        Word        f_record;   // Relative position in file / 252
        SWord       file_id;
        SectorType  type; // The sector type.
    };

    // A new file is a newly created file which not yet has an entry in
    // a directory sector (s_dir_sector), so the name of this file is unknown.
    // As soon as a new directory entry (s_dir_entry) is created for it, it is
    // removed from the list of new files (new_files) and the file is renamed
    // on the host file system.
    struct s_new_file
    {
        char filename[FLEX_FILENAME_LENGTH];
        st_t first; /* track and sector of first first first sector */
        st_t next; /* track and sector of next sector to be written */
        Word f_record; /* number of records (= sectors) */
        FILE *fp; /* file pointer on the target file system */
    };

public:
    NafsDirectoryContainer() = delete;
    NafsDirectoryContainer(const NafsDirectoryContainer &) = delete;
    NafsDirectoryContainer(NafsDirectoryContainer &&) = delete;
    NafsDirectoryContainer(const char *path);
    virtual ~NafsDirectoryContainer();

    NafsDirectoryContainer &operator=(const NafsDirectoryContainer &) = delete;
    NafsDirectoryContainer &operator=(NafsDirectoryContainer &&) = delete;

private:
    std::string directory;
    Byte attributes;
    bool isOpen;

    s_floppy param;

    // Some structures needed for a FLEX file system
    std::unique_ptr<s_link_table[]> pflex_links;         // link table
    std::array<s_sys_info_sector, 2> flex_sys_info; // system info sectors
    std::vector<s_dir_sector> flex_directory; // directory sectors
    std::unordered_map<SWord, s_new_file> new_files; // new file table
    st_t dir_extend;         // track and sector of directory extend sector

public:
    static NafsDirectoryContainer *Create(const char *dir,
                                          const char *name, int t, int s,
                                          int fmt = TYPE_DSK_CONTAINER);
    bool Close();
    bool IsContainerOpened() const;
    bool CheckFilename(const char *fileName) const;
    bool ReadSector(Byte *buffer, int trk, int sec) const;
    bool WriteSector(const Byte *buffer, int trk, int sec);
    bool IsWriteProtected() const;
    bool IsTrackValid(int track) const;
    bool IsSectorValid(int track, int sector) const;
    int GetBytesPerSector() const;
    bool GetInfo(FlexContainerInfo &info) const;
    int GetContainerType() const;
    std::string GetPath() const;

private:
    void fill_flex_directory(bool is_write_protected);
    void initialize_header(Byte wp);
    void initialize_flex_sys_info_sectors(Word number);
    void initialize_flex_directory();
    void initialize_flex_link_table();
    void close_new_files();
    void mount(Word number);
    void free_memory();
    SWord next_free_dir_entry();
    std::string get_unix_filename(SWord file_id) const;
    std::string get_unix_filename(const s_dir_entry &dir_entry) const;
    bool add_to_link_table(
        SWord dir_index,
        off_t size,
        bool is_random,
        st_t &begin,
        st_t &end);
    void add_to_directory(
        char *name,
        char *ext,
        SWord dir_index,
        bool is_random,
        const struct stat &stat,
        const st_t &begin,
        const st_t &end,
        bool is_write_protected);
    void modify_random_file(char *path, const struct stat &stat,
                            const st_t &pbegin);
    bool IsFlexFilename(
        const char *filename,
        char *name = nullptr,
        char *ext = nullptr) const;
    bool is_in_file_random(const char *ppath, const char *pfilename);
    void check_for_delete(SWord dir_index, const s_dir_sector &d) const;
    void check_for_extend(SWord dir_index, const s_dir_sector &d);
    void check_for_rename(SWord dir_index, const s_dir_sector &d) const;
    void check_for_new_file(SWord dir_index, const s_dir_sector &d);
    bool extend_directory(SWord index, const s_dir_sector &d);
    bool set_file_time(
        const char *ppath,
        Byte month,
        Byte day,
        Byte year) const;
    st_t link_address() const;
    bool is_last_of_free_chain(Byte tr, Byte sr) const;
    SWord index_of_new_file(Byte track, Byte sector);
    Word record_nr_of_new_file(SWord new_file_index, Word index) const;
    void change_file_id_and_type(
        SWord index,
        SWord old_id,
        SWord new_id,
        SectorType new_type) const;
    static void update_sector_buffer_from_link(Byte *buffer,
                                               const s_link_table &link);
    static void update_link_from_sector_buffer(s_link_table &link,
                                               const Byte *buffer);
    void check_pointer(void *ptr);


};  // class NafsDirectoryContainer

#endif // NAFS
#endif // NDIRCONT_INCLUDED

