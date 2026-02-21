/*
    fixt_filecont.h


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


#include "gtest/gtest.h"
#include "typedefs.h"
#include "ffilecnt.h"
#include "dircont.h"
#include <sys/stat.h>
#ifdef _WIN32
#include <sys/utime.h>
#else
#include <utime.h>
#endif
#include <array>
#include <fstream>
#include <filesystem>
#include <numeric>
#include <fmt/format.h>
#ifdef _WIN32
#include "windefs.h"
#endif


namespace fs = std::filesystem;

class test_FlexDiskFixture : public ::testing::Test
{
protected:
    FileTimeAccess no_ft{FileTimeAccess::NONE};
    FileTimeAccess with_ft{FileTimeAccess::Get | FileTimeAccess::Set};

    // First index of diskFiles, diskPaths or disks:
    const int RO{0}; // read-only disk image index.
    const int RW{1}; // read-write disk image index.
    const int SP{2}; // read-write disk image index with space in path.
    const int U8{3}; // read-write disk image index with utf8-char in path.
    const int FT{4}; // read-write disk image index with file time.
    const int TGT{5};// read-write disk image index used as copy target.
    const int ROM{6}; // read-only disk image index stored in RAM (DSK only).
    const int RAM{7}; // read-write disk image index stored in RAM (DSK only).
    // Second index of diskFiles, diskPaths or disks:
    const int DSK{0};// *.dsk disk image file index.
    const int FLX{1};// *.flx disk image file index.
    const int DIR{2};// directory disk index.

    static constexpr const std::array<std::array<const char *, 3>, 8> diskFiles
    {{
        {"testdisk_ro.dsk", "testdisk_ro.flx", "testdir_ro"},
        {"testdisk_rw.dsk", "testdisk_rw.flx", "testdir_rw"},
        {"testdisk_ .dsk", "testdisk_ .flx", "testdir_ sp"},
        {u8"testdisk_\u2665.dsk", u8"testdisk_\u2665.flx", u8"testdir_\u2665"},
        {"testdisk_ft.dsk", "testdisk_ft.flx", "testdir_ft"},
        {"testdisk_tgt.dsk", "testdisk_tgt.flx", "testdir_tgt"},
        {"testdisk_rom.dsk", "testdisk_rom.flx", ""},
        {"testdisk_ram.dsk", "testdisk_ram.flx", ""},
    }};
    std::array<std::array<fs::path, 3>, 8> diskPaths;

    const int tracks = 35;
    const int sectors = 10;
    const fs::path temp_dir{ fs::temp_directory_path() };

    // Windows does not have the POSIX concept of owner, owner's group, or everybody.
    // Files only can be read-only or "all" access. Directories always have "all" access.
    // (not taking ACL features into account).
    // For details see:
    // https://docs.microsoft.com/en-us/cpp/standard-library/filesystem-enumerations?view=vs-2017#perms
    // https://docs.microsoft.com/en-us/cpp/standard-library/filesystem?view=vs-2017
#ifdef _WIN32
    static const auto write_perms = fs::perms::all;
#else
    static const auto write_perms = fs::perms::owner_write;
#endif

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
        fs::path diskPath;

        ASSERT_EQ(diskFiles.size(), diskPaths.size());
        ASSERT_EQ(diskFiles[RO].size(), diskPaths[RO].size());

        for (int idx = RO; idx <= RAM; ++idx)
        {
            for (int tidx = DSK; tidx <= DIR; ++tidx)
            {
                if (diskFiles[idx][tidx][0] == '\0')
                {
                    continue;
                }

                diskPath = temp_dir / fs::u8path(diskFiles[idx][tidx]);
                diskPaths[idx][tidx] = diskPath;
            }
        }

        for (int idx = RO; idx <= GetMaxDiskIndex(); ++idx)
        {
            for (int tidx = DSK; tidx <= FLX; ++tidx)
            {
                diskPath = diskPaths[idx][tidx];
                if (idx == TGT)
                {
                    auto *pdisk = FlexDisk::Create(diskPath,
                            no_ft, tracks, sectors, DiskType::DSK);
                    ASSERT_NE(pdisk, nullptr);
                    delete pdisk;
                }
                else
                {
                    auto ext = diskPath.extension().u8string();
                    const auto srcFile = fs::path(F_TESTDATADIR) /
                        (std::string("testdisk") + ext);
                    ASSERT_TRUE(fs::copy_file(srcFile, diskPath));
                    fs::permissions(diskPath, write_perms,
                            fs::perm_options::add);
                }
            }
        }

        for (int idx = RO; idx <= GetMaxDirIndex(); ++idx)
        {
            diskPath = diskPaths[idx][DIR];
            ASSERT_TRUE(fs::create_directory(diskPath)) << "path=" << diskPath;
#ifndef _WIN32
            fs::permissions(diskPath, write_perms, fs::perm_options::add);
#endif
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
                diskPath = diskPaths[idx][DIR];
                if (idx == TGT)
                {
                    continue;
                }

                const auto &ft = (idx == FT) ? with_ft : no_ft;
                const auto filePath = createFile(diskPath, filename, isTxt, fi);
                setDateTime(filePath.u8string(), BDate(11, 8, 2024),
                    BTime(22, 1), ft);
            }
        });

        for (int idx = RO; idx <= GetMaxDirIndex(); ++idx)
        {
            diskPath = diskPaths[idx][DIR];
            const auto filePath = diskPath / "test01.txt";

            if (fs::exists(filePath))
            {
                fs::permissions(filePath, write_perms,
                        fs::perm_options::remove);
            }

#ifndef _WIN32
            if (idx == RO && fs::exists(diskPath))
            {
                fs::permissions(diskPath, write_perms,
                        fs::perm_options::remove);
            }
#endif
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
            const auto diskPath = diskPaths[idx][DIR];
            const auto filePath = diskPath / "test01.txt";

#ifndef _WIN32
            fs::permissions(diskPath, write_perms, fs::perm_options::add);
#endif
            if (fs::exists(filePath))
            {
                fs::permissions(filePath, write_perms, fs::perm_options::add);
            }
            fs::remove_all(diskPath);
        }
    }

    static fs::path createFile(
            const fs::path &directory,
            const std::string &filename,
            bool isText, int sectors)
    {
        const auto path = directory / filename;
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

        EXPECT_TRUE(ofs.is_open()) << "path=" << path << "\n";
        for (int sector = 0; sector < sectors; ++sector)
        {
            ofs.write(reinterpret_cast<const char *>(line.data()), line.size());
            ofs.write(reinterpret_cast<const char *>(line.data()), line.size());
            ofs.write(reinterpret_cast<const char *>(line.data()), line.size());
            ofs.write(reinterpret_cast<const char *>(line.data()), line.size());
        }

        return path;
    }

    static bool setDateTime(const fs::path &path, const BDate &date,
            const BTime &time, FileTimeAccess ft_access)
    {
      struct stat sbuf{};
      struct utimbuf timebuf{};
      struct tm file_time{};
      const bool setFileTime =
          (ft_access & FileTimeAccess::Set) == FileTimeAccess::Set;

#ifdef _WIN32
      if (_wstat(path.wstring().c_str(), &sbuf) == 0)
#else
      if (stat(path.u8string().c_str(), &sbuf) == 0)
#endif
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

#ifdef _WIN32
          return _wutime(path.wstring().c_str(), &timebuf) == 0;
#else
          return utime(path.u8string().c_str(), &timebuf) == 0;
#endif
      }

      return false;
    }
};

