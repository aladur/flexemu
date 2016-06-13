/*
    filecnts.h

    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2004  W. Schwotzer

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

#ifndef __filecnts_h__
#define __filecnts_h__

#ifndef __fromflex__

#include <misc1.h>
#include "filecntb.h"

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
    Byte    unused1[16];
    char    disk_name[8];
    char    disk_ext[3];
    Byte    disk_number[2];
    Byte    fc_start_trk;
    Byte    fc_start_sec;
    Byte    fc_end_trk;
    Byte    fc_end_sec;
    Byte    free[2];
    Byte    month;
    Byte    day;
    Byte    year;
    Byte    last_trk;
    Byte    last_sec;
    Byte    unused2[216];
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
    Word        sec_trk;    /* spec. sector and tracknr. of as sect. */
    struct s_st st;
} t_st;

struct s_link_table
{
    t_st        next;       /* sector and tracknumber of next sect.*/
    Byte        record_nr[2];   /* FLEX record number */
    Word        f_record;   /* rel position in file / 252 */
    SWord       file_id;    /* index of file in directory */
};


/* structure of one FLEX directory entry */
struct s_dir_entry
{
    char    filename[8];
    char    file_ext[3];
    Byte    file_attr;
    Byte    reserved1;
    char    start_trk;
    char    start_sec;
    Byte    end_trk;
    Byte    end_sec;
    Byte    records[2];
    Byte    sector_map;
    Byte    reserved2;
    Byte    month;
    Byte    day;
    Byte    year;
};

/* structure of one FLEX directory sector */
struct s_dir_sector
{
    Byte    next_trk;
    Byte    next_sec;
    Byte    record_nr[2];
    Byte    unused[12];
    struct s_dir_entry dir_entry[10];
};

struct s_unused_sector
{
    Byte    next_trk;
    Byte    next_sec;
    Byte    unused[254];
};

struct s_floppy
{
    Byte        write_protect;  /* write protect flag of disk */
    Word        offset;     /* offset for fileheader */
    Word        byte_p_track;   /* total bytes per track */
    Word        byte_p_track0;  /* total bytes on track 0 */
    Word        byte_p_sector;  /* bytes per sector */
    Word        max_sector; /* max. nr of sectors per side */
    Word        max_sector0;    /* max. nr of sectors per side track 0*/
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
    virtual int  GetBytesPerSector(void) const = 0;
    virtual ~FileContainerIfSector() { };
};  /* class FileContainerIfSector */

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

#endif /* __filecnts_h__ */

