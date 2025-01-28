/*
    fixt_filecont.h


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
    const int RO{0}; // read-only disk image index.
    const int RW{1}; // read-write disk image index.
    const int FT{2}; // read-write disk image index with file time.
    const int TGT{3};// read-write disk image index used as copy target.
    const int ROM{4}; // read-only disk image index stored in RAM (DSK only).
    const int RAM{5}; // read-write disk image index stored in RAM (DSK only).
    // Second index of diskPaths or disks:
    const int DSK{0};// *.dsk disk image file index.
    const int FLX{1};// *.flx disk image file index.
    const int DIR{2};// directory disk index.

    const std::array<std::array<const char *, 3>, 6> diskPaths{{
        {"/tmp/testdisk_ro.dsk", "/tmp/testdisk_ro.flx", "/tmp/testdir_ro"},
        {"/tmp/testdisk_rw.dsk", "/tmp/testdisk_rw.flx", "/tmp/testdir_rw"},
        {"/tmp/testdisk_ft.dsk", "/tmp/testdisk_ft.flx", "/tmp/testdir_ft"},
        {"/tmp/testdisk_tgt.dsk", "/tmp/testdisk_tgt.flx", "/tmp/testdir_tgt"},
        {"/tmp/testdisk_rom.dsk", "/tmp/testdisk_rom.flx", ""},
        {"/tmp/testdisk_ram.dsk", "/tmp/testdisk_ram.flx", ""},
    }};

    const int tracks = 35;
    const int sectors = 10;

    virtual int GetMaxDiskIndex()
    {
        return RAM;
    }

    virtual int GetMaxDirIndex()
    {
        return TGT;
    }

    void SetUp() override
    {
        for (int idx = RO; idx <= GetMaxDiskIndex(); ++idx)
        {
            for (int tidx = DSK; tidx <= FLX; ++tidx)
            {
                if (idx == TGT)
                {
                    auto *pdisk = FlexDisk::Create(diskPaths[idx][tidx],
                            no_ft, tracks, sectors, DiskType::DSK);
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
        }

        for (int idx = RO; idx <= GetMaxDirIndex(); ++idx)
        {
            if (diskPaths[idx][DIR][0] != '\0')
            {
                ASSERT_TRUE(fs::create_directory(diskPaths[idx][DIR])) <<
                    "path=" << diskPaths[idx][DIR];
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
            for (int idx = RO; idx <= GetMaxDirIndex(); ++idx)
            {
                if (idx != TGT)
                {
                    const auto &ft = (idx == FT) ? with_ft : no_ft;

                    const auto path = createFile(diskPaths[idx][DIR], filename,
                            isTxt, fi);
                    setDateTime(path, BDate(11, 8, 2024), BTime(22, 1), ft);
                }
            }
        });

        for (int idx = RO; idx <= GetMaxDirIndex(); ++idx)
        {
            auto path = std::string(diskPaths[idx][DIR]) + PATHSEPARATOR;
            path = path += "test01.txt";
            struct stat sbuf{};
            static const auto perms = fs::perms::owner_write |
                                      fs::perms::group_write |
                                      fs::perms::others_write;

            if (!stat(path.c_str(), &sbuf))
            {
                fs::permissions(path, perms, fs::perm_options::remove);
            }

            if (idx == RO && !stat(diskPaths[idx][DIR], &sbuf))
            {
                fs::permissions(diskPaths[idx][DIR], perms,
                        fs::perm_options::remove);
            }
        }
    }

    void TearDown() override
    {
        for (int idx = RO; idx <= GetMaxDiskIndex(); ++idx)
        {
            fs::remove(diskPaths[idx][DSK]);
            fs::remove(diskPaths[idx][FLX]);
        }

        for (int idx = RO; idx <= GetMaxDirIndex(); ++idx)
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

