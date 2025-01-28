/*
    test_filecntb.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2024-2025  W. Schwotzer

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
#include <memory>
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
            const std::string &path,
            unsigned diskNumber,
            unsigned jvcHeaderSize,
            int expectedTracks,
            int expectedSectors,
            int expectedSectorSize,
            DiskType type,
            DiskOptions options,
            bool isWriteProtected)
    {
        auto diskName = flx::toupper(flx::getFileName(path));
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
            type == DiskType::Directory)
        {
            ASSERT_TRUE(diskInfo.GetIsFlexFormat());
            EXPECT_EQ(diskInfo.GetDate(), BDate::Now());
            if ((options & DiskOptions::HasSectorIF) != DiskOptions::NONE)
            {
                EXPECT_EQ(diskInfo.GetFree(),
                  (expectedTracks - 1) * expectedSectors * expectedSectorSize);
                EXPECT_EQ(diskInfo.GetTotalSize(),
                  expectedTracks * expectedSectors * expectedSectorSize);
            }
        }
        EXPECT_EQ(diskInfo.GetOptions(), options);
        EXPECT_EQ(diskInfo.GetPath(), path);
        EXPECT_EQ(diskInfo.GetName(), diskName);
        EXPECT_EQ(diskInfo.GetNumber(), diskNumber);
        int tracks = 0;
        int sectors = 0;
        diskInfo.GetTrackSector(tracks, sectors);
        EXPECT_EQ(tracks, expectedTracks);
        EXPECT_EQ(sectors, expectedSectors);
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
    const std::string path1{"/tmp/flexdisk.dsk"};
    const auto ft_access = FileTimeAccess::NONE;
    const auto mode = std::ios::in | std::ios::binary;
    FlexDiskBasePtr disk{};

    for (int tracks = 2; tracks <= 255; tracks += 5)
    {
        int sectors = std::max(tracks / 3, 6);
        auto type = DiskType::DSK;
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
        EXPECT_EQ(disk->GetPath(), path1);
        FlexDiskAttributes diskInfo{};
        ASSERT_TRUE(disk->GetDiskAttributes(diskInfo));
        CheckAttributes(diskInfo, path1, 0U, 0U, tracks, sectors, SECTOR_SIZE,
                type, options, isWP);
        disk.reset();
        fs::remove(path1);
    }

    const std::string path2{"/tmp/flexdisk.flx"};

    for (int tracks = 2; tracks <= 255; tracks += 5)
    {
        int sectors = std::max(tracks / 3, 6);
        auto type = DiskType::FLX;
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
        CheckAttributes(diskInfo, path2, 0U, 0U, tracks, sectors, SECTOR_SIZE,
                type, options, isWP);
        disk.reset();
        fs::remove(path2);
    }
}

// test IFlexDiskBase interface for class FlexDisk with a JVC header.
TEST_F(test_IFlexDiskBase, fcts_FlexDisk_JvcHeader)
{
    const int tracks = 35;
    const int sectors = 10;
    const std::string path{"/tmp/flexdisk.dsk"};
    const std::string path_jvc{"/tmp/flexdisk.jvc.dsk"};
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
        std::string buffer(size, '\0');
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
                SECTOR_SIZE, type, options, false);
        EXPECT_EQ(diskInfo.GetJvcFileHeader(), jvcHeader);
        disk.reset();
        fs::remove(path_jvc);
    }
}

// test IFlexDiskBase interface for class FlexDirectoryDiskByFile.
TEST_F(test_IFlexDiskBase, fcts_FlexDirectoryByFile)
{
    const std::string path{"/tmp/flexdir12345678"};
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
    CheckAttributes(diskInfo, path, 0U, 0U, 0, 0, 0, type, options, false);
    disk.reset();
    fs::remove_all(path);
}

// test IFlexDiskBase interface for class FlexDirectoryBySector.
TEST_F(test_IFlexDiskBase, fcts_FlexDirectoryBySector)
{
    const std::string path{"/tmp/flexdir1234567"};
    const auto ft_access = FileTimeAccess::NONE;
    unsigned diskNumber = 0U;
    FlexDiskBasePtr disk{};

    for (int tracks = 2; tracks <= 255; tracks += 5)
    {
        int sectors = std::max(tracks / 3, 6);
        auto type = DiskType::Directory;
        auto options = DiskOptions::HasSectorIF;
        ASSERT_TRUE(fs::create_directory(path));
        auto *pdisk =
            new FlexDirectoryDiskBySector(path, ft_access, tracks, sectors);
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
                SECTOR_SIZE, type, options, false);
        disk.reset();
        fs::remove_all(path);
        ++diskNumber;
    }
}

