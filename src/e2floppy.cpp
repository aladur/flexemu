/*
    e2floppy.cpp

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


#include "misc1.h"
#include <string>
#include <sstream>
#include <iterator>
#include "e2floppy.h"
#include "ffilecnt.h"
#include "rfilecnt.h"
#include "ndircont.h"
#include "fcinfo.h"
#include "flexerr.h"
#include "bdir.h"


E2floppy::E2floppy() :
    status(0), drisel(0), selected(4), pfs(nullptr)
{
    Word i;

    disk_dir = "";

    for (i = 0; i <= 4; i++)
    {
        track[i] = 1; // position all drives to track != 0  !!!
        floppy[i] = nullptr;
        driveStatus[i] = DISK_STAT_EMPTY;
    }

    memset(sector_buffer, 0, sizeof(sector_buffer));
} // E2floppy


E2floppy::~E2floppy()
{
    std::lock_guard<std::mutex> guard(status_mutex);

    for (int i = 0; i < 4; i++)
    {
        if (floppy[i] != nullptr)
        {
            try
            {
                floppy[i]->Close();
                delete floppy[i];
                floppy[i] = nullptr;
                driveStatus[i] = DISK_STAT_EMPTY;
            }
            catch (...)
            {
                // ignore errors
            }
        }
    }
} // ~E2floppy

bool E2floppy::umount_drive(Word drive_nr)
{
    FileContainerIfSector *pfloppy;

    if (drive_nr > 3 || (pfloppy = floppy[drive_nr]) == nullptr)
    {
        return 0;
    }

    std::lock_guard<std::mutex> guard(status_mutex);

    try
    {
        pfloppy->Close();
        delete pfloppy;
        floppy[drive_nr] = nullptr;
        driveStatus[drive_nr] = DISK_STAT_EMPTY;
    }
    catch (FlexException &)
    {
        // ignore errors
    }

    return 1;
} // umount_drive

bool E2floppy::mount_drive(const char *path, Word drive_nr, tMountOption option)
{
    int i = 0;
    FileContainerIfSector   *pfloppy = nullptr;

    if (drive_nr > 3 || path == nullptr || strlen(path) == 0)
    {
        return false;
    }

    // check if already mounted
    if (floppy[drive_nr] != nullptr)
    {
        return false;
    }

    track[drive_nr] = 1;    // position to a track != 0  !!!

    std::string containerPath(path);

    // first try with given path
#ifdef _WIN32
    std::string::iterator it;

    for (it = containerPath.begin(); it != containerPath.end(); ++it)
    {
        if (*it == '|')
        {
            *it = ':';
        }
        if (*it == '/')
        {
            *it = '\\';
        }
    }
#endif

    for (i = 0; i < 2; i++)
    {
#ifdef NAFS

        if (BDirectory::Exists(containerPath))
        {
            try
            {
                pfloppy = new NafsDirectoryContainer(containerPath.c_str());
            }
            catch (FlexException &)
            {
                // just ignore
            }
        }
        else
#endif
            if (option == MOUNT_RAM)
            {
                try
                {
                    pfloppy = new FlexRamFileContainer(containerPath.c_str(),
                                                       "rb+");
                }
                catch (FlexException &)
                {
                    try
                    {
                        pfloppy = new FlexRamFileContainer(
                                                   containerPath.c_str(), "rb");
                    }
                    catch (FlexException &)
                    {
                        // just ignore
                    }
                }
            }
            else
            {
                try
                {
                    pfloppy = new FlexFileContainer(containerPath.c_str(),
                                                    "rb+");
                }
                catch (FlexException &)
                {
                    try
                    {
                        pfloppy = new FlexFileContainer(containerPath.c_str(),
                                                        "rb");
                    }
                    catch (FlexException &)
                    {
                        // just ignore
                    }
                }
            }

        std::lock_guard<std::mutex> guard(status_mutex);
        floppy[drive_nr] = pfloppy;

        if (pfloppy != nullptr)
        {
            driveStatus[drive_nr] = DISK_STAT_ACTIVE;
            return true;
        }

        // second try with path within disk_dir directory
        containerPath = disk_dir;

        if (containerPath.length() > 0 &&
            containerPath[containerPath.length()-1] != PATHSEPARATOR)
        {
            containerPath += PATHSEPARATORSTRING;
        }

        containerPath += path;
    } // for

    return (pfloppy != nullptr);
} // mount_drive

void E2floppy::disk_directory(const char *x_disk_dir)
{
    disk_dir = x_disk_dir;
}

void E2floppy::mount_all_drives(std::string drive[])
{
    int i;

    for (i = 0; i < 4; i++)
    {
        mount_drive(drive[i].c_str(), i);
    }

    selected = 4;           // deselect all drives
    pfs = nullptr;
}  // mount_all_drives

bool E2floppy::umount_all_drives()
{
    Word i;
    bool result;

    result = true;

    for (i = 0; i < 4; i++)
        if (!umount_drive(i))
        {
            result = false;
        }

    return result;
}  // umount_all_drives

// get info for corresponding drive or nullptr
// the info string should not exceed 512 Bytes
// it is dynamically allocated and should be freed
// by the calling program

std::string E2floppy::drive_info(Word drive_nr)
{
    std::stringstream str;

    if (drive_nr <= 3)
    {
        std::lock_guard<std::mutex> guard(status_mutex);
        FileContainerIfSector *pfl;

        if ((pfl = floppy[drive_nr]) == nullptr)
        {
            str << "drive #" << drive_nr << " not ready" << std::endl;
        }
        else
        {
            FlexContainerInfo info;
            int trk, sec;

            try
            {
                pfl->GetInfo(info);
            }
            catch (FlexException &ex)
            {
                str << ex.what() << std::endl;
                return str.str().c_str();
            }

            info.GetTrackSector(&trk, &sec);
            str << "drive       #" << drive_nr << std::endl
                << "type:       " << info.GetTypeString().c_str() << std::endl
                << "name:       " << info.GetName() << std::endl
                << "path:       " << info.GetPath().c_str() << std::endl
                << "tracks:     " << trk << std::endl
                << "sectors:    " << sec << std::endl
                << "write-prot: " << (pfl->IsWriteProtected() ? "yes" : "no")
                << std::endl;
        }
    }

    return str.str().c_str();
}

const char *E2floppy::open_mode(char *path)
{
    int wp;    // write protect flag
    const char *mode;

    wp = access(path, W_OK);
    mode = wp ? "rb" : "rb+";
    return mode;
} // open_mode


bool E2floppy::update_all_drives()
{
    Word i;
    bool result;

    result = true;

    for (i = 0; i < 4; i++)
    {
        FileContainerIfSector *pfloppy = floppy[i];

        if (pfloppy == nullptr)
            // no error if drive not ready
        {
            continue;
        }

        if (!update_drive(i))
        {
            result = false;
        }
    }

    return result;
} // update_all_drives

bool E2floppy::update_drive(Word drive_nr)
{
    FileContainerIfSector *pfloppy;

    if (drive_nr > 3)
    {
        return false;
    }

    pfloppy = floppy[drive_nr];

    if (pfloppy == nullptr)
        // error if drive not ready
    {
        return false;
    }

    return 1;
} // update_drive

void E2floppy::resetIo()
{
    drisel = 0;
    Wd1793::resetIo();
}


Byte E2floppy::readIo(Word offset)
{
    if (offset <= 3)
    {
        return Wd1793::readIo(offset);
    }

    status = 0xff;  // unused is logical high

    if (!getSide())
    {
        status &= 0xfd;
    }

    if (!isIrq())
    {
        status &= 0xbf;
    }

    if (!isDrq())
    {
        status &= 0x7f;
    }

    return status;
} // readIo


void E2floppy::writeIo(Word offset, Byte val)
{
    if (offset <= 3)
    {
        Wd1793::writeIo(offset, val);
    }
    else
    {
        drisel = val;
        setSide((drisel & 0x10) ? 1 : 0);
        track[selected] = getTrack();

        switch (drisel & 0x0f)
        {
            case 0x01 :
                selected = 0;
                break;

            case 0x02 :
                selected = 1;
                break;

            case 0x04 :
                selected = 2;
                break;

            case 0x08 :
                selected = 3;
                break;

            default   :
                selected = 4;
        };

        pfs = floppy[selected];

        setTrack(track[selected]);
    }
} // writeIo


Byte E2floppy::readByte(Word index)
{
    if (pfs == nullptr)
    {
        return 0;
    }

    std::lock_guard<std::mutex> guard(status_mutex);

    if (index == pfs->GetBytesPerSector())
    {
        driveStatus[selected] = DISK_STAT_ACTIVE;

        if (!pfs->ReadSector((Byte *)&sector_buffer, getTrack(), getSector()))
        {
            setStatusRecordNotFound();
        }
    }

    return sector_buffer[SECTOR_SIZE - index];
}


void E2floppy::writeByte(Word index)
{
    std::lock_guard<std::mutex> guard(status_mutex);

    sector_buffer[SECTOR_SIZE - index] = getDataRegister();

    if (index == 1)
    {
        driveStatus[selected] = DISK_STAT_ACTIVE;

        if (!pfs->WriteSector((Byte *)&sector_buffer, getTrack(), getSector()))
        {
            setStatusRecordNotFound();
        }
    }
}


bool E2floppy::isRecordNotFound()
{
    if (pfs == nullptr)
    {
        return true;
    }

    return !pfs->IsSectorValid(getTrack(), getSector());
} // isRecordNotFound

bool E2floppy::isSeekError(Byte new_track)
{
    if (pfs == nullptr)
    {
        return true;
    }

    return !pfs->IsTrackValid(new_track);
} // isSeekError

bool E2floppy::isDriveReady() const
{
    return pfs != nullptr;
}  // isDriveReady

bool E2floppy::isWriteProtect()
{
    if (pfs == nullptr)
    {
        return true;
    }

    return pfs->IsWriteProtected();
}  // isWriteProtect

void E2floppy::get_drive_status(tDiskStatus stat[4])
{
    Word i;

    std::lock_guard<std::mutex> guard(status_mutex);

    for (i = 0; i < 4; ++i)
    {
        stat[i] = driveStatus[i];

        if (driveStatus[i] != DISK_STAT_EMPTY)
        {
            driveStatus[i] = DISK_STAT_INACTIVE;
        }
    }
}

bool E2floppy::format_disk(SWord trk, SWord sec, const char *name,
                           int type /* = TYPE_DSK_CONTAINER */)
{
    FileContainerIfSector *pfloppy = nullptr;

    try
    {
        switch (type)
        {
#ifdef NAFS

            case TYPE_NAFS_DIRECTORY:
                pfloppy = NafsDirectoryContainer::Create(disk_dir, name,
                          trk, sec, type);
                break;
#endif

            case TYPE_DSK_CONTAINER:
            case TYPE_FLX_CONTAINER:
                pfloppy = FlexFileContainer::Create(disk_dir, name,
                                                    trk, sec, type);
                break;
        }
    }
    catch (FlexException &)
    {
        printf("FlexException disk_dir=%s\n", disk_dir);
        return false;
    }

    delete pfloppy;
    return true;
} // format_disk

