/*
    test_rndcheck.cpp


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


#include "gtest/gtest.h"
#include "misc1.h"
#include "filecnts.h"
#include "rndcheck.h"
#ifdef _WIN32
#include "cvtwchar.h"
#endif
#include <fmt/format.h>
#include <cstddef>
#include <ios>
#include <numeric>
#include <string>
#include <array>
#include <vector>
#include <fstream>
#include <filesystem>


namespace fs = std::filesystem;

class test_FlexRandomFileFixture : public ::testing::Test
{
protected:
    const int RO{0}; // read-only disk directory with random list file
                     // (random).
    const int RW{1}; // read-write disk directory with random list file
                     // (random).
    const int SP{2}; // read-write disk directory with space in path and
                     // random list file (random).
    const int U8{3}; // read-write disk directory with utf8-char in path and
                     // random list file (random).
    const int RWO{4}; // read-write disk directory with read-only random list
                      // file (random).
    const int RWD{5}; // read-write disk directory with new random list file
                      // (.random).
    const int RWDO{6};// read-write disk directory with new read-only random
                      // list file (.random).
    const int RWA{7}; // read-write disk directory with attributes.

    const std::array<const char *, 8> diskdirs{{
        "testdir_ro", "testdir_rw", "testdir_ sp", u8"testdir_\u2665",
        "testdir_rwo", "testdir_rwd", "testdir_rwdo", "testdir_rwa"
    }};
    std::vector<fs::path> randomListFiles;
    const fs::path temp_dir{ fs::temp_directory_path() };
#ifdef _WIN32
    static const auto write_perms = fs::perms::all;
#else
    static const auto write_perms = fs::perms::owner_write;
#endif

public:
    void SetUp() override
    {
        std::array<std::ofstream, 8> streams;

        ASSERT_EQ(diskdirs.size(), streams.size());
        for (int idx = RO; idx <= RWA; ++idx)
        {
            const auto diskdir = temp_dir / fs::u8path(diskdirs[idx]);
            ASSERT_TRUE(fs::create_directory(diskdir)) <<
                "dir=" << diskdir;
            fs::permissions(diskdir, write_perms, fs::perm_options::add);

            if (idx != RWA)
            {
                auto filePath = diskdir;
                filePath /= (idx == RWD || idx == RWDO) ?
                    RANDOM_FILE_LIST_NEW : RANDOM_FILE_LIST;
                randomListFiles.emplace_back(filePath);
                streams[idx].open(filePath);
                ASSERT_TRUE(streams[idx].is_open()) << "path=" <<
                    filePath.u8string();
            }
            else
            {
                randomListFiles.emplace_back("");
            }
        }

        for (int idx = RO; idx <= RWA; ++idx)
        {
            const auto diskdir = temp_dir / fs::u8path(diskdirs[idx]);
            for (int val = 3; val <= 11; ++val)
            {
                const auto filename = fmt::format("random{:02}.dat", val);
                const auto path = createFile(diskdir, filename, true, val);
                // Random file "random11.dat" is not in random list file
                // instead has file attributes.
                if (val != 11)
                {
                    streams[idx] << filename << "\n";
                }
                if (idx == RWA || val == 11)
                {
                    setFileAttribute(path);
                }
            }
            streams[idx].close();

            for (int val = 1; val <= 4; ++val)
            {
                const auto filename = fmt::format("nornd{:02}.dat", val);
                const auto path = createFile(diskdir, filename, false, val);
                if (val == 4)
                {
                    setFileAttribute(path);
                }
            }

            if ((idx == RWO || idx == RWDO))
            {
                fs::permissions(randomListFiles[idx],
                    write_perms, fs::perm_options::remove);
            }

#ifndef _WIN32
            // Directories on Windows have nothing like POSIX permissions.
            // RO has same behavior as RW.
            if (idx == RO)
            {
                fs::permissions(diskdir, write_perms, fs::perm_options::remove);
            }
#endif
        }

        ASSERT_EQ(diskdirs.size(), randomListFiles.size());
    }

    void TearDown() override
    {
        for (int idx = RO; idx <= RWA; ++idx)
        {
            const auto diskdir = temp_dir / fs::u8path(diskdirs[idx]);

            if (fs::exists(randomListFiles[idx]))
            {
                fs::permissions(randomListFiles[idx],
                    write_perms, fs::perm_options::add);
            }

#ifndef _WIN32
            fs::permissions(diskdir, write_perms, fs::perm_options::add);
#endif
            fs::remove_all(diskdir);
        }
    }

    static fs::path createFile(
            const fs::path &directory,
            const std::string &filename,
            bool isRandom, int sectors)
    {
        const auto path = directory / filename;
        const auto mode =
            isRandom ? (std::ios::out | std::ios::binary) : std::ios::out;
        std::array<Byte, 63> line{};
        std::fstream ofs(path, mode);
        std::iota(line.begin(), line.end(), '\0');

        EXPECT_TRUE(ofs.is_open());
        if (isRandom)
        {
            // Create a minimum sector map.
            SectorMap_t sectorMap{};

            EXPECT_GE(sectors, 3);
            sectors -= 2;
            sectorMap[0] = 0x01; // Track of first data sector
            sectorMap[1] = 0x01; // Sector of first data sector
            sectorMap[2] = static_cast<Byte>(sectors); // Data sector count
            ofs.write(reinterpret_cast<const char *>(sectorMap.data()),
                      sectorMap.size());
        }

        for (int sector = 0; sector < sectors; ++sector)
        {
            ofs.write(reinterpret_cast<const char *>(line.data()), line.size());
            ofs.write(reinterpret_cast<const char *>(line.data()), line.size());
            ofs.write(reinterpret_cast<const char *>(line.data()), line.size());
            ofs.write(reinterpret_cast<const char *>(line.data()), line.size());
        }

        return path;
    }

    static void setFileAttribute(const fs::path &path)
    {
#ifdef _WIN32
        const auto wPath = path.wstring();
        DWord attributes = GetFileAttributes(wPath.c_str());

        if (attributes != INVALID_FILE_ATTRIBUTES)
        {
                attributes |= FILE_ATTRIBUTE_HIDDEN;
                SetFileAttributes(wPath.c_str(), attributes);
        }
#endif
#ifdef UNIX
        if (fs::exists(path))
        {
            fs::permissions(path, fs::perms::owner_exec, fs::perm_options::add);
        }
#endif
    }
};

TEST_F(test_FlexRandomFileFixture, fct_IsRandomFile)
{
    for (int idx = RO; idx <= RWDO; ++idx)
    {
        const auto diskdir = temp_dir / fs::u8path(diskdirs[idx]);
        RandomFileCheck randomFileCheck(diskdir);

        for (int val = 3; val <= 11; ++val)
        {
            const auto filename = fmt::format("random{:02}.dat", val);
            bool status = val != 11 || idx == RWA;
            EXPECT_EQ(randomFileCheck.IsRandomFile(filename), status) <<
                "dir=" << diskdir.u8string() << " file=" << filename << "\n";
        }

        for (int val = 1; val <= 4; ++val)
        {
            const auto filename = fmt::format("nornd{:02}.dat", val);
            EXPECT_EQ(randomFileCheck.IsRandomFile(filename), false) <<
                "dir=" << diskdir.u8string() << " file=" << filename << "\n";
        }
    }

    const auto diskdir = temp_dir / fs::u8path(diskdirs[RWA]);
    RandomFileCheck randomFileCheck(diskdir);

    for (int val = 3; val <= 11; ++val)
    {
        const auto filename = fmt::format("random{:02}.dat", val);
        EXPECT_EQ(randomFileCheck.IsRandomFile(filename), false) <<
                "dir=" << diskdir.u8string() << " file=" << filename << "\n";
    }

    for (int val = 1; val <= 4; ++val)
    {
        const auto filename = fmt::format("nornd{:02}.dat", val);
        EXPECT_EQ(randomFileCheck.IsRandomFile(filename), false) <<
                "dir=" << diskdir.u8string() << " file=" << filename << "\n";
    }
}

TEST_F(test_FlexRandomFileFixture, fct_CheckForRandom)
{
    for (int idx = RO; idx <= RWA; ++idx)
    {
        const auto diskdir = temp_dir / fs::u8path(diskdirs[idx]);
        RandomFileCheck randomFileCheck(diskdir);

        for (int val = 3; val <= 11; ++val)
        {
            const auto filename = fmt::format("random{:02}.dat", val);
            EXPECT_EQ(randomFileCheck.CheckForRandom(filename), true);
        }

        for (int val = 1; val <= 4; ++val)
        {
            const auto filename = fmt::format("nornd{:02}.dat", val);
            EXPECT_EQ(randomFileCheck.CheckForRandom(filename), false);
            EXPECT_EQ(randomFileCheck.IsRandomFile(filename), false);
        }
    }
}

TEST_F(test_FlexRandomFileFixture, fct_CheckForRandomAndUpdate)
{
    for (int idx = RO; idx <= RWA; ++idx)
    {
        const auto diskdir = temp_dir / fs::u8path(diskdirs[idx]);
        RandomFileCheck randomFileCheck(diskdir);

        for (int val = 3; val <= 11; ++val)
        {
            const auto filename = fmt::format("random{:02}.dat", val);
            EXPECT_EQ(randomFileCheck.CheckForRandomAndUpdate(filename), true);
            EXPECT_EQ(randomFileCheck.IsRandomFile(filename), true);
        }

        for (int val = 1; val <= 4; ++val)
        {
            const auto filename = fmt::format("nornd{:02}.dat", val);
            EXPECT_EQ(randomFileCheck.CheckForRandomAndUpdate(filename), false);
            EXPECT_EQ(randomFileCheck.IsRandomFile(filename), false);
        }
    }
}

TEST_F(test_FlexRandomFileFixture, fct_AddToRandomList)
{
    for (int idx = RO; idx <= RWA; ++idx)
    {
        const auto diskdir = temp_dir / fs::u8path(diskdirs[idx]);
        RandomFileCheck randomFileCheck(diskdir);
        std::string filename{"newrnd1.dat"};
        EXPECT_EQ(randomFileCheck.AddToRandomList(filename), true) <<
            "dir=" << diskdir.u8string();
        EXPECT_EQ(randomFileCheck.IsRandomFile(filename), true) <<
            "dir=" << diskdir.u8string();
        filename = "NEWRND2.DAT";
        EXPECT_EQ(randomFileCheck.AddToRandomList(filename), true) <<
            "dir=" << diskdir.u8string();
        EXPECT_EQ(randomFileCheck.IsRandomFile(filename), true) <<
            "dir=" << diskdir.u8string();
        filename = "RANDOM03.DAT";
        EXPECT_EQ(randomFileCheck.AddToRandomList(filename), idx == RWA) <<
            "dir=" << diskdir.u8string();
        EXPECT_EQ(randomFileCheck.IsRandomFile(filename), true) <<
            "dir=" << diskdir.u8string();
    }
}

TEST_F(test_FlexRandomFileFixture, fct_RemoveFromRandomList)
{
    for (int idx = RO; idx <= RWA; ++idx)
    {
        const auto diskdir = temp_dir / fs::u8path(diskdirs[idx]);
        RandomFileCheck randomFileCheck(diskdir);
        std::string filename{"newrnd1.dat"};
        EXPECT_EQ(randomFileCheck.RemoveFromRandomList(filename), false) <<
            "dir=" << diskdir.u8string();
        EXPECT_EQ(randomFileCheck.IsRandomFile(filename), false) <<
            "dir=" << diskdir.u8string();
        filename = "RANDOM03.DAT";
        EXPECT_EQ(randomFileCheck.RemoveFromRandomList(filename), idx != RWA) <<
            "dir=" << diskdir.u8string();
        EXPECT_EQ(randomFileCheck.IsRandomFile(filename), false) <<
            "dir=" << diskdir.u8string();
        filename = "random04.dat";
        EXPECT_EQ(randomFileCheck.RemoveFromRandomList(filename), idx != RWA) <<
            "dir=" << diskdir.u8string();
        EXPECT_EQ(randomFileCheck.IsRandomFile(filename), false) <<
            "dir=" << diskdir.u8string();
    }
}

TEST_F(test_FlexRandomFileFixture, fct_CheckForFileAttributeAndUpdate)
{
    const std::array<int, 2> indices{ RW, RWA };

    for (int idx : indices)
    {
        const auto diskdir = temp_dir / fs::u8path(diskdirs[idx]);
        if (idx == RW)
        {
            fs::remove(diskdir / RANDOM_FILE_LIST);
        }
        RandomFileCheck randomFileCheck(diskdir);
        std::string filename{"random03.dat"};
        const bool expected = (idx == RWA);
        auto result = randomFileCheck.CheckForFileAttributeAndUpdate(filename);
        EXPECT_EQ(result, expected) << "dir=" << diskdir;
        EXPECT_EQ(randomFileCheck.IsRandomFile(filename), expected) <<
            "dir=" << diskdir.u8string();

        filename = "nornd01.dat";
        result = randomFileCheck.CheckForFileAttributeAndUpdate(filename);
        EXPECT_EQ(result, false) << "dir=" << diskdir;
        EXPECT_EQ(randomFileCheck.IsRandomFile(filename), false) <<
            "dir=" << diskdir.u8string();
        filename = "nornd02.dat";
        result = randomFileCheck.CheckForFileAttributeAndUpdate(filename);
        EXPECT_EQ(result, false) << "dir=" << diskdir;
        EXPECT_EQ(randomFileCheck.IsRandomFile(filename), false) <<
            "dir=" << diskdir.u8string();
        filename = "nornd03.dat";
        result = randomFileCheck.CheckForFileAttributeAndUpdate(filename);
        EXPECT_EQ(result, false) << "dir=" << diskdir;
        EXPECT_EQ(randomFileCheck.IsRandomFile(filename), false) <<
            "dir=" << diskdir.u8string();
    }
}

TEST_F(test_FlexRandomFileFixture, fct_IsValidSectorMap)
{
    SectorMap_t sectorMap{};

    EXPECT_FALSE(RandomFileCheck::IsValidSectorMap(sectorMap, 2 * DBPS));
    // 0 sectors, 1 expected based on fileSize
    EXPECT_FALSE(RandomFileCheck::IsValidSectorMap(sectorMap, 2 * DBPS + 1));
    EXPECT_FALSE(RandomFileCheck::IsValidSectorMap(sectorMap, 3 * DBPS - 1));
    EXPECT_FALSE(RandomFileCheck::IsValidSectorMap(sectorMap, 3 * DBPS));
    sectorMap[0] = 1U;
    sectorMap[1] = 1U;
    sectorMap[2] = 2U; // 2 sectors, 1 expected based on fileSize
    EXPECT_FALSE(RandomFileCheck::IsValidSectorMap(sectorMap, 3 * DBPS));
    sectorMap[2] = 1U;
    EXPECT_TRUE(RandomFileCheck::IsValidSectorMap(sectorMap, 3 * DBPS));
    sectorMap[3] = 1U; // values != 0 after all sectors found
    sectorMap[4] = 0U;
    sectorMap[5] = 0U;
    EXPECT_FALSE(RandomFileCheck::IsValidSectorMap(sectorMap, 3 * DBPS));
    sectorMap[3] = 0U;
    sectorMap[4] = 1U; // value != 0 after all sectors found
    EXPECT_FALSE(RandomFileCheck::IsValidSectorMap(sectorMap, 3 * DBPS));
    sectorMap[4] = 0U;
    sectorMap[5] = 1U; // value != 0 after all sectors found
    EXPECT_FALSE(RandomFileCheck::IsValidSectorMap(sectorMap, 3 * DBPS));
    for (size_t i = 0; i < sectorMap.size(); i += 3)
    {
        sectorMap[i] = 1U;
        sectorMap[i + 1] = 1U;
        sectorMap[i + 2] = 1U;
    }
    EXPECT_TRUE(RandomFileCheck::IsValidSectorMap(sectorMap,
                DBPS * ((sectorMap.size() / 3) + 2)));
}

TEST_F(test_FlexRandomFileFixture, fct_UpdateRandomListToFile)
{
    const std::array<const char *, 8> randomListFileNames{
        RANDOM_FILE_LIST,
        RANDOM_FILE_LIST_NEW,
        RANDOM_FILE_LIST_NEW,
        RANDOM_FILE_LIST_NEW,
        RANDOM_FILE_LIST,
        RANDOM_FILE_LIST_NEW,
        RANDOM_FILE_LIST_NEW,
        RANDOM_FILE_LIST_NEW,
    };

    ASSERT_EQ(diskdirs.size(), randomListFileNames.size());

    for (int idx = RO; idx <= RWA; ++idx)
    {
        const auto diskdir = temp_dir / fs::u8path(diskdirs[idx]);
        RandomFileCheck randomFileCheck(diskdir);
#ifdef _WIN32
        // Directories on Windows have nothing like POSIX permissions.
        // RO has same behavior as RW.
        const bool noUpdate = (idx == RWO || idx == RWDO);
#else
        const bool noUpdate = (idx == RO || idx == RWO || idx == RWDO);
#endif

        EXPECT_EQ(randomFileCheck.UpdateRandomListToFile(), false) <<
            "dir=" << diskdir.u8string();
        std::string filename{"newrnd1.dat"};
        EXPECT_EQ(randomFileCheck.AddToRandomList(filename), true) <<
            "dir=" << diskdir.u8string();
        // Temporarily disable this testcase on Windows for unicode
        // path name, until fixed.
#ifdef _WIN32
        if (idx != U8)
        {
#endif
        EXPECT_EQ(randomFileCheck.UpdateRandomListToFile(), !noUpdate) <<
            "dir=" << diskdir.u8string();
        filename = "random04.dat";
        EXPECT_EQ(randomFileCheck.AddToRandomList(filename), idx == RWA) <<
            "dir=" << diskdir.u8string();
        EXPECT_EQ(randomFileCheck.UpdateRandomListToFile(), idx == RWA) <<
            "dir=" << diskdir.u8string();

        auto randomListFile = diskdir / randomListFileNames[idx];
        bool isExecuteTest = true;
#ifdef _WIN32
        // Directories on Windows have nothing like POSIX permissions.
        // RO has same behavior as RW.
        isExecuteTest = (idx != RO);
#endif
        if (isExecuteTest)
        {
            EXPECT_TRUE(fs::exists(randomListFile));
        }
        if (idx == RW)
        {
            randomListFile = diskdir / RANDOM_FILE_LIST;
            EXPECT_TRUE(!fs::exists(randomListFile));
        }
#ifdef _WIN32
        }
#endif
    }
}

TEST_F(test_FlexRandomFileFixture, fct_IsWriteProtected)
{
    const std::array<bool, 8> expectedWP{
        true, false, false, false, true, false, true, false
    };

    ASSERT_EQ(diskdirs.size(), expectedWP.size());

    int idx = RO;
#ifdef _WIN32
    // Directories on Windows have nothing like POSIX permissions.
    // RO has same behavior as RW, means RO can be skipped.
    idx = RW;
#endif
    for ( ; idx <= RWA; ++idx)
    {
        const auto diskdir = temp_dir / fs::u8path(diskdirs[idx]);
        RandomFileCheck randomFileCheck(diskdir);

        // Temporarily disable this testcase on Windows for unicode
        // path name, until fixed.
#ifdef _WIN32
        if (idx != U8)
        {
#endif
        EXPECT_EQ(randomFileCheck.IsWriteProtected(), expectedWP[idx]) <<
            "dir=" << diskdir.u8string();
#ifdef _WIN32
        }
#endif
    }
}

TEST_F(test_FlexRandomFileFixture, fct_CheckAllFilesAttributeAndUpdate)
{
    const std::array<int, 2> indices{ RW, RWA };

    for (int idx : indices)
    {
        const auto diskdir = temp_dir / fs::u8path(diskdirs[idx]);
        if (idx == RW)
        {
            fs::remove(diskdir / RANDOM_FILE_LIST);
        }
        RandomFileCheck randomFileCheck(diskdir);
        randomFileCheck.CheckAllFilesAttributeAndUpdate();
        std::string filename{"random03.dat"};
        const bool expected = (idx == RWA);
        EXPECT_EQ(randomFileCheck.IsRandomFile(filename), expected) <<
            "dir=" << diskdir.u8string();
        filename = "random04.dat";
        EXPECT_EQ(randomFileCheck.IsRandomFile(filename), expected) <<
            "dir=" << diskdir.u8string();

        filename = "nornd01.dat";
        EXPECT_EQ(randomFileCheck.IsRandomFile(filename), false) <<
            "dir=" << diskdir.u8string();
        filename = "nornd03.dat";
        EXPECT_EQ(randomFileCheck.IsRandomFile(filename), false) <<
            "dir=" << diskdir.u8string();
    }
}
