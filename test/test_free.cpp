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
#include "flexerr.h"
#include <vector>
#include <regex>
#include <numeric>

static std::vector<std::string> split(const std::string &str, char delimiter)
{
    size_t start_pos = 0U;
    std::vector<std::string> result;

    auto pos = str.find(delimiter, start_pos);
    while (pos != std::string::npos)
    {
        if (pos - start_pos > 0U)
        {
            result.emplace_back(str.substr(start_pos, pos - start_pos));
        }
        start_pos = pos + 1;
        pos = str.find(delimiter, start_pos);
    }

    if (start_pos < str.size())
    {
        result.emplace_back(str.substr(start_pos, str.size() - start_pos));
    }

    return result;
}

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

// 0 hex bytes, data pointer == nullptr or bytesPerLine == 0.
TEST(test_free, fct_hex_dump_0)
{
    std::vector<Byte> data;
    std::stringstream stream1;
    flx::hex_dump(stream1, data.data(), data.size(), 16U, true, 0x0004);
    std::string linebuffer;
    ASSERT_TRUE(stream1.good());
    std::getline(stream1, linebuffer);
    ASSERT_TRUE(linebuffer.empty());
    ASSERT_FALSE(stream1.good());
    std::stringstream stream2;
    flx::hex_dump(stream2, nullptr, 1U, 16U, true, 0x0004);
    ASSERT_TRUE(stream2.good());
    std::getline(stream2, linebuffer);
    ASSERT_TRUE(linebuffer.empty());
    ASSERT_FALSE(stream2.good());
    std::stringstream stream3;
    EXPECT_THAT([&](){
        flx::hex_dump(stream3, data.data(), data.size(), 0U, false); },
        testing::Throws<FlexException>());
}

// 32 hex bytes, with address, with ascii, 16 bytes per line, offset 4.
TEST(test_free, fct_hex_dump_1)
{
    std::vector<Byte> data;
    data.resize(32U);
    std::iota(data.begin(), data.end(), '\0');
    std::stringstream stream;
    flx::hex_dump(stream, data.data(), data.size(), 16U, true, 0x0004);
    std::string linebuffer;
    // Test first line.
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    auto tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 71);
    ASSERT_EQ(tokens.size(), 14);
    EXPECT_EQ(linebuffer[6], ' '); // first hex padding
    EXPECT_EQ(linebuffer[17], ' '); // last hex padding
    EXPECT_NE(linebuffer[18], ' '); // first hex digit
    EXPECT_NE(linebuffer[52], ' '); // last hex digit
    EXPECT_EQ(linebuffer[55], ' '); // first ascii padding
    EXPECT_EQ(linebuffer[58], ' '); // last ascii padding
    EXPECT_NE(linebuffer[59], ' '); // first ascii
    EXPECT_NE(linebuffer[70], ' '); // last ascii
    EXPECT_EQ(tokens[0], "0000"); // start addr % 16
    EXPECT_EQ(tokens[1], "00"); // first byte
    EXPECT_EQ(tokens[12], "0B"); // last byte
    EXPECT_EQ(tokens[13].size(), 12); // ascii char count
    // Test second line.
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 71);
    ASSERT_EQ(tokens.size(), 18);
    EXPECT_NE(linebuffer[6], ' '); // first hex digit
    EXPECT_NE(linebuffer[52], ' '); // last hex digit
    EXPECT_NE(linebuffer[55], ' '); // first ascii
    EXPECT_NE(linebuffer[70], ' '); // last ascii
    EXPECT_EQ(tokens[0], "0010"); // start addr % 16
    EXPECT_EQ(tokens[1], "0C"); // first byte
    EXPECT_EQ(tokens[16], "1B"); // last byte
    EXPECT_EQ(tokens[17].size(), 16); // ascii char count
    // Test third line.
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 59);
    ASSERT_EQ(tokens.size(), 6);
    EXPECT_NE(linebuffer[6], ' '); // first hex digit
    EXPECT_NE(linebuffer[16], ' '); // last hex digit
    EXPECT_EQ(linebuffer[18], ' '); // first hex padding
    EXPECT_EQ(linebuffer[52], ' '); // last hex padding
    EXPECT_NE(linebuffer[55], ' '); // first ascii
    EXPECT_NE(linebuffer[58], ' '); // last ascii
    EXPECT_EQ(tokens[0], "0020"); // start addr % 16
    EXPECT_EQ(tokens[1], "1C"); // first byte
    EXPECT_EQ(tokens[4], "1F"); // last byte
    EXPECT_EQ(tokens[5].size(), 4); // ascii char count
    // Test fourth line.
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    ASSERT_TRUE(linebuffer.empty());
    ASSERT_FALSE(stream.good());
}

// 16 hex bytes, with address, with ascii, 16 bytes per line, offset 0.
TEST(test_free, fct_hex_dump_2)
{
    std::vector<Byte> data;
    data.resize(16U);
    std::iota(data.begin(), data.end(), '\0');
    std::stringstream stream;
    flx::hex_dump(stream, data.data(), data.size(), 16U, true, 0x0100);
    std::string linebuffer;
    // Test first line.
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    auto tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 71);
    ASSERT_EQ(tokens.size(), 18);
    EXPECT_NE(linebuffer[6], ' '); // first hex digit
    EXPECT_NE(linebuffer[52], ' '); // last hex digit
    EXPECT_NE(linebuffer[55], ' '); // first ascii
    EXPECT_NE(linebuffer[70], ' '); // last ascii
    EXPECT_EQ(tokens[0], "0100"); // start addr % 16
    EXPECT_EQ(tokens[1], "00"); // first byte
    EXPECT_EQ(tokens[16], "0F"); // last byte
    EXPECT_EQ(tokens[17].size(), 16); // ascii char count
    // Test second line.
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    ASSERT_TRUE(linebuffer.empty());
    ASSERT_FALSE(stream.good());
}

// 4 hex bytes, with address, with ascii, 8 bytes per line, offset 2.
TEST(test_free, fct_hex_dump_3)
{
    std::vector<Byte> data;
    data.resize(4U);
    std::iota(data.begin(), data.end(), '\0');
    std::stringstream stream;
    flx::hex_dump(stream, data.data(), data.size(), 8U, true, 0x0202);
    std::string linebuffer;
    // Test first line.
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    auto tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 37);
    ASSERT_EQ(tokens.size(), 6);
    EXPECT_EQ(linebuffer[6], ' '); // first hex padding
    EXPECT_EQ(linebuffer[10], ' '); // last hex padding
    EXPECT_NE(linebuffer[12], ' '); // first hex digit
    EXPECT_NE(linebuffer[22], ' '); // last hex digit
    EXPECT_EQ(linebuffer[25], ' '); // first hex padding
    EXPECT_EQ(linebuffer[28], ' '); // last hex padding
    EXPECT_EQ(linebuffer[31], ' '); // first ascii padding
    EXPECT_EQ(linebuffer[32], ' '); // last ascii padding
    EXPECT_NE(linebuffer[34], ' '); // first ascii
    EXPECT_NE(linebuffer[37], ' '); // last ascii
    EXPECT_EQ(tokens[0], "0200"); // start addr % 16
    EXPECT_EQ(tokens[1], "00"); // first byte
    EXPECT_EQ(tokens[4], "03"); // last byte
    EXPECT_EQ(tokens[5].size(), 4); // ascii char count
    // Test second line.
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    ASSERT_TRUE(linebuffer.empty());
    ASSERT_FALSE(stream.good());
}

// 8 hex bytes, with address, no ascii, 8 bytes per line, offset 0.
TEST(test_free, fct_hex_dump_4)
{
    std::vector<Byte> data;
    data.resize(8U);
    std::iota(data.begin(), data.end(), '\0');
    std::stringstream stream;
    flx::hex_dump(stream, data.data(), data.size(), 8U, false, 0x0300);
    std::string linebuffer;
    // Test first line.
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    auto tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 29);
    ASSERT_EQ(tokens.size(), 9);
    EXPECT_NE(linebuffer[6], ' '); // first hex digit
    EXPECT_NE(linebuffer[28], ' '); // last hex digit
    EXPECT_EQ(tokens[0], "0300"); // start addr % 16
    EXPECT_EQ(tokens[1], "00"); // first byte
    EXPECT_EQ(tokens[8], "07"); // last byte
    // Test second line.
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    ASSERT_TRUE(linebuffer.empty());
    ASSERT_FALSE(stream.good());
}

// 8 hex bytes, no address, no ascii, 8 bytes per line, offset 0.
TEST(test_free, fct_hex_dump_5)
{
    std::vector<Byte> data;
    data.resize(8U);
    std::iota(data.begin(), data.end(), '\0');
    std::stringstream stream;
    flx::hex_dump(stream, data.data(), data.size(), 8U, false);
    std::string linebuffer;
    // Test first line.
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    auto tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 23);
    ASSERT_EQ(tokens.size(), 8);
    EXPECT_NE(linebuffer[0], ' '); // first hex digit
    EXPECT_NE(linebuffer[22], ' '); // last hex digit
    EXPECT_EQ(tokens[0], "00"); // first byte
    EXPECT_EQ(tokens[7], "07"); // last byte
    // Test second line.
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    ASSERT_TRUE(linebuffer.empty());
    ASSERT_FALSE(stream.good());
}
