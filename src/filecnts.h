/*
    filecnts.h

    FLEXplorer, An explorer for FLEX disk image files and directory disks.
    Copyright (C) 1998-2026  W. Schwotzer

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

#ifndef FILECNTS_INCLUDED
#define FILECNTS_INCLUDED

#ifndef __fromflex__

#include "typedefs.h"
#include "filecntb.h"
#include <memory>
#include <optional>
#include <array>
#include <ostream>

/* number of Kilo Byte at least available for each directory sector */
const int DIRSECTOR_PER_KB = 12800;
/* magic number for FLX container format.    */
/* It has to be stored in big endian format. */
const DWord MAGIC_NUMBER = 0x339a5c48;

/* Max. size of a JVC file header */
const Word MAX_JVC_HEADERSIZE = 5U;

enum tMountOption : uint8_t
{
    MOUNT_DEFAULT = 0,
    MOUNT_RAM = 1
};

/* POD structs are needed to read/write from disk image files */
/* NOLINTBEGIN(modernize-avoid-c-arrays) */
struct s_formats
{
    SDWord size; /* byte size of disk */
    Word tracks; /* number of tracks */
    Word sectors; /* number of sectors */
    Word sectors0; /* number of sectors on track 0*/
    Word dir_sectors; /* number of directory sectors */
    Word sides; /* 1 or 2 sides. If 0 estimate no. of sides */
    Word offset; /* offset for fileheader */
};

struct s_st
{
    Byte trk;
    Byte sec;

    bool operator< (const s_st &src) const
    {
        return (trk < src.trk) || (trk == src.trk && sec < src.sec);
    }
    bool operator== (const s_st &src) const
    {
        return (sec == src.sec) && (trk == src.trk);
    }
    bool operator!= (const s_st &src) const
    {
        return !operator==(src);
    }
};
using st_t = struct s_st;

/* structure of FLEX system information record (SIR) */
struct alignas(1) s_sys_info_record
{
    char disk_name[FLEX_DISKNAME_LENGTH]; // Name of this disk image
    char disk_ext[FLEX_DISKEXT_LENGTH]; // Extension of this disk image
    Byte disk_number[2]; // Number of this disk image
    st_t fc_start; // Start track/sector of free chain
    st_t fc_end; // End track/sector of free chain
    Byte free[2]; // Number of sectors in free chain
    Byte month; // Month when the disk was created, range 1 - 12
    Byte day; // Day when the disk was created, range 1 - 31
    Byte year; // Year when the disk was created, range 0 - 99
    st_t last; // Maximum track/sector number this disk image supports
};

/* structure of FLEX system information sector (SIS) */
struct alignas(1) s_sys_info_sector
{
    Byte unused1[16]; // To be initialized with 0
    s_sys_info_record sir; // System information record
    Byte unused2[216]; // To be initialized with 0
};

// (M)eta (D)ata (P)er (S)ector in Byte. It consists of the:
// - link to the next sector or 00-00.
// - 16-bit record number, zero based.
#define MDPS (static_cast<int>(sizeof(s_st)) + 2)

// Number of (D)ata (B)ytes (P)er (S)ector.
// It is the sector size minus the Meta Data Per Sector.
#define DBPS (static_cast<int>(SECTOR_SIZE) - MDPS)

// The maximum file size supported by FLEX:
// Using a FLEX harddisk with 256 tracks and 255 sectors (format 256hd).
// Sector 1 - 4 is defined as usual, sector 5 is the only directory sector.
// All remaining sectors are used for one file.
// This file has (256 * 255) - 5 sectors = 65275 sectors.
const int MAX_FILE_SECTORS = 65275;

// The maximum file size of random files supported by FLEX:
// The sector map can reference up to 168 * 255 sectors + 2
// sectors for the sector map itself.
const int MAX_RANDOM_FILE_SECTORS = 42842;

/* structure of one FLEX directory entry */
struct alignas(1) s_dir_entry
{
    char filename[FLEX_BASEFILENAME_LENGTH]; // Name of file
    char file_ext[FLEX_FILEEXT_LENGTH]; // Extension of file
    Byte file_attr; // File attributes, see flexFileAttributes
    Byte hour; // FLEX extension: hour of creation. Default: 0
    st_t start; // Track/secor of first sector of the file
    st_t end; // Track/sector of last sector of the file
    Byte records[2]; // Number of records (= sectors) the file has
    Byte sector_map; // Indicates a random access file, see IS_RANDOM_FILE
    Byte minute; // FLEX extension: minute of creation. Default: 0
    Byte month; // Month when the file was created, range 1 - 12
    Byte day; // Day when the file was created, range 1 - 31
    Byte year; // Year when the file was created, range 0 - 99
};

/* filename[0] within s_dir_entry has two magic numbers: */
// 1. An empty directory entry
constexpr char DE_EMPTY{'\0'};

// 2. A deleted directory entry
// In the FLEX Advanced Programmer's Guide it is stated that a deleted file
// has the leftmost bit of the first byte of the filename set. But when using
// the utility DELETE.CMD the first byte is set to 0xFF.
constexpr char DE_DELETED{'\xFF'};

/* structure of a FLEX directory sector */
struct alignas(1) s_dir_sector
{
    st_t next; // Link to next track/sector in the chain
    Byte record_nr[2]; // Logical record number of sector in file, zero based
    Byte unused[12]; // To be initialized with 0
    struct s_dir_entry dir_entries[DIRENTRIES]; // Dir. entries in one sector
};

struct s_floppy
{
    Byte write_protect; /* write protect flag of disk */
    Word offset; /* offset for fileheader */
    DWord byte_p_track; /* total bytes per track */
    DWord byte_p_track0; /* total bytes on track 0 */
    Word byte_p_sector; /* bytes per sector */
    Word max_sector; /* max. nr of sectors (all sides) */
    Word max_sector0; /* max. nr of sectors (all sides) track 0*/
    Word max_track; /* max. tracknumber of disk */
    Word track; /* actual tracknumber */
    DiskType type; /* type of disk */
    DiskOptions options; /* disk options */
    Word sides; /* number of sides on track 0 */
    Word sides0; /* number of sides */

};

// This interface describes a sector oriented interface to a FLEX disk image.
// Rename: FileContainerIfSector => IFlexDiskBySector
// Polymorphic interface, virtual dtor is required.
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions
class IFlexDiskBySector : public IFlexDiskBase
{
    /* Track/Sector oriented File container interface
       (to be used within flexemu) */
public:
    virtual bool ReadSector(Byte *buffer, int trk, int sec,
                            std::optional<int> side = std::nullopt) const = 0;
    virtual bool WriteSector(const Byte *buffer, int trk, int sec,
                            std::optional<int> side = std::nullopt) = 0;
    virtual bool FormatSector(const Byte *buffer, int trk, int sec, int side,
                              unsigned sizecode) = 0;
    virtual bool IsFlexFormat() const = 0;
    virtual bool IsTrackValid(int track) const = 0;
    virtual bool IsSectorValid(int track, int sector) const = 0;
    virtual unsigned GetBytesPerSector() const = 0;

    ~IFlexDiskBySector() override = default;
};

using IFlexDiskBySectorPtr = std::unique_ptr<IFlexDiskBySector>;

/* Track/sector of system info sector */
constexpr st_t sis_trk_sec{0, 3};
/* Track/sector of first directory sector */
constexpr st_t first_dir_trk_sec{0, 5};

// Array contains the max. track and sector number
// for all well known disk formats.
// To get the total number of tracks the st_t.trk
// member has to be incremented by 1.
constexpr std::array<st_t, 16> flex_formats
{{
    {34, 10}, // 5" Single-sided, single-density
    {34, 20}, // 5" Double-sided, single-density
    {39, 10}, // 5" Single-sided, single-density
    {39, 18}, // 5" Single-sided, double-density
    {39, 20}, // 5" Double-sided, single-density
    {39, 36}, // 5" Double-sided, double-density
    {79, 18}, // 5" Single-sided, double-density
    {79, 20}, // 5" Double-sided, single-density
    {79, 36}, // 5" Double-sided, double-density
    {79, 72}, // 5" Double-sided, quad-density
    {76, 15}, // 8" Single-sided, single-density
    {76, 26}, // 8" Single-sided, double-density
    {76, 30}, // 8" Double-sided, single-density
    {76, 52}, // 8" Double-sided, double-density
    {254, 255}, // Hard disk, 255 Tracks
    {255, 255}, // Hard disk, 256 Tracks
}};

constexpr std::array<const char *, 16> flex_format_descriptions
{{
    "35-10 5 1/4 inch, Single-sided, single-density, 87.5 KByte",
    "35-20 5 1/4 inch, Double-sided, single-density, 175 KByte",
    "40-10 5 1/4 inch, Single-sided, single-density, 100 KByte",
    "40-18 5 1/4 inch, Single-sided, double-density, 180 KByte",
    "40-20 5 1/4 inch, Double-sided, single-density, 200 KByte",
    "40-36 5 1/4 inch, Double-sided, double-density, 360 KByte",
    "80-18 5 1/4 inch, Single-sided, double-density, 360 KByte",
    "80-20 5 1/4 inch, Double-sided, single-density, 400 KByte",
    "80-36 5 1/4 inch, Double-sided, double-density, 720 KByte",
    "80-72 5 1/4 inch, Double-sided, quad-density, 1440 KByte",
    "77-15 8 inch, Single-sided, single-density, 288.75 KByte",
    "77-26 8 inch, Single-sided, double-density, 500.5 KByte",
    "77-30 8 inch, Double-sided, single-density, 577.5 KByte",
    "77-52 8 inch, Double-sided, double-density, 1001 KByte",
    "255-255 Hard disk, 16256.25 KByte",
    "256-255 Hard disk, 16320 KByte",
}};

constexpr std::array<const char *, 16> flex_format_shortcuts
{{
    "35sssd",
    "35dssd",
    "40sssd",
    "40ssdd",
    "40dssd",
    "40dsdd",
    "80ssdd",
    "80dssd",
    "80dsdd",
    "80dsqd",
    "77sssd",
    "77ssdd",
    "77dssd",
    "77dsdd",
    "255hd",
    "256hd",
}};

#endif /* #ifndef __fromflex__ */

/* datastructure describing the header of a FLEX disk image file */

struct s_flex_header
{
    DWord magic_number; /* to identify right format, big endian format */
    Byte write_protect; /* if != 0 disk image is write-prot*/
    Byte sizecode; /* 128 * 2 ^ n Bytes per Sector */
    Byte sides0; /* nr. of sides on track 0 */
    Byte sectors0; /* nr. of sect. on track 0 (one side)*/
    Byte sides; /* nr. of sides on track != 0 */
    Byte sectors; /* nr. of sectors on track != 0 */
    Byte tracks; /* nr. of tracks total */
    Byte dummy1; /* for future extension */
    Byte dummy2, dummy3, dummy4, dummy5; /* and for alignment 4*/
#ifndef __fromflex__
    void initialize(uint32_t sector_size, int tracks, int sectors0,
                    int sectors, int sides0, int sides);
#endif /* #ifndef __fromflex__ */
};

#ifndef __fromflex__
/* NOLINTEND(modernize-avoid-c-arrays) */

using SectorBuffer_t = std::array<Byte, SECTOR_SIZE>;
using BootSectorBuffer_t = std::array<Byte, 2 * SECTOR_SIZE>;
using SectorMap_t = std::array<Byte, 2 * DBPS>;

extern std::ostream& operator<<(std::ostream& os, const st_t &st);

extern Word getTrack0SectorCount(int tracks, int sectors);
extern Word getSides(int tracks, int sectors);
extern Word getBytesPerSector(uint32_t sizecode);
extern size_t getFileSize(const s_flex_header &header);
extern std::string getDiskName(const std::string &filename);

#endif /* #ifndef __fromflex__ */

#endif /* FILECNTS_INCLUDED */

