#include "gtest/gtest.h"
#include "ffilecnt.h"
#include "dircont.h"
#include <array>
#include <fstream>
#include <filesystem>
#include <numeric>
#include <fmt/format.h>


namespace fs = std::filesystem;

class test_FlexDiskFixture : public ::testing::Test
{
protected:
    FileTimeAccess no_ft{FileTimeAccess::NONE};
    FileTimeAccess with_ft{FileTimeAccess::Get | FileTimeAccess::Set};

    // First index of diskPaths or disks:
    const int RO{0}; // read-only disk image index stored in RAM.
    const int ROM{1}; // read-only disk image index (DSK only).
    const int RW{2}; // read-write disk image index.
    const int RAM{3}; // read-write disk image index stored in RAM (DSK only).
    const int FT{4}; // read-write disk image index with file time.
    const int TGT{5};// read-write disk image index used as copy target.
    // Second index of diskPaths or disks:
    const int DSK{0};// *.dsk disk image file index.
    const int FLX{1};// *.flx disk image file index.
    const int DIR{2};// directory disk index.

    const std::array<std::array<const char *, 3>, 6> diskPaths{{
        {"/tmp/testdisk_ro.dsk", "/tmp/testdisk_ro.flx", "/tmp/testdir_ro"},
        {"/tmp/testdisk_rom.dsk", "/tmp/testdisk_rom.flx", ""},
        {"/tmp/testdisk_rw.dsk", "/tmp/testdisk_rw.flx", "/tmp/testdir_rw"},
        {"/tmp/testdisk_ram.dsk", "/tmp/testdisk_ram.flx", ""},
        {"/tmp/testdisk_ft.dsk", "/tmp/testdisk_ft.flx", "/tmp/testdir_ft"},
        {"/tmp/testdisk_tgt.dsk", "/tmp/testdisk_tgt.flx", "/tmp/testdir_tgt"}
    }};

    const int tracks = 35;
    const int sectors = 10;

    void SetUp() override
    {
        for (int idx = RO; idx <= TGT; ++idx)
        {
            for (int tidx = DSK; tidx <= FLX; ++tidx)
            {
                if (idx == TGT)
                {
                    auto *pdisk = FlexDisk::Create(diskPaths[idx][tidx],
                            no_ft, tracks, sectors);
                    ASSERT_NE(pdisk, nullptr);
                    delete pdisk;
                }
                else
                {
                    auto ext = flx::getFileExtension(diskPaths[idx][tidx]);
                    ASSERT_TRUE(fs::copy_file(fs::current_path() /
                            (std::string("data/testdisk" + ext)),
                            diskPaths[idx][tidx]));
                    fs::permissions(diskPaths[idx][tidx],
                            fs::perms::owner_write, fs::perm_options::add);
                }
            }

            if (diskPaths[idx][DIR][0] != '\0')
            {
                ASSERT_TRUE(fs::create_directory(diskPaths[idx][DIR]));
                    fs::permissions(diskPaths[idx][DIR],
                            fs::perms::owner_write, fs::perm_options::add);
            }
        }

        std::array<int, 20> indices{};
        std::iota(indices.begin(), indices.end(), 0);
        std::for_each(indices.begin(), indices.end(), [&](int val){
            const auto fi = (val % 10) + 1;
            const bool isTxt = val < 10;
            std::string ext = (isTxt ? "txt" : "bin");
            const auto filename = fmt::format("test{:02}.{}", fi, ext);
            std::vector<int> dirIndices{RO, RW, FT};
            for (int idx : dirIndices)
            {
                std::string path;
                const auto &ft = (idx == FT) ? with_ft : no_ft;

                if (diskPaths[idx][DIR][0] != '\0')
                {
                    path = createFile(diskPaths[idx][DIR], filename, isTxt,
                           fi);
                    setDateTime(path, BDate(11, 8, 2024), BTime(22, 1), ft);
                }
            }
        });

        std::vector<int> dirIndices{RO, RW, FT, TGT};
        for (int idx : dirIndices)
        {
            auto path = std::string(diskPaths[idx][DIR]) + PATHSEPARATOR;
            path = path += "test01.txt";
            struct stat sbuf{};

            if (!stat(diskPaths[idx][DIR], &sbuf))
            {
                chmod(path.c_str(), sbuf.st_mode &
                        static_cast<unsigned>(~S_IWUSR));
            }
            if (idx == RO)
            {

                if (!stat(diskPaths[idx][DIR], &sbuf))
                {
                    chmod(diskPaths[idx][DIR], sbuf.st_mode &
                            static_cast<unsigned>(~S_IWUSR));
                }
            }
        }
    }

    void TearDown() override
    {
        for (int idx = RO; idx <= TGT; ++idx)
        {
            fs::remove(diskPaths[idx][DSK]);
            fs::remove(diskPaths[idx][FLX]);
            if (diskPaths[idx][DIR][0] != '\0')
            {
                fs::permissions(diskPaths[idx][DIR], fs::perms::owner_write,
                        fs::perm_options::add);
                auto path = std::string(diskPaths[idx][DIR]) + PATHSEPARATOR;
                path += "test01.txt";
                if (fs::exists(path))
                {
                    fs::permissions(path, fs::perms::owner_write,
                            fs::perm_options::add);
                }
                fs::remove_all(diskPaths[idx][DIR]);
            }
        }
    }

    static std::string createFile(
            const std::string &directory,
            const std::string &filename,
            bool isText, int sectors)
    {
        const auto path = directory + PATHSEPARATOR + filename;
        const auto mode =
            isText ? std::ios::out : (std::ios::out | std::ios::binary);
        std::array<Byte, 63> line{};
        std::fstream ofs(path, mode);
        if (isText)
        {
            std::iota(line.begin(), line.begin() + 10, '0');
            std::iota(line.begin() + 10, line.begin() + 36, 'A');
            std::iota(line.begin() + 36, line.end() - 1, 'a');
            *(line.end() - 1) = '\r';
        }
        else
        {
            std::iota(line.begin(), line.end(), '\0');
        }

        EXPECT_TRUE(ofs.is_open());
        for (int sector = 0; sector < sectors; ++sector)
        {
            ofs.write(reinterpret_cast<const char *>(line.data()), line.size());
            ofs.write(reinterpret_cast<const char *>(line.data()), line.size());
            ofs.write(reinterpret_cast<const char *>(line.data()), line.size());
            ofs.write(reinterpret_cast<const char *>(line.data()), line.size());
        }

        return path;
    }

    static bool setDateTime(const std::string &path, const BDate &date,
            const BTime &time, FileTimeAccess ft_access)
    {
      struct stat sbuf{};
      struct utimbuf timebuf{};
      struct tm file_time{};
      const bool setFileTime =
          (ft_access & FileTimeAccess::Set) == FileTimeAccess::Set;

      if (stat(path.c_str(), &sbuf) == 0)
      {
          timebuf.actime = sbuf.st_atime;
          file_time.tm_sec = 0;
          file_time.tm_min = setFileTime ? time.GetMinute() : 0;
          file_time.tm_hour = setFileTime ? time.GetHour() : 0;
          file_time.tm_mon = date.GetMonth() - 1;
          file_time.tm_mday = date.GetDay();
          file_time.tm_year = date.GetYear() - 1900;
          file_time.tm_isdst = -1;
          timebuf.modtime = mktime(&file_time);

          return utime(path.c_str(), &timebuf) == 0;
      }

      return false;
    }
};

