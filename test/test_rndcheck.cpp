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
#include "gmock/gmock.h"
#include "misc1.h"
#include "filecnts.h"
#include "rndcheck.h"
#include <numeric>
#include <fstream>
#include <filesystem>
#include <fmt/format.h>


namespace fs = std::filesystem;

class test_FlexRandomFileFixture : public ::testing::Test
{
protected:
    const int RO{0}; // read-only disk directory with random list file
                     // (random).
    const int RW{1}; // read-write disk directory with random list file
                     // (random).
    const int RWO{2}; // read-write disk directory with read-only random list
                      // file (random).
    const int RWD{3}; // read-write disk directory with new random list file
                      // (.random).
    const int RWDO{4};// read-write disk directory with new read-only random
                      // list file (.random).
    const int RWA{5}; // read-write disk directory with attributes.

    const std::array<const char *, 6> diskPaths{
        {"/tmp/testdir_ro", "/tmp/testdir_rw", "/tmp/testdir_rwo",
         "/tmp/testdir_rwd", "/tmp/testdir_rwdo", "/tmp/testdir_rwa"},
    };
    std::vector<std::string> randomListFiles;

public:
    void SetUp() override
    {
        std::array<std::ofstream, 6> streams;

        ASSERT_EQ(diskPaths.size(), streams.size());
        for (int idx = RO; idx <= RWA; ++idx)
        {
            ASSERT_TRUE(fs::create_directory(diskPaths[idx])) <<
                "path=" << diskPaths[idx];
            fs::permissions(diskPaths[idx],
                fs::perms::owner_write, fs::perm_options::add);

            if (idx != RWA)
            {
                fs::path path = diskPaths[idx];
                path /= (idx == RWD || idx == RWDO) ?
                    RANDOM_FILE_LIST_NEW : RANDOM_FILE_LIST;
                randomListFiles.emplace_back(std::string(path));
                streams[idx].open(path);
                ASSERT_TRUE(streams[idx].is_open()) << "path=" << path;
            }
            else
            {
                randomListFiles.emplace_back("");
            }
        }

        for (int idx = RO; idx <= RWA; ++idx)
        {
            for (int val = 3; val <= 11; ++val)
            {
                const auto filename = fmt::format("random{:02}.dat", val);
                const auto path =
                    createFile(diskPaths[idx], filename, true, val);
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
                const auto path =
                    createFile(diskPaths[idx], filename, false, val);
                if (val == 4)
                {
                    setFileAttribute(path);
                }
            }

            if ((idx == RWO || idx == RWDO))
            {
                fs::permissions(randomListFiles[idx],
                    fs::perms::owner_write, fs::perm_options::remove);
            }

            if (idx == RO)
            {
                fs::permissions(diskPaths[idx],
                    fs::perms::owner_write, fs::perm_options::remove);
            }
        }

        ASSERT_EQ(diskPaths.size(), randomListFiles.size());
    }

    void TearDown() override
    {
        for (int idx = RO; idx <= RWA; ++idx)
        {
            struct stat sbuf{};

            if (stat(randomListFiles[idx].c_str(), &sbuf) == 0)
            {
                fs::permissions(randomListFiles[idx],
                    fs::perms::owner_write, fs::perm_options::add);
            }

            fs::permissions(diskPaths[idx],
                fs::perms::owner_write, fs::perm_options::add);
            fs::remove_all(diskPaths[idx]);
        }
    }

    static std::string createFile(
            const std::string &directory,
            const std::string &filename,
            bool isRandom, int sectors)
    {
        const auto path = directory + PATHSEPARATOR + filename;
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

    static void setFileAttribute(const std::string &path)
    {
#ifdef _WIN32
        const auto wPath = ConvertToUtf16String(path);
        DWord attributes = GetFileAttributes(wFilePath.c_str());

        if (attributes != INVALID_FILE_ATTRIBUTES)
        {
                attributes |= FILE_ATTRIBUTE_HIDDEN;
                SetFileAttributes(wPath.c_str(), attributes);
        }
#endif
#ifdef UNIX
        struct stat sbuf{};

        if (stat(path.c_str(), &sbuf) == 0)
        {
            chmod(path.c_str(), sbuf.st_mode | S_IXUSR);
        }
#endif
    }
};

TEST_F(test_FlexRandomFileFixture, fct_IsRandomFile)
{
    for (int idx = RO; idx <= RWDO; ++idx)
    {
        RandomFileCheck randomFileCheck(diskPaths[idx]);

        for (int val = 3; val <= 11; ++val)
        {
            const auto filename = fmt::format("random{:02}.dat", val);
            bool status = val != 11 || idx == RWA;
            EXPECT_EQ(randomFileCheck.IsRandomFile(filename), status) <<
                "dir=" << diskPaths[idx] << " file=" << filename << "\n";
        }

        for (int val = 1; val <= 4; ++val)
        {
            const auto filename = fmt::format("nornd{:02}.dat", val);
            EXPECT_EQ(randomFileCheck.IsRandomFile(filename), false) <<
                "dir=" << diskPaths[idx] << " file=" << filename << "\n";
        }
    }

    RandomFileCheck randomFileCheck(diskPaths[RWA]);

    for (int val = 3; val <= 11; ++val)
    {
        const auto filename = fmt::format("random{:02}.dat", val);
        EXPECT_EQ(randomFileCheck.IsRandomFile(filename), false) <<
                "dir=" << diskPaths[RWA] << " file=" << filename << "\n";
    }

    for (int val = 1; val <= 4; ++val)
    {
        const auto filename = fmt::format("nornd{:02}.dat", val);
        EXPECT_EQ(randomFileCheck.IsRandomFile(filename), false) <<
                "dir=" << diskPaths[RWA] << " file=" << filename << "\n";
    }
}

TEST_F(test_FlexRandomFileFixture, fct_CheckForRandom)
{
    for (int idx = RO; idx <= RWA; ++idx)
    {
        RandomFileCheck randomFileCheck(diskPaths[idx]);

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
        RandomFileCheck randomFileCheck(diskPaths[idx]);

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
        RandomFileCheck randomFileCheck(diskPaths[idx]);
        std::string filename{"newrnd1.dat"};
        EXPECT_EQ(randomFileCheck.AddToRandomList(filename), true) <<
            "path=" << diskPaths[idx];
        EXPECT_EQ(randomFileCheck.IsRandomFile(filename), true) <<
            "path=" << diskPaths[idx];
        filename = "NEWRND2.DAT";
        EXPECT_EQ(randomFileCheck.AddToRandomList(filename), true) <<
            "path=" << diskPaths[idx];
        EXPECT_EQ(randomFileCheck.IsRandomFile(filename), true) <<
            "path=" << diskPaths[idx];
        filename = "RANDOM03.DAT";
        EXPECT_EQ(randomFileCheck.AddToRandomList(filename), idx == RWA) <<
            "path=" << diskPaths[idx];
        EXPECT_EQ(randomFileCheck.IsRandomFile(filename), true) <<
            "path=" << diskPaths[idx];
    }
}

TEST_F(test_FlexRandomFileFixture, fct_RemoveFromRandomList)
{
    for (int idx = RO; idx <= RWA; ++idx)
    {
        RandomFileCheck randomFileCheck(diskPaths[idx]);
        std::string filename{"newrnd1.dat"};
        EXPECT_EQ(randomFileCheck.RemoveFromRandomList(filename), false) <<
            "path=" << diskPaths[idx];
        EXPECT_EQ(randomFileCheck.IsRandomFile(filename), false) <<
            "path=" << diskPaths[idx];
        filename = "RANDOM03.DAT";
        EXPECT_EQ(randomFileCheck.RemoveFromRandomList(filename), idx != RWA) <<
            "path=" << diskPaths[idx];
        EXPECT_EQ(randomFileCheck.IsRandomFile(filename), false) <<
            "path=" << diskPaths[idx];
        filename = "random04.dat";
        EXPECT_EQ(randomFileCheck.RemoveFromRandomList(filename), idx != RWA) <<
            "path=" << diskPaths[idx];
        EXPECT_EQ(randomFileCheck.IsRandomFile(filename), false) <<
            "path=" << diskPaths[idx];
    }
}

TEST_F(test_FlexRandomFileFixture, fct_CheckForFileAttributeAndUpdate)
{
    const std::array<int, 2> indices{ RW, RWA };

    for (int idx : indices)
    {
        if (idx == RW)
        {
            fs::remove(fs::path(diskPaths[idx]) / "random");
        }
        RandomFileCheck randomFileCheck(diskPaths[idx]);
        std::string filename{"random03.dat"};
        const bool expected = (idx == RWA);
        auto result = randomFileCheck.CheckForFileAttributeAndUpdate(filename);
        EXPECT_EQ(result, expected) << "path=" << diskPaths[idx];
        EXPECT_EQ(randomFileCheck.IsRandomFile(filename), expected) <<
            "path=" << diskPaths[idx];

        filename = "nornd01.dat";
        result = randomFileCheck.CheckForFileAttributeAndUpdate(filename);
        EXPECT_EQ(result, false) << "path=" << diskPaths[idx];
        EXPECT_EQ(randomFileCheck.IsRandomFile(filename), false) <<
            "path=" << diskPaths[idx];
        filename = "nornd02.dat";
        result = randomFileCheck.CheckForFileAttributeAndUpdate(filename);
        EXPECT_EQ(result, false) << "path=" << diskPaths[idx];
        EXPECT_EQ(randomFileCheck.IsRandomFile(filename), false) <<
            "path=" << diskPaths[idx];
        filename = "nornd03.dat";
        result = randomFileCheck.CheckForFileAttributeAndUpdate(filename);
        EXPECT_EQ(result, false) << "path=" << diskPaths[idx];
        EXPECT_EQ(randomFileCheck.IsRandomFile(filename), false) <<
            "path=" << diskPaths[idx];
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
    const std::array<const char *, 6> randomListFiles{
        "random", ".random", "random", ".random", ".random", ".random"
    };

    for (int idx = RO; idx <= RWA; ++idx)
    {
        RandomFileCheck randomFileCheck(diskPaths[idx]);
        const bool noUpdate = (idx == RO || idx == RWO || idx == RWDO);

        EXPECT_EQ(randomFileCheck.UpdateRandomListToFile(), false) <<
            "path=" << diskPaths[idx];
        std::string filename{"newrnd1.dat"};
        EXPECT_EQ(randomFileCheck.AddToRandomList(filename), true) <<
            "path=" << diskPaths[idx];
        EXPECT_EQ(randomFileCheck.UpdateRandomListToFile(), !noUpdate) <<
            "path=" << diskPaths[idx];
        filename = "random04.dat";
        EXPECT_EQ(randomFileCheck.AddToRandomList(filename), idx == RWA) <<
            "path=" << diskPaths[idx];
        EXPECT_EQ(randomFileCheck.UpdateRandomListToFile(), idx == RWA) <<
            "path=" << diskPaths[idx];

        auto randomListFile = fs::path(diskPaths[idx]) / randomListFiles[idx];
        struct stat sbuf{};

        EXPECT_TRUE(stat(randomListFile.c_str(), &sbuf) == 0);
        if (idx == RW)
        {
            randomListFile = fs::path(diskPaths[idx]) / "random";
            EXPECT_TRUE(stat(randomListFile.c_str(), &sbuf) != 0);
        }
    }
}

TEST_F(test_FlexRandomFileFixture, fct_IsWriteProtected)
{
    const std::array<bool, 6> expectedWP{
        true, false, true, false, true, false
    };

    for (int idx = RO; idx <= RWA; ++idx)
    {
        RandomFileCheck randomFileCheck(diskPaths[idx]);

        EXPECT_EQ(randomFileCheck.IsWriteProtected(), expectedWP[idx]);
    }
}

TEST_F(test_FlexRandomFileFixture, fct_CheckAllFilesAttributeAndUpdate)
{
    const std::array<int, 2> indices{ RW, RWA };

    for (int idx : indices)
    {
        if (idx == RW)
        {
            fs::remove(fs::path(diskPaths[idx]) / "random");
        }
        RandomFileCheck randomFileCheck(diskPaths[idx]);
        randomFileCheck.CheckAllFilesAttributeAndUpdate();
        std::string filename{"random03.dat"};
        const bool expected = (idx == RWA);
        EXPECT_EQ(randomFileCheck.IsRandomFile(filename), expected) <<
            "path=" << diskPaths[idx];
        filename = "random04.dat";
        EXPECT_EQ(randomFileCheck.IsRandomFile(filename), expected) <<
            "path=" << diskPaths[idx];

        filename = "nornd01.dat";
        EXPECT_EQ(randomFileCheck.IsRandomFile(filename), false) <<
            "path=" << diskPaths[idx];
        filename = "nornd03.dat";
        EXPECT_EQ(randomFileCheck.IsRandomFile(filename), false) <<
            "path=" << diskPaths[idx];
    }
}
