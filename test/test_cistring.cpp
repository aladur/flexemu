/*
    test_cistring.cpp


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


#include <gtest/gtest.h>
#include "cistring.h"
#include <string>

TEST(test_ci_string, class_ci_string)
{
    ci_string ci_str1("AbCDeF");
    auto result = ci_str1.compare("abcdef");
    EXPECT_EQ(result, 0);
    result = ci_str1.compare("ABCDEF");
    EXPECT_EQ(result, 0);
    ci_string ci_str2("aBcdEf");
    result = ci_str1.compare(ci_str2);
    EXPECT_EQ(result, 0);
    std::string str1("aBcdEf");
    /* False positive. */
    /* NOLINTNEXTLINE(readability-redundant-string-cstr) */
    result = ci_str1.compare(str1.c_str());
    std::string str2("aBcdf");
    /* False positive. */
    /* NOLINTNEXTLINE(readability-redundant-string-cstr) */
    result = ci_str1.compare(str2.c_str());
    EXPECT_NE(result, 0);
    ci_string ci_str3("aBcdEfG");
    result = ci_str1.compare(ci_str3);
    EXPECT_NE(result, 0);
    ci_string ci_str4("0AcdX");
    result = ci_str4.compare("0aCDx");
    EXPECT_EQ(result, 0);
    ci_string ci_str5("abc");
    result = ci_str5.compare("acb");
    EXPECT_NE(result, 0);
    ci_string ci_str6("0123456");
    result = ci_str6.compare("0123456");
    EXPECT_EQ(result, 0);
}

