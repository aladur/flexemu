/*
    e2floppy.cpp

    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2024  W. Schwotzer

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
#include "crc.h"
#include "soptions.h"


E2floppy::E2floppy(const struct sOptions &x_options)
    : selected(MAX_DRIVES)
    , pfs(nullptr)
    , writeTrackState(WriteTrackState::Inactive)
    , options(x_options)
{
    for (auto i = 0U; i <= MAX_DRIVES; i++)
    {
        track[i] = 1; // position all drives to track != 0 !!!
        drive_status[i] = DiskStatus::EMPTY;
    }

    memset(sector_buffer, 0, sizeof(sector_buffer));
} // E2floppy


E2floppy::~E2floppy()
{
    std::lock_guard<std::mutex> guard(status_mutex);

    for (auto drive_nr = 0U; drive_nr < MAX_DRIVES; drive_nr++)
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
    if (drive_nr >= MAX_DRIVES || (floppy[drive_nr].get() == nullptr))
    {
        return false;
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

    return true;
} // umount_drive

bool E2floppy::mount_drive(const std::string &path,
                           Word drive_nr,
                           tMountOption option)
{
    if (drive_nr >= MAX_DRIVES || path.empty())
    {
        return false;
    }

    // check if already mounted
    if (floppy[drive_nr].get() != nullptr)
    {
        return false;
    }

    track[drive_nr] = 1; // position to a track != 0 !!!

    // Intentionally use value argument, it may be changed on Windows.
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    auto TryMount = [&](std::string containerPath) -> bool
    {
        FileContainerIfSectorPtr pfloppy;

#ifdef _WIN32
        for (auto it = containerPath.begin(); it != containerPath.end(); ++it)
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
        if (BDirectory::Exists(containerPath))
        {
            if (options.isDirectoryDiskActive)
            {
                try
                {
                    pfloppy = FileContainerIfSectorPtr(
                     new NafsDirectoryContainer(
                         containerPath,
                         options.fileTimeAccess,
                         options.directoryDiskTracks,
                         options.directoryDiskSectors));
                }
                catch (FlexException &)
                {
                    // just ignore
                }
            }
        }
        else
        {
            struct stat sbuf;
            bool fileExists = !stat(containerPath.c_str(), &sbuf);

            // Empty or non existing files are only mounted if
            // option canFormatDrive is set.
            if (!options.canFormatDrive[drive_nr] &&
                (!fileExists ||
                (fileExists && S_ISREG(sbuf.st_mode) && sbuf.st_size == 0)))
            {
                return false;
            }
            // A file which does not exist or has file size zero
            // is marked as unformatted.
            bool is_formatted = !stat(containerPath.c_str(), &sbuf) &&
                                (S_ISREG(sbuf.st_mode) && sbuf.st_size);

            if (is_formatted && option == MOUNT_RAM)
            {
                try
                {
                    pfloppy = FileContainerIfSectorPtr(
                     new FlexRamFileContainer(containerPath.c_str(), "rb+",
                                              options.fileTimeAccess));
                }
                catch (FlexException &)
                {
                    try
                    {
                        pfloppy = FileContainerIfSectorPtr(
                         new FlexRamFileContainer(containerPath.c_str(), "rb",
                                                  options.fileTimeAccess));
                    }
                    catch (FlexException &)
                    {
                        // just ignore
                    }
                }
            }
            else
            {
                const char *mode = is_formatted ? "rb+" : "wb+";
                try
                {
                    pfloppy = FileContainerIfSectorPtr(
                      new FlexFileContainer(containerPath, mode,
                                            options.fileTimeAccess));
                }
                catch (FlexException &)
                {
                    if (is_formatted)
                    {
                        try
                        {
                            pfloppy = FileContainerIfSectorPtr(
                             new FlexFileContainer(containerPath, "rb",
                                                   options.fileTimeAccess));
                        }
                        catch (FlexException &)
                        {
                            // just ignore
                        }
                    }
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

        return false;
    };

    // first try with given path
    if (TryMount(path))
    {
        return true;
    }

    if (isAbsolutePath(path))
    {
        return false;
    }

    // If path is relative, second try with full path in disk_dir directory
    auto fullPath = disk_dir;

    if (fullPath.length() > 0 && fullPath[fullPath.length()-1] != PATHSEPARATOR)
    {
        fullPath += PATHSEPARATORSTRING;
    }
    fullPath += path;

    return TryMount(fullPath);
} // mount_drive

void E2floppy::disk_directory(const std::string &x_disk_dir)
{
    disk_dir = x_disk_dir;
}

void E2floppy::mount_all_drives(std::string drive[])
{
    for (Word drive_nr = 0U; drive_nr < MAX_DRIVES; drive_nr++)
    {
        mount_drive(drive[drive_nr], drive_nr);
    }

    selected = MAX_DRIVES; // deselect all drives
    pfs = nullptr;
}  // mount_all_drives

bool E2floppy::umount_all_drives()
{
    bool result = true;

    for (Word drive_nr = 0U; drive_nr < MAX_DRIVES; drive_nr++)
    {
        if (!umount_drive(drive_nr))
        {
            result = false;
        }
    }

    return result;
}  // umount_all_drives

std::string E2floppy::drive_info_string(Word drive_nr)
{
    FlexContainerInfo info = drive_info(drive_nr);
    std::stringstream stream;

    if (!info.IsValid())
    {
        stream << "drive #" << drive_nr << " not ready" << std::endl;
    }
    else
    {
        int trk;
        int sec;
        bool is_write_protected = info.GetIsWriteProtected();

        info.GetTrackSector(trk, sec);
        stream << "drive       #" << drive_nr << std::endl
            << "type:       " << info.GetTypeString().c_str() << std::endl;

        if (info.GetIsFlexFormat())
        {
            stream << "name:       " << info.GetName() << " #" <<
                                        info.GetNumber() << std::endl;
        }
        stream << "path:       " << info.GetPath().c_str() << std::endl
               << "tracks:     " << trk << std::endl
               << "sectors:    " << sec << std::endl
               << "write-prot: " << (is_write_protected ? "yes" : "no")
               << std::endl
               << "FLEX format:" << (info.GetIsFlexFormat() ? "yes" : "no")
               << std::endl;
        if (info.GetType() & TYPE_DSK_CONTAINER)
        {
            auto header = info.GetJvcFileHeader();

            stream << "JVC header: ";
            if (header.empty())
            {
                stream << "none";
            }
            else
            {
                bool isAppend = false;
                for (const auto value : header)
                {
                    stream << ((isAppend) ? "," : "") << (Word)value;
                    isAppend = true;
                }
            }
            stream << std::endl;
        }
    }

    return stream.str();
}

// get info for corresponding drive. If drive is not ready the result is empty.
FlexContainerInfo E2floppy::drive_info(Word drive_nr)
{
    FlexContainerInfo info;

    if (drive_nr < MAX_DRIVES)
    {
        std::lock_guard<std::mutex> guard(status_mutex);

        if (floppy[drive_nr].get() == nullptr)
        {
            return info;
        }

        try
        {
            floppy[drive_nr]->GetInfo(info);
        }
        catch (FlexException &)
        {
            return FlexContainerInfo();
        }
    }

    return info;
}

bool E2floppy::sync_all_drives(tMountOption option)
{
    bool result = true;

    for (Word drive_nr = 0U; drive_nr < MAX_DRIVES; ++drive_nr)
    {
        if (floppy[drive_nr].get() == nullptr)
        {
            // no error if drive not ready
            continue;
        }

        if (!sync_drive(drive_nr, option))
        {
            result = false;
        }
    }

    return result;
} // sync_all_drives

bool E2floppy::sync_drive(Word drive_nr, tMountOption option)
{
    bool result = true;

    // Return false if invalid drive number or drive not ready.
    if (drive_nr >= MAX_DRIVES || floppy[drive_nr].get() == nullptr)
    {
        return false;
    }

    if (floppy[drive_nr]->GetContainerType() & TYPE_DIRECTORY)
    {
        auto path = floppy[drive_nr]->GetPath();
        result = umount_drive(drive_nr);
        result &= mount_drive(path, drive_nr, option);
    }

    return result;
} // sync_drive

void E2floppy::resetIo()
{
    Wd1793::resetIo();
}

void E2floppy::select_drive(Byte new_selected)
{
    if (new_selected > MAX_DRIVES)
    {
        new_selected = MAX_DRIVES;
    }

    if (new_selected != selected)
    {
        track[selected] = getTrack();
        selected = new_selected;
        pfs = floppy[selected].get();
        setTrack(track[selected]);
    }
}

Byte E2floppy::readByte(Word index, Byte command_un)
{
    if (pfs != nullptr)
    {
        std::lock_guard<std::mutex> guard(status_mutex);

        switch (command_un)
        {
            case CMD_READSECTOR:
            case CMD_READSECTOR_MULT:
                return readByteInSector(index);

            case CMD_READTRACK:
                return readByteInTrack(index);

            case CMD_READADDRESS:
                return readByteInAddress(index);
        }
    }

    return 0U;
}

Byte E2floppy::readByteInSector(Word index)
{
    if (index == pfs->GetBytesPerSector())
    {
        drive_status[selected] = DiskStatus::ACTIVE;

        if (!pfs->ReadSector(&sector_buffer[0], getTrack(), getSector(),
                             getSide() ? 1 : 0))
        {
            setStatusReadError();
        }
    }

    return sector_buffer[pfs->GetBytesPerSector() - index];
}

// Unfinished feature.
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
Byte E2floppy::readByteInTrack(Word)
{
    // TODO unfinished
    return 0U;
}

Byte E2floppy::readByteInAddress(Word index)
{
    if (index == 6)
    {
        Crc<Word> crc16(0x1021);

        sector_buffer[0] = getTrack();
        sector_buffer[1] = getSide() ? 1 : 0;
        sector_buffer[2] = 1; // sector address
        sector_buffer[3] = getSizeCode(); // sector sizecode

        auto crc = crc16.GetResult(&sector_buffer[0], &sector_buffer[4]);

        sector_buffer[4] = static_cast<Byte>(crc >> 8);
        sector_buffer[5] = static_cast<Byte>(crc & 0xFF);
    }

    return sector_buffer[6 - index];
}

bool E2floppy::startCommand(Byte command_un)
{
    // Special actions may be needed when starting a command.
    // command_un is the upper nibble of the WD1793 command.
    if (command_un == CMD_WRITETRACK)
    {
        // CMD_WRITETRACK means a new disk has to be formatted
        // within the emulation.
        // Only unformatted file containers, if IsFlexFormat()
        // returns false, can be formatted.
        if (pfs->IsFlexFormat() || !options.canFormatDrive[selected])
        {
            return false;
        }

        writeTrackState = WriteTrackState::Inactive;
    }

    return true;
}

void E2floppy::writeByte(Word &index, Byte command_un)
{
    if (pfs != nullptr)
    {
        std::lock_guard<std::mutex> guard(status_mutex);

        switch (command_un)
        {
            case CMD_WRITESECTOR:
            case CMD_WRITESECTOR_MULT:
                writeByteInSector(index);
                break;

            case CMD_WRITETRACK:
                writeByteInTrack(index);
                break;
        }
    }
}

void E2floppy::writeByteInTrack(Word &index)
{
    Word i;
    int sizecode;


    switch (writeTrackState)
    {
        case WriteTrackState::Inactive:
            drive_status[selected] = DiskStatus::ACTIVE;
            writeTrackState = WriteTrackState::WaitForIdAddressMark;
            index = 256U;
            break;

        case WriteTrackState::WaitForIdAddressMark:
            if (getDataRegister() == ID_ADDRESS_MARK)
            {
                writeTrackState = WriteTrackState::IdAddressMark;
                offset = sizeof(idAddressMark);
            }
            break;

        case WriteTrackState::IdAddressMark:
            idAddressMark[sizeof(idAddressMark) - offset] = getDataRegister();
            if (--offset == 0)
            {
                writeTrackState = WriteTrackState::WaitForDataAddressMark;
                index = 64U;
            }
            break;

        case WriteTrackState::WaitForDataAddressMark:
            if (getDataRegister() == DATA_ADDRESS_MARK)
            {
                writeTrackState = WriteTrackState::WriteData;
                sizecode = idAddressMark[3] & 0x03;
                offset = ::getBytesPerSector(sizecode);
                index = offset + 2;
            }
            break;

        case WriteTrackState::WriteData:
            sizecode = idAddressMark[3] & 0x03;
            i = ::getBytesPerSector(sizecode) - offset;
            sector_buffer[i] = getDataRegister();
            if (--offset == 0U)
            {
                pfs->FormatSector(&sector_buffer[0],
                        idAddressMark[0], idAddressMark[2],
                        idAddressMark[1], idAddressMark[3] & 0x03);

                writeTrackState = WriteTrackState::WaitForCrc;
                index = 2U;
            }
            break;

        case WriteTrackState::WaitForCrc:
            if (getDataRegister() == TWO_CRCS)
            {
                drive_status[selected] = DiskStatus::ACTIVE;
                writeTrackState = WriteTrackState::WaitForIdAddressMark;
                index = 96U;
            }
            break;
    }
}

void E2floppy::writeByteInSector(Word index)
{
    sector_buffer[pfs->GetBytesPerSector() - index] = getDataRegister();

    if (index == 1)
    {
        drive_status[selected] = DiskStatus::ACTIVE;

        if (!pfs->WriteSector(&sector_buffer[0], getTrack(), getSector(),
                              getSide() ? 1 : 0))
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

void E2floppy::get_drive_status(DiskStatus stat[MAX_DRIVES])
{
    std::lock_guard<std::mutex> guard(status_mutex);

    for (auto i = 0U; i < MAX_DRIVES; ++i)
    {
        stat[i] = drive_status[i];

        if (drive_status[i] != DiskStatus::EMPTY)
        {
            drive_status[i] = DiskStatus::INACTIVE;
        }
    }
}

bool E2floppy::format_disk(SWord trk, SWord sec,
                           const std::string &name,
                           int fmt)
{
    FileContainerIfSectorPtr pfloppy;
    FileTimeAccess fileTimeAccess = FileTimeAccess::NONE;

    try
    {
        switch (fmt)
        {
            case TYPE_NAFS_DIRECTORY:
                if (options.isDirectoryDiskActive)
                {
                    pfloppy = FileContainerIfSectorPtr(
                        NafsDirectoryContainer::Create(
                            disk_dir, name, options.fileTimeAccess, trk, sec,
                            fmt));
                }
                break;

            case TYPE_DSK_CONTAINER:
            case TYPE_FLX_CONTAINER:
                pfloppy = FileContainerIfSectorPtr(
                FlexFileContainer::Create(
                    disk_dir, name, fileTimeAccess, trk, sec, fmt));
                break;
        }
    }
    catch (FlexException &)
    {
        return false;
    }

    return true;
}

Word E2floppy::getBytesPerSector() const
{
    if (pfs == nullptr)
    {
        return 0U;
    }

    return static_cast<Word>(pfs->GetBytesPerSector());
}

Byte E2floppy::getSizeCode() const
{
    switch (getBytesPerSector())
    {
        case 256U: return 1U;
        case 128U: return 0U;
        case 512U: return 2U;
        case 1024U: return 3U;
        default: return 0U;
    }
}

