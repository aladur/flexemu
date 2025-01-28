/*
    test_benv.cpp


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
#include "benv.h"
#include <string>


TEST(test_benv, get_set_int)
{
    int value;

    EXPECT_TRUE(BEnvironment::SetValue("FLEXEMU_TEST_VALUE", 4711));
    EXPECT_TRUE(BEnvironment::SetValue("flexemu_test_value", 3722));
    EXPECT_TRUE(BEnvironment::SetValue("FlexemuTestValue", 5522));
    EXPECT_TRUE(BEnvironment::SetValue("FlexemuNoIntValue", "TEST"));
    EXPECT_FALSE(BEnvironment::SetValue("FLEXEMU_=_NOT_ALLOWED", 0));

    EXPECT_TRUE(BEnvironment::GetValue("FLEXEMU_TEST_VALUE", value));
    EXPECT_EQ(value, 4711);
    EXPECT_TRUE(BEnvironment::GetValue("flexemu_test_value", value));
    EXPECT_EQ(value, 3722);
    EXPECT_TRUE(BEnvironment::GetValue("FlexemuTestValue", value));
    EXPECT_EQ(value, 5522);
    EXPECT_FALSE(BEnvironment::GetValue("FlexemuNoIntValue", value));

    BEnvironment::RemoveKey("FLEXEMU_TEST_VALUE");
    BEnvironment::RemoveKey("flexemu_test_value");
    BEnvironment::RemoveKey("FlexemuTestValue");
    BEnvironment::RemoveKey("FlexemuNoIntValue");

    EXPECT_FALSE(BEnvironment::GetValue("FLEXEMU_TEST_VALUE", value));
    EXPECT_FALSE(BEnvironment::GetValue("flexemu_test_value", value));
    EXPECT_FALSE(BEnvironment::GetValue("FlexemuTestValue", value));
}

TEST(test_benv, get_set_string)
{
    std::string value;

    EXPECT_TRUE(BEnvironment::SetValue("FLEXEMU_TEST_VALUE", "4711"));
    EXPECT_TRUE(BEnvironment::SetValue("flexemu_test_value", "3722"));
    EXPECT_TRUE(BEnvironment::SetValue("FlexemuTestValue", "5522"));
    EXPECT_FALSE(BEnvironment::SetValue("Flexemu=NotAllowed", "equal_char"));

    EXPECT_TRUE(BEnvironment::GetValue("FLEXEMU_TEST_VALUE", value));
    EXPECT_EQ(value, "4711");
    EXPECT_TRUE(BEnvironment::GetValue("flexemu_test_value", value));
    EXPECT_EQ(value, "3722");
    EXPECT_TRUE(BEnvironment::GetValue("FlexemuTestValue", value));
    EXPECT_EQ(value, "5522");

    BEnvironment::RemoveKey("FLEXEMU_TEST_VALUE");
    BEnvironment::RemoveKey("flexemu_test_value");
    BEnvironment::RemoveKey("FlexemuTestValue");

    EXPECT_FALSE(BEnvironment::GetValue("FLEXEMU_TEST_VALUE", value));
    EXPECT_FALSE(BEnvironment::GetValue("flexemu_test_value", value));
    EXPECT_FALSE(BEnvironment::GetValue("FlexemuTestValue", value));
}

