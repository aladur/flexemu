/*
    filecnts.h

    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2023  W. Schwotzer

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

#include "misc1.h"
#include "filecntb.h"
#include <memory>
#include <ostream>
#include <array>

/* number of Kilo Byte at least available for each directory sector */
const int DIRSECTOR_PER_KB  = 12800;
/* magic number for FLX container format.    */
/* It has to be stored in big endian format. */
const DWord MAGIC_NUMBER    = 0x339a5c48;

/* Max. size of a JVC file header */
const Word MAX_JVC_HEADERSIZE = 5U;

enum tMountOption
{
    MOUNT_DEFAULT   = 0,
    MOUNT_RAM       = 1
};

struct s_formats
{
    SDWord      size;       /* byte size of disk           */
    Word        tracks;     /* number of tracks            */
    Word        sectors;    /* number of sectors           */
    Word        sectors0;   /* number of sectors on track 0*/
    Word        dir_sectors;    /* number of directory sectors */
    Word        sides;      /* 1 or 2 sides. If 0 estimate no. of sides */
    Word        offset;     /* offset for fileheader */
};

typedef struct s_st
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
} st_t;

/* structure of FLEX system information record (SIR) */
struct s_sys_info_record
{
    char    disk_name[FLEX_DISKNAME_LENGTH]; // Name of this disk image
    char    disk_ext[FLEX_DISKEXT_LENGTH]; // Extension of this disk image
    Byte    disk_number[2]; // Number of this disk image
    st_t    fc_start; // Start track/sector of free chain
    st_t    fc_end; // End track/sector of free chain
    Byte    free[2]; // Number of sectors in free chain
    Byte    month; // Month when the disk was created, range 1 - 12
    Byte    day; // Day when the disk was created, range 1 - 31
    Byte    year; // Year when the disk was created, range 0 - 99
    st_t    last; // Maximum track/sector number this disk image supports
};

/* structure of FLEX system information sector (SIS) */
struct s_sys_info_sector
{
    Byte unused1[16]; // To be initialized with 0
    s_sys_info_record sir; // System information record
    Byte unused2[216]; // To be initialized with 0
};

// (M)eta (D)ata (P)er (S)ector in Byte. It consists of the:
// - link to the next sector
// - 16-bit record number
#define MDPS (sizeof(s_st) + 2)

// Number of (D)ata (B)ytes (P)er (S)ector.
// It is the sector size minus the Meta Data Per Sector.
#define DBPS (SECTOR_SIZE - MDPS)


/* structure of one FLEX directory entry */
struct s_dir_entry
{
    char    filename[FLEX_BASEFILENAME_LENGTH]; // Name of file
    char    file_ext[FLEX_FILEEXT_LENGTH]; // Extension of file
    Byte    file_attr; // File attributes, see flexFileAttributes
    Byte    hour; // FLEX extension: hour of creation. Default: 0
    st_t    start; // Track/secor of first sector of the file
    st_t    end; // Track/sector of last sector of the file
    Byte    records[2]; // Number of records (= sectors) the file has
    Byte    sector_map; // Indicates a random access file, see IS_RANDOM_FILE
    Byte    minute; // FLEX extension: minute of creation. Default: 0
    Byte    month; // Month when the file was created, range 1 - 12
    Byte    day; // Day when the file was created, range 1 - 31
    Byte    year; // Year when the file was created, range 0 - 99
};

/* filename[0] within s_dir_entry has two magic numbers: */
// 1. An empty directory entry
#define DE_EMPTY    '\0'
// 2. A deleted directory entry
// In the FLEX Advanced Programmer's Guide it is stated that a deleted file
// has the leftmost bit of the first byte of the filename set. But when using
// the utility DELETE.CMD the first byte is set to 0xFF.
#define DE_DELETED  '\xFF'

/* structure of one FLEX directory sector */
struct s_dir_sector
{
    st_t    next; // Link to next track/sector in the chain
    Byte    record_nr[2]; // Logical record number of sector in file, zero based
    Byte    unused[12]; // To be initialized with 0
    struct s_dir_entry dir_entry[DIRENTRIES]; // directory entries in one sector
};


struct s_floppy
{
    Byte        write_protect;  /* write protect flag of disk */
    Word        offset;     /* offset for fileheader */
    DWord       byte_p_track;   /* total bytes per track */
    DWord       byte_p_track0;  /* total bytes on track 0 */
    Word        byte_p_sector;  /* bytes per sector */
    Word        max_sector; /* max. nr of sectors (all sides) */
    Word        max_sector0;    /* max. nr of sectors (all sides) track 0*/
    Word        max_track;  /* max. tracknumber of disk */
    Word        track;      /* actual tracknumber */
    Word        type;       /* type of container */
    Word        sides;      /* number of sides on track 0 */
    Word        sides0;     /* number of sides */

};



class FileContainerIfSector : public FileContainerIfBase
{
    /* Track/Sector oriented File container interface
       (to be used within flexemu) */
public:
    virtual bool ReadSector(Byte *buffer, int trk, int sec,
                            int side = -1) const = 0;
    virtual bool WriteSector(const Byte *buffer, int trk, int sec,
                             int side = -1) = 0;
    virtual bool FormatSector(const Byte *buffer, int trk, int sec, int side,
                              int sizecode) = 0;
    virtual bool IsFlexFormat() const = 0;
    virtual bool IsTrackValid(int track) const = 0;
    virtual bool IsSectorValid(int track, int sector) const = 0;
    virtual int  GetBytesPerSector() const = 0;
    virtual ~FileContainerIfSector() { };
};  /* class FileContainerIfSector */

using FileContainerIfSectorPtr = std::unique_ptr<FileContainerIfSector>;

/* Track/sector of system info sector */
constexpr st_t sis_trk_sec{0, 3};
/* Track/sector of first directory sector */
constexpr st_t first_dir_trk_sec{0, 5};

constexpr std::array<st_t, 15> flex_formats
{{
    {35, 10}, // 5" Single-sided, single-density
    {35, 20}, // 5" Double-sided, single-density
    {40, 10}, // 5" Single-sided, single-density
    {40, 18}, // 5" Single-sided, double-density
    {40, 20}, // 5" Double-sided, single-density
    {40, 36}, // 5" Double-sided, double-density
    {80, 18}, // 5" Single-sided, double-density
    {80, 20}, // 5" Double-sided, single-density
    {80, 36}, // 5" Double-sided, double-density
    {80, 72}, // 5" Double-sided, quad-density
    {77, 15}, // 8" Single-sided, single-density
    {77, 26}, // 8" Single-sided, double-density
    {77, 30}, // 8" Double-sided, single-density
    {77, 52}, // 8" Double-sided, double-density
    {255, 255} // Hard disk
}};

constexpr std::array<const char *, 15> flex_format_descriptions
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
    "255-255 Hard disk, 16256.25 KByte"
}};

constexpr std::array<const char *, 15> flex_format_shortcuts
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
    "256hd"
}};

#endif /* #ifndef __fromflex__ */

/* datastructure describing the header of a FLEX disk image file */

struct s_flex_header
{
    DWord magic_number; /* to identify right format, big endian format */
    Byte write_protect; /* if != 0 disk image is write-prot*/
    Byte sizecode;      /* 128 * 2 ^ n Bytes per Sector  */
    Byte sides0;        /* nr. of sides on track 0   */
    Byte sectors0;      /* nr. of sect. on track 0 (one side)*/
    Byte sides;     /* nr. of sides on track != 0    */
    Byte sectors;       /* nr. of sectors on track != 0  */
    Byte tracks;        /* nr. of tracks total       */
    Byte dummy1;        /* for future extension      */
    Byte dummy2, dummy3, dummy4, dummy5; /* and for alignment 4*/
#ifndef __fromflex__
    void initialize(int sector_size, int tracks, int sectors0,
                    int sectors, int sides0, int sides);
#endif /* #ifndef __fromflex__ */
};

#ifndef __fromflex__
extern std::ostream& operator<<(std::ostream& os, const  st_t &st);

extern Word getTrack0SectorCount(int tracks, int sectors);
extern Word getSides(int tracks, int sectors);
extern Word getBytesPerSector(int sizecode);
extern size_t getFileSize(const s_flex_header &header);
#endif /* #ifndef __fromflex__ */

#endif /* FILECNTS_INCLUDED */

