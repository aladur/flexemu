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
#include "bdir.h"
#include <string>
#include <array>
#include <fstream>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;


class test_BDirectory : public ::testing::Test
{
protected:
    static constexpr const unsigned HAS_FILES{1U};
    static constexpr const unsigned HAS_SUBDIRS{2U};
    static constexpr const unsigned HAS_UTF8{4U};
    static constexpr const unsigned MAX_INDEX{7U};

    static constexpr const std::array<const char *, 9> filenames{{
        "f", "filename.txt", "_filename.txt",
        "~file", "_", "@",
        "file_sp_ .txt",
        "this_is_a_very_long_filename_with_an.extension",
        u8"filename\u2665.ext\u2665",
    }};

    static constexpr const std::array<const char *, 9> subdirnames{{
        "d", "dirname", "_dir.ext",
        "~dir", "__", "@@",
        "dir_sp_ .ext",
        "this_is_a_very_long_dirname",
        u8"directory\u2665",
    }};

    static fs::path createTestDirectoryPathFor(unsigned index)
    {
        fs::path dirname("testdir");
        bool withUtf8 = ((index & HAS_UTF8) != 0);

        if (index & HAS_FILES)
        {
            dirname += "_files";
        }

        if (index & HAS_SUBDIRS)
        {
            dirname += "_subdirs";
        }

        if (withUtf8)
        {
            dirname += fs::u8path(u8"_utf8\u2665");
        }

        return fs::temp_directory_path() / dirname;
    }

    static fs::path createNameFor(const fs::path &name, bool withUtf8)
    {
        auto path = fs::path(withUtf8 ? u8"utf8\u2665_" : "");
        path += name;
        return path;
    }

    static void addFiles(const fs::path &dirname, bool withUtf8)
    {
        for (const std::string filename : filenames)
        {
            auto path = dirname / createNameFor(filename, withUtf8);
            std::ofstream ofs(path);

            ASSERT_TRUE(ofs.is_open()) << path.u8string();
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
            ASSERT_TRUE(!error) << path.u8string();

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
            ASSERT_TRUE(!error) << path.u8string();
        }
    }

};

TEST_F(test_BDirectory, fct_GetSubDirectories)
{
    bool withUtf8 = false;
    const auto transformType_fct =
        [&](const char *src){
            return createNameFor(src, withUtf8).u8string();
        };

    for (unsigned index = 0; index <= MAX_INDEX; ++index)
    {
        const auto path = createTestDirectoryPathFor(index);
        auto items = BDirectory::GetSubDirectories(path);
        withUtf8 = ((index & HAS_UTF8) != 0);

        if ((index & HAS_SUBDIRS) != 0U)
        {
            PathList_t expectedItems;

            ASSERT_EQ(items.size(), subdirnames.size()) << path.u8string();
            std::transform(subdirnames.cbegin(), subdirnames.cend(),
                std::back_inserter(expectedItems), transformType_fct);
            std::sort(items.begin(), items.end());
            std::sort(expectedItems.begin(), expectedItems.end());
            ASSERT_EQ(items, expectedItems) << path.u8string();
        }
        else
        {
            ASSERT_TRUE(items.empty()) << path.u8string();
        }
    }
}

TEST_F(test_BDirectory, fct_GetFiles)
{
    bool withUtf8 = false;
    const auto transformType_fct =
        [&](const char *src){
            return createNameFor(src, withUtf8).u8string();
        };

    for (unsigned index = 0; index <= MAX_INDEX; ++index)
    {
        const auto path = createTestDirectoryPathFor(index);
        auto items = BDirectory::GetFiles(path);
        withUtf8 = ((index & HAS_UTF8) != 0);

        if ((index & HAS_FILES) != 0U)
        {
            PathList_t expectedItems;

            ASSERT_EQ(items.size(), filenames.size()) << path.u8string();
            std::transform(filenames.cbegin(), filenames.cend(),
                std::back_inserter(expectedItems), transformType_fct);
            std::sort(items.begin(), items.end());
            std::sort(expectedItems.begin(), expectedItems.end());
            ASSERT_EQ(items, expectedItems) << path.u8string();
        }
        else
        {
            ASSERT_TRUE(items.empty()) << path.u8string();
        }
    }
}

