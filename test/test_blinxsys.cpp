/*
    test_blinxsys.cpp


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
#include "blinxsys.h"
#include <string>


#ifdef __linux__

TEST(test_blinxsys, fct_toString)
{
    BLinuxSysInfo info;

    const auto typeString = BLinuxSysInfo::ToString(BLinuxSysInfoType::LED);
    EXPECT_EQ(typeString, "LED");
}

TEST(test_blinxsys, fct_read)
{
    BLinuxSysInfo info;

    // On linux servers there may be no terminal attached in which case
    // the numlock LED does not exist.
    const auto value1 = info.Read(BLinuxSysInfoType::LED, "numlock");
    EXPECT_TRUE(value1.empty() || value1 == "0" || value1 == "1");
    const auto value2 = info.Read(BLinuxSysInfoType::LED, "capslock");
    EXPECT_TRUE(value2.empty() || value2 == "0" || value1 == "1");
    const auto value3 = info.Read(BLinuxSysInfoType::LED, "notexistent");
    EXPECT_TRUE(value3.empty());
}

#endif

