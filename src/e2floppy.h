/*
    e2floppy.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2018  W. Schwotzer

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


#ifndef __e2floppy_h__
#define __e2floppy_h__

#include "misc1.h"

#ifndef __fromflex__

    #include <stdio.h>
    #include <sys/stat.h>
    #include "flexemu.h"
    #include "wd1793.h"
    #include "filecnts.h"
    #include <string>
    #include <mutex>

#else
    #include "typedefs.h"
#endif /* #ifndef __fromflex__ */

#ifndef __fromflex__


class E2floppy : public Wd1793
{
private:

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

    FileContainerIfSector   *floppy[5];
    FileContainerIfSector   *pfs;
    Byte            track[5];
    DiskStatus      drive_status[5];
    char            sector_buffer[256];
    const char      *disk_dir;
    std::mutex      status_mutex;

public:
    E2floppy();
    virtual ~E2floppy();

    // public interface
public:
    virtual void         resetIo();
    virtual const char   *getName()
    {
        return "e2floppy";
    };

    virtual void         get_drive_status(DiskStatus status[4]);
    virtual void         disk_directory(const char *x_disk_dir);
    virtual void         mount_all_drives(std::string drive[]);
    virtual bool         update_all_drives();
    virtual bool         umount_all_drives();
    virtual bool         mount_drive(const char *path, Word drive_nr,
                                     tMountOption option = MOUNT_DEFAULT);
    virtual bool         format_disk(SWord trk, SWord sec,
                                     const char *name,
                                     int type = TYPE_DSK_CONTAINER);
    virtual bool         update_drive(Word drive_nr);
    virtual bool         umount_drive(Word drive_nr);
    virtual std::string  drive_info(Word drive_nr);
    virtual void         select_drive(Byte new_selected);

private:

    virtual Byte         readByte(Word index);
    virtual void         writeByte(Word index);
    virtual bool         isDriveReady() const;
    virtual bool         isWriteProtect();
    virtual bool         isRecordNotFound();
    virtual bool         isSeekError(Byte new_track);
    const char       *open_mode(char *path);
};

#endif /* #ifndef __fromflex__ */
#endif /* __e2floppy_h__ */

