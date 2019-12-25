/*
    filecnts.h

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

#ifndef FILECNTS_INCLUDED
#define FILECNTS_INCLUDED

#ifndef __fromflex__

#include "misc1.h"
#include "filecntb.h"
#include <memory>

/* number of Kilo Byte at least available for each directory sector */
const int DIRSECTOR_PER_KB  = 12800;
/* magic number for FLX container format */
const DWord MAGIC_NUMBER    = 0x485c9a33L;

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
    Word        dir_sectors;    /* number of directory sectors */
};

/* structure of FLEX system info sector */
struct s_sys_info_sector
{
    Byte    unused1[16]; // To be initialized with 0
    char    disk_name[8]; // Name of this disk image
    char    disk_ext[3]; // Extension of this disk image
    Byte    disk_number[2]; // Number of this disk image
    Byte    fc_start_trk; // Start track of free chain
    Byte    fc_start_sec; // Start sector of free chain
    Byte    fc_end_trk; // End track of free chain
    Byte    fc_end_sec; // End sector of free chain
    Byte    free[2]; // Number of sectors in free chain
    Byte    month; // Month when the disk was created, range 1 - 12
    Byte    day; // Day when the disk was created, range 1 - 31
    Byte    year; // Year when the disk was created, range 0 - 99
    Byte    last_trk; // Maximum track number this disk image supports
    Byte    last_sec; // Maximum sector number this disk image supports
    Byte    unused2[216]; // To be initialized with 0
};

struct s_st
{
#ifdef WORDS_BIGENDIAN
    Byte sec;
    Byte trk;
#else
    Byte trk;
    Byte sec;
#endif
};

typedef union
{
    Word sec_trk; // Specifies track and sector number of a sector
    struct s_st st;
} st_t;

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
    char    filename[8]; // Name of file
    char    file_ext[3]; // Extension of file
    Byte    file_attr; // File attributes, see flexFileAttributes
    Byte    reserved1; // To be initialized with 0
    char    start_trk; // Track of first sector of the file
    char    start_sec; // First sector of this file
    Byte    end_trk; // Track of last sector of the file
    Byte    end_sec; // Last sector of the file
    Byte    records[2]; // Number of records (= sectors) the file has
    Byte    sector_map; // Indicates a random access file, see IS_RANDOM_FILE
    Byte    reserved2; // To be initialized with 0
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
    Byte    next_trk; // Link to track of next sector in the chain
    Byte    next_sec; // Link to next sector in the chain
    Byte    record_nr[2]; // Logical record number of sector in file, zero based
    Byte    unused[12]; // To be initialized with 0
    struct s_dir_entry dir_entry[DIRENTRIES]; // directory entries in one sector
};


struct s_floppy
{
    Byte        write_protect;  /* write protect flag of disk */
    Word        offset;     /* offset for fileheader */
    Word        byte_p_track;   /* total bytes per track */
    Word        byte_p_track0;  /* total bytes on track 0 */
    Word        byte_p_sector;  /* bytes per sector */
    Word        max_sector; /* max. nr of sectors (all sides) */
    Word        max_sector0;    /* max. nr of sectors (all sides) track 0*/
    Word        max_track;  /* max. tracknumber of disk */
    Word        track;      /* actual tracknumber */
    Word        type;       /* type of container */
};



class FileContainerIfSector : public FileContainerIfBase
{
    /* Track/Sector oriented File container interface
       (to be used within flexemu) */
public:
    virtual bool ReadSector(Byte *buffer, int trk, int sec) const = 0;
    virtual bool WriteSector(const Byte *buffer, int trk, int sec) = 0;
    virtual bool IsTrackValid(int track) const = 0;
    virtual bool IsSectorValid(int track, int sector) const = 0;
    virtual int  GetBytesPerSector() const = 0;
    virtual ~FileContainerIfSector() { };
};  /* class FileContainerIfSector */

using FileContainerIfSectorPtr = std::unique_ptr<FileContainerIfSector>;

#endif /* #ifndef __fromflex__ */

/* datastructure describing the header of a FLEX disk image file */

struct s_flex_header
{
    DWord magic_number; /* to identify right format  */
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
    void initialize(int secsize, int trk, int sec0, int sec, int sides);
#endif /* #ifndef __fromflex__ */
};

#endif /* FILECNTS_INCLUDED */

