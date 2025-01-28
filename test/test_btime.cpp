/*
    test_btime.cpp


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
#include "btime.h"


TEST(test_btime, default_ctor)
{
    BTime time1;
    EXPECT_EQ(time1.GetHour(), 0);
    EXPECT_EQ(time1.GetMinute(), 0);
    EXPECT_EQ(time1.GetSecond(), 0);
    BTime time2(23, 59, 59);
    EXPECT_EQ(time2.GetHour(), 23);
    EXPECT_EQ(time2.GetMinute(), 59);
    EXPECT_EQ(time2.GetSecond(), 59);
}

TEST(test_btime, copy_ctor)
{
    BTime time_src(14, 12, 33);
    BTime time_tgt(time_src);
    EXPECT_EQ(time_tgt.GetHour(), 14);
    EXPECT_EQ(time_tgt.GetMinute(), 12);
    EXPECT_EQ(time_tgt.GetSecond(), 33);
}

TEST(test_btime, copy_assignment)
{
    BTime time_src(3, 13, 59);
    auto time_tgt = time_src;
    EXPECT_EQ(time_tgt.GetHour(), 3);
    EXPECT_EQ(time_tgt.GetMinute(), 13);
    EXPECT_EQ(time_tgt.GetSecond(), 59);
}

TEST(test_btime, get_set)
{
    BTime time1;
    time1.Set(12, 0, 37);
    EXPECT_EQ(time1.GetHour(), 12);
    EXPECT_EQ(time1.GetMinute(), 0);
    EXPECT_EQ(time1.GetSecond(), 37);
    int hour;
    int minute;
    int second;
    time1.Get(hour, minute, second);
    EXPECT_EQ(hour, 12);
    EXPECT_EQ(minute, 0);
    EXPECT_EQ(second, 37);
    hour = 15;
    minute = 11;
    second = 57;
    time1.Set(hour, minute, second);
    EXPECT_EQ(time1.GetHour(), hour);
    EXPECT_EQ(time1.GetMinute(), minute);
    EXPECT_EQ(time1.GetSecond(), second);
    EXPECT_EQ(time1.ToSeconds(), 54717U);
    BTime time2(14, 26);
    EXPECT_EQ(time2.GetHour(), 14);
    EXPECT_EQ(time2.GetMinute(), 26);
    EXPECT_EQ(time2.GetSecond(), 0);
    time1.Set(time2);
    EXPECT_EQ(time1.GetHour(), 14);
    EXPECT_EQ(time1.GetMinute(), 26);
    EXPECT_EQ(time1.GetSecond(), 0);
    auto time3 = BTime::Now();
    EXPECT_TRUE(time3.GetHour() >= 0 && time3.GetHour() <= 23);
    EXPECT_TRUE(time3.GetMinute() >= 0 && time3.GetMinute() <= 59);
    EXPECT_TRUE(time3.GetSecond() >= 0 && time3.GetSecond() <= 59);
}

TEST(test_btime, get_string)
{
    BTime time(4, 2, 33);
    EXPECT_EQ(time.AsString(), "04:02:33");
    EXPECT_EQ(time.AsString(BTime::Format::HHMM), "04:02");
    time.Set(333, 154, -3);
    EXPECT_EQ(time.AsString(BTime::Format::HHMMSS), "333:154:-3");
}

TEST(test_btime, cmp_fct)
{
    BTime time1(19, 4, 22);
    BTime time2(19, 4, 23);
    EXPECT_TRUE(time2 > time1);
    EXPECT_TRUE(time2 >= time1);
    EXPECT_TRUE(time1 < time2);
    EXPECT_TRUE(time1 <= time2);
    EXPECT_TRUE(time1 != time2);
    EXPECT_FALSE(time2 < time1);
    EXPECT_FALSE(time1 > time2);
    EXPECT_FALSE(time1 == time2);
    time2 = time1;
    EXPECT_TRUE(time1 == time2);
    EXPECT_FALSE(time1 != time2);
}

