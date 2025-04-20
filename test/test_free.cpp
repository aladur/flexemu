/*
    test_free.cpp


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
#include "typedefs.h"
#include "free.h"
#include <vector>
#include <regex>

TEST(test_free, fct_find_regex_string)
{
    auto flags = std::regex_constants::extended;
    std::vector<Byte> data1{ '\x05', 'y', 'a', 'b', 'c', '\x04' };
    const std::regex regex1(".*(ab.*)", flags);
    auto result = flx::find_regex_string(regex1, '\x04', data1);
    EXPECT_EQ(result, "abc");
    result = flx::find_regex_string(regex1, '\x00', data1);
    EXPECT_TRUE(result.empty());
    std::vector<Byte> data21{ 'a', '\04', '\x06', 'a', 'b', 'c', '\x04' };
    const std::regex regex21(".*(ab.*)", flags);
    result = flx::find_regex_string(regex21, '\x04', data21);
    EXPECT_EQ(result, "abc");
    std::vector<Byte> data22{ 'a', 'b', '\x04', 'y', 'a', 'b', 'c', '\x04' };
    result = flx::find_regex_string(regex21, '\x04', data22);
    EXPECT_EQ(result, "ab");
    std::vector<Byte> data3{ 'a', 'b', '\00', 'a', 'b', 'c', '\x04' };
    const std::regex regex3(".*(ab.*)", flags);
    result = flx::find_regex_string(regex3, '\x04', data3);
    EXPECT_EQ(result, "abc");
    result = flx::find_regex_string(regex3, '\x00', data3);
    EXPECT_EQ(result, "ab");
    std::vector<Byte> data4{ 'y', 'a', 'b', 'c' };
    const std::regex regex4(".*(ab.*)", flags);
    result = flx::find_regex_string(regex4, '\x04', data4);
    EXPECT_TRUE(result.empty());
    std::vector<Byte> data5{ 'y', 'A', 'b', 'c', '\x04' };
    const std::regex regex5(".*(ab.*)", flags);
    result = flx::find_regex_string(regex5, '\x04', data5);
    EXPECT_TRUE(result.empty());
    flags = std::regex_constants::extended | std::regex_constants::icase;
    std::vector<Byte> data6{ 'y', 'A', 'B', 'C', '\x00' };
    const std::regex regex6(".*(ab.*)", flags);
    result = flx::find_regex_string(regex6, '\x00', data6);
    EXPECT_EQ(result, "ABC");
}
