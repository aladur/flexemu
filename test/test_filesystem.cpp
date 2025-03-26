/*
    test_filesystem.cpp


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
#include <filesystem>


using ::testing::EndsWith;
namespace fs = std::filesystem;

// Test std::filesystem behaviour just do see how it works
// and handles corner cases and unicode support.
TEST(test_filesystem, fct_temp_directory_path)
{
    const auto path = fs::temp_directory_path();
    const auto status = fs::status(path);

    EXPECT_TRUE(!path.empty());
    EXPECT_TRUE(fs::exists(path));
    EXPECT_TRUE(fs::is_directory(status));
}

TEST(test_filesystem, fct_current_path)
{
    const auto path = fs::current_path();
    const auto status = fs::status(path);
    EXPECT_TRUE(!path.empty());
    EXPECT_TRUE(fs::exists(path));
    EXPECT_TRUE(fs::is_directory(status));
}

TEST(test_filesystem, fct_path_absolute)
{
    const auto path = fs::temp_directory_path();
    auto absolute_path = fs::absolute(path);
    EXPECT_TRUE(absolute_path.is_absolute());
    auto absolute_path_string = fs::absolute(u8"tmp").u8string();
    EXPECT_THAT(absolute_path_string, EndsWith(u8"tmp"));
}

TEST(test_filesystem, fct_path_filename)
{
    auto result = fs::u8path(u8"").filename();
    EXPECT_EQ(result, fs::u8path(u8""));
    result = fs::u8path(u8".").filename();
    EXPECT_EQ(result, fs::u8path(u8"."));
    result = fs::u8path(u8"..").filename();
    EXPECT_EQ(result, fs::u8path(u8".."));
    result = fs::u8path(u8"filename").filename();
    EXPECT_EQ(result, fs::u8path(u8"filename"));
    result = fs::u8path(u8"filename.ext").filename();
    EXPECT_EQ(result, fs::u8path(u8"filename.ext"));
    result = fs::u8path(u8"filename.ext1.ext2").filename();
    EXPECT_EQ(result, fs::u8path(u8"filename.ext1.ext2"));
    result = fs::u8path(u8".filename").filename();
    EXPECT_EQ(result, fs::u8path(u8".filename"));
    result = fs::u8path(u8"..filename").filename();
    EXPECT_EQ(result, fs::u8path(u8"..filename"));
    result = fs::u8path(u8"filename\u2665.ext\u2665").filename();
    EXPECT_EQ(result, fs::u8path(u8"filename\u2665.ext\u2665"));
#ifdef _WIN32
    result = fs::u8path(u8"C:\\").filename();
    EXPECT_EQ(result, fs::u8path(u8""));
    result = fs::u8path(u8"C:\\dir1\\dir2\\.").filename();
    EXPECT_EQ(result, fs::u8path(u8"."));
    result = fs::u8path(u8"C:\\dir1\\dir2\\..").filename();
    EXPECT_EQ(result, fs::u8path(u8".."));
    result = fs::u8path(u8"C:\\dir1\\dir2\\").filename();
    EXPECT_EQ(result, fs::u8path(u8""));
    result = fs::u8path(u8"C:\\dir1\\dir2").filename();
    EXPECT_EQ(result, fs::u8path(u8"dir2"));
    result = fs::u8path(u8"C:\\dir\\filename.ext").filename();
    EXPECT_EQ(result, fs::u8path(u8"filename.ext"));
    result = fs::u8path(u8"C:\\dir\\filename.ext1.ext2").filename();
    EXPECT_EQ(result, fs::u8path(u8"filename.ext1.ext2"));
    result = fs::u8path(u8"C:\\dir\\.filename").filename();
    EXPECT_EQ(result, fs::u8path(u8".filename"));
    result = fs::u8path(u8"C:\\dir\\..filename").filename();
    EXPECT_EQ(result, fs::u8path(u8"..filename"));
    const auto path = u8"C:\\dir\u2665\\filename\u2665.ext\u2665";
    result = fs::u8path(path).filename();
    EXPECT_EQ(result, fs::u8path(u8"filename\u2665.ext\u2665"));
#else
    result = fs::u8path(u8"/").filename();
    EXPECT_EQ(result, fs::u8path(u8""));
    result = fs::u8path(u8"/dir1/dir2/.").filename();
    EXPECT_EQ(result, fs::u8path(u8"."));
    result = fs::u8path(u8"/dir1/dir2/..").filename();
    EXPECT_EQ(result, fs::u8path(u8".."));
    result = fs::u8path(u8"/dir1/dir2/").filename();
    EXPECT_EQ(result, fs::u8path(u8""));
    result = fs::u8path(u8"/dir1/dir2").filename();
    EXPECT_EQ(result, fs::u8path(u8"dir2"));
    result = fs::u8path(u8"/dir/filename.ext").filename();
    EXPECT_EQ(result, fs::u8path(u8"filename.ext"));
    result = fs::u8path(u8"/dir/filename.ext1.ext2").filename();
    EXPECT_EQ(result, fs::u8path(u8"filename.ext1.ext2"));
    result = fs::u8path(u8"/dir/.filename").filename();
    EXPECT_EQ(result, fs::u8path(u8".filename"));
    result = fs::u8path(u8"/dir/..filename").filename();
    EXPECT_EQ(result, fs::u8path(u8"..filename"));
    const auto *path = u8"/dir\u2665/filename\u2665.ext\u2665";
    result = fs::u8path(path).filename();
    EXPECT_EQ(result, fs::u8path(u8"filename\u2665.ext\u2665"));
#endif
}

TEST(test_filesystem, fct_path_extension)
{
    auto result = fs::u8path(u8"").extension();
    EXPECT_EQ(result, fs::u8path(u8""));
    result = fs::u8path(u8".").extension();
    EXPECT_EQ(result, fs::u8path(u8""));
    result = fs::u8path(u8"..").extension();
    EXPECT_EQ(result, fs::u8path(u8""));
    result = fs::u8path(u8"filename.ext").extension();
    EXPECT_EQ(result, fs::u8path(u8".ext"));
    result = fs::u8path(u8"filename.ext1.ext2").extension();
    EXPECT_EQ(result, fs::u8path(u8".ext2"));
    result = fs::u8path(u8"filename").extension();
    EXPECT_EQ(result, "");
    EXPECT_EQ(result, fs::u8path(u8""));
    result = fs::u8path(u8"filename..ext").extension();
    EXPECT_EQ(result, fs::u8path(u8".ext"));
    result = fs::u8path(u8"filename\u2665.ext\u2665").extension();
    EXPECT_EQ(result, fs::u8path(u8".ext\u2665"));
#ifdef _WIN32
    result = fs::u8path(u8"C:\\").extension();
    EXPECT_EQ(result, "");
    result = fs::u8path(u8"C:\\dir\\.").extension();
    EXPECT_EQ(result, "");
    result = fs::u8path(u8"C:\\dir\\..").extension();
    EXPECT_EQ(result, "");
    result = fs::u8path(u8"C:\\dir\\").extension();
    EXPECT_EQ(result, "");
    result = fs::u8path(u8"C:\\dir").extension();
    EXPECT_EQ(result, "");
    result = fs::u8path(u8"C:\\dir\\filename.ext").extension();
    EXPECT_EQ(result, ".ext");
    const auto *path = u8"C:\\dir\u2665\\filename\u2665.ext\u2665";
    result = fs::u8path(path).extension();
    EXPECT_EQ(result, fs::u8path(u8".ext\u2665"));
#else
    result = fs::u8path(u8"/").extension();
    EXPECT_EQ(result, "");
    result = fs::u8path(u8"/dir/.").extension();
    EXPECT_EQ(result, "");
    result = fs::u8path(u8"/dir/..").extension();
    EXPECT_EQ(result, "");
    result = fs::u8path(u8"/dir/").extension();
    EXPECT_EQ(result, "");
    result = fs::u8path(u8"/dir").extension();
    EXPECT_EQ(result, "");
    result = fs::u8path(u8"/dir/filename.ext").extension();
    EXPECT_EQ(result, fs::u8path(u8".ext"));
    result = fs::u8path(u8"/dir\u2665/filename\u2665.ext\u2665").extension();
    EXPECT_EQ(result, fs::u8path(u8".ext\u2665"));
#endif
}

TEST(test_filesystem, fct_path_stem)
{
    auto result = fs::u8path(u8"").stem();
    EXPECT_EQ(result, fs::u8path(u8""));
    result = fs::u8path(u8".").stem();
    EXPECT_EQ(result, fs::u8path(u8"."));
    result = fs::u8path(u8"..").stem();
    EXPECT_EQ(result, fs::u8path(u8".."));
    result = fs::u8path(u8".ext").stem();
    EXPECT_EQ(result, fs::u8path(u8".ext"));
    result = fs::u8path(u8".ext1.ext2").stem();
    EXPECT_EQ(result, fs::u8path(u8".ext1"));
    result = fs::u8path(u8"filename").stem();
    EXPECT_EQ(result, fs::u8path(u8"filename"));
    result = fs::u8path(u8"filename.ext").stem();
    EXPECT_EQ(result, fs::u8path(u8"filename"));
    result = fs::u8path(u8"filename.ext1.ext2").stem();
    EXPECT_EQ(result, fs::u8path(u8"filename.ext1"));
    result = fs::u8path(u8"filename..ext").stem();
    EXPECT_EQ(result, fs::u8path(u8"filename."));
    result = fs::u8path(u8"filename\u2665.ext\u2665").stem();
    EXPECT_EQ(result, fs::u8path(u8"filename\u2665"));
#ifdef _WIN32
    result = fs::u8path(u8"C:\\").stem();
    EXPECT_EQ(result, fs::u8path(u8""));
    result = fs::u8path(u8"C:\\dir\\.").stem();
    EXPECT_EQ(result, fs::u8path(u8"."));
    result = fs::u8path(u8"C:\\dir\\..").stem();
    EXPECT_EQ(result, fs::u8path(u8".."));
    result = fs::u8path(u8"C:\\dir\\").stem();
    EXPECT_EQ(result, fs::u8path(u8""));
    result = fs::u8path(u8"C:\\dir\\filename").stem();
    EXPECT_EQ(result, fs::u8path(u8"filename"));
    result = fs::u8path(u8"C:\\dir\\filename.ext").stem();
    EXPECT_EQ(result, fs::u8path(u8"filename"));
    result = fs::u8path(u8"C:\\dir\\filename.ext").stem();
    EXPECT_EQ(result, fs::u8path(u8"filename"));
    result = fs::u8path(u8"C:\\dir\\filename.ext1.ext2").stem();
    EXPECT_EQ(result, fs::u8path(u8"filename.ext1"));
    result = fs::u8path(u8"C:\\dir\u2665\\filename\u2665.ext\u2665").stem();
    EXPECT_EQ(result, fs::u8path(u8"filename\u2665"));
#else
    result = fs::u8path(u8"/").stem();
    EXPECT_EQ(result, fs::u8path(u8""));
    result = fs::u8path(u8"/dir/.").stem();
    EXPECT_EQ(result, fs::u8path(u8"."));
    result = fs::u8path(u8"/dir/..").stem();
    EXPECT_EQ(result, fs::u8path(u8".."));
    result = fs::u8path(u8"/dir/").stem();
    EXPECT_EQ(result, fs::u8path(u8""));
    result = fs::u8path(u8"/dir/filename").stem();
    EXPECT_EQ(result, fs::u8path(u8"filename"));
    result = fs::u8path(u8"/dir/filename.ext").stem();
    EXPECT_EQ(result, fs::u8path(u8"filename"));
    result = fs::u8path(u8"/dir/filename.ext1.ext2").stem();
    EXPECT_EQ(result, fs::u8path(u8"filename.ext1"));
    result = fs::u8path(u8"/dir\u2665/filename\u2665.ext\u2665").stem();
    EXPECT_EQ(result, fs::u8path(u8"filename\u2665"));
#endif
}

TEST(test_filesystem, fct_path_parent_path)
{
    auto result = fs::u8path(u8"").parent_path();
    EXPECT_EQ(result, fs::u8path(u8""));
    result = fs::u8path(u8"filename.txt").parent_path();
    EXPECT_EQ(result, fs::u8path(u8""));
    result = fs::u8path(u8"filename").parent_path();
    EXPECT_EQ(result, fs::u8path(u8""));
#ifdef _WIN32
    result = fs::u8path(u8"C:\\").parent_path();
    EXPECT_EQ(result, fs::u8path(u8"C:\\"));
    result = fs::u8path(u8"C:\\dir\\").parent_path();
    EXPECT_EQ(result, fs::u8path(u8"C:\\dir"));
    result = fs::u8path(u8"C:\\dir\\.").parent_path();
    EXPECT_EQ(result, fs::u8path(u8"C:\\dir"));
    result = fs::u8path(u8"C:\\dir\\..").parent_path();
    EXPECT_EQ(result, fs::u8path(u8"C:\\dir"));
    result = fs::u8path(u8"C:\\dir1\\dir2\\..").parent_path();
    EXPECT_EQ(result, fs::u8path(u8"C:\\dir1\\dir2"));
    result = fs::u8path(u8"C:\\dir1\\dir2\\..\\..").parent_path();
    EXPECT_EQ(result, fs::u8path(u8"C:\\dir1\\dir2\\.."));
    result = fs::u8path(u8"C:\\dir\\filename").parent_path();
    EXPECT_EQ(result, fs::u8path(u8"C:\\dir"));
    result = fs::u8path(u8"C:\\dir\\filename.ext").parent_path();
    EXPECT_EQ(result, fs::u8path(u8"C:\\dir"));
    result = fs::u8path(u8"C:\\dir\u2665\\").parent_path();
    EXPECT_EQ(result, fs::u8path(u8"C:\\dir\u2665"));
#else
    result = fs::u8path(u8"/").parent_path();
    EXPECT_EQ(result, fs::u8path(u8"/"));
    result = fs::u8path(u8"/dir/").parent_path();
    EXPECT_EQ(result, fs::u8path(u8"/dir"));
    result = fs::u8path(u8"/dir/.").parent_path();
    EXPECT_EQ(result, fs::u8path(u8"/dir"));
    result = fs::u8path(u8"/dir/..").parent_path();
    EXPECT_EQ(result, fs::u8path(u8"/dir"));
    result = fs::u8path(u8"/dir1/dir2/..").parent_path();
    EXPECT_EQ(result, fs::u8path(u8"/dir1/dir2"));
    result = fs::u8path(u8"/dir1/dir2/../..").parent_path();
    EXPECT_EQ(result, fs::u8path(u8"/dir1/dir2/.."));
    result = fs::u8path(u8"/dir/filename").parent_path();
    EXPECT_EQ(result, fs::u8path(u8"/dir"));
    result = fs::u8path(u8"/dir/filename.ext").parent_path();
    EXPECT_EQ(result, fs::u8path(u8"/dir"));
    result = fs::u8path(u8"/dir\u2665/").parent_path();
    EXPECT_EQ(result, fs::u8path(u8"/dir\u2665"));
#endif
}

TEST(test_filesystem, fct_path_is_absolute)
{
    auto result = fs::u8path(u8"").is_absolute();
    EXPECT_FALSE(result);
    result = fs::u8path(".").is_absolute();
    EXPECT_FALSE(result);
    result = fs::u8path("filename.ext").is_absolute();
    EXPECT_FALSE(result);
#ifdef _WIN32
    result = fs::u8path(u8"C:\\Temp").is_absolute();
    EXPECT_TRUE(result);
    result = fs::u8path("\\\\SERVER\\SHARE\\rootdir").is_absolute();
    EXPECT_TRUE(result);
    result = fs::u8path("dir1\\dir2").is_absolute();
    EXPECT_FALSE(result);
    result = fs::u8path("\\dir1\\dir2").is_absolute();
    EXPECT_FALSE(result);
    result = fs::u8path(u8"C:\\\u2665Temp").is_absolute();
    EXPECT_TRUE(result);
    result = fs::u8path(u8"\u2665Temp").is_absolute();
    EXPECT_FALSE(result);
#else
    result = fs::u8path(u8"/usr").is_absolute();
    EXPECT_TRUE(result);
    result = fs::u8path(u8"//SERVER/SHARE/rootdir").is_absolute();
    EXPECT_TRUE(result);
    result = fs::u8path(u8"dir1/dir2").is_absolute();
    EXPECT_FALSE(result);
    result = fs::u8path(u8"/\u2665usr").is_absolute();
    EXPECT_TRUE(result);
    result = fs::u8path(u8"\u2665usr").is_absolute();
    EXPECT_FALSE(result);
#endif
}

TEST(test_filesystem, fct_path_equal)
{
#ifdef _WIN32
    auto path1 = fs::u8path(u8"C:\\Temp\\test.txt");
    auto path2 = fs::u8path(u8"C:\\Temp\\test.txt");
    EXPECT_TRUE(path1 == path2);
    path1 = fs::u8path(u8"C:\\Temp\\test.txt");
    path2 = fs::u8path(u8"C:\\Temp\\test.txx");
    EXPECT_FALSE(path1 == path2);
    path1 = fs::u8path(u8"C:\\Temp\\test.txt");
    path2 = fs::u8path(u8"C:\\TEMP\\TEST.TXT");
    EXPECT_FALSE(path1 == path2);
    path1 = fs::u8path(u8"C:\\Temp\\test.txt");
    path2 = fs::u8path(u8"C:\\Temp\\test.tx");
    EXPECT_FALSE(path1 == path2);
    path1 = fs::u8path(u8"C:\\Temp\\test\u2665.txt");
    path2 = fs::u8path(u8"C:\\Temp\\test\u2665.txt");
    EXPECT_TRUE(path1 == path2);
    path1 = fs::u8path(u8"C:\\Temp\\test.txt");
    path2 = fs::u8path(u8"C:\\Temp\\tes\u2665.txt");
    EXPECT_FALSE(path1 == path2);
#else
    auto path1 = fs::u8path(u8"/usr/lib/libtest.so");
    auto path2 = fs::u8path(u8"/usr/lib/libtest.so");
    EXPECT_TRUE(path1 == path2);
    path1 = fs::u8path(u8"/usr/lib/libtest.so");
    path2 = fs::u8path(u8"/usr/lib/libtest.sa");
    EXPECT_FALSE(path1 == path2);
    path1 = fs::u8path(u8"/usr/lib/libtest.so");
    path2 = fs::u8path(u8"/usr/lib/libTest.so");
    EXPECT_FALSE(path1 == path2);
    path1 = fs::u8path(u8"/usr/lib/libtest\u2665.so");
    path2 = fs::u8path(u8"/usr/lib/libtest\u2665.so");
    EXPECT_TRUE(path1 == path2);
    path1 = fs::u8path(u8"/usr/lib/libtest.so");
    path2 = fs::u8path(u8"/usr/lib/libtes\u2665.so");
    EXPECT_FALSE(path1 == path2);
#endif
}
