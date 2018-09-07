/*
    ndircont.h


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

#ifndef NDIRCONT_INCLUDED
#define NDIRCONT_INCLUDED

#ifdef NAFS

#include "misc1.h"

#include "flexerr.h"
#include "filecnts.h"
#include "flexemu.h"
#include <string>
#include <memory>

#define ERR_SIZE    (200)

const int MAX_TRACK             = 79;       // max. nr. of tracks - 1
const int MAX_SECTOR            = 36;       // nr of sect. per track,
                                            // side 0 and 1
const int INIT_DIR_SECTORS      = (20 - 4); // initial nr. of directory sectors
const int LINK_TABLE_SIZE       = ((MAX_TRACK + 1) * MAX_SECTOR);
const int INIT_NEW_FILES        = 4;        // initial nr of new files to be
// managed at a time
enum
{
    FREE_CHAIN  = -1,
    DIRECTORY   = -2,
    SYSTEM      = -3,
    NEW_FILE1   = -4
};

struct s_new_file
{
    char filename[FLEX_FILENAME_LENGTH];
    t_st first;
    t_st next;
    Word f_record;
    FILE *fp;
};

class NafsDirectoryContainer : public FileContainerIfSector
{

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
    std::unique_ptr<s_sys_info_sector[]> pflex_sys_info; // system info sectors
    std::unique_ptr<s_unused_sector> pflex_unused;       // unused sector
    std::unique_ptr<s_dir_sector[]> pflex_directory;     // directory entries
    std::unique_ptr<s_new_file[]> pnew_file;             // new file table
    Word dir_sectors;        // nr. of dir sectors in flex_dir.
    Word new_files;          // nr. of new file entries
    t_st dir_extend;         // track/sector of dir. ext. sect.

public:
    static NafsDirectoryContainer *Create(const char *dir,
                                          const char *name, int t, int s,
                                          int fmt = TYPE_DSK_CONTAINER);
    int Close();
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
    void fill_flex_directory(Byte dwp);
    void initialize_header(Byte wp);
    void initialize_flex_sys_info_sectors(Word number);
    void initialize_flex_unused_sector();
    void initialize_flex_directory();
    void initialize_flex_link_table();
    void initialize_new_file_table();
    void close_new_files();
    void mount(Word number);
    void free_memory();
    Byte open_files();
    SWord next_free_dir_entry();
    std::string unix_filename(SWord dir_index) const;
    std::string get_unix_filename(const char *pfn) const;
    Byte    add_to_link_table(
        SWord dir_index,
        off_t size,
        Byte random,
        t_st *begin,
        t_st *end);
    void add_to_directory(
        char *name,
        char *ext,
        SWord dir_index,
        Byte random,
        struct stat *pstat,
        t_st *begin,
        t_st *end,
        Byte wp);
    void modify_random_file(char *path, struct stat *pstat, t_st *pbegin);
    bool IsFlexFilename(
        const char *filename,
        char *name = nullptr,
        char *ext = nullptr) const;
    bool is_in_file_random(const char *ppath, const char *pfilename);
    void check_for_delete(SWord dir_index, s_dir_sector *buffer) const;
    void check_for_extend(SWord dir_index, s_dir_sector *buffer);
    void check_for_rename(SWord dir_index, s_dir_sector *pb) const;
    Byte check_for_new_file(SWord dir_index, s_dir_sector *pd) const;
    Byte extend_directory(SWord index, s_dir_sector *pdb);
    SWord set_file_time(
        char *ppath,
        Byte month,
        Byte day,
        Byte year) const;
    t_st *link_address() const;
    Byte last_of_free_chain(Byte tr, Byte sr) const;
    SWord index_of_new_file(Byte track, Byte sector);
    Word record_nr_of_new_file(SWord new_file_index, Word index) const;
    void change_file_id(
        SWord index,
        SWord old_id,
        SWord new_id) const;

};  // class NafsDirectoryContainer

#endif // NAFS
#endif // NDIRCONT_INCLUDED

