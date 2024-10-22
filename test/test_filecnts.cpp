// Test IFlexDiskBySector interface.

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "misc1.h"
#include "ffilecnt.h"
#include "rfilecnt.h"
#include "ndircont.h"
#include "filfschk.h"
#include "fixt_filecont.h"
#include <array>
#include <fstream>
#include <filesystem>


using ::testing::StartsWith;
namespace fs = std::filesystem;

class test_IFlexDiskBySector : public test_FlexDiskFixture
{
protected:
    std::array<std::array<IFlexDiskBySectorPtr, 3>, 6> disks;

     int GetMaxDiskIndex() override
     {
         return RAM;
     }

     int GetMaxDirIndex() override
     {
         return FT;
     }

    void SetUp() override
    {
        test_FlexDiskFixture::SetUp();

        const auto mode = std::ios::in | std::ios::out | std::ios::binary;
        const auto romode = std::ios::in | std::ios::binary;

        for (int idx = RO; idx <= GetMaxDiskIndex(); ++idx)
        {
            const auto ios_mode = (idx == RO || idx == ROM) ? romode : mode;
            const auto &ft = (idx == FT) ? with_ft : no_ft;
            FlexDisk *pdisk{};

            for (int tidx = DSK; tidx <= FLX; ++tidx)
            {
                pdisk = (idx == RAM || idx == ROM) ?
                    new FlexRamDisk(diskPaths[idx][tidx], ios_mode, ft) :
                    new FlexDisk(diskPaths[idx][tidx], ios_mode, ft);
                disks[idx][tidx].reset(cast(pdisk));
                ASSERT_NE(disks[idx][tidx].get(), nullptr)
                    << "path=" << diskPaths[idx][tidx];
            }
        }

        for (int idx = RO; idx <= GetMaxDirIndex(); ++idx)
        {
            const auto &ft = (idx == FT) ? with_ft : no_ft;
            auto *pdir = new FlexDirectoryDiskBySector(diskPaths[idx][DIR],
                    ft, tracks, sectors);
            disks[idx][DIR].reset(cast(pdir));
            ASSERT_NE(disks[idx][DIR].get(), nullptr)
                << "path=" << diskPaths[idx][DIR];
        }
    }

    void TearDown() override
    {
        for (int idx = RO; idx <= GetMaxDiskIndex(); ++idx)
        {
            for (int tidx = DSK; tidx <= DIR; ++tidx)
            {
                disks[idx][tidx].reset();
            }
        }

        test_FlexDiskFixture::TearDown();
    }

    static IFlexDiskBySector *cast(FlexDisk *disk)
    {
         return static_cast<IFlexDiskBySector *>(disk);
    }

    static IFlexDiskBySector *cast(FlexDirectoryDiskBySector *dirDisk)
    {
         return static_cast<IFlexDiskBySector *>(dirDisk);
    }
};

TEST_F(test_IFlexDiskBySector, fct_ReadSector)
{
    std::array<Byte, SECTOR_SIZE> buffer{};

    for (int idx = RO; idx <= GetMaxDiskIndex(); ++idx)
    {
        for (int tidx = DSK; tidx <= DIR; ++tidx)
        {
            auto *disk = disks[idx][tidx].get();

            if (disk == nullptr)
            {
                continue;
            }

            for (int track = 0; track < tracks; ++track)
            {
                for (int sector = 1; sector <= sectors; ++sector)
                {
                    EXPECT_TRUE(disk->ReadSector(buffer.data(), track, sector))
                        << "path=" << disk->GetPath() << " track=" << track <<
                           " sector=" << sector;
                }

                EXPECT_FALSE(disk->ReadSector(buffer.data(), track, 0))
                        << "path=" << disk->GetPath() << " track=" << track;
                EXPECT_FALSE(disk->ReadSector(buffer.data(), track, -1))
                        << "path=" << disk->GetPath() << " track=" << track;
                EXPECT_FALSE(disk->ReadSector(buffer.data(), track,
                            sectors + 1))
                        << "path=" << disk->GetPath() << " track=" << track;
            }

            EXPECT_FALSE(disk->ReadSector(buffer.data(), -1, 1))
                    << "path=" << disk->GetPath();
            EXPECT_FALSE(disk->ReadSector(buffer.data(), 0, -1))
                    << "path=" << disk->GetPath();
            EXPECT_FALSE(disk->ReadSector(buffer.data(), -1, -1))
                    << "path=" << disk->GetPath();
            EXPECT_FALSE(disk->ReadSector(buffer.data(), tracks, 1))
                    << "path=" << disk->GetPath();
        }
    }
}

TEST_F(test_IFlexDiskBySector, fct_ReadSector_content)
{
    struct s_sys_info_sector sis{};
    struct s_dir_sector dirSector{};

    for (int idx = RO; idx <= GetMaxDiskIndex(); ++idx)
    {
        for (int tidx = DSK; tidx <= DIR; ++tidx)
        {
            auto *disk = disks[idx][tidx].get();

            if (disk == nullptr || idx == TGT)
            {
                continue;
            }
            EXPECT_TRUE(disk->ReadSector(
                        reinterpret_cast<Byte *>(&sis), 0, 3))
                    << "path=" << disk->GetPath();
            auto name = flx::getstr<>(sis.sir.disk_name);
            EXPECT_EQ(name, tidx == DIR ? "TESTDIR_" : "TESTDISK")
                    << "path=" << disk->GetPath();

            for (int didx = 5; didx <= 6; ++didx)
            {
                EXPECT_TRUE(disk->ReadSector(
                            reinterpret_cast<Byte *>(&dirSector), 0, didx))
                        << "path=" << disk->GetPath();
                for (const auto &dir_entry : dirSector.dir_entries)
                {
                    auto filestem = flx::getstr<>(dir_entry.filename);
                    EXPECT_THAT(filestem, StartsWith("TEST"))
                        << "path=" << disk->GetPath();
                    auto fileext = flx::getstr<>(dir_entry.file_ext);
                    EXPECT_TRUE(fileext == "BIN" || fileext == "TXT");
                }
            }
        }
    }
}

TEST_F(test_IFlexDiskBySector, fct_WriteSector)
{
    const auto ft_access = FileTimeAccess::Get | FileTimeAccess::Set;
    std::array<Byte, SECTOR_SIZE> buffer{};

    for (int idx = RO; idx <= GetMaxDiskIndex(); ++idx)
    {
        for (int tidx = DSK; tidx <= DIR; ++tidx)
        {
            // Writing 0 to all available sectors is too much for a directory
            // disk => test intentionally skipped.
            if (idx != RO && tidx == DIR)
            {
                continue;
            }

            auto *disk = disks[idx][tidx].get();

            if (disk == nullptr || idx == TGT)
            {
                continue;
            }

            for (int track = 0; track < tracks; ++track)
            {
                for (int sector = 1; sector <= sectors; ++sector)
                {
                    EXPECT_EQ(disk->WriteSector(buffer.data(),
                                track, sector), idx != RO && idx != ROM)
                        << "path=" << disk->GetPath() << " track=" << track <<
                           " sector=" << sector;
                }
            }
            FlexDiskCheck checker(*disk, ft_access);

            EXPECT_EQ(checker.CheckFileSystem(), idx == RO || idx == ROM)
                << " path=" << disk->GetPath();
        }
    }
}

TEST_F(test_IFlexDiskBySector, fct_FormatSector)
{
    std::array<Byte, SECTOR_SIZE> buffer{};

    // Try to format an already formated disk image.
    for (int idx = RO; idx <= GetMaxDiskIndex(); ++idx)
    {
        for (int tidx = DSK; tidx <= DIR; ++tidx)
        {
            auto *disk = disks[idx][tidx].get();

            if (disk == nullptr)
            {
                continue;
            }

            EXPECT_FALSE(disk->FormatSector(buffer.data(), 0, 1, 0, 1))
                << "path=" << disk->GetPath();
        }
    }

    // Test corner cases.
    const auto ft_access = FileTimeAccess::Get | FileTimeAccess::Set;
    const std::ios::openmode mode = std::ios::out | std::ios::trunc |
        std::ios::binary;
    const std::string filename = "format_sector.flx";
    const std::string path = fs::temp_directory_path() / filename;
    std::fstream fs(path, mode);
    ASSERT_TRUE(fs.is_open());
    fs.close();
    auto disk = static_cast<IFlexDiskBySectorPtr>(new FlexDisk(path, mode,
                ft_access));

    EXPECT_FALSE(disk->FormatSector(buffer.data(), -1, 1, 0, 1));
    EXPECT_FALSE(disk->FormatSector(buffer.data(), 256, 1, 0, 1));
    EXPECT_FALSE(disk->FormatSector(buffer.data(), 0, -1, 0, 1));
    EXPECT_FALSE(disk->FormatSector(buffer.data(), 0, 256, 0, 1));
    EXPECT_FALSE(disk->FormatSector(buffer.data(), 0, 1, -1, 1));
    EXPECT_FALSE(disk->FormatSector(buffer.data(), 0, 1, 2, 1));
    EXPECT_FALSE(disk->FormatSector(buffer.data(), 0, 1, 0, 4));

    EXPECT_TRUE(disk->FormatSector(buffer.data(), 0, 1, 0, 1));
    EXPECT_FALSE(disk->FormatSector(buffer.data(), 0, 1, 0, 0));
    EXPECT_FALSE(disk->FormatSector(buffer.data(), 0, 1, 0, 2));
    EXPECT_FALSE(disk->FormatSector(buffer.data(), 0, 1, 0, 3));
    disk.reset();
    fs::remove(path);
}

TEST_F(test_IFlexDiskBySector, FormatSector_format_flex_disk)
{
    std::array<Byte, SECTOR_SIZE> buffer{};
    const auto ft_access = FileTimeAccess::Get | FileTimeAccess::Set;
    const std::ios::openmode newmode = std::ios::in | std::ios::out |
        std::ios::binary | std::ios::trunc;
    const std::string filename = "format_flex.flx";
    const std::string path = fs::temp_directory_path() / filename;

    // Format disk with all standard formats (except for harddisk).
    for (const auto &flex_format : flex_formats)
    {
        int trcks = flex_format.trk;
        int secs = flex_format.sec;

        // Format harddisk is supported but not tested to limit test time.
        if (secs == 255)
        {
            continue;
        }

        std::fstream fs(path, newmode);
        ASSERT_TRUE(fs.is_open());
        fs.close();

        auto disk = static_cast<IFlexDiskBySectorPtr>(new FlexDisk(path,
                    newmode, ft_access));
        for (int track = 0; track < trcks; ++track)
        {
            for (int sector = 1; sector <= secs; ++sector)
            {
                auto next = (sector == secs) ?
                    st_t{static_cast<Byte>(track + 1), 1} :
                    st_t{static_cast<Byte>(track),
                         static_cast<Byte>(sector + 1)};
                next = ((track == 0 && (sector < 5 || sector == secs)) ||
                        (track == trcks - 1 && sector == secs)) ?
                    st_t{} : next;
                buffer[0] = next.trk;
                buffer[1] = next.sec;
                EXPECT_TRUE(disk->FormatSector(buffer.data(), track, sector,
                            0, 1)) << "track=" << track << " sector=" << sector;
            }
        }

        EXPECT_FALSE(disk->IsFlexFormat());
        struct s_sys_info_sector sis{};
        auto name = filename.substr(0, filename.find_first_of('.'));
        name = flx::toupper(name);
        std::copy_n(name.cbegin(), FLEX_DISKNAME_LENGTH,
                std::begin(sis.sir.disk_name));
        sis.sir.last = st_t{static_cast<Byte>(trcks - 1),
                            static_cast<Byte>(secs)};
        sis.sir.fc_start = st_t{1, 1};
        sis.sir.fc_end = sis.sir.last;
        auto free = static_cast<Word>((trcks - 1) * secs);
        flx::setValueBigEndian<Word>(&sis.sir.free[0], free);
        sis.sir.month = 8;
        sis.sir.day = 21;
        sis.sir.year = 24;
        EXPECT_TRUE(disk->WriteSector(reinterpret_cast<Byte *>(&sis), 0, 3, 0));
        EXPECT_TRUE(disk->IsFlexFormat());

        FlexDiskCheck checker(*disk, ft_access);
        EXPECT_TRUE(checker.CheckFileSystem());
        disk.reset();
        fs::remove(path);
    }
}

TEST_F(test_IFlexDiskBySector, FormatSector_format_flex_disk_interleave)
{
    std::array<Byte, SECTOR_SIZE> buffer{};
    const auto ft_access = FileTimeAccess::Get | FileTimeAccess::Set;
    const std::ios::openmode newmode = std::ios::in | std::ios::out |
        std::ios::binary | std::ios::trunc;
    const std::string filename = "format_interleave.flx";
    const std::string path = fs::temp_directory_path() / filename;
    const int interleave = 3;

    std::fstream fs(path, newmode);
    ASSERT_TRUE(fs.is_open());
    fs.close();

    auto disk = static_cast<IFlexDiskBySectorPtr>(new FlexDisk(path,
                newmode, ft_access));
    for (int track = 0; track < tracks; ++track)
    {
        int sector = 1;
        for (int count = 1; count <= sectors; ++count)
        {
            auto next = (sector == sectors) ?
                st_t{static_cast<Byte>(track + 1), 1} :
                st_t{static_cast<Byte>(track),
                     static_cast<Byte>(sector + 1)};
            next = ((track == 0 && (sector < 5 || sector == sectors)) ||
                    (track == tracks - 1 && sector == sectors)) ?
                st_t{} : next;
            buffer[0] = next.trk;
            buffer[1] = next.sec;
            EXPECT_TRUE(disk->FormatSector(buffer.data(), track, sector,
                        0, 1)) << "track=" << track << " sector=" << sector;
            sector = (((sector - 1) + interleave) % sectors) + 1;
        }
    }

    EXPECT_FALSE(disk->IsFlexFormat());
    struct s_sys_info_sector sis{};
    auto name = filename.substr(0, filename.find_first_of('.'));
    name = flx::toupper(name);
    std::copy_n(name.cbegin(), FLEX_DISKNAME_LENGTH,
            std::begin(sis.sir.disk_name));
    sis.sir.last = st_t{static_cast<Byte>(tracks - 1),
                        static_cast<Byte>(sectors)};
    sis.sir.fc_start = st_t{1, 1};
    sis.sir.fc_end = sis.sir.last;
    auto free = static_cast<Word>((tracks - 1) * sectors);
    flx::setValueBigEndian<Word>(&sis.sir.free[0], free);
    sis.sir.month = 8;
    sis.sir.day = 21;
    sis.sir.year = 24;
    EXPECT_TRUE(disk->WriteSector(reinterpret_cast<Byte *>(&sis), 0, 3, 0));
    EXPECT_TRUE(disk->IsFlexFormat());

    FlexDiskCheck checker(*disk, ft_access);
    EXPECT_TRUE(checker.CheckFileSystem());
    disk.reset();
    fs::remove(path);
}

TEST_F(test_IFlexDiskBySector, FormatSector_format_cpm_disk)
{
    // Format a CP/M disk with 40 tracks, 5 sectors, two sides,
    // 1024 byte/sector.
    const int sector_size = 1024;
    std::array<Byte, sector_size> buffer{};
    const auto ft_access = FileTimeAccess::Get | FileTimeAccess::Set;
    const std::ios::openmode newmode = std::ios::in | std::ios::out |
        std::ios::binary | std::ios::trunc;
    const std::string filename = "format_cpm.flx";
    const std::string path = fs::temp_directory_path() / filename;
    const int trcks = 40;
    const int secs = 5;
    struct stat sbuf{};

    std::fstream fs(path, newmode);
    ASSERT_TRUE(fs.is_open());
    fs.close();

    std::fill(buffer.begin(), buffer.end(), '\xE5');
    auto disk = static_cast<IFlexDiskBySectorPtr>(new FlexDisk(path,
                newmode, ft_access));
    for (int track = 0; track < trcks; ++track)
    {
        for (int sector = 1; sector <= secs; ++sector)
        {
            EXPECT_TRUE(disk->FormatSector(buffer.data(), track, sector,
                        0, 3)) << "track=" << track << " sector=" << sector;
            EXPECT_TRUE(disk->FormatSector(buffer.data(), track, sector,
                        1, 3)) << "track=" << track << " sector=" << sector;
        }
    }

    EXPECT_FALSE(disk->IsFlexFormat());
    ASSERT_TRUE(stat(path.c_str(), &sbuf) == 0);
    EXPECT_EQ(sbuf.st_size, trcks * secs * 2 * sector_size + 16);
    disk.reset();
    fs::remove(path);
}

TEST_F(test_IFlexDiskBySector, fct_IsFlexFormat)
{
    for (int idx = RO; idx <= GetMaxDiskIndex(); ++idx)
    {
        for (int tidx = DSK; tidx <= DIR; ++tidx)
        {
            auto *disk = disks[idx][tidx].get();

            if (disk == nullptr)
            {
                continue;
            }

            EXPECT_TRUE(disk->IsFlexFormat()) << "path=" << disk->GetPath();
        }
    }
}

TEST_F(test_IFlexDiskBySector, fct_IsTrackValid)
{
    for (int idx = RO; idx <= GetMaxDiskIndex(); ++idx)
    {
        for (int tidx = DSK; tidx <= DIR; ++tidx)
        {
            auto *disk = disks[idx][tidx].get();

            if (disk == nullptr)
            {
                continue;
            }

            for (int track = 0; track < tracks; ++track)
            {
                EXPECT_TRUE(disk->IsTrackValid(track))
                        << "path=" << disk->GetPath() << " track=" << track;
            }

            EXPECT_FALSE(disk->IsTrackValid(-1)) << "path=" << disk->GetPath();
            EXPECT_FALSE(disk->IsTrackValid(tracks))
                << "path=" << disk->GetPath();
        }
    }
}

TEST_F(test_IFlexDiskBySector, fct_IsSectorValid)
{
    for (int idx = RO; idx <= GetMaxDiskIndex(); ++idx)
    {
        for (int tidx = DSK; tidx <= DIR; ++tidx)
        {
            auto *disk = disks[idx][tidx].get();

            if (disk == nullptr)
            {
                continue;
            }

            for (int track = 0; track < tracks; ++track)
            {
                for (int sector = 1; sector <= sectors; ++sector)
                {
                    EXPECT_TRUE(disk->IsSectorValid(track, sector))
                        << "track=" << track << " sector=" << sector;
                }

                EXPECT_FALSE(disk->IsSectorValid(track, 0))
                    << "path=" << disk->GetPath() << " track=" << track;
                EXPECT_FALSE(disk->IsSectorValid(track, -1))
                    << "path=" << disk->GetPath() << " track=" << track;
                EXPECT_FALSE(disk->IsSectorValid(track, sectors + 1))
                    << "path=" << disk->GetPath() << " track=" << track;
            }

            EXPECT_FALSE(disk->IsSectorValid(-1, 1))
                    << "path=" << disk->GetPath();
            EXPECT_FALSE(disk->IsSectorValid(0, -1))
                    << "path=" << disk->GetPath();
            EXPECT_FALSE(disk->IsSectorValid(-1, -1))
                    << "path=" << disk->GetPath();
            EXPECT_FALSE(disk->IsSectorValid(tracks, 1))
                    << "path=" << disk->GetPath();
        }
    }
}

TEST_F(test_IFlexDiskBySector, fct_GetBytesPerSector)
{
    for (int idx = RO; idx <= GetMaxDiskIndex(); ++idx)
    {
        for (int tidx = DSK; tidx <= DIR; ++tidx)
        {
            auto *disk = disks[idx][tidx].get();

            if (disk == nullptr)
            {
                continue;
            }

            EXPECT_EQ(disk->GetBytesPerSector(), SECTOR_SIZE)
                << " path=" << disk->GetPath();
        }
    }
}

TEST_F(test_IFlexDiskBySector, exec_FlexDiskCheck)
{
    const auto ft_access = FileTimeAccess::Get | FileTimeAccess::Set;

    for (int idx = RO; idx <= GetMaxDiskIndex(); ++idx)
    {
        for (int tidx = DSK; tidx <= DIR; ++tidx)
        {
            auto *disk = disks[idx][tidx].get();

            if (disk == nullptr)
            {
                continue;
            }

            FlexDiskCheck checker(*disk, ft_access);
            EXPECT_TRUE(checker.CheckFileSystem())
                << " path=" << disk->GetPath();
        }
    }
}
