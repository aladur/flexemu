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
#include "typedefs.h"
#include "free.h"
#include "fixt_debugout.h"
//#include <cstddef>
#include <limits>
#include <string>
#include <vector>
#include <iostream>
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

TEST(test_free, fct_convert_to_int)
{
    int int_val;
    unsigned int uint_val;
    short short_val;
    unsigned short ushort_val;
    long long int long_val;
    long long unsigned int ulong_val;
    char ch_val;
    unsigned char uch_val;

    // Positive tests.
    EXPECT_TRUE(flx::convert("0000", int_val));
    EXPECT_EQ(int_val, 0);
    EXPECT_TRUE(flx::convert("0", short_val));
    EXPECT_EQ(int_val, 0);
    EXPECT_TRUE(flx::convert("-0", int_val));
    EXPECT_EQ(int_val, 0);
    EXPECT_TRUE(flx::convert("000012345", int_val));
    EXPECT_EQ(int_val, 12345);
    EXPECT_TRUE(flx::convert("12345", int_val));
    EXPECT_EQ(int_val, 12345);
    EXPECT_TRUE(flx::convert("-12345", int_val));
    EXPECT_EQ(int_val, -12345);
    EXPECT_TRUE(flx::convert("0", uint_val));
    EXPECT_EQ(uint_val, 0U);
    EXPECT_TRUE(flx::convert("12345", uint_val));
    EXPECT_EQ(uint_val, 12345U);
    EXPECT_TRUE(flx::convert("4294967295", uint_val));
    const auto uint_expected = std::numeric_limits<decltype(uint_val)>::max();
    EXPECT_EQ(uint_val, uint_expected);
    EXPECT_TRUE(flx::convert("0", short_val));
    EXPECT_EQ(short_val, 0);
    EXPECT_TRUE(flx::convert("12345", short_val));
    EXPECT_EQ(short_val, 12345);
    EXPECT_TRUE(flx::convert("-12345", short_val));
    EXPECT_EQ(short_val, -12345);
    EXPECT_TRUE(flx::convert("0", ushort_val));
    EXPECT_EQ(ushort_val, 0);
    EXPECT_TRUE(flx::convert("12345", ushort_val));
    EXPECT_EQ(ushort_val, 12345);
    EXPECT_TRUE(flx::convert("65535", ushort_val));
    const auto ushort_expected =
        std::numeric_limits<decltype(ushort_val)>::max();
    EXPECT_EQ(ushort_val, ushort_expected);
    EXPECT_TRUE(flx::convert("0", long_val));
    EXPECT_EQ(long_val, 0);
    EXPECT_TRUE(flx::convert("12345", long_val));
    EXPECT_EQ(long_val, 12345);
    EXPECT_TRUE(flx::convert("-12345", long_val));
    EXPECT_EQ(long_val, -12345);
    EXPECT_TRUE(flx::convert("0", ulong_val));
    EXPECT_EQ(ulong_val, 0);
    EXPECT_TRUE(flx::convert("12345", ulong_val));
    EXPECT_EQ(ulong_val, 12345);
    EXPECT_TRUE(flx::convert("18446744073709551615", ulong_val));
    const auto ulong_expected = std::numeric_limits<decltype(ulong_val)>::max();
    EXPECT_EQ(ulong_val, ulong_expected);
    EXPECT_TRUE(flx::convert("0", ch_val));
    EXPECT_EQ(ch_val, 0);
    EXPECT_TRUE(flx::convert("123", ch_val));
    EXPECT_EQ(ch_val, 123);
    EXPECT_TRUE(flx::convert("-123", ch_val));
    EXPECT_EQ(ch_val, -123);
    EXPECT_TRUE(flx::convert("0", uch_val));
    EXPECT_EQ(uch_val, 0);
    EXPECT_TRUE(flx::convert("123", uch_val));
    EXPECT_EQ(uch_val, 123);
    EXPECT_TRUE(flx::convert("255", uch_val));
    const auto uch_expected = std::numeric_limits<decltype(uch_val)>::max();
    EXPECT_EQ(uch_val, uch_expected);
    EXPECT_TRUE(flx::convert("0", int_val, 16));
    EXPECT_EQ(int_val, 0);
    EXPECT_TRUE(flx::convert("ABCDE", int_val, 16));
    EXPECT_EQ(int_val, 0xABCDE);
    EXPECT_TRUE(flx::convert("-ABCDE", int_val, 16));
    EXPECT_EQ(int_val, -0xABCDE);
    EXPECT_TRUE(flx::convert("0", int_val, 2));
    EXPECT_EQ(int_val, 0);
    EXPECT_TRUE(flx::convert("011011100010101011101", int_val, 2));
    EXPECT_EQ(int_val, 0b011011100010101011101);
    EXPECT_TRUE(flx::convert("-011011100010101011101", int_val, 2));
    EXPECT_EQ(int_val, -0b011011100010101011101);
    // Negative tests.
    EXPECT_FALSE(flx::convert("", int_val));
    EXPECT_FALSE(flx::convert("xxx", int_val));
    EXPECT_FALSE(flx::convert(" 1", int_val));
    EXPECT_FALSE(flx::convert("2147483648", int_val));
    EXPECT_FALSE(flx::convert("-2147483649", int_val));
    EXPECT_FALSE(flx::convert("4294967296", uint_val));
    EXPECT_FALSE(flx::convert("-1", uint_val));
    EXPECT_FALSE(flx::convert("32768", short_val));
    EXPECT_FALSE(flx::convert("-32769", short_val));
    EXPECT_FALSE(flx::convert("65536", ushort_val));
    EXPECT_FALSE(flx::convert("-0", ushort_val));
    EXPECT_FALSE(flx::convert("-1", ushort_val));
    EXPECT_FALSE(flx::convert("1.3", int_val));
    EXPECT_FALSE(flx::convert("1e8", int_val));
    EXPECT_FALSE(flx::convert("+12345", int_val));
    EXPECT_FALSE(flx::convert("123456789A", int_val));
    EXPECT_FALSE(flx::convert("ABCDEFG", int_val, 16));
    EXPECT_FALSE(flx::convert("012", int_val, 2));
    EXPECT_FALSE(flx::convert("0xABCDEF", int_val, 16));
    EXPECT_FALSE(flx::convert("0XABCDEF", int_val, 16));
    EXPECT_FALSE(flx::convert("0b01", int_val, 2));
    EXPECT_FALSE(flx::convert("0B01", int_val, 2));
}

TEST(test_free, fct_countSetBits)
{
    unsigned short ushort_val;
    unsigned char uch_val;

    EXPECT_EQ(flx::countSetBits(0U), 0U);
    EXPECT_EQ(flx::countSetBits(1U), 1U);
    EXPECT_EQ(flx::countSetBits(2U), 1U);
    EXPECT_EQ(flx::countSetBits(3U), 2U);
    EXPECT_EQ(flx::countSetBits(0xFFFFU), 16U);
    EXPECT_EQ(flx::countSetBits(0xFFFFFFFFU), 32U);
    EXPECT_EQ(flx::countSetBits(0xFFFFFFFFFFFFFFFFULL), 64U);
    EXPECT_EQ(flx::countSetBits(0xFFFFFFFFFFFFFFFEULL), 63U);
    EXPECT_EQ(flx::countSetBits(0x7FFFFFFFFFFFFFFFULL), 63U);
    EXPECT_EQ(flx::countSetBits(0xF0F0F0F0F0F0F0F0ULL), 32U);
    EXPECT_EQ(flx::countSetBits(0x0F0F0F0F0F0F0F0FULL), 32U);
    EXPECT_EQ(flx::countSetBits(0xAAAAAAAAAAAAAAAAULL), 32U);
    EXPECT_EQ(flx::countSetBits(0x1111111111111111ULL), 16U);
    EXPECT_EQ(flx::countSetBits(0x8000000000000000ULL), 1U);
    EXPECT_EQ(flx::countSetBits(0), 0U);
    EXPECT_EQ(flx::countSetBits(1), 1U);
    EXPECT_EQ(flx::countSetBits(2), 1U);
    EXPECT_EQ(flx::countSetBits(3), 2U);
    EXPECT_EQ(flx::countSetBits(0xFFFF), 16U);
    EXPECT_EQ(flx::countSetBits(0xFFFFFFFF), 32U);
    EXPECT_EQ(flx::countSetBits(0xFFFFFFFFFFFFFFFFLL), 64U);
    EXPECT_EQ(flx::countSetBits(0xFFFFFFFFFFFFFFFELL), 63U);
    EXPECT_EQ(flx::countSetBits(0x7FFFFFFFFFFFFFFFLL), 63U);
    EXPECT_EQ(flx::countSetBits(0xF0F0F0F0F0F0F0F0LL), 32U);
    EXPECT_EQ(flx::countSetBits(0x0F0F0F0F0F0F0F0FLL), 32U);
    EXPECT_EQ(flx::countSetBits(0xAAAAAAAAAAAAAAAALL), 32U);
    EXPECT_EQ(flx::countSetBits(0x1111111111111111LL), 16U);
    EXPECT_EQ(flx::countSetBits(0x8000000000000000LL), 1U);
    ushort_val = 0U;
    EXPECT_EQ(flx::countSetBits(ushort_val), 0U);
    ushort_val = 1U;
    EXPECT_EQ(flx::countSetBits(ushort_val), 1U);
    ushort_val = 0x8000U;
    EXPECT_EQ(flx::countSetBits(ushort_val), 1U);
    ushort_val = 0xFFFFU;
    EXPECT_EQ(flx::countSetBits(ushort_val), 16U);
    ushort_val = 0xFFFEU;
    EXPECT_EQ(flx::countSetBits(ushort_val), 15U);
    ushort_val = 0x7FFFU;
    EXPECT_EQ(flx::countSetBits(ushort_val), 15U);
    ushort_val = 0xAAAAU;
    EXPECT_EQ(flx::countSetBits(ushort_val), 8U);
    ushort_val = 0xF0F0U;
    EXPECT_EQ(flx::countSetBits(ushort_val), 8U);
    ushort_val = 0x0F0FU;
    EXPECT_EQ(flx::countSetBits(ushort_val), 8U);
    ushort_val = 0x1111U;
    EXPECT_EQ(flx::countSetBits(ushort_val), 4U);
    uch_val = 0U;
    EXPECT_EQ(flx::countSetBits(uch_val), 0U);
    uch_val = 1U;
    EXPECT_EQ(flx::countSetBits(uch_val), 1U);
    uch_val = 0x80U;
    EXPECT_EQ(flx::countSetBits(uch_val), 1U);
    uch_val = 0xFFU;
    EXPECT_EQ(flx::countSetBits(uch_val), 8U);
    uch_val = 0xFEU;
    EXPECT_EQ(flx::countSetBits(uch_val), 7U);
    uch_val = 0x7FU;
    EXPECT_EQ(flx::countSetBits(uch_val), 7U);
    uch_val = 0xAAU;
    EXPECT_EQ(flx::countSetBits(uch_val), 4U);
    uch_val = 0xF0U;
    EXPECT_EQ(flx::countSetBits(uch_val), 4U);
    uch_val = 0x0FU;
    EXPECT_EQ(flx::countSetBits(uch_val), 4U);
    uch_val = 0x11U;
    EXPECT_EQ(flx::countSetBits(uch_val), 2U);
    // There is an issue with g++ 11.3.0, g++ 12.2.0: endless loop.
    // compiler option: -O2, with -O0 or -O1 it works.
    // Issue fixed on x86-64 with g++ >= 13.1
    // Still issues on arm64 with g++ 15.1
    // Not reproducible on:
    // clang++ 14.0.6, clang++ 16.0.6, clang++ 19.1.4, clang++ 21.0.0
    //EXPECT_EQ(flx::countSetBits(static_cast<int>(0xFFFFFFFFFLL)), 32);
    EXPECT_EQ(flx::countSetBits(static_cast<short>(0xFFFFF)), 16U);
    EXPECT_EQ(flx::countSetBits(static_cast<char>(0xFFF)), 8U);
    EXPECT_EQ(flx::countSetBits(static_cast<unsigned int>(0xFFFFFFFFFULL)),
        32U);
    EXPECT_EQ(flx::countSetBits(static_cast<unsigned short>(0xFFFFFU)), 16U);
    EXPECT_EQ(flx::countSetBits(static_cast<unsigned char>(0xFFFU)), 8U);
}

TEST(test_free, fct_convert_to_char)
{
    unsigned int uint_val;
    unsigned short ushort_val;
    long long unsigned int ulong_val;
    unsigned char uch_val;
    std::string result;

    // Positive tests.
    EXPECT_TRUE(flx::convert(0U, result, 10));
    EXPECT_EQ(result, "0");
    EXPECT_TRUE(flx::convert(32767U, result, 10));
    EXPECT_EQ(result, "32767");
    EXPECT_TRUE(flx::convert(65535, result, 10));
    EXPECT_EQ(result, "65535");
    uint_val = std::numeric_limits<decltype(uint_val)>::max();
    EXPECT_TRUE(flx::convert(uint_val, result, 10));
    EXPECT_EQ(result, "4294967295");
    ushort_val = 0U;
    EXPECT_TRUE(flx::convert(ushort_val, result, 10));
    EXPECT_EQ(result, "0");
    ushort_val = 32767U;
    EXPECT_TRUE(flx::convert(ushort_val, result, 10));
    EXPECT_EQ(result, "32767");
    ushort_val = std::numeric_limits<decltype(ushort_val)>::max();
    EXPECT_TRUE(flx::convert(ushort_val, result, 10));
    EXPECT_EQ(result, "65535");
    EXPECT_TRUE(flx::convert(0x0U, result, 16));
    EXPECT_EQ(result, "0");
    EXPECT_TRUE(flx::convert(0x7FFFU, result, 16));
    EXPECT_EQ(result, "7FFF");
    EXPECT_TRUE(flx::convert(0xAAAA, result, 16));
    EXPECT_EQ(result, "AAAA");
    EXPECT_TRUE(flx::convert(0x5555, result, 16));
    EXPECT_EQ(result, "5555");
    EXPECT_TRUE(flx::convert(0xFFFFU, result, 16));
    EXPECT_EQ(result, "FFFF");
    uint_val = std::numeric_limits<decltype(uint_val)>::max();
    EXPECT_TRUE(flx::convert(uint_val, result, 16));
    EXPECT_EQ(result, "FFFFFFFF");
    ushort_val = 0U;
    EXPECT_TRUE(flx::convert(ushort_val, result, 16));
    EXPECT_EQ(result, "0");
    ushort_val = 0x7FFFU;
    EXPECT_TRUE(flx::convert(ushort_val, result, 16));
    EXPECT_EQ(result, "7FFF");
    ushort_val = 0xAAAA;
    EXPECT_TRUE(flx::convert(ushort_val, result, 16));
    EXPECT_EQ(result, "AAAA");
    ushort_val = 0x5555;
    EXPECT_TRUE(flx::convert(ushort_val, result, 16));
    EXPECT_EQ(result, "5555");
    ushort_val = std::numeric_limits<decltype(ushort_val)>::max();
    EXPECT_TRUE(flx::convert(ushort_val, result, 16));
    EXPECT_EQ(result, "FFFF");
    ulong_val = 0U;
    EXPECT_TRUE(flx::convert(ulong_val, result, 16));
    EXPECT_EQ(result, "0");
    ulong_val = 0x7FFFFFFFFFFFFFFFU;
    EXPECT_TRUE(flx::convert(ulong_val, result, 16));
    EXPECT_EQ(result, "7FFFFFFFFFFFFFFF");
    ulong_val = 0xAAAAAAAAAAAAAAAA;
    EXPECT_TRUE(flx::convert(ulong_val, result, 16));
    EXPECT_EQ(result, "AAAAAAAAAAAAAAAA");
    ulong_val = 0x5555555555555555;
    EXPECT_TRUE(flx::convert(ulong_val, result, 16));
    EXPECT_EQ(result, "5555555555555555");
    ulong_val = std::numeric_limits<decltype(ulong_val)>::max();
    EXPECT_TRUE(flx::convert(ulong_val, result, 16));
    EXPECT_EQ(result, "FFFFFFFFFFFFFFFF");
    ulong_val = std::numeric_limits<decltype(ulong_val)>::max();
    EXPECT_TRUE(flx::convert(ulong_val, result, 10));
    EXPECT_EQ(result, "18446744073709551615");
    ulong_val = std::numeric_limits<decltype(ulong_val)>::max();
    EXPECT_TRUE(flx::convert(ulong_val, result, 2));
    std::string expected =
        "1111111111111111111111111111111111111111111111111111111111111111";
    EXPECT_EQ(result, expected);
    uch_val = 0x00;
    EXPECT_TRUE(flx::convert(uch_val, result, 16, 2));
    EXPECT_EQ(result, "00");
    uch_val = 0x01;
    EXPECT_TRUE(flx::convert(uch_val, result, 16, 2));
    EXPECT_EQ(result, "01");
    uch_val = 0x7F;
    EXPECT_TRUE(flx::convert(uch_val, result, 16, 2));
    EXPECT_EQ(result, "7F");
    uch_val = 0xFF;
    EXPECT_TRUE(flx::convert(uch_val, result, 16, 2));
    EXPECT_EQ(result, "FF");
    EXPECT_TRUE(flx::convert(0U, result, 16, 4));
    EXPECT_EQ(result, "0000");
    EXPECT_TRUE(flx::convert(1U, result, 16, 4));
    EXPECT_EQ(result, "0001");
    EXPECT_TRUE(flx::convert(0x10U, result, 16, 4));
    EXPECT_EQ(result, "0010");
    EXPECT_TRUE(flx::convert(0xFFU, result, 16, 3));
    EXPECT_EQ(result, "0FF");
    EXPECT_TRUE(flx::convert(0xFFU, result, 16, 2));
    EXPECT_EQ(result, "FF");
    EXPECT_TRUE(flx::convert(0xFFFFU, result, 16, 4, false));
    EXPECT_EQ(result, "ffff");
    // Negative tests.
    EXPECT_FALSE(flx::convert(0, result, 0));
    EXPECT_FALSE(flx::convert(0, result, 1));
    EXPECT_FALSE(flx::convert(0, result, 37));
    EXPECT_FALSE(flx::convert(0, result, 10, -1));
    EXPECT_FALSE(flx::convert(0, result, 10, 65));
}
