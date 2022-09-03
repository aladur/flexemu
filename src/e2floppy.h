/*
    e2floppy.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2022  W. Schwotzer

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


#ifndef E2FLOPPY_INCLUDED
#define E2FLOPPY_INCLUDED

#include "misc1.h"

#ifndef __fromflex__

    #include <stdio.h>
    #include <sys/stat.h>
    #include "flexemu.h"
    #include "wd1793.h"
    #include "filecnts.h"
    #include "fcinfo.h"
    #include <string>
    #include <mutex>

#else
    #include "typedefs.h"
#endif /* #ifndef __fromflex__ */

#ifndef __fromflex__

struct sOptions;

class E2floppy : public Wd1793
{
public:
    static const unsigned MAX_DRIVES{4U}; // max. number of disk drives.

private:

    // States for writing a track (formatting a disk).
    enum class WriteTrackState
    {
        Inactive,
        WaitForIdAddressMark,
        IdAddressMark,
        WaitForDataAddressMark,
        WriteData,
        WaitForCrc,
    };


    // Internal registers:
    //
    //  selected        Selected drive as index into floppy array

    Byte selected;

    // Interface to operating system:
    //
    //  floppy          Pointers to all file containers (drive 4 deselects fdc)
    //  pfs             Pointer to currently selected file container
    //  track           Track number of all drives
    //  drive_status    Status of all drives
    //  sector_buffer   Current sector to read from or write to
    //  disk_dir        Disk directory
    // Drive nr. 4 means no drive selected

    FileContainerIfSectorPtr floppy[MAX_DRIVES + 1U];
    FileContainerIfSector *pfs;
    Byte            track[MAX_DRIVES + 1U];
    DiskStatus      drive_status[MAX_DRIVES + 1U];
    Byte            sector_buffer[1024];
    std::string     disk_dir;
    mutable std::mutex status_mutex;
    // data for CMD_WRITETRACK
    WriteTrackState writeTrackState; // Write track state
    Word            offset; // offset when reading a track
    char            idAddressMark[4]; // Contains track, side, sector, sizecode
    const struct sOptions &options;

public:
    E2floppy() = delete;
    E2floppy(const struct sOptions &options);
    virtual ~E2floppy();

    // public interface
public:
    void resetIo() override;
    const char *getName() override
    {
        return "fdc";
    };

    virtual void get_drive_status(DiskStatus status[MAX_DRIVES]);
    virtual void         disk_directory(const char *x_disk_dir);
    virtual void         mount_all_drives(std::string drive[]);
    virtual bool         sync_all_drives(
                                    tMountOption option = MOUNT_DEFAULT);
    virtual bool         umount_all_drives();
    virtual bool         mount_drive(const char *path, Word drive_nr,
                                     tMountOption option = MOUNT_DEFAULT);
    virtual bool         format_disk(SWord trk, SWord sec,
                                     const char *name,
                                     int type = TYPE_DSK_CONTAINER);
    virtual bool         sync_drive(Word drive_nr,
                                    tMountOption option = MOUNT_DEFAULT);
    virtual bool         umount_drive(Word drive_nr);
    virtual FlexContainerInfo drive_info(Word drive_nr);
    virtual std::string  drive_info_string(Word drive_nr);
    virtual void         select_drive(Byte new_selected);

private:

    bool startCommand(Byte command_un) override;
    Byte readByte(Word index, Byte command_un) override;
    Byte readByteInSector(Word index);
    Byte readByteInTrack(Word index);
    Byte readByteInAddress(Word index);
    void writeByte(Word &index, Byte command_un) override;
    void writeByteInSector(Word index);
    void writeByteInTrack(Word &index);
    bool isDriveReady() const override;
    bool isWriteProtect() const override;
    bool isRecordNotFound() const override;
    bool isSeekError(Byte new_track) const override;
    Word getBytesPerSector() const override;
    Byte getSizeCode() const;
    const char       *open_mode(char *path);
};

#endif /* #ifndef __fromflex__ */
#endif /* E2FLOPPY_INCLUDED */

