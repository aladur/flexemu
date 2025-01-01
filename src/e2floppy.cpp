/*
    e2floppy.cpp

    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2025  W. Schwotzer

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
#include "filecntb.h"
#include "e2floppy.h"
#include "ffilecnt.h"
#include "rfilecnt.h"
#include "ndircont.h"
#include "fcinfo.h"
#include "flexerr.h"
#include "bdir.h"
#include "crc.h"
#include "soptions.h"
#include <cassert>
#include <array>


E2floppy::E2floppy(const struct sOptions &p_options)
    : options(p_options)
{
    assert(track.size() == drive_status.size());
    assert(track.size() == floppy.size());

    for (auto i = 0U; i <= MAX_DRIVES; ++i)
    {
        track[i] = 1; // position all drives to track != 0 !!!
        drive_status[i] = DiskStatus::EMPTY;
    }
}

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
}

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
}

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
        IFlexDiskBySectorPtr pfloppy;

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
                    pfloppy = IFlexDiskBySectorPtr(
                     new FlexDirectoryDiskBySector(
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
            struct stat sbuf{};
            bool fileExists = !stat(containerPath.c_str(), &sbuf);

            // Empty or non existing files are only mounted if
            // option canFormatDrive is set.
            if (!options.canFormatDrives[drive_nr] &&
                (!fileExists ||
                (fileExists && S_ISREG(sbuf.st_mode) && sbuf.st_size == 0)))
            {
                return false;
            }
            // A file which does not exist or has file size zero
            // is marked as unformatted.
            bool is_formatted = !stat(containerPath.c_str(), &sbuf) &&
                                (S_ISREG(sbuf.st_mode) && sbuf.st_size);
            auto mode = std::ios::in | std::ios::out | std::ios::binary;

            if (is_formatted && option == MOUNT_RAM)
            {
                try
                {
                    pfloppy = IFlexDiskBySectorPtr(
                     new FlexRamDisk(containerPath, mode,
                                     options.fileTimeAccess));
                }
                catch (FlexException &)
                {
                    try
                    {
                        mode &= ~std::ios::out;
                        pfloppy = IFlexDiskBySectorPtr(
                         new FlexRamDisk(containerPath, mode,
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
                if (!is_formatted)
                {
                    mode |= std::ios::trunc;
                }
                try
                {
                    pfloppy = IFlexDiskBySectorPtr(
                      new FlexDisk(containerPath, mode,
                                   options.fileTimeAccess));
                }
                catch (FlexException &)
                {
                    if (is_formatted)
                    {
                        try
                        {
                            mode &= ~std::ios::out;
                            pfloppy = IFlexDiskBySectorPtr(
                             new FlexDisk(containerPath, mode,
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

    if (flx::isAbsolutePath(path))
    {
        return false;
    }

    // If path is relative, second try with full path in disk_dir directory
    auto fullPath = disk_dir;

    if (!fullPath.empty() && fullPath[fullPath.length()-1] != PATHSEPARATOR)
    {
        fullPath += PATHSEPARATORSTRING;
    }
    fullPath += path;

    return TryMount(fullPath);
}

void E2floppy::disk_directory(const std::string &p_disk_dir)
{
    disk_dir = p_disk_dir;
}

void E2floppy::mount_all_drives(
        const std::array<std::string, MAX_DRIVES> &drives)
{
    Word drive_nr = 0U;

    for (const auto &drive : drives)
    {
        mount_drive(drive, drive_nr++);
    }

    selected = MAX_DRIVES; // deselect all drives
    pfs = nullptr;
}

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
}

std::string E2floppy::drive_attributes_string(Word drive_nr)
{
    auto diskAttributes = drive_attributes(drive_nr);
    std::stringstream stream;

    if (!diskAttributes.IsValid())
    {
        stream << "drive #" << drive_nr << " not ready\n";
    }
    else
    {
        int trk;
        int sec;
        bool is_write_protected = diskAttributes.GetIsWriteProtected();
        bool is_flex_format = diskAttributes.GetIsFlexFormat();

        diskAttributes.GetTrackSector(trk, sec);
        stream << "drive       #" << drive_nr << '\n'
            << "type:       " << diskAttributes.GetTypeString().c_str()
            << '\n';

        if (diskAttributes.GetIsFlexFormat())
        {
            stream << "name:       " << diskAttributes.GetName() << " #" <<
                                        diskAttributes.GetNumber() << '\n';
        }
        stream << "path:       " << diskAttributes.GetPath().c_str() << '\n'
               << "tracks:     " << trk << '\n'
               << "sectors:    " << sec << '\n'
               << "write-prot: " << (is_write_protected ? "yes" : "no") << '\n'
               << "FLEX format:" << (is_flex_format ? "yes" : "no")
               << '\n';
        if (diskAttributes.GetType() == DiskType::DSK)
        {
            auto header = diskAttributes.GetJvcFileHeader();

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
                    stream << ((isAppend) ? "," : "") <<
                              static_cast<Word>(value);
                    isAppend = true;
                }
            }
            stream << '\n';
        }
    }

    return stream.str();
}

// get attributes for corresponding drive. If drive is not ready the result
// is empty.
FlexDiskAttributes E2floppy::drive_attributes(Word drive_nr)
{
    FlexDiskAttributes diskAttributes;

    if (drive_nr < MAX_DRIVES)
    {
        std::lock_guard<std::mutex> guard(status_mutex);

        if (floppy[drive_nr].get() == nullptr)
        {
            return diskAttributes;
        }

        try
        {
            floppy[drive_nr]->GetDiskAttributes(diskAttributes);
        }
        catch (FlexException &)
        {
            return {};
        }
    }

    return diskAttributes;
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
}

bool E2floppy::sync_drive(Word drive_nr, tMountOption option)
{
    bool result = true;

    // Return false if invalid drive number or drive not ready.
    if (drive_nr >= MAX_DRIVES || floppy[drive_nr].get() == nullptr)
    {
        return false;
    }

    if (floppy[drive_nr]->GetFlexDiskType() == DiskType::Directory)
    {
        auto path = floppy[drive_nr]->GetPath();
        result = umount_drive(drive_nr);
        result &= mount_drive(path, drive_nr, option);
    }

    return result;
}

void E2floppy::resetIo()
{
    Wd1793::resetIo();
}

void E2floppy::select_drive(Byte new_selected)
{
    new_selected = std::min(new_selected, MAX_DRIVES);

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

        if (!pfs->ReadSector(sector_buffer.data(), getTrack(), getSector(),
                             getSide() ? 1 : 0))
        {
            setStatusReadError();
        }
    }

    return sector_buffer[pfs->GetBytesPerSector() - index];
}

// Unfinished feature.
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
Byte E2floppy::readByteInTrack(Word /*index*/)
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

        auto crc =
            crc16.GetResult(sector_buffer.data(), sector_buffer.data() + 4U);

        sector_buffer[4] = static_cast<Byte>(crc >> 8U);
        sector_buffer[5] = static_cast<Byte>(crc);
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
        if (pfs->IsFlexFormat() || !options.canFormatDrives[selected])
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
    unsigned sizecode;


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
                offset = static_cast<Word>(idAddressMark.size());
            }
            break;

        case WriteTrackState::IdAddressMark:
            idAddressMark[idAddressMark.size() - offset] = getDataRegister();
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
                sizecode = idAddressMark[Id::SizeCode] & 0x03U;
                offset = ::getBytesPerSector(sizecode);
                index = offset + 2;
            }
            break;

        case WriteTrackState::WriteData:
            sizecode = idAddressMark[Id::SizeCode] & 0x03U;
            i = ::getBytesPerSector(sizecode) - offset;
            sector_buffer[i] = getDataRegister();
            if (--offset == 0U)
            {
                pfs->FormatSector(sector_buffer.data(),
                        idAddressMark[Id::Track],
                        idAddressMark[Id::Sector],
                        idAddressMark[Id::Side],
                        idAddressMark[Id::SizeCode] & 0x03U);

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

        if (!pfs->WriteSector(sector_buffer.data(), getTrack(), getSector(),
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
}

bool E2floppy::isSeekError(Byte new_track) const
{
    if (pfs == nullptr)
    {
        return true;
    }

    return !pfs->IsTrackValid(new_track);
}

bool E2floppy::isDriveReady() const
{
    return pfs != nullptr;
}

bool E2floppy::isWriteProtect() const
{
    if (pfs == nullptr)
    {
        return true;
    }

    return pfs->IsWriteProtected();
}

void E2floppy::get_drive_status(std::array<DiskStatus, MAX_DRIVES> &stat)
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
                           DiskType disk_type)
{
    IFlexDiskBySectorPtr pfloppy;
    FileTimeAccess fileTimeAccess = FileTimeAccess::NONE;
    auto path = disk_dir;
    if (!flx::endsWithPathSeparator(path))
    {
        path += PATHSEPARATORSTRING;
    }
    path += name;

    try
    {
        switch (disk_type)
        {
            case DiskType::Directory:
                if (options.isDirectoryDiskActive)
                {
                    pfloppy = IFlexDiskBySectorPtr(
                        FlexDirectoryDiskBySector::Create(
                            path, options.fileTimeAccess, trk, sec, disk_type));
                }
                break;

            case DiskType::DSK:
            case DiskType::FLX:
                pfloppy = IFlexDiskBySectorPtr(
                    FlexDisk::Create(path, fileTimeAccess, trk, sec,
                        disk_type));
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

IFlexDiskBySector const *E2floppy::get_drive(Word drive_nr) const
{
    if (drive_nr >= MAX_DRIVES)
    {
        return nullptr;
    }

    return floppy[drive_nr].get();
}

