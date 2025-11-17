/*
    ffilecnt.h


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

#ifndef FFILECNT_INCLUDED
#define FFILECNT_INCLUDED

#include "efiletim.h"
#include "filecont.h"
#include "filecntb.h"
#include "filecnts.h"
#include "fdirent.h"
#include <ios>
#include <optional>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;


class FlexDiskAttributes;
class BDate;
class FlexCopyManager;
class FlexDiskIteratorImp;

#define CHECK_CONTAINER_WRITEPROTECTED     \
    if (IsWriteProtected())                \
    {                                      \
        FlexDiskAttributes diskAttributes; \
        \
        GetDiskAttributes(diskAttributes);              \
        throw FlexException(FERR_CONTAINER_IS_READONLY, \
                            diskAttributes.GetDiskname());  \
    }

// class FlexDisk implements both the sector and file oriented interface
// for FLEX disk image files (*.dsk).
// Optionally a *.dsk file may have a JVC header.
// In addition to that it also supports FLX disk image files (*.flx).
//
// Rename: FlexFileContainer => FlexDisk
class FlexDisk : public IFlexDiskBySector, public IFlexDiskByFile
{
    friend class FlexDiskIteratorImp; // corresponding iterator class

    // Struct defining the directory sector's track/sector and directory index
    // within this sector referencing a file.
    struct s_dir_pos
    {
        st_t trk_sec; // track/sector of the directory sector.
        Byte idx; // index within the directory sector.
    };

protected:
    fs::path path;
    mutable std::fstream fstream;
    s_floppy param{};
    DWord file_size{};
    const FileTimeAccess &ft_access{};
    DiskType disk_type{};

    // Variables only used for FLX format when formatting a disk
    bool is_flex_format{false}; // true when this is a FLEX compatible format.
    bool is_filenames_initialized{false}; // Used to lazy init. filenames.
    int sectors0_side0_max{}; // Max. sector number on side0 for track 0
    int sectors_side0_max{}; // Max. sector number on side0 for track != 0
    st_t next_dir_trk_sec{}; // Next directory track/sector to be used for
                             // new created files. May be 00-00.
    s_flex_header flx_header{};
    // All filenames available and their position in the directory.
    std::unordered_map<std::string, s_dir_pos> filenames;

private:
    Byte attributes{};

public:
    FlexDisk() = delete;
    FlexDisk(fs::path p_path, std::ios::openmode mode,
             const FileTimeAccess &fileTimeAccess);
    ~FlexDisk() override = default;
    FlexDisk(const FlexDisk &src) = delete;
    FlexDisk(FlexDisk &&src) = delete;
    FlexDisk &operator= (const FlexDisk &src) = delete;
    FlexDisk &operator= (FlexDisk &&src) = delete;

    static bool onTrack0OnlyDirSectors;

    static FlexDisk *Create(const fs::path &path,
                            const FileTimeAccess &fileTimeAccess,
                            int tracks, int sectors,
                            DiskType type,
                            const std::optional<fs::path> &bsFile =
                                std::nullopt);
    static void SetBootSectorFile(const fs::path &bootSectorFile);
    static fs::path &GetBootSectorFile();
    static void InitializeClass();

    // IFlexDiskBase interface declaration
    bool IsWriteProtected() const override;
    bool GetDiskAttributes(FlexDiskAttributes &diskAttributes) const override;
    DiskType GetFlexDiskType() const override;
    DiskOptions GetFlexDiskOptions() const override;
    fs::path GetPath() const override;

    // IFlexDiskByFile interface declaration (to be used in flexemu).
    bool ReadSector(Byte *buffer, int trk, int sec,
            std::optional<int> side = std::nullopt) const override;
    bool WriteSector(const Byte *buffer, int trk, int sec,
            std::optional<int> side = std::nullopt) override;
    bool FormatSector(const Byte *target, int track, int sector, int side,
                      unsigned sizecode) override;
    bool IsFlexFormat() const override;
    bool IsTrackValid(int track) const override;
    bool IsSectorValid(int track, int sector) const override;
    unsigned GetBytesPerSector() const override;

    // IFlexDiskByFile interface declaration (to be used within flexplorer).
    IFlexDiskByFile *begin() override
    {
        return this;
    };
    IFlexDiskByFile *end() const override
    {
        return nullptr;
    };
    bool FindFile(const std::string &wildcard, FlexDirEntry &dirEntry) override;
    bool DeleteFile(const std::string &wildcard) override;
    bool RenameFile(const std::string &oldName,
                    const std::string &newName) override;
    bool SetAttributes(const std::string &wildcard, unsigned setMask,
                       unsigned clearMask = ~0U) override;
    FlexFileBuffer ReadToBuffer(const std::string &fileName) override;
    bool WriteFromBuffer(const FlexFileBuffer &buffer,
                         const char *fileName = nullptr) override;
    bool FileCopy(const std::string &sourceName, const std::string &destName,
                  IFlexDiskByFile &destination) override;
    std::string GetSupportedAttributes() const override;

protected:
    int ByteOffset(int trk, int sec, std::optional<int> side) const;
    void EvaluateTrack0SectorCount();
    bool CreateDirEntry(FlexDirEntry &dirEntry);

    void Initialize_for_flx_format(const s_flex_header &header);
    void Initialize_for_dsk_format(const s_formats &format);
    void Initialize_unformatted_disk();
    static void Create_boot_sectors(BootSectorBuffer_t &bootSectors,
                                    std::optional<fs::path> bsFile);
    bool GetFlexTracksSectors(Word &tracks, Word &sectors, Word offset) const;
    bool IsFlexFileFormat(DiskType disk_type) const;
    st_t ExtendDirectory(s_dir_sector last_dir_sector, const st_t &st_last);
    std::vector<Byte> GetJvcFileHeader() const;

    static void Create_sys_info_sector(
        s_sys_info_sector &sis,
        const std::string &name,
        struct s_formats &format);
    static bool Write_dir_sectors(std::fstream &fs, struct s_formats &format);
    static bool Write_sectors(std::fstream &fs, struct s_formats &format);
    static void Create_format_table(
        DiskType p_disk_type,
        int trk,
        int sec,
        struct s_formats &format);
    static void Format_disk(
        const fs::path &path,
        int tracks,
        int sectors,
        DiskType p_disk_type,
        const std::optional<fs::path> &bsFile);
    static FlexDirEntry CreateDirEntryFrom(const s_dir_entry &dir_entry,
            const std::string &filename);

    // IFlexDiskByFile interface performance functions.
    void InitializeFilenames();
    void SetNextDirectoryPosition(const st_t &dirTS);
    void DeleteFromFilenames(const std::string &filename);
    void AddToFilenames(const std::string &filename, const s_dir_pos &dir_pos);
    bool FindInFilenames(const std::string &filename);

private:
    IFlexDiskIteratorImpPtr IteratorFactory() override;

};

#endif // FFILECNT_INCLUDED
