/*
    test_bdir.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2025  W. Schwotzer

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
#include "bdir.h"
#include <string>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;


class test_BDirectory : public ::testing::Test
{
protected:
    static const unsigned HAS_FILES{1U};
    static const unsigned HAS_SUBDIRS{2U};
    static const unsigned HAS_UTF8{4U};
    static const unsigned MAX_INDEX{7U};

    static constexpr std::array<const char *, 8> filenames{{
        u8"f", u8"filename.txt", u8"_filename.txt",
        u8"~file", u8"_", u8"@",
        u8"file_sp_ .txt",
        u8"this_is_a_very_long_filename_with_an.extension",
    }};

    static constexpr std::array<const char *, 8> subdirnames{{
        u8"d", u8"dirname", u8"_dir.ext",
        u8"~dir", u8"__", u8"@@",
        u8"dir_sp_ .txt",
        u8"this_is_a_very_long_dirname",
    }};

    static fs::path createTestDirectoryPathFor(unsigned index)
    {
        std::string dirname = u8"testdir";
        bool withUtf8 = ((index & HAS_UTF8) != 0);

        if (index & HAS_FILES)
        {
            dirname += u8"_files";
        }

        if (index & HAS_SUBDIRS)
        {
            dirname += u8"_subdirs";
        }

        if (withUtf8)
        {
            dirname += u8"_utf8\u2665";
        }

        return fs::temp_directory_path() / dirname;
    }

    static std::string createNameFor(const std::string &name, bool withUtf8)
    {
        const auto prefix = std::string(withUtf8 ? u8"utf8\u2665_" : u8"");

        return prefix + name;
    }

    static void addFiles(const fs::path &dirname, bool withUtf8)
    {
        for (const std::string filename : filenames)
        {
            auto path = dirname / createNameFor(filename, withUtf8);
            std::ofstream ofs(path);

            ASSERT_TRUE(ofs.is_open());
            ofs << "arbitrary_file_content";
            ofs.close();
        }
    }

    static void addSubdirs(const fs::path &dirname, bool withUtf8)
    {
        for (const std::string subdir : subdirnames)
        {
            auto path = dirname / createNameFor(subdir, withUtf8);
            std::error_code error;

            fs::create_directories(path, error);
            ASSERT_TRUE(!error);
        }

    }
public:
    void SetUp() override
    {
        for (unsigned index = 0; index <= MAX_INDEX; ++index)
        {
            const auto path = createTestDirectoryPathFor(index);
            bool withUtf8 = ((index & HAS_UTF8) != 0);
            std::error_code error;

            fs::create_directory(path, error);
            ASSERT_TRUE(!error);

            if ((index & HAS_FILES) != 0)
            {
                addFiles(path, withUtf8);
            }
            if ((index & HAS_SUBDIRS) != 0)
            {
                addSubdirs(path, withUtf8);
            }
        }
    }

    void TearDown() override
    {
        for (unsigned index = 0; index <= MAX_INDEX; ++index)
        {
            const auto path = createTestDirectoryPathFor(index);
            std::error_code error;

            fs::remove_all(path, error);
            ASSERT_TRUE(!error);
        }
    }

};

TEST_F(test_BDirectory, fct_GetSubDirectories)
{
    for (unsigned index = 0; index <= MAX_INDEX; ++index)
    {
        const auto path = createTestDirectoryPathFor(index);
        const auto items = BDirectory::GetSubDirectories(path.u8string());

        if ((index & HAS_SUBDIRS) != 0U)
        {
            ASSERT_EQ(items.size(), subdirnames.size());
        }
        else
        {
            ASSERT_TRUE(items.empty());
        }
    }
}

TEST_F(test_BDirectory, fct_GetFiles)
{
    for (unsigned index = 0; index <= MAX_INDEX; ++index)
    {
        const auto path = createTestDirectoryPathFor(index);
        const auto items = BDirectory::GetFiles(path.u8string());

        for (const auto &item : items)
        {
            std::cout << item << "\n";
        }

        if ((index & HAS_FILES) != 0U)
        {
            ASSERT_EQ(items.size(), filenames.size());
        }
        else
        {
            ASSERT_TRUE(items.empty());
        }
    }
}

