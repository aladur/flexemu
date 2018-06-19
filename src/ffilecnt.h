/*
    ffilecnt.h


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


#define CHECK_NO_CONTAINER_OPEN                 \
    if (fp == nullptr) {                   \
        throw FlexException(FERR_NO_CONTAINER_OPEN);    \
    }

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

private:
    int     attributes;

public:
    FlexFileContainer(const char *path, const char *mode);
    virtual ~FlexFileContainer();       // public destructor

    // basic interface (to be used within flexemu)
public:
    static std::string bootSectorFile;
    static FlexFileContainer *Create(const char *dir, const char *name,
                                     int t, int s,
                                     int fmt = TYPE_DSK_CONTAINER);
    int  Close();
    bool CheckFilename(const char *fileName) const;
    bool IsContainerOpened() const;
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
    bool SetAttributes(const char *fileName, int setMask, int clearMask);
    void ReadToBuffer(const char *fileName, FlexFileBuffer &buffer);
    bool WriteFromBuffer(const FlexFileBuffer &buffer,
                         const char *fileName = nullptr);
    bool FileCopy(const char *sourceName, const char *destName,
                  FileContainerIf &destination);

    // internal interface
protected:
    int ByteOffset(const int trk, const int sec) const;
    bool CreateDirEntry(FlexDirEntry &entry);

    void Initialize_for_flx_format(
        s_floppy        *pfloppy,
        s_flex_header   *pheader,
        bool        wp);
    void Initialize_for_dsk_format(
        s_floppy        *pfloppy,
        s_formats       *pformat,
        bool        wp);
    static void     Create_boot_sector(Byte sec_buf[]);
    static void     Create_sector2(
        Byte    sec_buf[],
        struct  s_formats *fmt);
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
        SWord trk,
        SWord sec,
        struct s_formats *pformat);
    static void Format_disk(
        SWord trk,
        SWord sec,
        const char *disk_dir,
        const char *name,
        int type);
private:
    FlexFileContainer();    // should not be used
    FileContainerIteratorImp *IteratorFactory();

};  // class FlexFileContainer

#endif // FFILECNT_INCLUDED
