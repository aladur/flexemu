/*
    ffilecnt.h


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


const int MAX_OPEN_FILES = 1;


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
    SDWord      file_size;

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
    bool ReadSector(Byte *buffer, int trk, int sec) const;
    bool WriteSector(const Byte *buffer, int trk, int sec);
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
    int ByteOffset(const int trk, const int sec) const;
    void EvaluateTrack0SectorCount();
    bool CreateDirEntry(FlexDirEntry &entry);

    virtual void Initialize_for_flx_format(const s_flex_header &header,
                                           bool write_protected);
    virtual void Initialize_for_dsk_format(const s_formats &format,
                                           bool write_protected);
    static void     Create_boot_sectors(Byte sec_buf[], Byte sec_buf2[]);
    static void     Create_sys_info_sector(
        Byte    sec_buf[],
        const char  *name,
        struct  s_formats *fmt);
    static bool         Write_dir_sectors(
        FILE *fp,
        struct  s_formats *fmt);
    static bool         Write_sectors(
        FILE *fp,
        struct  s_formats *fmt);
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
