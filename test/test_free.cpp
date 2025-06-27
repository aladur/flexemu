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
#include "fixt_debugout.h"
#include "flexerr.h"
#include <vector>
#include <regex>
#include <numeric>
#include <sstream>

class test_free : public test_DebugOutputFixture
{
public:
    template<class T>
    void DebugOutput(int level, const T &value)
    {
        if (HasMinDebugLevel(level))
        {
            std::cout << value << "\n";
        }
    }
protected:
    struct sHexDumpPropsTestPositionAndResult
    {
        DWord row{};
        DWord column{};
        flx::sHexDumpProperties expected{};
    };

    struct sHexDumpPropertiesTestPatterns
    {
        size_t size{};
        DWord bytesPerLine{};
        bool withAscii{};
        bool isDisplayAddress{};
        DWord startAddress{};
        std::optional<DWord> extraSpace{std::nullopt};
        std::vector<sHexDumpPropsTestPositionAndResult> params;
    };

    static void checkProperties(
            const flx::sHexDumpProperties &item,
            const flx::sHexDumpProperties &expected)
    {
        EXPECT_EQ(item.type, expected.type);
        EXPECT_EQ(item.beginHexCol, expected.beginHexCol);
        EXPECT_EQ(item.endHexCol, expected.endHexCol);
        EXPECT_EQ(item.endCol, expected.endCol);
        EXPECT_EQ(item.address, expected.address);
        EXPECT_EQ(item.isUpperNibble, expected.isUpperNibble);
    }

    static void testHexDumpProperties(
            const sHexDumpPropertiesTestPatterns &patterns)
    {
        for (const auto &[row, column, expected] : patterns.params)
        {
            std::stringstream scope;

            scope << "row=" << row << " column=" << column;
            const auto props = flx::get_hex_dump_properties(
                    row, column, patterns.size, patterns.bytesPerLine,
                    patterns.withAscii, patterns.isDisplayAddress,
                    patterns.startAddress, patterns.extraSpace);
            SCOPED_TRACE(scope.str());
            checkProperties(props, expected);
        }
    }
};

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

TEST_F(test_free, fct_find_regex_string)
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
TEST_F(test_free, fct_hex_dump_0)
{
    const DWord bpl = 16U; // bytes per line
    std::vector<Byte> data;
    std::stringstream stream1;
    flx::hex_dump(stream1, data.data(), data.size(), bpl, true, 0x0004);
    std::string linebuffer;
    ASSERT_TRUE(stream1.good());
    std::getline(stream1, linebuffer);
    ASSERT_TRUE(linebuffer.empty());
    ASSERT_FALSE(stream1.good());
    std::stringstream stream2;
    flx::hex_dump(stream2, nullptr, 1U, bpl, true, 0x0004);
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
TEST_F(test_free, fct_hex_dump_1)
{
    const DWord bpl = 16U; // bytes per line
    std::vector<Byte> data;
    data.resize(32U);
    std::iota(data.begin(), data.end(), '\0');
    std::stringstream stream;
    flx::hex_dump(stream, data.data(), data.size(), bpl, true, 0x0004);
    std::string linebuffer;
// Test first row.
// 0000              00 01 02 03 04 05 06 07 08 09 0A 0B      ____________
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    DebugOutput(1, linebuffer);
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
    EXPECT_NE(linebuffer[linebuffer.size() - 1], ' '); // last ascii
    EXPECT_EQ(tokens[0], "0000"); // start addr
    EXPECT_EQ(tokens[1], "00"); // first byte
    EXPECT_EQ(tokens[tokens.size() - 2], "0B"); // last byte
    EXPECT_EQ(tokens[tokens.size() - 1].size(), 12U); // ascii char count
// Test second row.
// 0010  0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19 1A 1B  ________________
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    DebugOutput(1, linebuffer);
    tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 71);
    ASSERT_EQ(tokens.size(), bpl + 2U);
    EXPECT_NE(linebuffer[6], ' '); // first hex digit
    EXPECT_NE(linebuffer[52], ' '); // last hex digit
    EXPECT_NE(linebuffer[55], ' '); // first ascii
    EXPECT_NE(linebuffer[linebuffer.size() - 1], ' '); // last ascii
    EXPECT_EQ(tokens[0], "0010"); // start addr
    EXPECT_EQ(tokens[1], "0C"); // first byte
    EXPECT_EQ(tokens[tokens.size() - 2], "1B"); // last byte
    EXPECT_EQ(tokens[tokens.size() - 1].size(), bpl); // ascii char count
// Test third row.
// 0020  1C 1D 1E 1F                                      ____
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    DebugOutput(1, linebuffer);
    tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 59);
    ASSERT_EQ(tokens.size(), 6);
    EXPECT_NE(linebuffer[6], ' '); // first hex digit
    EXPECT_NE(linebuffer[16], ' '); // last hex digit
    EXPECT_EQ(linebuffer[18], ' '); // first hex padding
    EXPECT_EQ(linebuffer[52], ' '); // last hex padding
    EXPECT_NE(linebuffer[55], ' '); // first ascii
    EXPECT_NE(linebuffer[linebuffer.size() - 1], ' '); // last ascii
    EXPECT_EQ(tokens[0], "0020"); // start addr
    EXPECT_EQ(tokens[1], "1C"); // first byte
    EXPECT_EQ(tokens[tokens.size() - 2], "1F"); // last byte
    EXPECT_EQ(tokens[tokens.size() - 1].size(), 4U); // ascii char count
// Test fourth row.
    ASSERT_FALSE(stream.good());
}

// 16 hex bytes, with address, with ascii, 16 bytes per line, offset 0.
TEST_F(test_free, fct_hex_dump_2)
{
    const DWord bpl = 16U; // bytes per line
    std::vector<Byte> data;
    data.resize(16U);
    std::iota(data.begin(), data.end(), '\0');
    std::stringstream stream;
    flx::hex_dump(stream, data.data(), data.size(), bpl, true, 0x0100);
    std::string linebuffer;
// Test first row.
// 0100  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  ________________
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    DebugOutput(1, linebuffer);
    auto tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 71);
    ASSERT_EQ(tokens.size(), bpl + 2U);
    EXPECT_NE(linebuffer[6], ' '); // first hex digit
    EXPECT_NE(linebuffer[52], ' '); // last hex digit
    EXPECT_NE(linebuffer[55], ' '); // first ascii
    EXPECT_NE(linebuffer[linebuffer.size() - 1], ' '); // last ascii
    EXPECT_EQ(tokens[0], "0100"); // start addr
    EXPECT_EQ(tokens[1], "00"); // first byte
    EXPECT_EQ(tokens[tokens.size() - 2], "0F"); // last byte
    EXPECT_EQ(tokens[tokens.size() - 1].size(), bpl); // ascii char count
// Test second row.
    ASSERT_FALSE(stream.good());
}

// 4 hex bytes, with address, with ascii, 8 bytes per line, offset 2.
TEST_F(test_free, fct_hex_dump_3)
{
    const DWord bpl = 8U; // bytes per line
    std::vector<Byte> data;
    data.resize(4U);
    std::iota(data.begin(), data.end(), '\0');
    std::stringstream stream;
    flx::hex_dump(stream, data.data(), data.size(), bpl, true, 0x0202);
    std::string linebuffer;
// Test first row.
// 0200        00 01 02 03          ____
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    DebugOutput(1, linebuffer);
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
    EXPECT_NE(linebuffer[33], ' '); // first ascii
    EXPECT_NE(linebuffer[linebuffer.size() - 1], ' '); // last ascii
    EXPECT_EQ(tokens[0], "0200"); // start addr
    EXPECT_EQ(tokens[1], "00"); // first byte
    EXPECT_EQ(tokens[tokens.size() - 2], "03"); // last byte
    EXPECT_EQ(tokens[tokens.size() - 1].size(), 4U); // ascii char count
// Test second row.
    ASSERT_FALSE(stream.good());
}

// 8 hex bytes, with address, no ascii, 8 bytes per line, offset 0.
TEST_F(test_free, fct_hex_dump_4)
{
    const DWord bpl = 8U; // bytes per line
    std::vector<Byte> data;
    data.resize(8U);
    std::iota(data.begin(), data.end(), '\0');
    std::stringstream stream;
    flx::hex_dump(stream, data.data(), data.size(), bpl, false, 0x0300);
    std::string linebuffer;
// Test first row.
// 0300  00 01 02 03 04 05 06 07
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    DebugOutput(1, linebuffer);
    auto tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 29);
    ASSERT_EQ(tokens.size(), bpl + 1U);
    EXPECT_NE(linebuffer[6], ' '); // first hex digit
    EXPECT_NE(linebuffer[linebuffer.size() - 1], ' '); // last hex digit
    EXPECT_EQ(tokens[0], "0300"); // start addr
    EXPECT_EQ(tokens[1], "00"); // first byte
    EXPECT_EQ(tokens[tokens.size() - 1], "07"); // last byte
// Test second row.
    ASSERT_FALSE(stream.good());
}

// 8 hex bytes, no address, no ascii, 8 bytes per line, offset 0.
TEST_F(test_free, fct_hex_dump_5)
{
    const DWord bpl = 8U; // bytes per line
    std::vector<Byte> data;
    data.resize(8U);
    std::iota(data.begin(), data.end(), '\0');
    std::stringstream stream;
    flx::hex_dump(stream, data.data(), data.size(), bpl, false);
    std::string linebuffer;
// Test first row.
// 00 01 02 03 04 05 06 07
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    DebugOutput(1, linebuffer);
    auto tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 23);
    ASSERT_EQ(tokens.size(), bpl);
    EXPECT_NE(linebuffer[0], ' '); // first hex digit
    EXPECT_NE(linebuffer[linebuffer.size() - 1], ' '); // last hex digit
    EXPECT_EQ(tokens[0], "00"); // first byte
    EXPECT_EQ(tokens[tokens.size() - 1], "07"); // last byte
// Test second row.
    ASSERT_FALSE(stream.good());
}

// 20 hex bytes, with address, no ascii, 13 bytes per line, offset 9.
TEST_F(test_free, fct_hex_dump_6)
{
    const DWord bpl = 13U; // bytes per line
    std::vector<Byte> data;
    data.resize(20U);
    std::iota(data.begin(), data.end(), '\0');
    std::stringstream stream;
    flx::hex_dump(stream, data.data(), data.size(), bpl, false, 0x0100);
    std::string linebuffer;
// Test first row.
// 00F7                             00 01 02 03
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    DebugOutput(1, linebuffer);
    auto tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 44);
    ASSERT_EQ(tokens.size(), 5);
    EXPECT_EQ(linebuffer[6], ' '); // first hex padding
    EXPECT_EQ(linebuffer[31], ' '); // last hex padding
    EXPECT_NE(linebuffer[33], ' '); // first hex digit
    EXPECT_NE(linebuffer[linebuffer.size() - 1], ' '); // last hex digit
    EXPECT_EQ(tokens[0], "00F7"); // start addr
    EXPECT_EQ(tokens[1], "00"); // first byte
    EXPECT_EQ(tokens[tokens.size() - 1], "03"); // last byte
// Test second row.
// 0104  04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    DebugOutput(1, linebuffer);
    tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 44);
    ASSERT_EQ(tokens.size(), bpl + 1);
    EXPECT_NE(linebuffer[6], ' '); // first hex digit
    EXPECT_NE(linebuffer[linebuffer.size() - 1], ' '); // last hex digit
    EXPECT_EQ(tokens[0], "0104"); // start addr
    EXPECT_EQ(tokens[1], "04"); // first byte
    EXPECT_EQ(tokens[tokens.size() - 1], "10"); // last byte
// Test third row.
// 0111  11 12 13
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    DebugOutput(1, linebuffer);
    tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 14);
    ASSERT_EQ(tokens.size(), 4);
    EXPECT_NE(linebuffer[6], ' '); // first hex digit
    EXPECT_NE(linebuffer[linebuffer.size() - 1], ' '); // last hex digit
    EXPECT_EQ(tokens[0], "0111"); // start addr
    EXPECT_EQ(tokens[1], "11"); // first byte
    EXPECT_EQ(tokens[tokens.size() - 1], "13"); // last byte
// Test fourth row.
    ASSERT_FALSE(stream.good());
}

// 20 hex bytes, with address, with ascii, 13 bytes per line, offset 9.
TEST_F(test_free, fct_hex_dump_7)
{
    const DWord bpl = 13U; // bytes per line
    std::vector<Byte> data;
    data.resize(20U);
    std::iota(data.begin(), data.end(), '\0');
    std::stringstream stream;
    flx::hex_dump(stream, data.data(), data.size(), bpl, true, 0x0100);
    std::string linebuffer;
// Test first row.
// 00F7                             00 01 02 03           ____
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    DebugOutput(1, linebuffer);
    auto tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 59);
    ASSERT_EQ(tokens.size(), 6);
    EXPECT_EQ(linebuffer[6], ' '); // first hex padding
    EXPECT_EQ(linebuffer[31], ' '); // last hex padding
    EXPECT_NE(linebuffer[33], ' '); // first hex digit
    EXPECT_NE(linebuffer[43], ' '); // last hex digit
    EXPECT_EQ(linebuffer[46], ' '); // first ascii padding
    EXPECT_EQ(linebuffer[54], ' '); // last ascii padding
    EXPECT_NE(linebuffer[55], ' '); // first ascii
    EXPECT_NE(linebuffer[linebuffer.size() - 1], ' '); // last ascii
    EXPECT_EQ(tokens[0], "00F7"); // start addr
    EXPECT_EQ(tokens[1], "00"); // first byte
    EXPECT_EQ(tokens[tokens.size() - 2], "03"); // last byte
    EXPECT_EQ(tokens[tokens.size() - 1].size(), 4U); // ascii char count
// Test second row.
// 0104  04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10  _____________
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    DebugOutput(1, linebuffer);
    tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 59);
    ASSERT_EQ(tokens.size(), bpl + 2);
    EXPECT_NE(linebuffer[6], ' '); // first hex digit
    EXPECT_NE(linebuffer[43], ' '); // last hex digit
    EXPECT_NE(linebuffer[46], ' '); // first ascii
    EXPECT_NE(linebuffer[linebuffer.size() - 1], ' '); // last ascii
    EXPECT_EQ(tokens[0], "0104"); // start addr
    EXPECT_EQ(tokens[1], "04"); // first byte
    EXPECT_EQ(tokens[tokens.size() - 2], "10"); // last byte
    EXPECT_EQ(tokens[tokens.size() - 1].size(), bpl); // ascii char count
// Test third row.
// 0111  11 12 13                                ___
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    DebugOutput(1, linebuffer);
    tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 49);
    ASSERT_EQ(tokens.size(), 5);
    EXPECT_NE(linebuffer[6], ' '); // first hex digit
    EXPECT_NE(linebuffer[13], ' '); // last hex digit
    EXPECT_EQ(linebuffer[15], ' '); // first hex padding
    EXPECT_EQ(linebuffer[43], ' '); // last hex padding
    EXPECT_NE(linebuffer[46], ' '); // first ascii
    EXPECT_NE(linebuffer[linebuffer.size() - 1], ' '); // last ascii
    EXPECT_EQ(tokens[0], "0111"); // start addr
    EXPECT_EQ(tokens[1], "11"); // first byte
    EXPECT_EQ(tokens[tokens.size() - 2], "13"); // last byte
    EXPECT_EQ(tokens[tokens.size() - 1].size(), 3U); // ascii char count
// Test fourth row.
    ASSERT_FALSE(stream.good());
}

// 27 hex bytes, with address, with ascii, 13 bytes per line, offset 4.
// extra space each 5 bytes.
TEST_F(test_free, fct_hex_dump_8)
{
    const DWord bpl = 13U; // bytes per line
    std::vector<Byte> data;
    data.resize(27U);
    std::iota(data.begin(), data.end(), '\0');
    std::stringstream stream;
    flx::hex_dump(stream, data.data(), data.size(), bpl, true, 0x00C7, 5U);
    std::string linebuffer;
// Test first row.
// 00C3              00  01 02 03 04 05  06 07 08      _ _____ ___
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    DebugOutput(1, linebuffer);
    auto tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 63);
    ASSERT_EQ(tokens.size(), bpl);
    EXPECT_EQ(linebuffer[6], ' '); // first hex padding
    EXPECT_EQ(linebuffer[16], ' '); // last hex padding
    EXPECT_NE(linebuffer[18], ' '); // first hex digit
    EXPECT_NE(linebuffer[45], ' '); // last hex digit
    EXPECT_EQ(linebuffer[48], ' '); // first ascii padding
    EXPECT_EQ(linebuffer[51], ' '); // last ascii padding
    EXPECT_NE(linebuffer[52], ' '); // first ascii
    EXPECT_NE(linebuffer[linebuffer.size() - 1], ' '); // last ascii
    EXPECT_EQ(tokens[0], "00C3"); // start addr
    EXPECT_EQ(tokens[1], "00"); // first byte
    EXPECT_EQ(tokens[tokens.size() - 4], "08"); // last byte
    EXPECT_EQ(tokens[tokens.size() - 3].size(), 1U); // ascii char count
    EXPECT_EQ(tokens[tokens.size() - 2].size(), 5U); // ascii char count
    EXPECT_EQ(tokens[tokens.size() - 1].size(), 3U); // ascii char count
// Test second row.
// 00D0  09 0A 0B 0C 0D  0E 0F 10 11 12  13 14 15  _____ _____ ___
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    DebugOutput(1, linebuffer);
    tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 63);
    ASSERT_EQ(tokens.size(), 17);
    EXPECT_NE(linebuffer[6], ' '); // first hex digit
    EXPECT_NE(linebuffer[45], ' '); // last hex digit
    EXPECT_NE(linebuffer[48], ' '); // first ascii
    EXPECT_NE(linebuffer[linebuffer.size() - 1], ' '); // last ascii
    EXPECT_EQ(tokens[0], "00D0"); // start addr
    EXPECT_EQ(tokens[1], "09"); // first byte
    EXPECT_EQ(tokens[tokens.size() - 4], "15"); // last byte
    EXPECT_EQ(tokens[tokens.size() - 3].size(), 5U); // ascii char count
    EXPECT_EQ(tokens[tokens.size() - 2].size(), 5U); // ascii char count
    EXPECT_EQ(tokens[tokens.size() - 1].size(), 3U); // ascii char count
// Test third row.
// 00DD  16 17 18 19 1A                            _____
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    DebugOutput(1, linebuffer);
    tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 53);
    ASSERT_EQ(tokens.size(), 7);
    EXPECT_NE(linebuffer[6], ' '); // first hex digit
    EXPECT_NE(linebuffer[19], ' '); // last hex digit
    EXPECT_EQ(linebuffer[23], ' '); // first hex padding
    EXPECT_EQ(linebuffer[46], ' '); // last hex padding
    EXPECT_NE(linebuffer[48], ' '); // first ascii
    EXPECT_NE(linebuffer[linebuffer.size() - 1], ' '); // last ascii
    EXPECT_EQ(tokens[0], "00DD"); // start addr
    EXPECT_EQ(tokens[1], "16"); // first byte
    EXPECT_EQ(tokens[tokens.size() - 2], "1A"); // last byte
    EXPECT_EQ(tokens[tokens.size() - 1].size(), 5U); // ascii char count
// Test fourth row.
    ASSERT_FALSE(stream.good());
}

// 32 hex bytes, no address, no ascii, 13 bytes per line, offset 4.
// extra space each 5 bytes.
TEST_F(test_free, fct_hex_dump_9)
{
    const DWord bpl = 13U; // bytes per line
    std::vector<Byte> data;
    data.resize(32U);
    std::iota(data.begin(), data.end(), '\0');
    std::stringstream stream;
    flx::hex_dump(stream, data.data(), data.size(), bpl, false, std::nullopt,
            5U);
    std::string linebuffer;
// Test first row.
// 00 01 02 03 04  05 06 07 08 09  0A 0B 0C
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    DebugOutput(1, linebuffer);
    auto tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 40);
    ASSERT_EQ(tokens.size(), bpl);
    EXPECT_NE(linebuffer[0], ' '); // first hex digit
    EXPECT_NE(linebuffer[linebuffer.size() - 1], ' '); // last hex digit
    EXPECT_EQ(tokens[0], "00"); // first byte
    EXPECT_EQ(tokens[tokens.size() - 1], "0C"); // last byte
// Test second row.
// 0D 0E 0F 10 11  12 13 14 15 16  17 18 19
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    DebugOutput(1, linebuffer);
    tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 40);
    ASSERT_EQ(tokens.size(), bpl);
    EXPECT_NE(linebuffer[6], ' '); // first hex digit
    EXPECT_NE(linebuffer[linebuffer.size() - 1], ' '); // last hex digit
    EXPECT_EQ(tokens[0], "0D"); // first byte
    EXPECT_EQ(tokens[tokens.size() - 1], "19"); // last byte
// Test third row.
// 1A 1B 1C 1D 1E  1F
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    DebugOutput(1, linebuffer);
    tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 18);
    ASSERT_EQ(tokens.size(), 6);
    EXPECT_NE(linebuffer[0], ' '); // first hex digit
    EXPECT_NE(linebuffer[linebuffer.size() - 1], ' '); // last hex digit
    EXPECT_EQ(tokens[0], "1A"); // first byte
    EXPECT_EQ(tokens[tokens.size() - 1], "1F"); // last byte
// Test fourth row.
    ASSERT_FALSE(stream.good());
}

// 15 hex bytes, no address, with ascii, 15 bytes per line,
// extra space each 5 bytes.
TEST_F(test_free, fct_hex_dump_10)
{
    const DWord bpl = 15U; // bytes per line
    std::vector<Byte> data;
    data.resize(bpl);
    std::iota(data.begin(), data.end(), '\0');
    std::stringstream stream;
    flx::hex_dump(stream, data.data(), data.size(), bpl, true, std::nullopt,
            5U);
    std::string linebuffer;
// Test first row.
// 00 01 02 03 04  05 06 07 08 09  0A 0B 0C 0D 0E  _____ _____ _____
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    DebugOutput(1, linebuffer);
    auto tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 65);
    ASSERT_EQ(tokens.size(), 18);
    EXPECT_NE(linebuffer[0], ' '); // first hex digit
    EXPECT_NE(linebuffer[45], ' '); // last hex digit
    EXPECT_NE(linebuffer[48], ' '); // first ascii
    EXPECT_NE(linebuffer[linebuffer.size() - 1], ' '); // last ascii
    EXPECT_EQ(tokens[0], "00"); // first byte
    EXPECT_EQ(tokens[tokens.size() - 4], "0E"); // last byte
    EXPECT_EQ(tokens[tokens.size() - 3].size(), 5U); // ascii char count
    EXPECT_EQ(tokens[tokens.size() - 2].size(), 5U); // ascii char count
    EXPECT_EQ(tokens[tokens.size() - 1].size(), 5U); // ascii char count
// Test second row.
    ASSERT_FALSE(stream.good());
}

// 16 hex bytes, with address, with ascii, 16 bytes per line, offset 8.
// extra space each 8 bytes.
TEST_F(test_free, fct_hex_dump_13)
{
    const DWord bpl = 16U; // bytes per line
    std::vector<Byte> data;
    data.resize(16U);
    std::iota(data.begin(), data.end(), '\0');
    std::stringstream stream;
    flx::hex_dump(stream, data.data(), data.size(), bpl, true, 0x0008, 8U);
    std::string linebuffer;
// Test first row.
// 0000                           00 01 02 03 04 05 06 07           ________
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    DebugOutput(1, linebuffer);
    auto tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 73U);
    ASSERT_EQ(tokens.size(), 10U);
    EXPECT_EQ(linebuffer[6], ' '); // first hex padding
    EXPECT_EQ(linebuffer[28], ' '); // last hex padding
    EXPECT_NE(linebuffer[31], ' '); // first hex digit
    EXPECT_NE(linebuffer[53], ' '); // last hex digit
    EXPECT_EQ(linebuffer[56], ' '); // first ascii padding
    EXPECT_EQ(linebuffer[63], ' '); // last ascii padding
    EXPECT_NE(linebuffer[65], ' '); // first ascii
    EXPECT_NE(linebuffer[linebuffer.size() - 1], ' '); // last ascii
    EXPECT_EQ(tokens[0], "0000"); // start addr
    EXPECT_EQ(tokens[1], "00"); // first byte
    EXPECT_EQ(tokens[tokens.size() - 2], "07"); // last byte
    EXPECT_EQ(tokens[tokens.size() - 1].size(), 8U); // ascii char count
// Test second row.
// 0010  08 09 0A 0B 0C 0D 0E 0F                           ________
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    DebugOutput(1, linebuffer);
    tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 64U);
    ASSERT_EQ(tokens.size(), 10U);
    EXPECT_NE(linebuffer[6], ' '); // first hex digit
    EXPECT_NE(linebuffer[28], ' '); // last hex digit
    EXPECT_EQ(linebuffer[31], ' '); // first hex padding
    EXPECT_EQ(linebuffer[53], ' '); // last hex padding
    EXPECT_NE(linebuffer[56], ' '); // first ascii
    EXPECT_NE(linebuffer[linebuffer.size() - 1], ' '); // last ascii
    EXPECT_EQ(tokens[0], "0010"); // start addr
    EXPECT_EQ(tokens[1], "08"); // first byte
    EXPECT_EQ(tokens[tokens.size() - 2], "0F"); // last byte
    EXPECT_EQ(tokens[tokens.size() - 1].size(), 8U); // ascii char count
// Test third row.
    ASSERT_FALSE(stream.good());
}

// Test scale with 16 bytes, with address, with ascii.
TEST_F(test_free, fct_hex_dump_scale_1)
{
    std::stringstream stream;
    const DWord bpl = 16U; // bytes per line
    flx::hex_dump_scale(stream, bpl, true, 0x0004);
    std::string linebuffer;
// Test first row.
//       00                                               0
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    DebugOutput(1, linebuffer);
    auto tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 71);
    ASSERT_EQ(tokens.size(), 2);
    EXPECT_NE(linebuffer[6], ' '); // first hex digit
    EXPECT_NE(linebuffer[7], ' '); // last hex digit
    EXPECT_EQ(linebuffer[9], ' '); // first hex padding
    EXPECT_EQ(linebuffer[52], ' '); // last hex padding
    EXPECT_NE(linebuffer[55], ' '); // first ASCII
    EXPECT_EQ(tokens[0], "00"); // first byte
    EXPECT_EQ(tokens[tokens.size() - 1].size(), 1U); // ASCII char count
// Test second row.
//       00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  0123456789ABCDEF
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    DebugOutput(1, linebuffer);
    tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 71);
    ASSERT_EQ(tokens.size(), bpl + 1U);
    EXPECT_NE(linebuffer[6], ' '); // first hex digit
    EXPECT_NE(linebuffer[52], ' '); // last hex digit
    EXPECT_NE(linebuffer[55], ' '); // first ASCII
    EXPECT_NE(linebuffer[linebuffer.size() - 1], ' '); // last ASCII
    EXPECT_EQ(tokens[0], "00"); // first byte
    EXPECT_EQ(tokens[tokens.size() - 2], "0F"); // last byte
    EXPECT_EQ(tokens[tokens.size() - 1].size(), bpl); // ASCII char count
// Test third row.
    ASSERT_FALSE(stream.good());
}

// Test scale with 8 bytes per line, with address, with ascii.
TEST_F(test_free, fct_hex_dump_scale_3)
{
    std::stringstream stream;
    const DWord bpl = 8U; // bytes per line
    flx::hex_dump_scale(stream, bpl, true, 0x0202);
    std::string linebuffer;
// Test first row.
//       00                       0
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    DebugOutput(1, linebuffer);
    auto tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 39);
    ASSERT_EQ(tokens.size(), 2);
    EXPECT_NE(linebuffer[6], ' '); // first hex digit
    EXPECT_NE(linebuffer[7], ' '); // last hex digit
    EXPECT_EQ(linebuffer[9], ' '); // first hex padding
    EXPECT_EQ(linebuffer[28], ' '); // last hex padding
    EXPECT_NE(linebuffer[31], ' '); // first ASCII
    EXPECT_EQ(tokens[0], "00"); // first byte
    EXPECT_EQ(tokens[tokens.size() - 1].size(), 1U); // ASCII char count
// Test second row.
//       00 01 02 03 04 05 06 07  01234567
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    DebugOutput(1, linebuffer);
    tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 39);
    ASSERT_EQ(tokens.size(), bpl + 1);
    EXPECT_NE(linebuffer[6], ' '); // first hex digit
    EXPECT_NE(linebuffer[28], ' '); // last hex digit
    EXPECT_NE(linebuffer[31], ' '); // first ASCII
    EXPECT_NE(linebuffer[linebuffer.size() - 1], ' '); // last ASCII
    EXPECT_EQ(tokens[0], "00"); // first byte
    EXPECT_EQ(tokens[tokens.size() - 2], "07"); // last byte
    EXPECT_EQ(tokens[tokens.size() - 1].size(), 8U); // ASCII char count
// Test third row.
    ASSERT_FALSE(stream.good());
}

// Test scale with 8 bytes per line, with address, no ascii.
TEST_F(test_free, fct_hex_dump_scale_4)
{
    std::stringstream stream;
    const DWord bpl = 8U; // bytes per line
    flx::hex_dump_scale(stream, bpl, false, 0x0202);
    std::string linebuffer;
// Test first row.
//       00
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    DebugOutput(1, linebuffer);
    auto tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 29);
    ASSERT_EQ(tokens.size(), 1);
    EXPECT_NE(linebuffer[6], ' '); // first hex digit
    EXPECT_NE(linebuffer[7], ' '); // last hex digit
    EXPECT_EQ(tokens[0], "00"); // first byte
// Test second row.
//       00 01 02 03 04 05 06 07
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    DebugOutput(1, linebuffer);
    tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 29);
    ASSERT_EQ(tokens.size(), bpl);
    EXPECT_NE(linebuffer[6], ' '); // first hex digit
    EXPECT_NE(linebuffer[linebuffer.size() - 1], ' '); // last hex digit
    EXPECT_EQ(tokens[0], "00"); // first byte
    EXPECT_EQ(tokens[tokens.size() - 1], "07"); // last byte
// Test third row.
    ASSERT_FALSE(stream.good());
}

// Test scale with 8 bytes per line, no address, no ascii.
TEST_F(test_free, fct_hex_dump_scale_5)
{
    std::stringstream stream;
    const DWord bpl = 8U; // bytes per line
    flx::hex_dump_scale(stream, bpl, false);
    std::string linebuffer;
// Test first row.
// 00
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    DebugOutput(1, linebuffer);
    auto tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 23);
    ASSERT_EQ(tokens.size(), 1);
    EXPECT_NE(linebuffer[0], ' '); // first hex digit
    EXPECT_NE(linebuffer[1], ' '); // last hex digit
    EXPECT_EQ(tokens[0], "00"); // first byte
// Test second row.
// 00 01 02 03 04 05 06 07
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    DebugOutput(1, linebuffer);
    tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 23);
    ASSERT_EQ(tokens.size(), bpl);
    EXPECT_NE(linebuffer[0], ' '); // first hex digit
    EXPECT_NE(linebuffer[linebuffer.size() - 1], ' '); // last hex digit
    EXPECT_EQ(tokens[0], "00"); // first byte
    EXPECT_EQ(tokens[tokens.size() - 1], "07"); // last byte
// Test third row.
    ASSERT_FALSE(stream.good());
}

// Test scale with 13 bytes per line, with address, with ascii, extra space
// each 5 bytes.
TEST_F(test_free, fct_hex_dump_scale_8)
{
    std::stringstream stream;
    const DWord bpl = 13U; // bytes per line
    flx::hex_dump_scale(stream, bpl, true, 0x00C7, 5U);
    std::string linebuffer;
// Test first row.
//       00                                        0
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    DebugOutput(1, linebuffer);
    auto tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 63);
    ASSERT_EQ(tokens.size(), 2);
    EXPECT_NE(linebuffer[6], ' '); // first hex digit
    EXPECT_NE(linebuffer[7], ' '); // last hex digit
    EXPECT_EQ(linebuffer[9], ' '); // first hex padding
    EXPECT_EQ(linebuffer[45], ' '); // last hex padding
    EXPECT_NE(linebuffer[48], ' '); // first ascii
    EXPECT_EQ(tokens[0], "00"); // first byte
// Test second row.
//       00 01 02 03 04  05 06 07 08 09  0A 0B 0C  01234 56789 ABC
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    DebugOutput(1, linebuffer);
    tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 63);
    ASSERT_EQ(tokens.size(), bpl + 3);
    EXPECT_NE(linebuffer[6], ' '); // first hex digit
    EXPECT_NE(linebuffer[45], ' '); // last hex digit
    EXPECT_EQ(tokens[0], "00"); // first byte
    EXPECT_EQ(tokens[tokens.size() - 4], "0C"); // last byte
    EXPECT_EQ(tokens[tokens.size() - 3].size(), 5U); // ascii char count
    EXPECT_EQ(tokens[tokens.size() - 2].size(), 5U); // ascii char count
    EXPECT_EQ(tokens[tokens.size() - 1].size(), 3U); // ascii char count
// Test third row.
    ASSERT_FALSE(stream.good());
}

// Test scale with 13 bytes per line, no address, no ascii, extra space each
// 5 bytes.
TEST_F(test_free, fct_hex_dump_scale_9)
{
    std::stringstream stream;
    const DWord bpl = 13U; // bytes per line
    flx::hex_dump_scale(stream, bpl, false, std::nullopt, 5U);
    std::string linebuffer;
// Test first row.
// 00
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    DebugOutput(1, linebuffer);
    auto tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 40);
    ASSERT_EQ(tokens.size(), 1);
    EXPECT_NE(linebuffer[0], ' '); // first hex digit
    EXPECT_NE(linebuffer[1], ' '); // last hex digit
    EXPECT_EQ(tokens[0], "00"); // first byte
// Test second row.
// 00 01 02 03 04  05 06 07 08 09  0A 0B 0C
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    DebugOutput(1, linebuffer);
    tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 40);
    ASSERT_EQ(tokens.size(), bpl);
    EXPECT_NE(linebuffer[0], ' '); // first hex digit
    EXPECT_NE(linebuffer[linebuffer.size() - 1], ' '); // last hex digit
    EXPECT_EQ(tokens[0], "00"); // first byte
    EXPECT_EQ(tokens[tokens.size() - 1], "0C"); // last byte
// Test third row.
    ASSERT_FALSE(stream.good());
}

// Test scale with 15 bytes per line, no address, with ascii, extra space each
// 5 bytes.
TEST_F(test_free, fct_hex_dump_scale_10)
{
    std::stringstream stream;
    const DWord bpl = 15U; // bytes per line
    flx::hex_dump_scale(stream, bpl, true, std::nullopt, 5U);
    std::string linebuffer;
// Test first row.
// 00                                              0
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    DebugOutput(1, linebuffer);
    auto tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 65);
    ASSERT_EQ(tokens.size(), 2);
    EXPECT_NE(linebuffer[0], ' '); // first hex digit
    EXPECT_NE(linebuffer[1], ' '); // last hex digit
    EXPECT_EQ(tokens[0], "00"); // first byte
// Test second row.
// 00 01 02 03 04  05 06 07 08 09  0A 0B 0C 0D 0E  01234 56789 ABCDE
    ASSERT_TRUE(stream.good());
    std::getline(stream, linebuffer);
    DebugOutput(1, linebuffer);
    tokens = split(linebuffer, ' ');
    ASSERT_EQ(linebuffer.size(), 65);
    ASSERT_EQ(tokens.size(), 18);
    EXPECT_NE(linebuffer[0], ' '); // first hex digit
    EXPECT_NE(linebuffer[45], ' '); // last hex digit
    EXPECT_NE(linebuffer[48], ' '); // first ascii
    EXPECT_NE(linebuffer[linebuffer.size() - 1], ' '); // last ascii
    EXPECT_EQ(tokens[0], "00"); // first byte
    EXPECT_EQ(tokens[tokens.size() - 4], "0E"); // last byte
    EXPECT_EQ(tokens[tokens.size() - 3].size(), 5U); // ascii char count
    EXPECT_EQ(tokens[tokens.size() - 2].size(), 5U); // ascii char count
    EXPECT_EQ(tokens[tokens.size() - 1].size(), 5U); // ascii char count
// Test third row.
    ASSERT_FALSE(stream.good());
}

// 32 hex bytes, with address, with ascii, 16 bytes per line, offset 4.
TEST_F(test_free, fct_get_hex_dump_properties_1)
{
    using flx::HexDumpType;

    const sHexDumpPropertiesTestPatterns patterns =
    {
    //  size bpl  ASCI? addr? addr    extraSpace
        32LLU, 16U, true, true, 0x0004, std::nullopt, {
// Test first row
// 0000              00 01 02 03 04 05 06 07 08 09 0A 0B      ____________
        { 0, 0, { HexDumpType::NONE, false, 18, 52, 70 }},
        { 0, 6, { HexDumpType::NONE, false, 18, 52, 70 }},
        { 0, 15, { HexDumpType::NONE, false, 18, 52, 70 }},
        { 0, 16, { HexDumpType::NONE, false, 18, 52, 70 }},
        { 0, 17, { HexDumpType::NONE, false, 18, 52, 70 }},
        { 0, 18, { HexDumpType::HexByte, true, 18, 52, 70, 0x0004 }},
        { 0, 19, { HexDumpType::HexByte, false, 18, 52, 70, 0x0004 }},
        { 0, 20, { HexDumpType::NONE, false, 18, 52, 70 }},
        { 0, 33, { HexDumpType::HexByte, true, 18, 52, 70, 0x0009 }},
        { 0, 34, { HexDumpType::HexByte, false, 18, 52, 70, 0x0009 }},
        { 0, 35, { HexDumpType::NONE, false, 18, 52, 70 }},
        { 0, 51, { HexDumpType::HexByte, true, 18, 52, 70, 0x000F }},
        { 0, 52, { HexDumpType::HexByte, false, 18, 52, 70, 0x000F }},
        { 0, 53, { HexDumpType::NONE, false, 18, 52, 70 }},
        { 0, 58, { HexDumpType::NONE, false, 18, 52, 70 }},
        { 0, 59, { HexDumpType::AsciiChar, false, 18, 52, 70, 0x0004 }},
        { 0, 64, { HexDumpType::AsciiChar, false, 18, 52, 70, 0x0009 }},
        { 0, 70, { HexDumpType::AsciiChar, false, 18, 52, 70, 0x000F }},
        { 0, 71, { HexDumpType::NONE, false, 18, 52, 70 }},
// Test second row
// 0010  0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19 1A 1B  ________________
        { 1, 0, { HexDumpType::NONE, false, 6, 52, 70 }},
        { 1, 5, { HexDumpType::NONE, false, 6, 52, 70 }},
        { 1, 6, { HexDumpType::HexByte, true, 6, 52, 70, 0x0010 }},
        { 1, 7, { HexDumpType::HexByte, false, 6, 52, 70, 0x0010 }},
        { 1, 8, { HexDumpType::NONE, false, 6, 52, 70 }},
        { 1, 33, { HexDumpType::HexByte, true, 6, 52, 70, 0x0019 }},
        { 1, 34, { HexDumpType::HexByte, false, 6, 52, 70, 0x0019 }},
        { 1, 35, { HexDumpType::NONE, false, 6, 52, 70 }},
        { 1, 51, { HexDumpType::HexByte, true, 6, 52, 70, 0x001F }},
        { 1, 52, { HexDumpType::HexByte, false, 6, 52, 70, 0x001F }},
        { 1, 53, { HexDumpType::NONE, false, 6, 52, 70 }},
        { 1, 54, { HexDumpType::NONE, false, 6, 52, 70 }},
        { 1, 55, { HexDumpType::AsciiChar, false, 6, 52, 70, 0x0010 }},
        { 1, 64, { HexDumpType::AsciiChar, false, 6, 52, 70, 0x0019 }},
        { 1, 70, { HexDumpType::AsciiChar, false, 6, 52, 70, 0x001F }},
        { 1, 71, { HexDumpType::NONE, false, 6, 52, 70 }},
// Test third row
// 0020  1C 1D 1E 1F                                      ____
        { 2, 0, { HexDumpType::NONE, false, 6, 16, 58 }},
        { 2, 5, { HexDumpType::NONE, false, 6, 16, 58 }},
        { 2, 6, { HexDumpType::HexByte, true, 6, 16, 58, 0x0020 }},
        { 2, 7, { HexDumpType::HexByte, false, 6, 16, 58, 0x0020 }},
        { 2, 8, { HexDumpType::NONE, false, 6, 16, 58 }},
        { 2, 15, { HexDumpType::HexByte, true, 6, 16, 58, 0x0023 }},
        { 2, 16, { HexDumpType::HexByte, false, 6, 16, 58, 0x0023 }},
        { 2, 17, { HexDumpType::NONE, false, 6, 16, 58 }},
        { 2, 54, { HexDumpType::NONE, false, 6, 16, 58 }},
        { 2, 55, { HexDumpType::AsciiChar, false, 6, 16, 58, 0x0020 }},
        { 2, 58, { HexDumpType::AsciiChar, false, 6, 16, 58, 0x0023 }},
        { 2, 59, { HexDumpType::NONE, false, 6, 16, 58 }},
    }};

    testHexDumpProperties(patterns);
}

// 16 hex bytes, with address, with ascii, 16 bytes per line, offset 0.
TEST_F(test_free, fct_get_hex_dump_properties_2)
{
    using flx::HexDumpType;

    const sHexDumpPropertiesTestPatterns patterns =
    {
    //  size bpl  ASCI? addr? addr    extraSpace
        16LLU, 16U, true, true, 0x0100, std::nullopt, {
// Test first row
// 0100  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  ________________
        { 0, 0, { HexDumpType::NONE, false, 6, 52, 70 }},
        { 0, 5, { HexDumpType::NONE, false, 6, 52, 70 }},
        { 0, 6, { HexDumpType::HexByte, true, 6, 52, 70, 0x0100 }},
        { 0, 7, { HexDumpType::HexByte, false, 6, 52, 70, 0x0100 }},
        { 0, 8, { HexDumpType::NONE, false, 6, 52, 70 }},
        { 0, 33, { HexDumpType::HexByte, true, 6, 52, 70, 0x0109 }},
        { 0, 34, { HexDumpType::HexByte, false, 6, 52, 70, 0x0109 }},
        { 0, 35, { HexDumpType::NONE, false, 6, 52, 70 }},
        { 0, 51, { HexDumpType::HexByte, true, 6, 52, 70, 0x010F }},
        { 0, 52, { HexDumpType::HexByte, false, 6, 52, 70, 0x010F }},
        { 0, 53, { HexDumpType::NONE, false, 6, 52, 70 }},
        { 0, 54, { HexDumpType::NONE, false, 6, 52, 70 }},
        { 0, 55, { HexDumpType::AsciiChar, false, 6, 52, 70, 0x0100 }},
        { 0, 64, { HexDumpType::AsciiChar, false, 6, 52, 70, 0x0109 }},
        { 0, 70, { HexDumpType::AsciiChar, false, 6, 52, 70, 0x010F }},
        { 0, 71, { HexDumpType::NONE, false, 6, 52, 70 }},
    }};

    testHexDumpProperties(patterns);
}

// 4 hex bytes, with address, with ascii, 8 bytes per line, offset 2.
TEST_F(test_free, fct_get_hex_dump_properties_3)
{
    using flx::HexDumpType;

    const sHexDumpPropertiesTestPatterns patterns =
    {
    //  size bpl  ASCI? addr? addr    extraSpace
        4LLU, 8U, true, true, 0x0202, std::nullopt, {
// Test first row
// 0200        00 01 02 03          ____
        { 0, 0, { HexDumpType::NONE, false, 12, 22, 36 }},
        { 0, 5, { HexDumpType::NONE, false, 12, 22, 36 }},
        { 0, 6, { HexDumpType::NONE, false, 12, 22, 36 }},
        { 0, 12, { HexDumpType::HexByte, true, 12, 22, 36, 0x0202 }},
        { 0, 13, { HexDumpType::HexByte, false, 12, 22, 36, 0x0202 }},
        { 0, 14, { HexDumpType::NONE, false, 12, 22, 36 }},
        { 0, 18, { HexDumpType::HexByte, true, 12, 22, 36, 0x0204 }},
        { 0, 19, { HexDumpType::HexByte, false, 12, 22, 36, 0x0204 }},
        { 0, 20, { HexDumpType::NONE, false, 12, 22, 36 }},
        { 0, 21, { HexDumpType::HexByte, true, 12, 22, 36, 0x0205 }},
        { 0, 22, { HexDumpType::HexByte, false, 12, 22, 36, 0x0205 }},
        { 0, 23, { HexDumpType::NONE, false, 12, 22, 36 }},
        { 0, 24, { HexDumpType::NONE, false, 12, 22, 36 }},
        { 0, 32, { HexDumpType::NONE, false, 12, 22, 36 }},
        { 0, 33, { HexDumpType::AsciiChar, false, 12, 22, 36, 0x0202 }},
        { 0, 35, { HexDumpType::AsciiChar, false, 12, 22, 36, 0x0204 }},
        { 0, 36, { HexDumpType::AsciiChar, false, 12, 22, 36, 0x0205 }},
        { 0, 37, { HexDumpType::NONE, false, 12, 22, 36 }},
    }};

    testHexDumpProperties(patterns);
}

// 8 hex bytes, with address, no ascii, 8 bytes per line, offset 0.
TEST_F(test_free, fct_get_hex_dump_properties_4)
{
    using flx::HexDumpType;

    const sHexDumpPropertiesTestPatterns patterns =
    {
    //  size bpl  ASCI? addr? addr    extraSpace
        8LLU, 8U, false, true, 0x0300, std::nullopt, {
// Test first row
// 0300  00 01 02 03 04 05 06 07
        { 0, 0, { HexDumpType::NONE, false, 6, 28, 28 }},
        { 0, 5, { HexDumpType::NONE, false, 6, 28, 28 }},
        { 0, 6, { HexDumpType::HexByte, true, 6, 28, 28, 0x0300 }},
        { 0, 7, { HexDumpType::HexByte, false, 6, 28, 28, 0x0300 }},
        { 0, 8, { HexDumpType::NONE, false, 6, 28, 28 }},
        { 0, 15, { HexDumpType::HexByte, true, 6, 28, 28, 0x0303 }},
        { 0, 16, { HexDumpType::HexByte, false, 6, 28, 28, 0x0303 }},
        { 0, 17, { HexDumpType::NONE, false, 6, 28, 28 }},
        { 0, 27, { HexDumpType::HexByte, true, 6, 28, 28, 0x0307 }},
        { 0, 28, { HexDumpType::HexByte, false, 6, 28, 28, 0x0307 }},
        { 0, 29, { HexDumpType::NONE, false, 6, 28, 28 }},
    }};

    testHexDumpProperties(patterns);
}

// 8 hex bytes, no address, no ascii, 8 bytes per line, offset 0.
TEST_F(test_free, fct_get_hex_dump_properties_5)
{
    using flx::HexDumpType;

    const sHexDumpPropertiesTestPatterns patterns =
    {
    //  size bpl  ASCI? addr? addr    extraSpace
        8LLU, 8U, false, false, 0x0300, std::nullopt, {
// Test first row
// 00 01 02 03 04 05 06 07
        { 0, 0, { HexDumpType::HexByte, true, 0, 22, 22, 0x0300 }},
        { 0, 1, { HexDumpType::HexByte, false, 0, 22, 22, 0x0300 }},
        { 0, 2, { HexDumpType::NONE, false, 0, 22, 22 }},
        { 0, 9, { HexDumpType::HexByte, true, 0, 22, 22, 0x0303 }},
        { 0, 10, { HexDumpType::HexByte, false, 0, 22, 22, 0x0303 }},
        { 0, 11, { HexDumpType::NONE, false, 0, 22, 22 }},
        { 0, 21, { HexDumpType::HexByte, true, 0, 22, 22, 0x0307 }},
        { 0, 22, { HexDumpType::HexByte, false, 0, 22, 22, 0x0307 }},
        { 0, 23, { HexDumpType::NONE, false, 0, 22, 22 }},
    }};

    testHexDumpProperties(patterns);
}

// 20 hex bytes, with address, no ascii, 13 bytes per line, offset 9.
TEST_F(test_free, fct_get_hex_dump_properties_6)
{
    using flx::HexDumpType;

    const sHexDumpPropertiesTestPatterns patterns =
    {
    //  size   bpl  ASCI? addr? addr    extraSpace
        20LLU, 13U, false, true, 0x0100, std::nullopt, {
// Test first row
// 00F7                             00 01 02 03
        { 0, 0, { HexDumpType::NONE, false, 33, 43, 43 }},
        { 0, 6, { HexDumpType::NONE, false, 33, 43, 43 }},
        { 0, 7, { HexDumpType::NONE, false, 33, 43, 43 }},
        { 0, 30, { HexDumpType::NONE, false, 33, 43, 43 }},
        { 0, 31, { HexDumpType::NONE, false, 33, 43, 43 }},
        { 0, 32, { HexDumpType::NONE, false, 33, 43, 43 }},
        { 0, 33, { HexDumpType::HexByte, true, 33, 43, 43, 0x0100 }},
        { 0, 34, { HexDumpType::HexByte, false, 33, 43, 43, 0x0100 }},
        { 0, 35, { HexDumpType::NONE, false, 33, 43, 43 }},
        { 0, 39, { HexDumpType::HexByte, true, 33, 43, 43, 0x0102 }},
        { 0, 40, { HexDumpType::HexByte, false, 33, 43, 43, 0x0102 }},
        { 0, 41, { HexDumpType::NONE, false, 33, 43, 43 }},
        { 0, 42, { HexDumpType::HexByte, true, 33, 43, 43, 0x0103 }},
        { 0, 43, { HexDumpType::HexByte, false, 33, 43, 43, 0x0103 }},
        { 0, 44, { HexDumpType::NONE, false, 33, 43, 43 }},
// Test second row
// 0104  04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10
        { 1, 0, { HexDumpType::NONE, false, 6, 43, 43 }},
        { 1, 5, { HexDumpType::NONE, false, 6, 43, 43 }},
        { 1, 6, { HexDumpType::HexByte, true, 6, 43, 43, 0x0104 }},
        { 1, 7, { HexDumpType::HexByte, false, 6, 43, 43, 0x0104 }},
        { 1, 8, { HexDumpType::NONE, false, 6, 43, 43 }},
        { 1, 24, { HexDumpType::HexByte, true, 6, 43, 43, 0x010A }},
        { 1, 25, { HexDumpType::HexByte, false, 6, 43, 43, 0x010A }},
        { 1, 26, { HexDumpType::NONE, false, 6, 43, 43 }},
        { 1, 42, { HexDumpType::HexByte, true, 6, 43, 43, 0x0110 }},
        { 1, 43, { HexDumpType::HexByte, false, 6, 43, 43, 0x0110 }},
// Test third row
// 0111  11 12 13
        { 2, 0, { HexDumpType::NONE, false, 6, 13, 13 }},
        { 2, 5, { HexDumpType::NONE, false, 6, 13, 13 }},
        { 2, 6, { HexDumpType::HexByte, true, 6, 13, 13, 0x0111 }},
        { 2, 7, { HexDumpType::HexByte, false, 6, 13, 13, 0x0111 }},
        { 2, 8, { HexDumpType::NONE, false, 6, 13, 13 }},
        { 2, 12, { HexDumpType::HexByte, true, 6, 13, 13, 0x0113 }},
        { 2, 13, { HexDumpType::HexByte, false, 6, 13, 13, 0x0113 }},
        { 2, 14, { HexDumpType::NONE, false, 6, 13, 13 }},
    }};

    testHexDumpProperties(patterns);
}

// 20 hex bytes, with address, with ascii, 13 bytes per line, offset 9.
TEST_F(test_free, fct_get_hex_dump_properties_7)
{
    using flx::HexDumpType;

    const sHexDumpPropertiesTestPatterns patterns =
    {
    //  size   bpl  ASCI? addr? addr    extraSpace
        20LLU, 13U, true, true, 0x0100, std::nullopt, {
// Test first row
// 00F7                             00 01 02 03           ____
        { 0, 0, { HexDumpType::NONE, false, 33, 43, 58 }},
        { 0, 6, { HexDumpType::NONE, false, 33, 43, 58 }},
        { 0, 7, { HexDumpType::NONE, false, 33, 43, 58 }},
        { 0, 30, { HexDumpType::NONE, false, 33, 43, 58 }},
        { 0, 31, { HexDumpType::NONE, false, 33, 43, 58 }},
        { 0, 32, { HexDumpType::NONE, false, 33, 43, 58 }},
        { 0, 33, { HexDumpType::HexByte, true, 33, 43, 58, 0x0100 }},
        { 0, 34, { HexDumpType::HexByte, false, 33, 43, 58, 0x0100 }},
        { 0, 35, { HexDumpType::NONE, false, 33, 43, 58 }},
        { 0, 39, { HexDumpType::HexByte, true, 33, 43, 58, 0x0102 }},
        { 0, 40, { HexDumpType::HexByte, false, 33, 43, 58, 0x0102 }},
        { 0, 41, { HexDumpType::NONE, false, 33, 43, 58 }},
        { 0, 42, { HexDumpType::HexByte, true, 33, 43, 58, 0x0103 }},
        { 0, 43, { HexDumpType::HexByte, false, 33, 43, 58, 0x0103 }},
        { 0, 44, { HexDumpType::NONE, false, 33, 43, 58 }},
        { 0, 54, { HexDumpType::NONE, false, 33, 43, 58 }},
        { 0, 55, { HexDumpType::AsciiChar, false, 33, 43, 58, 0x0100 }},
        { 0, 57, { HexDumpType::AsciiChar, false, 33, 43, 58, 0x0102 }},
        { 0, 58, { HexDumpType::AsciiChar, false, 33, 43, 58, 0x0103 }},
        { 0, 59, { HexDumpType::NONE, false, 33, 43, 58 }},
// Test second row
// 0104  04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10  _____________
        { 1, 0, { HexDumpType::NONE, false, 6, 43, 58 }},
        { 1, 5, { HexDumpType::NONE, false, 6, 43, 58 }},
        { 1, 6, { HexDumpType::HexByte, true, 6, 43, 58, 0x0104 }},
        { 1, 7, { HexDumpType::HexByte, false, 6, 43, 58, 0x0104 }},
        { 1, 8, { HexDumpType::NONE, false, 6, 43, 58 }},
        { 1, 24, { HexDumpType::HexByte, true, 6, 43, 58, 0x010A }},
        { 1, 25, { HexDumpType::HexByte, false, 6, 43, 58, 0x010A }},
        { 1, 26, { HexDumpType::NONE, false, 6, 43, 58 }},
        { 1, 42, { HexDumpType::HexByte, true, 6, 43, 58, 0x0110 }},
        { 1, 43, { HexDumpType::HexByte, false, 6, 43, 58, 0x0110 }},
        { 1, 44, { HexDumpType::NONE, false, 6, 43, 58 }},
        { 1, 45, { HexDumpType::NONE, false, 6, 43, 58 }},
        { 1, 46, { HexDumpType::AsciiChar, false, 6, 43, 58, 0x0104 }},
        { 1, 52, { HexDumpType::AsciiChar, false, 6, 43, 58, 0x010A }},
        { 1, 58, { HexDumpType::AsciiChar, false, 6, 43, 58, 0x0110 }},
        { 1, 59, { HexDumpType::NONE, false, 6, 43, 58 }},
// Test third row
// 0111  11 12 13                                ___
        { 2, 0, { HexDumpType::NONE, false, 6, 13, 48 }},
        { 2, 5, { HexDumpType::NONE, false, 6, 13, 48 }},
        { 2, 6, { HexDumpType::HexByte, true, 6, 13, 48, 0x0111 }},
        { 2, 7, { HexDumpType::HexByte, false, 6, 13, 48, 0x0111 }},
        { 2, 8, { HexDumpType::NONE, false, 6, 13, 48 }},
        { 2, 12, { HexDumpType::HexByte, true, 6, 13, 48, 0x0113 }},
        { 2, 13, { HexDumpType::HexByte, false, 6, 13, 48, 0x0113 }},
        { 2, 14, { HexDumpType::NONE, false, 6, 13, 48 }},
        { 2, 45, { HexDumpType::NONE, false, 6, 13, 48 }},
        { 2, 46, { HexDumpType::AsciiChar, false, 6, 13, 48, 0x0111 }},
        { 2, 47, { HexDumpType::AsciiChar, false, 6, 13, 48, 0x0112 }},
        { 2, 48, { HexDumpType::AsciiChar, false, 6, 13, 48, 0x0113 }},
        { 2, 49, { HexDumpType::NONE, false, 6, 13, 48 }},
    }};

    testHexDumpProperties(patterns);
}

// 27 hex bytes, with address, with ascii, 13 bytes per line, offset 4.
// extra space each 5 bytes.
TEST_F(test_free, fct_get_hex_dump_properties_8)
{
    using flx::HexDumpType;

    const sHexDumpPropertiesTestPatterns patterns =
    {
    //  size   bpl  ASCI? addr? addr    extraSpace
        27LLU, 13U, true, true, 0x00C7, 5, {
// Test first row
// 00C3              00  01 02 03 04 05  06 07 08      _ _____ ___
        { 0, 0, { HexDumpType::NONE, false, 18, 45, 62 }},
        { 0, 6, { HexDumpType::NONE, false, 18, 45, 62 }},
        { 0, 7, { HexDumpType::NONE, false, 18, 45, 62 }},
        { 0, 15, { HexDumpType::NONE, false, 18, 45, 62 }},
        { 0, 16, { HexDumpType::NONE, false, 18, 45, 62 }},
        { 0, 17, { HexDumpType::NONE, false, 18, 45, 62 }},
        { 0, 18, { HexDumpType::HexByte, true, 18, 45, 62, 0x00C7 }},
        { 0, 19, { HexDumpType::HexByte, false, 18, 45, 62, 0x00C7 }},
        { 0, 20, { HexDumpType::NONE, false, 18, 45, 62 }},
        { 0, 21, { HexDumpType::NONE, false, 18, 45, 62 }},
        { 0, 22, { HexDumpType::HexByte, true, 18, 45, 62, 0x00C8 }},
        { 0, 23, { HexDumpType::HexByte, false, 18, 45, 62, 0x00C8 }},
        { 0, 24, { HexDumpType::NONE, false, 18, 45, 62 }},
        { 0, 34, { HexDumpType::HexByte, true, 18, 45, 62, 0x00CC }},
        { 0, 35, { HexDumpType::HexByte, false, 18, 45, 62, 0x00CC }},
        { 0, 36, { HexDumpType::NONE, false, 18, 45, 62 }},
        { 0, 37, { HexDumpType::NONE, false, 18, 45, 62 }},
        { 0, 38, { HexDumpType::HexByte, true, 18, 45, 62, 0x00CD }},
        { 0, 39, { HexDumpType::HexByte, false, 18, 45, 62, 0x00CD }},
        { 0, 40, { HexDumpType::NONE, false, 18, 45, 62 }},
        { 0, 44, { HexDumpType::HexByte, true, 18, 45, 62, 0x00CF }},
        { 0, 45, { HexDumpType::HexByte, false, 18, 45, 62, 0x00CF }},
        { 0, 46, { HexDumpType::NONE, false, 18, 45, 62 }},
        { 0, 51, { HexDumpType::NONE, false, 18, 45, 62 }},
        { 0, 52, { HexDumpType::AsciiChar, false, 18, 45, 62, 0x00C7 }},
        { 0, 53, { HexDumpType::NONE, false, 18, 45, 62 }},
        { 0, 54, { HexDumpType::AsciiChar, false, 18, 45, 62, 0x00C8 }},
        { 0, 58, { HexDumpType::AsciiChar, false, 18, 45, 62, 0x00CC }},
        { 0, 59, { HexDumpType::NONE, false, 18, 45, 62 }},
        { 0, 60, { HexDumpType::AsciiChar, false, 18, 45, 62, 0x00CD }},
        { 0, 62, { HexDumpType::AsciiChar, false, 18, 45, 62, 0x00CF }},
        { 0, 63, { HexDumpType::NONE, false, 18, 45, 62 }},
// Test second row
// 00D0  09 0A 0B 0C 0D  0E 0F 10 11 12  13 14 15  _____ _____ ___
        { 1, 0, { HexDumpType::NONE, false, 6, 45, 62 }},
        { 1, 5, { HexDumpType::NONE, false, 6, 45, 62 }},
        { 1, 6, { HexDumpType::HexByte, true, 6, 45, 62, 0x00D0 }},
        { 1, 7, { HexDumpType::HexByte, false, 6, 45, 62, 0x00D0 }},
        { 1, 8, { HexDumpType::NONE, false, 6, 45, 62 }},
        { 1, 18, { HexDumpType::HexByte, true, 6, 45, 62, 0x00D4 }},
        { 1, 19, { HexDumpType::HexByte, false, 6, 45, 62, 0x00D4 }},
        { 1, 20, { HexDumpType::NONE, false, 6, 45, 62 }},
        { 1, 21, { HexDumpType::NONE, false, 6, 45, 62 }},
        { 1, 22, { HexDumpType::HexByte, true, 6, 45, 62, 0x00D5 }},
        { 1, 23, { HexDumpType::HexByte, false, 6, 45, 62, 0x00D5 }},
        { 1, 24, { HexDumpType::NONE, false, 6, 45, 62 }},
        { 1, 34, { HexDumpType::HexByte, true, 6, 45, 62, 0x00D9 }},
        { 1, 35, { HexDumpType::HexByte, false, 6, 45, 62, 0x00D9 }},
        { 1, 36, { HexDumpType::NONE, false, 6, 45, 62 }},
        { 1, 37, { HexDumpType::NONE, false, 6, 45, 62 }},
        { 1, 38, { HexDumpType::HexByte, true, 6, 45, 62, 0x00DA }},
        { 1, 39, { HexDumpType::HexByte, false, 6, 45, 62, 0x00DA }},
        { 1, 40, { HexDumpType::NONE, false, 6, 45, 62 }},
        { 1, 44, { HexDumpType::HexByte, true, 6, 45, 62, 0x00DC }},
        { 1, 45, { HexDumpType::HexByte, false, 6, 45, 62, 0x00DC }},
        { 1, 46, { HexDumpType::NONE, false, 6, 45, 62 }},
        { 1, 47, { HexDumpType::NONE, false, 6, 45, 62 }},
        { 1, 48, { HexDumpType::AsciiChar, false, 6, 45, 62, 0x00D0 }},
        { 1, 52, { HexDumpType::AsciiChar, false, 6, 45, 62, 0x00D4 }},
        { 1, 53, { HexDumpType::NONE, false, 6, 45, 62 }},
        { 1, 54, { HexDumpType::AsciiChar, false, 6, 45, 62, 0x00D5 }},
        { 1, 58, { HexDumpType::AsciiChar, false, 6, 45, 62, 0x00D9 }},
        { 1, 59, { HexDumpType::NONE, false, 6, 45, 62 }},
        { 1, 60, { HexDumpType::AsciiChar, false, 6, 45, 62, 0x00DA }},
        { 1, 62, { HexDumpType::AsciiChar, false, 6, 45, 62, 0x00DC }},
        { 1, 63, { HexDumpType::NONE, false, 6, 45, 62 }},
// Test third row
// 00DD  16 17 18 19 1A                            _____
        { 2, 0, { HexDumpType::NONE, false, 6, 19, 52 }},
        { 2, 5, { HexDumpType::NONE, false, 6, 19, 52 }},
        { 2, 6, { HexDumpType::HexByte, true, 6, 19, 52, 0x00DD }},
        { 2, 7, { HexDumpType::HexByte, false, 6, 19, 52, 0x00DD }},
        { 2, 8, { HexDumpType::NONE, false, 6, 19, 52 }},
        { 2, 18, { HexDumpType::HexByte, true, 6, 19, 52, 0x00E1 }},
        { 2, 19, { HexDumpType::HexByte, false, 6, 19, 52, 0x00E1 }},
        { 2, 20, { HexDumpType::NONE, false, 6, 19, 52 }},
        { 2, 47, { HexDumpType::NONE, false, 6, 19, 52 }},
        { 2, 48, { HexDumpType::AsciiChar, false, 6, 19, 52, 0x00DD }},
        { 2, 52, { HexDumpType::AsciiChar, false, 6, 19, 52, 0x00E1 }},
        { 2, 53, { HexDumpType::NONE, false, 6, 19, 52 }},
    }};

    testHexDumpProperties(patterns);
}

// 32 hex bytes, no address, no ascii, 13 bytes per line, offset 4.
// extra space each 5 bytes.
TEST_F(test_free, fct_get_hex_dump_properties_9)
{
    using flx::HexDumpType;

    const sHexDumpPropertiesTestPatterns patterns =
    {
    //  size   bpl  ASCI?  addr? addr    extraSpace
        32LLU, 13U, false, false, 0x0000, 5, {
// Test first row
// 00 01 02 03 04  05 06 07 08 09  0A 0B 0C
        { 0, 0, { HexDumpType::HexByte, true, 0, 39, 39, 0x0000 }},
        { 0, 1, { HexDumpType::HexByte, false, 0, 39, 39, 0x0000 }},
        { 0, 2, { HexDumpType::NONE, false, 0, 39, 39 }},
        { 0, 12, { HexDumpType::HexByte, true, 0, 39, 39, 0x0004 }},
        { 0, 13, { HexDumpType::HexByte, false, 0, 39, 39, 0x0004 }},
        { 0, 14, { HexDumpType::NONE, false, 0, 39, 39 }},
        { 0, 15, { HexDumpType::NONE, false, 0, 39, 39 }},
        { 0, 16, { HexDumpType::HexByte, true, 0, 39, 39, 0x0005 }},
        { 0, 17, { HexDumpType::HexByte, false, 0, 39, 39, 0x0005 }},
        { 0, 18, { HexDumpType::NONE, false, 0, 39, 39 }},
        { 0, 28, { HexDumpType::HexByte, true, 0, 39, 39, 0x0009 }},
        { 0, 29, { HexDumpType::HexByte, false, 0, 39, 39, 0x0009 }},
        { 0, 30, { HexDumpType::NONE, false, 0, 39, 39 }},
        { 0, 31, { HexDumpType::NONE, false, 0, 39, 39 }},
        { 0, 32, { HexDumpType::HexByte, true, 0, 39, 39, 0x000A }},
        { 0, 33, { HexDumpType::HexByte, false, 0, 39, 39, 0x000A }},
        { 0, 34, { HexDumpType::NONE, false, 0, 39, 39 }},
        { 0, 38, { HexDumpType::HexByte, true, 0, 39, 39, 0x000C }},
        { 0, 39, { HexDumpType::HexByte, false, 0, 39, 39, 0x000C }},
        { 0, 40, { HexDumpType::NONE, false, 0, 39, 39 }},
// Test second row
// 0D 0E 0F 10 11  12 13 14 15 16  17 18 19
        { 1, 0, { HexDumpType::HexByte, true, 0, 39, 39, 0x000D }},
        { 1, 1, { HexDumpType::HexByte, false, 0, 39, 39, 0x000D }},
        { 1, 2, { HexDumpType::NONE, false, 0, 39, 39 }},
        { 1, 12, { HexDumpType::HexByte, true, 0, 39, 39, 0x0011 }},
        { 1, 13, { HexDumpType::HexByte, false, 0, 39, 39, 0x0011 }},
        { 1, 14, { HexDumpType::NONE, false, 0, 39, 39 }},
        { 1, 15, { HexDumpType::NONE, false, 0, 39, 39 }},
        { 1, 16, { HexDumpType::HexByte, true, 0, 39, 39, 0x0012 }},
        { 1, 17, { HexDumpType::HexByte, false, 0, 39, 39, 0x0012 }},
        { 1, 18, { HexDumpType::NONE, false, 0, 39, 39 }},
        { 1, 28, { HexDumpType::HexByte, true, 0, 39, 39, 0x0016 }},
        { 1, 29, { HexDumpType::HexByte, false, 0, 39, 39, 0x0016 }},
        { 1, 30, { HexDumpType::NONE, false, 0, 39, 39 }},
        { 1, 31, { HexDumpType::NONE, false, 0, 39, 39 }},
        { 1, 32, { HexDumpType::HexByte, true, 0, 39, 39, 0x0017 }},
        { 1, 33, { HexDumpType::HexByte, false, 0, 39, 39, 0x0017 }},
        { 1, 34, { HexDumpType::NONE, false, 0, 39, 39 }},
        { 1, 38, { HexDumpType::HexByte, true, 0, 39, 39, 0x0019 }},
        { 1, 39, { HexDumpType::HexByte, false, 0, 39, 39, 0x0019 }},
        { 1, 40, { HexDumpType::NONE, false, 0, 39, 39 }},
// Test third row
// 1A 1B 1C 1D 1E  1F
        { 2, 0, { HexDumpType::HexByte, true, 0, 17, 17, 0x001A }},
        { 2, 1, { HexDumpType::HexByte, false, 0, 17, 17, 0x001A }},
        { 2, 2, { HexDumpType::NONE, false, 0, 17, 17 }},
        { 2, 12, { HexDumpType::HexByte, true, 0, 17, 17, 0x001E }},
        { 2, 13, { HexDumpType::HexByte, false, 0, 17, 17, 0x001E }},
        { 2, 14, { HexDumpType::NONE, false, 0, 17, 17 }},
        { 2, 15, { HexDumpType::NONE, false, 0, 17, 17 }},
        { 2, 16, { HexDumpType::HexByte, true, 0, 17, 17, 0x001F }},
        { 2, 17, { HexDumpType::HexByte, false, 0, 17, 17, 0x001F }},
        { 2, 18, { HexDumpType::NONE, false, 0, 17, 17 }},
    }};

    testHexDumpProperties(patterns);
}

// 15 hex bytes, no address, with ascii, 15 bytes per line,
// extra space each 5 bytes.
TEST_F(test_free, fct_get_hex_dump_properties_10)
{
    using flx::HexDumpType;

    const sHexDumpPropertiesTestPatterns patterns =
    {
    //  size   bpl  ASCI? addr? addr    extraSpace
        15LLU, 15U, true, false, 0x0000, 5, {
// Test first row
// 00 01 02 03 04  05 06 07 08 09  0A 0B 0C 0D 0E  _____ _____ _____
        { 0, 0, { HexDumpType::HexByte, true, 0, 45, 64, 0x0000 }},
        { 0, 1, { HexDumpType::HexByte, false, 0, 45, 64, 0x0000 }},
        { 0, 2, { HexDumpType::NONE, false, 0, 45, 64 }},
        { 0, 12, { HexDumpType::HexByte, true, 0, 45, 64, 0x0004 }},
        { 0, 13, { HexDumpType::HexByte, false, 0, 45, 64, 0x0004 }},
        { 0, 14, { HexDumpType::NONE, false, 0, 45, 64 }},
        { 0, 15, { HexDumpType::NONE, false, 0, 45, 64 }},
        { 0, 16, { HexDumpType::HexByte, true, 0, 45, 64, 0x0005 }},
        { 0, 17, { HexDumpType::HexByte, false, 0, 45, 64, 0x0005 }},
        { 0, 18, { HexDumpType::NONE, false, 0, 45, 64 }},
        { 0, 28, { HexDumpType::HexByte, true, 0, 45, 64, 0x0009 }},
        { 0, 29, { HexDumpType::HexByte, false, 0, 45, 64, 0x0009 }},
        { 0, 30, { HexDumpType::NONE, false, 0, 45, 64 }},
        { 0, 31, { HexDumpType::NONE, false, 0, 45, 64 }},
        { 0, 32, { HexDumpType::HexByte, true, 0, 45, 64, 0x000A }},
        { 0, 33, { HexDumpType::HexByte, false, 0, 45, 64, 0x000A }},
        { 0, 34, { HexDumpType::NONE, false, 0, 45, 64 }},
        { 0, 44, { HexDumpType::HexByte, true, 0, 45, 64, 0x000E }},
        { 0, 45, { HexDumpType::HexByte, false, 0, 45, 64, 0x000E }},
        { 0, 46, { HexDumpType::NONE, false, 0, 45, 64 }},
        { 0, 47, { HexDumpType::NONE, false, 0, 45, 64 }},
        { 0, 48, { HexDumpType::AsciiChar, false, 0, 45, 64, 0x0000 }},
        { 0, 52, { HexDumpType::AsciiChar, false, 0, 45, 64, 0x0004 }},
        { 0, 53, { HexDumpType::NONE, false, 0, 45, 64 }},
        { 0, 54, { HexDumpType::AsciiChar, false, 0, 45, 64, 0x0005 }},
        { 0, 58, { HexDumpType::AsciiChar, false, 0, 45, 64, 0x0009 }},
        { 0, 59, { HexDumpType::NONE, false, 0, 45, 64 }},
        { 0, 60, { HexDumpType::AsciiChar, false, 0, 45, 64, 0x000A }},
        { 0, 64, { HexDumpType::AsciiChar, false, 0, 45, 64, 0x000E }},
        { 0, 65, { HexDumpType::NONE, false, 0, 45, 64 }},
    }};

    testHexDumpProperties(patterns);
}

// 12 hex bytes, with address, with ascii, 15 bytes per line,
// extra space each 3 bytes.
TEST_F(test_free, fct_get_hex_dump_properties_11)
{
    using flx::HexDumpType;

    const sHexDumpPropertiesTestPatterns patterns =
    {
    //  size   bpl  ASCI? addr? addr    extraSpace
        12LLU, 15U, true, true, 0x0009, 3, {
// Test first row
// 0000                                00 01 02  03 04 05              ___ ___
        { 0, 0, { HexDumpType::NONE, false, 36, 53, 74 }},
        { 0, 6, { HexDumpType::NONE, false, 36, 53, 74 }},
        { 0, 7, { HexDumpType::NONE, false, 36, 53, 74 }},
        { 0, 36, { HexDumpType::HexByte, true, 36, 53, 74, 0x0009 }},
        { 0, 37, { HexDumpType::HexByte, false, 36, 53, 74, 0x0009 }},
        { 0, 38, { HexDumpType::NONE, false, 36, 53, 74 }},
        { 0, 42, { HexDumpType::HexByte, true, 36, 53, 74, 0x000B }},
        { 0, 43, { HexDumpType::HexByte, false, 36, 53, 74, 0x000B }},
        { 0, 44, { HexDumpType::NONE, false, 36, 53, 74 }},
        { 0, 45, { HexDumpType::NONE, false, 36, 53, 74 }},
        { 0, 46, { HexDumpType::HexByte, true, 36, 53, 74, 0x000C }},
        { 0, 47, { HexDumpType::HexByte, false, 36, 53, 74, 0x000C }},
        { 0, 48, { HexDumpType::NONE, false, 36, 53, 74 }},
        { 0, 52, { HexDumpType::HexByte, true, 36, 53, 74, 0x000E }},
        { 0, 53, { HexDumpType::HexByte, false, 36, 53, 74, 0x000E }},
        { 0, 54, { HexDumpType::NONE, false, 36, 53, 74 }},
        { 0, 51, { HexDumpType::NONE, false, 36, 53, 74 }},
        { 0, 68, { HexDumpType::AsciiChar, false, 36, 53, 74, 0x0009 }},
        { 0, 70, { HexDumpType::AsciiChar, false, 36, 53, 74, 0x000B }},
        { 0, 71, { HexDumpType::NONE, false, 36, 53, 74 }},
        { 0, 72, { HexDumpType::AsciiChar, false, 36, 53, 74, 0x000C }},
        { 0, 74, { HexDumpType::AsciiChar, false, 36, 53, 74, 0x000E }},
        { 0, 75, { HexDumpType::NONE, false, 36, 53, 74 }},
// Test second row
// 000F  06 07 08  09 0A 0B                                ___ ___
        { 1, 0, { HexDumpType::NONE, false, 6, 23, 62 }},
        { 1, 6, { HexDumpType::HexByte, true, 6, 23, 62, 0x000F }},
        { 1, 7, { HexDumpType::HexByte, false, 6, 23, 62, 0x000F }},
        { 1, 8, { HexDumpType::NONE, false, 6, 23, 62 }},
        { 1, 12, { HexDumpType::HexByte, true, 6, 23, 62, 0x0011 }},
        { 1, 13, { HexDumpType::HexByte, false, 6, 23, 62, 0x0011 }},
        { 1, 14, { HexDumpType::NONE, false, 6, 23, 62 }},
        { 1, 15, { HexDumpType::NONE, false, 6, 23, 62 }},
        { 1, 16, { HexDumpType::HexByte, true, 6, 23, 62, 0x0012 }},
        { 1, 17, { HexDumpType::HexByte, false, 6, 23, 62, 0x0012 }},
        { 1, 18, { HexDumpType::NONE, false, 6, 23, 62 }},
        { 1, 22, { HexDumpType::HexByte, true, 6, 23, 62, 0x0014 }},
        { 1, 23, { HexDumpType::HexByte, false, 6, 23, 62, 0x0014 }},
        { 1, 24, { HexDumpType::NONE, false, 6, 23, 62 }},
        { 1, 55, { HexDumpType::NONE, false, 6, 23, 62 }},
        { 1, 56, { HexDumpType::AsciiChar, false, 6, 23, 62, 0x000F }},
        { 1, 58, { HexDumpType::AsciiChar, false, 6, 23, 62, 0x0011 }},
        { 1, 59, { HexDumpType::NONE, false, 6, 23, 62 }},
        { 1, 60, { HexDumpType::AsciiChar, false, 6, 23, 62, 0x0012 }},
        { 1, 62, { HexDumpType::AsciiChar, false, 6, 23, 62, 0x0014 }},
        { 1, 63, { HexDumpType::NONE, false, 6, 23, 62 }},
    }};

    testHexDumpProperties(patterns);
}

// 14 hex bytes, with address, with ascii, 15 bytes per line,
// extra space each 3 bytes.
TEST_F(test_free, fct_get_hex_dump_properties_12)
{
    using flx::HexDumpType;

    const sHexDumpPropertiesTestPatterns patterns =
    {
    //  size   bpl  ASCI? addr? addr    extraSpace
        14LLU, 15U, true, true, 0x0008, 3, {
// Test first row
// 0000                            00  01 02 03  04 05 06            _ ___ ___
        { 0, 0, { HexDumpType::NONE, false, 32, 53, 74 }},
        { 0, 6, { HexDumpType::NONE, false, 32, 53, 74 }},
        { 0, 7, { HexDumpType::NONE, false, 32, 53, 74 }},
        { 0, 32, { HexDumpType::HexByte, true, 32, 53, 74, 0x0008 }},
        { 0, 33, { HexDumpType::HexByte, false, 32, 53, 74, 0x0008 }},
        { 0, 34, { HexDumpType::NONE, false, 32, 53, 74 }},
        { 0, 35, { HexDumpType::NONE, false, 32, 53, 74 }},
        { 0, 36, { HexDumpType::HexByte, true, 32, 53, 74, 0x0009 }},
        { 0, 37, { HexDumpType::HexByte, false, 32, 53, 74, 0x0009 }},
        { 0, 38, { HexDumpType::NONE, false, 32, 53, 74 }},
        { 0, 42, { HexDumpType::HexByte, true, 32, 53, 74, 0x000B }},
        { 0, 43, { HexDumpType::HexByte, false, 32, 53, 74, 0x000B }},
        { 0, 44, { HexDumpType::NONE, false, 32, 53, 74 }},
        { 0, 45, { HexDumpType::NONE, false, 32, 53, 74 }},
        { 0, 46, { HexDumpType::HexByte, true, 32, 53, 74, 0x000C }},
        { 0, 47, { HexDumpType::HexByte, false, 32, 53, 74, 0x000C }},
        { 0, 48, { HexDumpType::NONE, false, 32, 53, 74 }},
        { 0, 52, { HexDumpType::HexByte, true, 32, 53, 74, 0x000E }},
        { 0, 53, { HexDumpType::HexByte, false, 32, 53, 74, 0x000E }},
        { 0, 54, { HexDumpType::NONE, false, 32, 53, 74 }},
        { 0, 51, { HexDumpType::NONE, false, 32, 53, 74 }},
        { 0, 68, { HexDumpType::AsciiChar, false, 32, 53, 74, 0x0009 }},
        { 0, 70, { HexDumpType::AsciiChar, false, 32, 53, 74, 0x000B }},
        { 0, 71, { HexDumpType::NONE, false, 32, 53, 74 }},
        { 0, 72, { HexDumpType::AsciiChar, false, 32, 53, 74, 0x000C }},
        { 0, 74, { HexDumpType::AsciiChar, false, 32, 53, 74, 0x000E }},
        { 0, 75, { HexDumpType::NONE, false, 32, 53, 74 }},
// Test second row
// 000F  07 08 09  0A 0B 0C  0D                            ___ ___ _
        { 1, 0, { HexDumpType::NONE, false, 6, 27, 64 }},
        { 1, 6, { HexDumpType::HexByte, true, 6, 27, 64, 0x000F }},
        { 1, 7, { HexDumpType::HexByte, false, 6, 27, 64, 0x000F }},
        { 1, 8, { HexDumpType::NONE, false, 6, 27, 64 }},
        { 1, 12, { HexDumpType::HexByte, true, 6, 27, 64, 0x0011 }},
        { 1, 13, { HexDumpType::HexByte, false, 6, 27, 64, 0x0011 }},
        { 1, 14, { HexDumpType::NONE, false, 6, 27, 64 }},
        { 1, 15, { HexDumpType::NONE, false, 6, 27, 64 }},
        { 1, 16, { HexDumpType::HexByte, true, 6, 27, 64, 0x0012 }},
        { 1, 17, { HexDumpType::HexByte, false, 6, 27, 64, 0x0012 }},
        { 1, 18, { HexDumpType::NONE, false, 6, 27, 64 }},
        { 1, 22, { HexDumpType::HexByte, true, 6, 27, 64, 0x0014 }},
        { 1, 23, { HexDumpType::HexByte, false, 6, 27, 64, 0x0014 }},
        { 1, 24, { HexDumpType::NONE, false, 6, 27, 64 }},
        { 1, 25, { HexDumpType::NONE, false, 6, 27, 64 }},
        { 1, 26, { HexDumpType::HexByte, true, 6, 27, 64, 0x0015 }},
        { 1, 27, { HexDumpType::HexByte, false, 6, 27, 64, 0x0015 }},
        { 1, 28, { HexDumpType::NONE, false, 6, 27, 64 }},
        { 1, 55, { HexDumpType::NONE, false, 6, 27, 64 }},
        { 1, 56, { HexDumpType::AsciiChar, false, 6, 27, 64, 0x000F }},
        { 1, 58, { HexDumpType::AsciiChar, false, 6, 27, 64, 0x0011 }},
        { 1, 59, { HexDumpType::NONE, false, 6, 27, 64 }},
        { 1, 60, { HexDumpType::AsciiChar, false, 6, 27, 64, 0x0012 }},
        { 1, 62, { HexDumpType::AsciiChar, false, 6, 27, 64, 0x0014 }},
        { 1, 63, { HexDumpType::NONE, false, 6, 27, 64 }},
        { 1, 64, { HexDumpType::AsciiChar, false, 6, 27, 64, 0x0015 }},
        { 1, 65, { HexDumpType::NONE, false, 6, 27, 64 }},
    }};

    testHexDumpProperties(patterns);
}

// 16 hex bytes, with address, with ascii, 16 bytes per line,
// extra space each 8 bytes.
TEST_F(test_free, fct_get_hex_dump_properties_13)
{
    using flx::HexDumpType;

    const sHexDumpPropertiesTestPatterns patterns =
    {
    //  size   bpl  ASCI? addr? addr    extraSpace
        16LLU, 16U, true, true, 0x0008, 8, {
// Test first row
// 0000                           00 01 02 03 04 05 06 07           ________
        { 0, 0, { HexDumpType::NONE, false, 31, 53, 72 }},
        { 0, 6, { HexDumpType::NONE, false, 31, 53, 72 }},
        { 0, 7, { HexDumpType::NONE, false, 31, 53, 72 }},
        { 0, 31, { HexDumpType::HexByte, true, 31, 53, 72, 0x0008 }},
        { 0, 32, { HexDumpType::HexByte, false, 31, 53, 72, 0x0008 }},
        { 0, 33, { HexDumpType::NONE, false, 31, 53, 72 }},
        { 0, 43, { HexDumpType::HexByte, true, 31, 53, 72, 0x000C }},
        { 0, 44, { HexDumpType::HexByte, false, 31, 53, 72, 0x000C }},
        { 0, 45, { HexDumpType::NONE, false, 31, 53, 72 }},
        { 0, 52, { HexDumpType::HexByte, true, 31, 53, 72, 0x000F }},
        { 0, 53, { HexDumpType::HexByte, false, 31, 53, 72, 0x000F }},
        { 0, 54, { HexDumpType::NONE, false, 31, 53, 72 }},
        { 0, 63, { HexDumpType::NONE, false, 31, 53, 72 }},
        { 0, 64, { HexDumpType::NONE, false, 31, 53, 72 }},
        { 0, 65, { HexDumpType::AsciiChar, false, 31, 53, 72, 0x0008 }},
        { 0, 72, { HexDumpType::AsciiChar, false, 31, 53, 72, 0x000F }},
        { 0, 73, { HexDumpType::NONE, false, 31, 53, 72 }},
// Test second row
// 0010  08 09 0A 0B 0C 0D 0E 0F                           ________
        { 1, 0, { HexDumpType::NONE, false, 6, 28, 63 }},
        { 1, 6, { HexDumpType::HexByte, true, 6, 28, 63, 0x0010 }},
        { 1, 7, { HexDumpType::HexByte, false, 6, 28, 63, 0x0010 }},
        { 1, 8, { HexDumpType::NONE, false, 6, 28, 63 }},
        { 1, 18, { HexDumpType::HexByte, true, 6, 28, 63, 0x0014 }},
        { 1, 19, { HexDumpType::HexByte, false, 6, 28, 63, 0x0014 }},
        { 1, 20, { HexDumpType::NONE, false, 6, 28, 63 }},
        { 1, 27, { HexDumpType::HexByte, true, 6, 28, 63, 0x0017 }},
        { 1, 28, { HexDumpType::HexByte, false, 6, 28, 63, 0x0017 }},
        { 1, 29, { HexDumpType::NONE, false, 6, 28, 63 }},
        { 1, 30, { HexDumpType::NONE, false, 6, 28, 63 }},
        { 1, 55, { HexDumpType::NONE, false, 6, 28, 63 }},
        { 1, 56, { HexDumpType::AsciiChar, false, 6, 28, 63, 0x0010 }},
        { 1, 60, { HexDumpType::AsciiChar, false, 6, 28, 63, 0x0014 }},
        { 1, 63, { HexDumpType::AsciiChar, false, 6, 28, 63, 0x0017 }},
        { 1, 64, { HexDumpType::NONE, false, 6, 28, 63 }},
    }};

    testHexDumpProperties(patterns);
}
