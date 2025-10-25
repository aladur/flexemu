/*
    test_breltime.cpp


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
#include "breltime.h"
#include <thread>
#include <chrono>


TEST(test_breltime, getTimeUsll)
{
    const auto time1 = BRelativeTime::GetTimeUsll();
    std::this_thread::sleep_for(std::chrono::microseconds(10000));
    const auto time2 = BRelativeTime::GetTimeUsll();
    EXPECT_TRUE(time2 - time1 > 9000ULL);
    const auto time3 = BRelativeTime::GetTimeUsll();
    EXPECT_TRUE(time3 - time2 < 10000ULL);
}

