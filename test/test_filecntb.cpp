/*
    test_filecntb.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2024-2026  W. Schwotzer

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


// Test IFlexDiskBase interface.
//
#include "gtest/gtest.h"
#include "misc1.h"
#include "fcinfo.h"
#include "ffilecnt.h"
#include "dircont.h"
#include "ndircont.h"
#include <ios>
#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <filesystem>


namespace fs = std::filesystem;
using FlexDiskBasePtr = std::unique_ptr<IFlexDiskBase>;

class test_IFlexDiskBase : public ::testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    static IFlexDiskBase *cast(FlexDisk *disk)
    {
        return static_cast<IFlexDiskBase *>(
                static_cast<IFlexDiskBySector *>(disk));
    }

    static IFlexDiskBase *cast(FlexDirectoryDiskByFile *dirDisk)
    {
        return static_cast<IFlexDiskBase *>(
                static_cast<IFlexDiskByFile *>(dirDisk));
    }

    static IFlexDiskBase *cast(FlexDirectoryDiskBySector *dirDisk)
    {
        return static_cast<IFlexDiskBase *>(
                static_cast<IFlexDiskBySector *>(dirDisk));
    }

    static void CheckAttributes(
            const FlexDiskAttributes &diskInfo,
            const fs::path &path,
            unsigned diskNumber,
            unsigned jvcHeaderSize,
            int expectedTracks,
            int expectedSectors,
            int expectedSectorsTrack0,
            int expectedSectorSize,
            DiskType type,
            DiskOptions options,
            bool isWriteProtected)
    {
        auto diskName = flx::toupper(path.filename().u8string());
        auto pos = diskName.find_first_of('.');
        if (pos != std::string::npos)
        {
            diskName = diskName.substr(0U, pos);
        }
        if (diskName.size() > FLEX_DISKNAME_LENGTH)
        {
            diskName.resize(FLEX_DISKNAME_LENGTH);
        }
        if (diskName.empty())
        {
            diskName = "FLEXDISK";
        }

        if (type == DiskType::DSK || type == DiskType::FLX ||
            type == DiskType::IMA || type == DiskType::Directory)
        {
            ASSERT_TRUE(diskInfo.GetIsFlexFormat());
            EXPECT_EQ(diskInfo.GetDate(), BDate::Now());
            if ((options & DiskOptions::HasSectorIF) != DiskOptions::NONE)
            {
                EXPECT_EQ(diskInfo.GetFree(),
                  (expectedTracks - 1) * expectedSectors * expectedSectorSize);
                const auto file_size = (type == DiskType::Directory) ?
                    0 : fs::file_size(path);
                const auto headerSize = (type == DiskType::FLX) ? 16 :
                    ((type == DiskType::DSK) ?
                     (file_size % expectedSectorSize) : 0);
                const auto fileSize = ((expectedTracks - 1) * expectedSectors +
                        expectedSectorsTrack0) * expectedSectorSize +
                        headerSize;
                if (type != DiskType::Directory)
                {
                    EXPECT_EQ(diskInfo.GetTotalSize(), file_size);
                }
                EXPECT_EQ(diskInfo.GetTotalSize(), fileSize);
            }
        }
        EXPECT_EQ(diskInfo.GetOptions(), options);
        EXPECT_EQ(diskInfo.GetPath(), path);
        EXPECT_EQ(diskInfo.GetDiskname(), diskName);
        EXPECT_EQ(diskInfo.GetNumber(), diskNumber);
        int tracks = 0;
        int sectors = 0;
        int sectors0 = 0;
        diskInfo.GetTrackSector(tracks, sectors, sectors0);
        EXPECT_EQ(tracks, expectedTracks);
        EXPECT_EQ(sectors, expectedSectors);
        EXPECT_EQ(sectors0, expectedSectorsTrack0);
        EXPECT_EQ(diskInfo.GetSectorSize(), expectedSectorSize);
        EXPECT_EQ(diskInfo.GetIsWriteProtected(), isWriteProtected);
        EXPECT_EQ(diskInfo.GetJvcFileHeader().size(), jvcHeaderSize);
        EXPECT_TRUE(diskInfo.IsValid());
        EXPECT_EQ(diskInfo.GetType(), type);
    }
};

// test IFlexDiskBase interface for class FlexDisk.
TEST_F(test_IFlexDiskBase, fcts_FlexDisk)
{
    const auto path1 =
        (fs::temp_directory_path() / u8"flexdisk.dsk").u8string();
    const auto ft_access = FileTimeAccess::NONE;
    const auto mode = std::ios::in | std::ios::binary;
    FlexDiskBasePtr disk{};

    for (int tracks = 2; tracks <= 255; tracks += 5)
    {
        auto type = DiskType::DSK;
        int sectors = std::max(tracks / 3, 6);
        auto options = DiskOptions::HasSectorIF;
        bool isWP = (tracks == 40);

        auto *pdisk =
            FlexDisk::Create(path1, ft_access, tracks, sectors, type);
        disk.reset(cast(pdisk));
        if (isWP)
        {
            disk.reset();
            disk.reset(cast(new FlexDisk(path1, mode, ft_access)));
        }

        ASSERT_NE(disk.get(), nullptr);
        EXPECT_TRUE(static_cast<bool>(disk));
        EXPECT_EQ(disk->IsWriteProtected(), isWP);
        EXPECT_EQ(disk->GetFlexDiskType(), type);
        EXPECT_EQ(disk->GetPath().u8string(), path1);
        FlexDiskAttributes diskInfo{};
        ASSERT_TRUE(disk->GetDiskAttributes(diskInfo));
        CheckAttributes(diskInfo, path1, 0U, 0U, tracks, sectors, sectors,
                SECTOR_SIZE, type, options, isWP);
        disk.reset();
        fs::remove(path1);
    }

    const auto path2 = fs::temp_directory_path() / u8"flexdisk.flx";

    for (int tracks = 2; tracks <= 255; tracks += 5)
    {
        auto type = DiskType::FLX;
        int sectors = std::max(tracks / 3, 6);
        int sectors0 = getTrack0SectorCount(type, tracks, sectors);
        auto options = DiskOptions::HasSectorIF;
        bool isWP = (tracks == 40);

        auto *pdisk =
            FlexDisk::Create(path2, ft_access, tracks, sectors, type);
        disk.reset(cast(pdisk));
        if (isWP)
        {
            disk.reset();
            disk.reset(cast(new FlexDisk(path2, mode, ft_access)));
        }

        ASSERT_NE(disk.get(), nullptr);
        EXPECT_TRUE(static_cast<bool>(disk));
        EXPECT_EQ(disk->IsWriteProtected(), tracks == 40);
        EXPECT_EQ(disk->GetFlexDiskType(), type);
        EXPECT_EQ(disk->GetPath(), path2);
        FlexDiskAttributes diskInfo{};
        ASSERT_TRUE(disk->GetDiskAttributes(diskInfo));
        CheckAttributes(diskInfo, path2, 0U, 0U, tracks, sectors,
                sectors0, SECTOR_SIZE, type, options, isWP);
        disk.reset();
        fs::remove(path2);
    }

    const auto path3 = fs::temp_directory_path() / u8"flexdisk.ima";

    for (const auto trk_sec : flex_formats)
    {
        auto type = DiskType::IMA;
        int tracks = trk_sec.trk + 1;
        int sectors = trk_sec.sec;
        int sectors0 = getTrack0SectorCount(type, tracks, sectors);
        auto options = DiskOptions::HasSectorIF;
        bool isWP = (tracks == 40);
        FlexDisk *pdisk{};

        if (sectors > 72)
        {
            continue;
        }

        EXPECT_NO_THROW(
            pdisk = FlexDisk::Create(path3, ft_access, tracks, sectors, type));
        disk.reset(cast(pdisk));
        if (isWP)
        {
            disk.reset();
            disk.reset(cast(new FlexDisk(path3, mode, ft_access)));
        }

        ASSERT_NE(disk.get(), nullptr);
        EXPECT_TRUE(static_cast<bool>(disk));
        EXPECT_EQ(disk->IsWriteProtected(), tracks == 40);
        EXPECT_EQ(disk->GetFlexDiskType(), type);
        EXPECT_EQ(disk->GetPath(), path3);
        FlexDiskAttributes diskInfo{};
        ASSERT_TRUE(disk->GetDiskAttributes(diskInfo));
        CheckAttributes(diskInfo, path3, 0U, 0U, tracks, sectors, sectors0,
                SECTOR_SIZE, type, options, isWP);
        disk.reset();
        fs::remove(path3);
    }
}

// test IFlexDiskBase interface for class FlexDisk with a JVC header.
TEST_F(test_IFlexDiskBase, fcts_FlexDisk_JvcHeader)
{
    const int tracks = 35;
    const int sectors = 10;
    const auto path = fs::temp_directory_path() / u8"flexdisk.dsk";
    const auto path_jvc = fs::temp_directory_path() / u8"flexdisk.jvc.dsk";
    auto ft_access = FileTimeAccess::NONE;
    const auto imode = std::ios::in | std::ios::binary | std::ios::ate;
    const auto omode = std::ios::out | std::ios::binary;
    const auto iomode = std::ios::in | std::ios::out | std::ios::binary;
    FlexDiskBasePtr disk{};

    for (unsigned jvcHeaderSize = 1; jvcHeaderSize <= 5; ++jvcHeaderSize)
    {
        std::vector<Byte> jvcHeader{ '\x0A', '\x01', '\x01', '\x01', '\x00'};
        auto type = DiskType::DSK;
        auto options = DiskOptions::HasSectorIF | DiskOptions::JvcHeader;

        for (unsigned i = jvcHeaderSize; i < 5; ++i)
        {
            jvcHeader.pop_back();
        }
        auto *pdisk = FlexDisk::Create(path, ft_access, tracks, sectors, type);
        delete pdisk;
        std::fstream ifs(path, imode);
        ASSERT_TRUE(ifs.is_open());
        std::fstream ofs(path_jvc, omode);
        ASSERT_TRUE(ofs.is_open());
        // Prepend disk image file with an JVC header.
        ofs.write(reinterpret_cast<const char *>(jvcHeader.data()),
                jvcHeaderSize);
        const auto size = ifs.tellg();
        std::string buffer(static_cast<int>(size), '\0');
        ifs.seekg(0);
        if (ifs.read(buffer.data(), size))
        {
            ofs.write(buffer.data(), size);
        }
        ofs.close();
        ifs.close();
        fs::remove(path);

        disk.reset(cast(new FlexDisk(path_jvc, iomode, ft_access)));
        ASSERT_NE(disk.get(), nullptr);
        EXPECT_TRUE(static_cast<bool>(disk));
        EXPECT_EQ(disk->IsWriteProtected(), false);
        EXPECT_EQ(disk->GetFlexDiskType(), type);
        EXPECT_EQ(disk->GetFlexDiskOptions(), options);
        EXPECT_EQ(disk->GetPath(), path_jvc);
        FlexDiskAttributes diskInfo{};
        ASSERT_TRUE(disk->GetDiskAttributes(diskInfo));
        CheckAttributes(diskInfo, path_jvc, 0U, jvcHeaderSize, tracks, sectors,
                sectors, SECTOR_SIZE, type, options, false);
        EXPECT_EQ(diskInfo.GetJvcFileHeader(), jvcHeader);
        disk.reset();
        fs::remove(path_jvc);
    }
}

// test IFlexDiskBase interface for class FlexDirectoryDiskByFile.
TEST_F(test_IFlexDiskBase, fcts_FlexDirectoryByFile)
{
    const auto path = fs::temp_directory_path() / u8"flexdir123456";
    const auto ft_access = FileTimeAccess::NONE;
    auto type = DiskType::Directory;
    auto options = DiskOptions::NONE;
    fs::create_directory(path);
    FlexDiskBasePtr disk{};

    disk.reset(cast(new FlexDirectoryDiskByFile(path, ft_access)));
    ASSERT_NE(disk.get(), nullptr);
    EXPECT_TRUE(static_cast<bool>(disk));
    EXPECT_EQ(disk->IsWriteProtected(), false);
    EXPECT_EQ(disk->GetFlexDiskType(), type);
    EXPECT_EQ(disk->GetFlexDiskOptions(), options);
    EXPECT_EQ(disk->GetPath(), path);
    FlexDiskAttributes diskInfo{};
    ASSERT_TRUE(disk->GetDiskAttributes(diskInfo));
    CheckAttributes(diskInfo, path, 0U, 0U, 0, 0, 0, 0, type, options, false);
    disk.reset();
    fs::remove_all(path);
}

// test IFlexDiskBase interface for class FlexDirectoryBySector.
TEST_F(test_IFlexDiskBase, fcts_FlexDirectoryBySector)
{
    const auto path = fs::temp_directory_path() / u8"flexdir123456";
    const auto ft_access = FileTimeAccess::NONE;
    unsigned diskNumber = 0U;
    FlexDiskBasePtr disk{};

    for (int tracks = 2; tracks <= 255; tracks += 5)
    {
        int sectors = std::max(tracks / 3, 6);
        auto type = DiskType::Directory;
        auto options = DiskOptions::HasSectorIF;
        ASSERT_TRUE(fs::create_directory(path));
        auto *pdisk = new FlexDirectoryDiskBySector(path, ft_access, tracks,
                                                    sectors, nullptr);
        disk.reset(cast(pdisk));
        ASSERT_NE(disk.get(), nullptr);
        EXPECT_TRUE(static_cast<bool>(disk));
        EXPECT_EQ(disk->IsWriteProtected(), false);
        EXPECT_EQ(disk->GetFlexDiskType(), type);
        EXPECT_EQ(disk->GetFlexDiskOptions(), options);
        EXPECT_EQ(disk->GetPath(), path);
        FlexDiskAttributes diskInfo{};
        ASSERT_TRUE(disk->GetDiskAttributes(diskInfo));
        CheckAttributes(diskInfo, path, diskNumber, 0U, tracks, sectors,
                sectors, SECTOR_SIZE, type, options, false);
        disk.reset();
        fs::remove_all(path);
        ++diskNumber;
    }
}

