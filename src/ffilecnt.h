/*
    ffilecnt.h


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 1998-2021  W. Schwotzer

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

#include "misc1.h"
#include <stdio.h>
#include "filecont.h"
#include "filecnts.h"
#include "fdirent.h"
#include "bfileptr.h"
#include "flexerr.h"

class FlexContainerInfo;
class BDate;
class FlexCopyManager;
class FlexFileContainerIteratorImp;

#define CHECK_CONTAINER_WRITEPROTECTED              \
    if (IsWriteProtected())                 \
    {                           \
        FlexContainerInfo info;             \
        \
        GetInfo(info);                  \
        throw FlexException(FERR_CONTAINER_IS_READONLY, \
                            info.GetName());            \
    }

class FlexFileContainer : public FileContainerIfSector, public FileContainerIf
{
    friend class FlexFileContainerIteratorImp; // corresponding iterator class

protected:
    BFilePtr    fp;
    s_floppy    param;
    DWord       file_size;

    // Variables only used for FLX format when formatting a disk
    bool        is_flex_format; // true when this is a FLEX compatible format.
    int         sectors0_side0_max; // Max. sector number on side0 for track 0
    int         sectors_side0_max; // Max. sector number on side0 for track != 0
    s_flex_header flx_header;

private:
    Byte attributes;

public:
    FlexFileContainer() = delete;
    FlexFileContainer(const FlexFileContainer &) = delete;
    FlexFileContainer(FlexFileContainer &&);
    FlexFileContainer(const char *path, const char *mode);
    virtual ~FlexFileContainer();       // public destructor

    FlexFileContainer &operator= (const FlexFileContainer &) = delete;
    FlexFileContainer &operator= (FlexFileContainer &&);

    // basic interface (to be used within flexemu)
public:
    static std::string bootSectorFile;
    static FlexFileContainer *Create(const char *dir, const char *name,
                                     int t, int s,
                                     int fmt = TYPE_DSK_CONTAINER);
    bool CheckFilename(const char *fileName) const;
    bool ReadSector(Byte *buffer, int trk, int sec, int side = -1) const;
    bool WriteSector(const Byte *buffer, int trk, int sec, int side = -1);
    bool FormatSector(const Byte *buffer, int trk, int sec, int side,
                      int sizecode);
    // Return true if file container is identified as a FLEX compatible
    // file container.
    bool IsFlexFormat() const;
    bool IsWriteProtected() const;
    bool IsTrackValid(int track) const;
    bool IsSectorValid(int track, int sector) const;
    int  GetBytesPerSector() const;
    bool GetInfo(FlexContainerInfo &info) const;
    int  GetContainerType() const;

    // enhanced interface (to be used within flexdisk)
public:
    std::string GetPath() const;
    FileContainerIf *begin()
    {
        return this;
    };
    FileContainerIf *end()   const
    {
        return nullptr;
    };
    bool FindFile(const char *fileName, FlexDirEntry &entry);
    bool DeleteFile(const char *fileName);
    bool RenameFile(const char *oldName, const char *newName);
    bool SetAttributes(const char *fileName, Byte setMask, Byte clearMask);
    FlexFileBuffer ReadToBuffer(const char *fileName);
    bool WriteFromBuffer(const FlexFileBuffer &buffer,
                         const char *fileName = nullptr);
    bool FileCopy(const char *sourceName, const char *destName,
                  FileContainerIf &destination);

    // internal interface
protected:
    int ByteOffset(int trk, int sec, int side) const;
    void EvaluateTrack0SectorCount();
    bool CreateDirEntry(FlexDirEntry &entry);

    virtual void Initialize_for_flx_format(const s_flex_header &header,
                                           bool write_protected);
    virtual void Initialize_for_dsk_format(const s_formats &format,
                                           bool write_protected);
    virtual void Initialize_unformatted_disk();
    static void     Create_boot_sectors(Byte sec_buf[], Byte sec_buf2[]);
    bool GetFlexTracksSectors(Word &tracks, Word &sectors, Word offset) const;
    bool IsFlexFileFormat(int type) const;
    st_t ExtendDirectory(s_dir_sector last_dir_sector, const st_t &st_last);

    static void     Create_sys_info_sector(
        Byte    sec_buf[],
        const char  *name,
        struct  s_formats &format);
    static bool         Write_dir_sectors(
        FILE *fp,
        struct  s_formats &format);
    static bool         Write_sectors(
        FILE *fp,
        struct  s_formats &format);
    static void     Create_format_table(
        int type,
        int trk,
        int sec,
        struct s_formats &format);
    static void Format_disk(
        int trk,
        int sec,
        const char *disk_dir,
        const char *name,
        int type);
private:
    FileContainerIteratorImpPtr IteratorFactory();

};  // class FlexFileContainer

#endif // FFILECNT_INCLUDED
