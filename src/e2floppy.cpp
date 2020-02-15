/*
    e2floppy.cpp

    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2019  W. Schwotzer

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


E2floppy::E2floppy() : selected(4), pfs(nullptr)
{
    Word i;

    disk_dir = "";

    for (i = 0; i <= 4; i++)
    {
        track[i] = 1; // position all drives to track != 0  !!!
        drive_status[i] = DiskStatus::EMPTY;
    }

    memset(sector_buffer, 0, sizeof(sector_buffer));
} // E2floppy


E2floppy::~E2floppy()
{
    std::lock_guard<std::mutex> guard(status_mutex);

    for (Word drive_nr = 0; drive_nr < 4; drive_nr++)
    {
        if (floppy[drive_nr].get() != nullptr)
        {
            try
            {
                floppy[drive_nr].reset(nullptr);
                drive_status[drive_nr] = DiskStatus::EMPTY;
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
    if (drive_nr > 3 || (floppy[drive_nr].get() == nullptr))
    {
        return 0;
    }

    std::lock_guard<std::mutex> guard(status_mutex);

    try
    {
        floppy[drive_nr].reset(nullptr);
        drive_status[drive_nr] = DiskStatus::EMPTY;
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
    FileContainerIfSectorPtr pfloppy;

    if (drive_nr > 3 || path == nullptr || strlen(path) == 0)
    {
        return false;
    }

    // check if already mounted
    if (floppy[drive_nr].get() != nullptr)
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
                pfloppy = FileContainerIfSectorPtr(
                 new NafsDirectoryContainer(containerPath.c_str()));
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
                    pfloppy = FileContainerIfSectorPtr(
                     new FlexRamFileContainer(containerPath.c_str(), "rb+"));
                }
                catch (FlexException &)
                {
                    try
                    {
                        pfloppy = FileContainerIfSectorPtr(
                         new FlexRamFileContainer(containerPath.c_str(), "rb"));
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
                    pfloppy = FileContainerIfSectorPtr(
                      new FlexFileContainer(containerPath.c_str(), "rb+"));
                }
                catch (FlexException &)
                {
                    try
                    {
                        pfloppy = FileContainerIfSectorPtr(
                         new FlexFileContainer(containerPath.c_str(), "rb"));
                    }
                    catch (FlexException &)
                    {
                        // just ignore
                    }
                }
            }

        std::lock_guard<std::mutex> guard(status_mutex);
        floppy[drive_nr] = std::move(pfloppy);

        if (floppy[drive_nr].get() != nullptr)
        {
            drive_status[drive_nr] = DiskStatus::ACTIVE;
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

    return floppy[drive_nr].get() != nullptr;
} // mount_drive

void E2floppy::disk_directory(const char *x_disk_dir)
{
    disk_dir = x_disk_dir;
}

void E2floppy::mount_all_drives(std::string drive[])
{
    Word drive_nr;

    for (drive_nr = 0; drive_nr < 4; drive_nr++)
    {
        mount_drive(drive[drive_nr].c_str(), drive_nr);
    }

    selected = 4;           // deselect all drives
    pfs = nullptr;
}  // mount_all_drives

bool E2floppy::umount_all_drives()
{
    Word drive_nr;
    bool result;

    result = true;

    for (drive_nr = 0; drive_nr < 4; drive_nr++)
        if (!umount_drive(drive_nr))
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
    std::stringstream stream;

    if (drive_nr <= 3)
    {
        std::lock_guard<std::mutex> guard(status_mutex);

        if (floppy[drive_nr].get() == nullptr)
        {
            stream << "drive #" << drive_nr << " not ready" << std::endl;
        }
        else
        {
            FlexContainerInfo info;
            int trk, sec;
            bool is_write_protected = false;

            try
            {
                floppy[drive_nr]->GetInfo(info);
                is_write_protected = floppy[drive_nr]->IsWriteProtected();
            }
            catch (FlexException &ex)
            {
                stream << ex.what() << std::endl;
                return stream.str().c_str();
            }

            info.GetTrackSector(trk, sec);
            stream << "drive       #" << drive_nr << std::endl
                << "type:       " << info.GetTypeString().c_str() << std::endl
                << "name:       " << info.GetName() << " #" <<
                                     info.GetNumber() << std::endl
                << "path:       " << info.GetPath().c_str() << std::endl
                << "tracks:     " << trk << std::endl
                << "sectors:    " << sec << std::endl
                << "write-prot: " << (is_write_protected ? "yes" : "no")
                << std::endl;
        }
    }

    return stream.str().c_str();
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
    Word drive_nr;
    bool result = true;

    for (drive_nr = 0; drive_nr < 4; drive_nr++)
    {
        if (floppy[drive_nr].get() == nullptr)
        {
            // no error if drive not ready
            continue;
        }

        if (!update_drive(drive_nr))
        {
            result = false;
        }
    }

    return result;
} // update_all_drives

bool E2floppy::update_drive(Word drive_nr)
{
    if (drive_nr > 3)
    {
        return false;
    }

    if (floppy[drive_nr].get() == nullptr)
    {
        // error if drive not ready
        return false;
    }

    return true;
} // update_drive

void E2floppy::resetIo()
{
    Wd1793::resetIo();
}

void E2floppy::select_drive(Byte new_selected)
{
    if (new_selected > 4)
    {
        new_selected = 4;
    }

    if (new_selected != selected)
    {
        track[selected] = getTrack();
        selected = new_selected;
        pfs = floppy[selected].get();
        setTrack(track[selected]);
    }
}

Byte E2floppy::readByte(Word index)
{
    if (pfs == nullptr)
    {
        return 0;
    }

    std::lock_guard<std::mutex> guard(status_mutex);

    if (index == pfs->GetBytesPerSector())
    {
        drive_status[selected] = DiskStatus::ACTIVE;

        if (!pfs->ReadSector(&sector_buffer[0], getTrack(), getSector()))
        {
            setStatusReadError();
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
        drive_status[selected] = DiskStatus::ACTIVE;

        if (!pfs->WriteSector(&sector_buffer[0], getTrack(), getSector()))
        {
            setStatusWriteError();
        }
    }
}


bool E2floppy::isRecordNotFound() const
{
    if (pfs == nullptr)
    {
        return true;
    }

    return !pfs->IsSectorValid(getTrack(), getSector());
} // isRecordNotFound

bool E2floppy::isSeekError(Byte new_track) const
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

bool E2floppy::isWriteProtect() const
{
    if (pfs == nullptr)
    {
        return true;
    }

    return pfs->IsWriteProtected();
}  // isWriteProtect

void E2floppy::get_drive_status(DiskStatus stat[4])
{
    Word i;

    std::lock_guard<std::mutex> guard(status_mutex);

    for (i = 0; i < 4; ++i)
    {
        stat[i] = drive_status[i];

        if (drive_status[i] != DiskStatus::EMPTY)
        {
            drive_status[i] = DiskStatus::INACTIVE;
        }
    }
}

bool E2floppy::format_disk(SWord trk, SWord sec, const char *name,
                           int type /* = TYPE_DSK_CONTAINER */)
{
    FileContainerIfSectorPtr pfloppy;

    try
    {
        switch (type)
        {
#ifdef NAFS

            case TYPE_NAFS_DIRECTORY:
                pfloppy = FileContainerIfSectorPtr(
                NafsDirectoryContainer::Create(disk_dir, name, trk, sec, type));
                break;
#endif

            case TYPE_DSK_CONTAINER:
            case TYPE_FLX_CONTAINER:
                pfloppy = FileContainerIfSectorPtr(
                FlexFileContainer::Create(disk_dir, name, trk, sec, type));
                break;
        }
    }
    catch (FlexException &)
    {
        printf("FlexException disk_dir=%s\n", disk_dir);
        return false;
    }

    return true;
} // format_disk

Word E2floppy::getBytesPerSector() const
{
    if (pfs == nullptr)
    {
        return 0U;
    }

    return pfs->GetBytesPerSector();
}

