/*
    test_misc1.cpp


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
#include "misc1.h"
#include <filesystem>


namespace fs = std::filesystem;

TEST(test_misc1, fct_strlower)
{
    std::string str("abC012");
    flx::strlower(str);
    EXPECT_EQ(str, "abc012");
    str = "ABC012";
    flx::strlower(str);
    EXPECT_EQ(str, "abc012");
}

TEST(test_misc1, fct_strupper)
{
    std::string str("abC012");
    flx::strupper(str);
    EXPECT_EQ(str, "ABC012");
    str = "ABC012";
    flx::strupper(str);
    EXPECT_EQ(str, "ABC012");
}

TEST(test_misc1, fct_tolower)
{
    std::string str("abC012");
    EXPECT_EQ(flx::tolower(str), "abc012");
    str = "aBC012";
    EXPECT_EQ(flx::tolower(str), "abc012");
    EXPECT_EQ(flx::tolower("ABCDe"), "abcde");
    EXPECT_EQ(flx::tolower("XYz"), "xyz");
    str = "MNOpqr123";
    EXPECT_EQ(flx::tolower(std::move(str)), "mnopqr123");
}

TEST(test_misc1, fct_toupper)
{
    std::string str("abC012");
    EXPECT_EQ(flx::toupper(str), "ABC012");
    str = "aBC012";
    EXPECT_EQ(flx::toupper(str), "ABC012");
    EXPECT_EQ(flx::toupper("ABCDe"), "ABCDE");
    EXPECT_EQ(flx::toupper("XYz"), "XYZ");
    str = "MNOpqr123";
    EXPECT_EQ(flx::toupper(std::move(str)), "MNOPQR123");
}

TEST(test_misc1, fct_getstr)
{
    // Array NUL terminated.
    /* This test intentionally uses a c-style array */
    /* NOLINTBEGIN(modernize-avoid-c-arrays) */
    char arr1[]{"abcd"};
    EXPECT_EQ(sizeof(arr1), 5U);
    auto str1 = flx::getstr<>(arr1);
    EXPECT_EQ(str1, "abcd");
    EXPECT_EQ(str1.size(), 4U);
    EXPECT_EQ(str1.c_str()[str1.size()], '\0');

    // Array not NUL terminated.
    char arr2[]{'a', 'b', 'c', 'd', 'e'};
    EXPECT_EQ(sizeof(arr2), 5U);
    auto str2 = flx::getstr<>(arr2);
    EXPECT_EQ(str2, "abcde");
    EXPECT_EQ(str2.size(), 5U);
    EXPECT_EQ(str2.c_str()[str2.size()], '\0');

    // Array NUL terminated early.
    char arr3[]{"abc\0\0\0\0"};
    EXPECT_EQ(sizeof(arr3), 8U);
    auto str3 = flx::getstr<>(arr3);
    EXPECT_EQ(str3, "abc");
    EXPECT_EQ(str3.size(), 3U);
    EXPECT_EQ(str3.c_str()[str3.size()], '\0');

    // Array with "empty string".
    char arr4[]{""};
    EXPECT_EQ(sizeof(arr4), 1U);
    auto str4 = flx::getstr<>(arr4);
    EXPECT_EQ(str4, "");
    EXPECT_EQ(str4.size(), 0U);
    EXPECT_EQ(str4.c_str()[str4.size()], '\0');

    // Tow consecutive arrays in a struct
    struct
    {
        char arr11[8]{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
        char arr12[8]{'1', '2', '3', '4', '5', '6', '7', '8'};
    } s;
    EXPECT_EQ(sizeof(s), 16U);
    EXPECT_EQ(sizeof(s.arr11), 8U);
    EXPECT_EQ(sizeof(s.arr12), 8U);
    auto str11 = flx::getstr<>(s.arr11);
    EXPECT_EQ(str11, "abcdefgh");
    EXPECT_EQ(str11.size(), 8U);
    auto str12 = flx::getstr<>(s.arr12);
    EXPECT_EQ(str12, "12345678");
    EXPECT_EQ(str12.size(), 8U);
    // NOLINTEND(modernize-avoid-c-arrays)
}

TEST(test_misc1, fct_binstr)
{
    auto result = flx::binstr(0x55);
    EXPECT_EQ(result, "01010101");
    result = flx::binstr(0x00);
    EXPECT_EQ(result, "00000000");
    result = flx::binstr(0x01);
    EXPECT_EQ(result, "00000001");
    result = flx::binstr(0xFF);
    EXPECT_EQ(result, "11111111");
}

TEST(test_misc1, fct_hexstr_byte)
{
    auto result = flx::hexstr(static_cast<Byte>(0x55));
    EXPECT_EQ(result, "55");
    result = flx::hexstr(static_cast<Byte>(0x00));
    EXPECT_EQ(result, "00");
    result = flx::hexstr(static_cast<Byte>(0x01));
    EXPECT_EQ(result, "01");
    result = flx::hexstr(static_cast<Byte>(0xFF));
    EXPECT_EQ(result, "FF");
}

TEST(test_misc1, fct_hexstr_word)
{
    auto result = flx::hexstr(static_cast<Word>(0x55AA));
    EXPECT_EQ(result, "55AA");
    result = flx::hexstr(static_cast<Word>(0x0000));
    EXPECT_EQ(result, "0000");
    result = flx::hexstr(static_cast<Word>(0x0001));
    EXPECT_EQ(result, "0001");
    result = flx::hexstr(static_cast<Word>(0xFFFF));
    EXPECT_EQ(result, "FFFF");
}

TEST(test_misc1, fct_ascchr)
{
    auto result = flx::ascchr('a');
    EXPECT_EQ(result, "a");
    result = flx::ascchr('3');
    EXPECT_EQ(result, "3");
    result = flx::ascchr('~');
    EXPECT_EQ(result, "~");
    result = flx::ascchr('\x7f');
    EXPECT_EQ(result, "_");
    result = flx::ascchr(' ');
    EXPECT_EQ(result, " ");
    result = flx::ascchr('\x1f');
    EXPECT_EQ(result, "_");
    result = flx::ascchr('\xa1');
    EXPECT_EQ(result, "_");
    result = flx::ascchr('a', '_');
    EXPECT_EQ(result, "a");
    result = flx::ascchr('3', '_');
    EXPECT_EQ(result, "3");
    result = flx::ascchr('\xa1', '_');
    EXPECT_EQ(result, "_");
}

TEST(test_misc1, fct_hexval)
{
    bool isValid;
    auto result = flx::hexval('0', isValid);
    EXPECT_TRUE(isValid);
    EXPECT_EQ(result, 0);
    result = flx::hexval('9', isValid);
    EXPECT_TRUE(isValid);
    EXPECT_EQ(result, 9);
    result = flx::hexval('a', isValid);
    EXPECT_TRUE(isValid);
    EXPECT_EQ(result, 10);
    result = flx::hexval('A', isValid);
    EXPECT_TRUE(isValid);
    EXPECT_EQ(result, 10);
    result = flx::hexval('f', isValid);
    EXPECT_TRUE(isValid);
    EXPECT_EQ(result, 15);
    result = flx::hexval('F', isValid);
    EXPECT_TRUE(isValid);
    EXPECT_EQ(result, 15);
    result = flx::hexval('\0', isValid);
    EXPECT_FALSE(isValid);
    result = flx::hexval(' ', isValid);
    EXPECT_FALSE(isValid);
    result = flx::hexval('/', isValid);
    EXPECT_FALSE(isValid);
    result = flx::hexval(':', isValid);
    EXPECT_FALSE(isValid);
    result = flx::hexval('@', isValid);
    EXPECT_FALSE(isValid);
    result = flx::hexval('G', isValid);
    EXPECT_FALSE(isValid);
    result = flx::hexval('~', isValid);
    EXPECT_FALSE(isValid);
    result = flx::hexval('\xAA', isValid);
    EXPECT_FALSE(isValid);
}

TEST(test_misc1, fct_matches)
{
    auto result = flx::matches("", "", false);
    EXPECT_TRUE(result);
    result = flx::matches("abcdef", "abcdef", false);
    EXPECT_TRUE(result);
    result = flx::matches("abcdef", "abcde", false);
    EXPECT_FALSE(result);
    result = flx::matches("abcdef", "AbCdEf", false);
    EXPECT_FALSE(result);
    result = flx::matches("abcdef", "AbCdEf", true);
    EXPECT_TRUE(result);
    result = flx::matches("abcdef", "ABCDEF", true);
    EXPECT_TRUE(result);
    result = flx::matches("abcdef", "ABCDE", true);
    EXPECT_FALSE(result);
    result = flx::matches("", "*.*", false);
    EXPECT_FALSE(result);
    result = flx::matches("xx", "*?", false);
    EXPECT_TRUE(result);
    result = flx::matches("xx", "*??", false);
    EXPECT_TRUE(result);
    result = flx::matches("xx", "*???", false);
    EXPECT_FALSE(result);
    result = flx::matches("abcdef", "", false);
    EXPECT_FALSE(result);
    result = flx::matches("abcdef", "abc*", false);
    EXPECT_TRUE(result);
    result = flx::matches("abcdef", "ABC*", true);
    EXPECT_TRUE(result);
    result = flx::matches("file.ext", "*.*", false);
    EXPECT_TRUE(result);
    result = flx::matches("file.ext", "*?", false);
    EXPECT_TRUE(result);
    result = flx::matches("file.ext", "?*", false);
    EXPECT_TRUE(result);
    result = flx::matches("file.ext", "f*.?xt", false);
    EXPECT_TRUE(result);
    result = flx::matches("file.ext", "f*.?xtx", false);
    EXPECT_FALSE(result);
    result = flx::matches("file.a", "f*.??", false);
    EXPECT_FALSE(result);
    result = flx::matches("file.a", "f*.?*", false);
    EXPECT_TRUE(result);
    result = flx::matches("file.a", "f*.*?", false);
    EXPECT_TRUE(result);
    result = flx::matches("file.ext", "F*.?xt", true);
    EXPECT_TRUE(result);
    result = flx::matches("file.a", "f*L*.a", false);
    EXPECT_FALSE(result);
    result = flx::matches("file.a", "f*l*.a", false);
    EXPECT_TRUE(result);
    result = flx::matches("file.a", "f*L*.a", true);
    EXPECT_TRUE(result);
    result = flx::matches("file.a", "f*l*.a", true);
    EXPECT_TRUE(result);
    result = flx::matches("file.ext", "?le.ext*", true);
    EXPECT_FALSE(result);
    result = flx::matches("file.ext", "???le.ext*", true);
    EXPECT_FALSE(result);
    result = flx::matches("file.ext", "file.ext*", true);
    EXPECT_TRUE(result);
    result = flx::matches("mississippi", "*sip*", false);
    EXPECT_TRUE(result);
    result = flx::matches("eee.ext", "*eee.ext", false);
    EXPECT_TRUE(result);
// The following test would need a more sophisticated algorithm.
// Such a pattern is currently not used.
//  result = matches("eee.ext", "*ee.ext", false);
//  EXPECT_TRUE(result);
    result = flx::matches("eee.ext", "*e.ext", false);
    EXPECT_TRUE(result);
    result = flx::matches("daadabdmada", "da*da*da*", false);
    EXPECT_TRUE(result);
}

TEST(test_misc1, fct_multimatches)
{
    auto result = flx::multimatches("", "", ';', false);
    EXPECT_TRUE(result);
    result = flx::multimatches("abcdef", "abcdef", ';', false);
    EXPECT_TRUE(result);
    result = flx::multimatches("abcdef", "abcde", ';', false);
    EXPECT_FALSE(result);
    result = flx::multimatches("abcdef", "AbCdEf", ';', false);
    EXPECT_FALSE(result);
    result = flx::multimatches("abcdef", "AbCdEf", ';', true);
    EXPECT_TRUE(result);
    result = flx::multimatches("abcdef", "ABCDEF", ';', true);
    EXPECT_TRUE(result);
    result = flx::multimatches("abcdef", "ABCDE", ';', true);
    EXPECT_FALSE(result);
    result = flx::multimatches("abcdef", "xyz;abcdef", ';', false);
    EXPECT_TRUE(result);
    result = flx::multimatches("abcdef", "xyz;abcde", ';', false);
    EXPECT_FALSE(result);
    result = flx::multimatches("abcdef", "xyz;AbCdEf", ';', false);
    EXPECT_FALSE(result);
    result = flx::multimatches("abcdef", "xyz;AbCdEf", ';', true);
    EXPECT_TRUE(result);
    result = flx::multimatches("abcdef", "xyz;ABCDEF", ';', true);
    EXPECT_TRUE(result);
    result = flx::multimatches("abcdef", "xyz;ABCDE", ';', true);
    EXPECT_FALSE(result);
    result = flx::multimatches("", "abc;xyz;kjl", ';', false);
    EXPECT_FALSE(result);
    result = flx::multimatches("", ";;*.*;", ';', false);
    EXPECT_TRUE(result);
    result = flx::multimatches("ab.c", "c;d;e;f;g;ab.*", ';', false);
    EXPECT_TRUE(result);
    result = flx::multimatches("xx", "abc;*?", ';', false);
    EXPECT_TRUE(result);
    result = flx::multimatches("daadabdmada", "x*;*DA*;da*da*da*", ';', false);
    EXPECT_TRUE(result);
}

#ifdef _WIN32
TEST(test_misc1, fct_getExecutablePath)
{
    const auto path = flx::getExecutablePath();
    const auto status = fs::status(path);
    bool result = fs::exists(path) && fs::is_directory(status);

    EXPECT_TRUE(result);
}
#endif

TEST(test_misc1, fct_getHomeDirectory)
{
    const auto path = flx::getHomeDirectory();
    const auto status = fs::status(path);
    bool result = fs::exists(path) && fs::is_directory(status);

    EXPECT_TRUE(result);
}

TEST(test_misc1, fct_getHostName)
{
    auto result = flx::getHostName();
    EXPECT_FALSE(result.empty());
}

TEST(test_misc1, macro_EXTEND8)
{
    Byte src1{0x55};
    auto tgt1 = EXTEND8(src1);
    EXPECT_EQ(tgt1, 0x0055);
    EXPECT_EQ(sizeof(tgt1), 2U);
    Byte src2{0xAA};
    auto tgt2 = EXTEND8(src2);
    EXPECT_EQ(tgt2, 0xFFAA);
    EXPECT_EQ(sizeof(tgt2), 2U);
}

TEST(test_misc1, fct_split)
{
    const auto strings1 = flx::split("abc;xyz;klm", ';', false);
    ASSERT_EQ(strings1.size(), 3U);
    EXPECT_EQ(strings1[0], "abc");
    EXPECT_EQ(strings1[1], "xyz");
    EXPECT_EQ(strings1[2], "klm");
    const auto strings2 = flx::split("abc;xyz;", ';', false);
    ASSERT_EQ(strings2.size(), 2U);
    EXPECT_EQ(strings2[0], "abc");
    EXPECT_EQ(strings2[1], "xyz");
    const auto strings3 = flx::split("**klm**", '*', false);
    ASSERT_EQ(strings3.size(), 1U);
    EXPECT_EQ(strings3[0], "klm");
    const auto strings4 = flx::split("", ';', false);
    ASSERT_TRUE(strings4.empty());
    const auto strings5 = flx::split("|||", '|', false);
    ASSERT_TRUE(strings5.empty());
    const auto strings6 = flx::split("abc;xyz;", ';', true);
    ASSERT_EQ(strings6.size(), 3U);
    EXPECT_EQ(strings6[0], "abc");
    EXPECT_EQ(strings6[1], "xyz");
    const auto strings7 = flx::split(";xyz;klm", ';', true);
    ASSERT_EQ(strings7.size(), 3U);
    EXPECT_TRUE(strings7[0].empty());
    EXPECT_EQ(strings7[1], "xyz");
    EXPECT_EQ(strings7[2], "klm");
    const auto strings8 = flx::split("**klm**", '*', true);
    ASSERT_EQ(strings8.size(), 5U);
    EXPECT_EQ(strings8[2], "klm");
    const auto strings9 = flx::split("", ';', true);
    ASSERT_EQ(strings9.size(), 1U);
    EXPECT_TRUE(strings9[0].empty());
    const auto strings10 = flx::split("|||", '|', true);
    ASSERT_EQ(strings10.size(), 4U);
    const auto strings11 = flx::split("abc;xyz;klm", ';', false, 2U);
    ASSERT_EQ(strings11.size(), 3U);
    EXPECT_EQ(strings11[0], "abc");
    EXPECT_EQ(strings11[1], "xyz");
    EXPECT_EQ(strings11[2], ";klm");
    const auto strings12 = flx::split("abc;xyz;;klm", ';', true, 2U);
    ASSERT_EQ(strings12.size(), 3U);
    EXPECT_EQ(strings12[0], "abc");
    EXPECT_EQ(strings12[1], "xyz");
    EXPECT_EQ(strings12[2], ";;klm");
    const auto strings13 = flx::split("abc;;xyz;klm", ';', true, 2U);
    ASSERT_EQ(strings13.size(), 3U);
    EXPECT_EQ(strings13[0], "abc");
    EXPECT_EQ(strings13[1], "");
    EXPECT_EQ(strings13[2], ";xyz;klm");
    const auto strings14 = flx::split("abc;;xyz;klm", ';', false, 2U);
    ASSERT_EQ(strings14.size(), 3U);
    EXPECT_EQ(strings14[0], "abc");
    EXPECT_EQ(strings14[1], "xyz");
    EXPECT_EQ(strings14[2], ";klm");
    const auto strings15 = flx::split("abc;xyz;klm;nop", ';', false, 4U);
    ASSERT_EQ(strings15.size(), 4U);
    EXPECT_EQ(strings15[0], "abc");
    EXPECT_EQ(strings15[1], "xyz");
    EXPECT_EQ(strings15[2], "klm");
    EXPECT_EQ(strings15[3], "nop");
    const auto strings16 = flx::split("**klm**nop**", '*', false, 2);
    ASSERT_EQ(strings16.size(), 3U);
    EXPECT_EQ(strings16[0], "klm");
    EXPECT_EQ(strings16[1], "nop");
    EXPECT_EQ(strings16[2], "**");
}

TEST(test_misc1, fct_isFlexFilename)
{
    auto result = flx::isFlexFilename("");
    EXPECT_FALSE(result);
    result = flx::isFlexFilename("aaa");
    EXPECT_FALSE(result);
    result = flx::isFlexFilename("a1234567");
    EXPECT_FALSE(result);
    result = flx::isFlexFilename(".cmd");
    EXPECT_FALSE(result);
    result = flx::isFlexFilename(".");
    EXPECT_FALSE(result);
    result = flx::isFlexFilename("..");
    EXPECT_FALSE(result);
    result = flx::isFlexFilename("1.cmd");
    EXPECT_FALSE(result);
    result = flx::isFlexFilename("1..cmd");
    EXPECT_FALSE(result);
    result = flx::isFlexFilename("a12345678.cmd");
    EXPECT_FALSE(result);
    result = flx::isFlexFilename("a1234567.1234");
    EXPECT_FALSE(result);
    result = flx::isFlexFilename("a.s");
    EXPECT_TRUE(result);
    result = flx::isFlexFilename("a1234567.a12");
    EXPECT_TRUE(result);
    result = flx::isFlexFilename("zzzzzzzz.zzz");
    EXPECT_TRUE(result);
    result = flx::isFlexFilename("a_______.a__");
    EXPECT_TRUE(result);
    result = flx::isFlexFilename("z-------.z--");
    EXPECT_TRUE(result);
    result = flx::isFlexFilename("a.a");
    EXPECT_TRUE(result);
#ifdef _WIN32
    result = flx::isFlexFilename("A.s");
    EXPECT_TRUE(result);
    result = flx::isFlexFilename("a.S");
    EXPECT_TRUE(result);
    result = flx::isFlexFilename("abcdefgH.abc");
    EXPECT_TRUE(result);
    result = flx::isFlexFilename("abcde123.abC");
    EXPECT_TRUE(result);
    result = flx::isFlexFilename("AbcDefG.AbC");
    EXPECT_TRUE(result);
#else
    result = flx::isFlexFilename("A.s");
    EXPECT_FALSE(result);
    result = flx::isFlexFilename("a.S");
    EXPECT_FALSE(result);
    result = flx::isFlexFilename("abcdefgH.abc");
    EXPECT_FALSE(result);
    result = flx::isFlexFilename("abcde123.abC");
    EXPECT_FALSE(result);
    result = flx::isFlexFilename("AbcDefG.AbC");
    EXPECT_FALSE(result);
#endif
}

TEST(test_misc1, fct_ltrim)
{
    std::string str1c("\n\t\r\f\v   abc   \n\t\r\f\v");
    EXPECT_EQ(flx::ltrim(str1c), "abc   \n\t\r\f\v");
    std::string str1m("\n\t\r\f\v   abc   \n\t\r\f\v");
    EXPECT_EQ(flx::ltrim(std::move(str1m)), "abc   \n\t\r\f\v");
    std::string str2c("   a b c   ");
    EXPECT_EQ(flx::ltrim(str2c), "a b c   ");
    std::string str2m("   a b c   ");
    EXPECT_EQ(flx::ltrim(std::move(str2m)), "a b c   ");
    std::string str3c(" \n\t\r\f\v a\n\t\r\f\v b\n\t\r\f\v c\n\t\r\f\v ");
    EXPECT_EQ(flx::ltrim(str3c), "a\n\t\r\f\v b\n\t\r\f\v c\n\t\r\f\v ");
    std::string str3m(" \n\t\r\f\v a\n\t\r\f\v b\n\t\r\f\v c\n\t\r\f\v ");
    EXPECT_EQ(flx::ltrim(std::move(str3m)), "a\n\t\r\f\v b\n\t\r\f\v c\n\t\r\f\v ");
}

TEST(test_misc1, fct_rtrim)
{
    std::string str1c("\n\t\r\f\v   abc   \n\t\r\f\v");
    EXPECT_EQ(flx::rtrim(str1c), "\n\t\r\f\v   abc");
    std::string str1m("\n\t\r\f\v   abc   \n\t\r\f\v");
    EXPECT_EQ(flx::rtrim(std::move(str1m)), "\n\t\r\f\v   abc");
    std::string str2c("   a b c   ");
    EXPECT_EQ(flx::rtrim(str2c), "   a b c");
    std::string str2m("   a b c   ");
    EXPECT_EQ(flx::rtrim(std::move(str2m)), "   a b c");
    std::string str3c(" \n\t\r\f\v a\n\t\r\f\v b\n\t\r\f\v c\n\t\r\f\v ");
    EXPECT_EQ(flx::rtrim(str3c), " \n\t\r\f\v a\n\t\r\f\v b\n\t\r\f\v c");
    std::string str3m(" \n\t\r\f\v a\n\t\r\f\v b\n\t\r\f\v c\n\t\r\f\v ");
    EXPECT_EQ(flx::rtrim(std::move(str3m)), " \n\t\r\f\v a\n\t\r\f\v b\n\t\r\f\v c");
}

TEST(test_misc1, fct_trim)
{
    std::string str1c("\n\t\r\f\v   abc   \n\t\r\f\v");
    EXPECT_EQ(flx::trim(str1c), "abc");
    std::string str1m("\n\t\r\f\v   abc   \n\t\r\f\v");
    EXPECT_EQ(flx::trim(std::move(str1m)), "abc");
    std::string str2c("   a b c   ");
    EXPECT_EQ(flx::trim(str2c), "a b c");
    std::string str2m("   a b c   ");
    EXPECT_EQ(flx::trim(std::move(str2m)), "a b c");
    std::string str3c(" \n\t\r\f\v a\n\t\r\f\v b\n\t\r\f\v c\n\t\r\f\v ");
    EXPECT_EQ(flx::trim(str3c), "a\n\t\r\f\v b\n\t\r\f\v c");
    std::string str3m(" \n\t\r\f\v a\n\t\r\f\v b\n\t\r\f\v c\n\t\r\f\v ");
    EXPECT_EQ(flx::trim(std::move(str3m)), "a\n\t\r\f\v b\n\t\r\f\v c");
}

TEST(test_misc1, fct_updateFilename)
{
    auto result = flx::updateFilename("abc", "stem", ".ext");
    EXPECT_EQ(result, "abc.ext");
    result = flx::updateFilename("abc.tar", "stem", ".ext");
    EXPECT_EQ(result, "abc.ext");
    result = flx::updateFilename("abc.tar.gz", "stem", ".xz");
    EXPECT_EQ(result, "abc.tar.xz");
    result = flx::updateFilename(".", "stem", ".ext");
    EXPECT_EQ(result, "stem.ext");
    result = flx::updateFilename("..", "stem", ".ext");
    EXPECT_EQ(result, "stem.ext");
    result = flx::updateFilename(".x", "stem", ".ext");
    EXPECT_EQ(result, "stem.ext");
    result = flx::updateFilename(".tar.gz", "stem", ".xz");
    EXPECT_EQ(result, "stem.tar.xz");
    result = flx::updateFilename("file.tar.gz", "stem", ".xz");
    EXPECT_EQ(result, "file.tar.xz");
#ifdef _WIN32
    result = flx::updateFilename("C:\\Temp\\sub.dir\\test", "stem", ".ext");
    EXPECT_EQ(result, "C:\\Temp\\sub.dir\\test.ext");
    result = flx::updateFilename("C:\\Temp\\sub.dir\\test.", "stem", ".ext");
    EXPECT_EQ(result, "C:\\Temp\\sub.dir\\test.ext");
    result = flx::updateFilename("C:\\Temp\\sub.dir\\test.tar", "stem", ".ext");
    EXPECT_EQ(result, "C:\\Temp\\sub.dir\\test.ext");
    result = flx::updateFilename("C:\\Temp\\sub.dir\\.", "stem", ".ext");
    EXPECT_EQ(result, "C:\\Temp\\sub.dir\\stem.ext");
    result = flx::updateFilename("C:\\Temp\\sub.dir\\..", "stem", ".ext");
    EXPECT_EQ(result, "C:\\Temp\\sub.dir\\stem.ext");
    result = flx::updateFilename("C:\\Temp\\sub.dir\\.x", "stem", ".ext");
    EXPECT_EQ(result, "C:\\Temp\\sub.dir\\stem.ext");
    result = flx::updateFilename("C:\\Temp\\sub.dir\\.tar.gz", "stem", ".xz");
    EXPECT_EQ(result, "C:\\Temp\\sub.dir\\stem.tar.xz");
    result = flx::updateFilename("C:\\Temp\\", "stem", ".ext");
    EXPECT_EQ(result, "C:\\Temp\\stem.ext");
    result = flx::updateFilename("C:\\Temp\\.", "stem", ".ext");
    EXPECT_EQ(result, "C:\\Temp\\stem.ext");
    result = flx::updateFilename("C:\\Temp\\..", "stem", ".ext");
    EXPECT_EQ(result, "C:\\Temp\\stem.ext");
    result = flx::updateFilename("C:\\Temp\\.x", "stem", ".ext");
    EXPECT_EQ(result, "C:\\Temp\\stem.ext");
    result = flx::updateFilename("C:\\Temp\\.tar.gz", "stem", ".xz");
    EXPECT_EQ(result, "C:\\Temp\\stem.tar.xz");
#else
    result = flx::updateFilename("/tmp/sub.dir/test", "stem", ".ext");
    EXPECT_EQ(result, "/tmp/sub.dir/test.ext");
    result = flx::updateFilename("/tmp/sub.dir/test.", "stem", ".ext");
    EXPECT_EQ(result, "/tmp/sub.dir/test.ext");
    result = flx::updateFilename("/tmp/sub.dir/test.tar", "stem", ".ext");
    EXPECT_EQ(result, "/tmp/sub.dir/test.ext");
    result = flx::updateFilename("/tmp/sub.dir/.", "stem", ".ext");
    EXPECT_EQ(result, "/tmp/sub.dir/stem.ext");
    result = flx::updateFilename("/tmp/sub.dir/..", "stem", ".ext");
    EXPECT_EQ(result, "/tmp/sub.dir/stem.ext");
    result = flx::updateFilename("/tmp/sub.dir/.x", "stem", ".ext");
    EXPECT_EQ(result, "/tmp/sub.dir/stem.ext");
    result = flx::updateFilename("/tmp/sub.dir/.tar.gz", "stem", ".xz");
    EXPECT_EQ(result, "/tmp/sub.dir/stem.tar.xz");
    result = flx::updateFilename("/tmp/", "stem", ".ext");
    EXPECT_EQ(result, "/tmp/stem.ext");
    result = flx::updateFilename("/tmp/.", "stem", ".ext");
    EXPECT_EQ(result, "/tmp/stem.ext");
    result = flx::updateFilename("/tmp/..", "stem", ".ext");
    EXPECT_EQ(result, "/tmp/stem.ext");
    result = flx::updateFilename("/tmp/.x", "stem", ".ext");
    EXPECT_EQ(result, "/tmp/stem.ext");
    result = flx::updateFilename("/tmp/.tar.gz", "stem", ".xz");
    EXPECT_EQ(result, "/tmp/stem.tar.xz");
#endif
}

TEST(test_misc1, fct_BTST)
{
    const Byte val8 = 0x55U;
    const Word val16 = 0xAAAAU;
    const DWord val32 = 0x55555555U;
    const QWord val64 = 0xAAAAAAAAAAAAAAAAU;

    // 8-bit value
    EXPECT_TRUE(BTST<Byte>(val8, 0U));
    EXPECT_FALSE(BTST<Byte>(val8, 1U));
    EXPECT_TRUE(BTST<Byte>(val8, 4U));
    EXPECT_FALSE(BTST<Byte>(val8, 7U));
    // 16-bit value
    EXPECT_FALSE(BTST<Word>(val16, 0U));
    EXPECT_TRUE(BTST<Word>(val16, 1U));
    EXPECT_FALSE(BTST<Word>(val16, 4U));
    EXPECT_TRUE(BTST<Word>(val16, 15U));
    // 32-bit value
    EXPECT_TRUE(BTST<DWord>(val32, 0U));
    EXPECT_FALSE(BTST<DWord>(val32, 1U));
    EXPECT_TRUE(BTST<DWord>(val32, 4U));
    EXPECT_FALSE(BTST<DWord>(val32, 31U));
    // 64-bit value
    EXPECT_FALSE(BTST<QWord>(val64, 0U));
    EXPECT_TRUE(BTST<QWord>(val64, 1U));
    EXPECT_FALSE(BTST<QWord>(val64, 4U));
    EXPECT_TRUE(BTST<QWord>(val64, 63U));
}

TEST(test_misc1, fct_BSET)
{
    Byte val8 = 0U;
    Word val16 = 0U;
    DWord val32 = 0U;
    QWord val64 = 0U;

    // 8-bit value
    BSET<Byte>(val8, 0U);
    EXPECT_EQ(val8, 0x01U);
    BSET<Byte>(val8, 1U);
    EXPECT_EQ(val8, 0x03U);
    BSET<Byte>(val8, 4U);
    EXPECT_EQ(val8, 0x13U);
    BSET<Byte>(val8, 7U);
    EXPECT_EQ(val8, 0x93U);
    BSET<Byte>(val8, 4U);
    EXPECT_EQ(val8, 0x93U);
    // 16-bit value
    BSET<Word>(val16, 0U);
    EXPECT_EQ(val16, 0x01U);
    BSET<Word>(val16, 1U);
    EXPECT_EQ(val16, 0x03U);
    BSET<Word>(val16, 4U);
    EXPECT_EQ(val16, 0x13U);
    BSET<Word>(val16, 15U);
    EXPECT_EQ(val16, 0x8013U);
    BSET<Word>(val16, 4U);
    EXPECT_EQ(val16, 0x8013U);
    // 32-bit value
    BSET<DWord>(val32, 0U);
    EXPECT_EQ(val32, 0x01U);
    BSET<DWord>(val32, 1U);
    EXPECT_EQ(val32, 0x03U);
    BSET<DWord>(val32, 4U);
    EXPECT_EQ(val32, 0x13U);
    BSET<DWord>(val32, 31U);
    EXPECT_EQ(val32, 0x80000013U);
    BSET<DWord>(val32, 4U);
    EXPECT_EQ(val32, 0x80000013U);
    // 64-bit value
    BSET<QWord>(val64, 0U);
    EXPECT_EQ(val64, 0x01U);
    BSET<QWord>(val64, 1U);
    EXPECT_EQ(val64, 0x03U);
    BSET<QWord>(val64, 4U);
    EXPECT_EQ(val64, 0x13U);
    BSET<QWord>(val64, 63U);
    EXPECT_EQ(val64, 0x8000000000000013U);
    BSET<QWord>(val64, 4U);
    EXPECT_EQ(val64, 0x8000000000000013U);
}

TEST(test_misc1, fct_BCLR)
{
    Byte val8 = 0xFFU;
    Word val16 = 0xFFFFU;
    DWord val32 = 0xFFFFFFFFU;
    QWord val64 = 0xFFFFFFFFFFFFFFFFU;

    // 8-bit value
    BCLR<Byte>(val8, 0U);
    EXPECT_EQ(val8, 0xFEU);
    BCLR<Byte>(val8, 1U);
    EXPECT_EQ(val8, 0xFCU);
    BCLR<Byte>(val8, 4U);
    EXPECT_EQ(val8, 0xECU);
    BCLR<Byte>(val8, 7U);
    EXPECT_EQ(val8, 0x6CU);
    BCLR<Byte>(val8, 4U);
    EXPECT_EQ(val8, 0x6CU);
    // 16-bit value
    BCLR<Word>(val16, 0U);
    EXPECT_EQ(val16, 0xFFFEU);
    BCLR<Word>(val16, 1U);
    EXPECT_EQ(val16, 0xFFFCU);
    BCLR<Word>(val16, 4U);
    EXPECT_EQ(val16, 0xFFECU);
    BCLR<Word>(val16, 15U);
    EXPECT_EQ(val16, 0x7FECU);
    BCLR<Word>(val16, 4U);
    EXPECT_EQ(val16, 0x7FECU);
    // 32-bit value
    BCLR<DWord>(val32, 0U);
    EXPECT_EQ(val32, 0xFFFFFFFEU);
    BCLR<DWord>(val32, 1U);
    EXPECT_EQ(val32, 0xFFFFFFFCU);
    BCLR<DWord>(val32, 4U);
    EXPECT_EQ(val32, 0xFFFFFFECU);
    BCLR<DWord>(val32, 31U);
    EXPECT_EQ(val32, 0x7FFFFFECU);
    BCLR<DWord>(val32, 4U);
    EXPECT_EQ(val32, 0x7FFFFFECU);
    // 64-bit value
    BCLR<QWord>(val64, 0U);
    EXPECT_EQ(val64, 0xFFFFFFFFFFFFFFFEU);
    BCLR<QWord>(val64, 1U);
    EXPECT_EQ(val64, 0xFFFFFFFFFFFFFFFCU);
    BCLR<QWord>(val64, 4U);
    EXPECT_EQ(val64, 0xFFFFFFFFFFFFFFECU);
    BCLR<QWord>(val64, 63U);
    EXPECT_EQ(val64, 0x7FFFFFFFFFFFFFECU);
    BCLR<QWord>(val64, 4U);
    EXPECT_EQ(val64, 0x7FFFFFFFFFFFFFECU);
}

TEST(test_misc1, fct_EXTEND8)
{
    EXPECT_EQ(EXTEND8(0U), 0U);
    EXPECT_EQ(EXTEND8(0x7FU), 0x7FU);
    EXPECT_EQ(EXTEND8(0x80U), 0xFF80U);
    EXPECT_EQ(EXTEND8(0xFFU), 0xFFFFU);
}

