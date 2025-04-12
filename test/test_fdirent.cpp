/*
    test_fdirent.cpp


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
#include "fattrib.h"
#include "fdirent.h"


TEST(test_fdirent, default_ctor)
{
    FlexDirEntry de;
    int tracks;
    int sectors;

    EXPECT_EQ(de.GetFileSize(), 0U);
    EXPECT_EQ(de.GetAttributes(), 0U);
    EXPECT_EQ(de.GetSectorMap(), 0);
    EXPECT_EQ(de.IsRandom(), 0);
    de.GetStartTrkSec(tracks, sectors);
    EXPECT_EQ(tracks, -1);
    EXPECT_EQ(sectors, 0);
    de.GetEndTrkSec(tracks, sectors);
    EXPECT_EQ(tracks, 0);
    EXPECT_EQ(sectors, 0);
    EXPECT_NE(de.IsEmpty(), 0);
    auto date = de.GetDate();
    EXPECT_EQ(date, BDate());
    auto time = de.GetTime();
    EXPECT_EQ(time, BTime());
    EXPECT_EQ(de.GetTotalFileName(), "");
}

TEST(test_fdirent, copy_ctor)
{
    FlexDirEntry de_src;

    de_src.SetFileSize(12877);
    de_src.SetAttributes(WRITE_PROTECT | CATALOG_PROTECT);
    de_src.SetSectorMap(IS_RANDOM_FILE);
    de_src.SetStartTrkSec(44, 2);
    de_src.SetEndTrkSec(45, 3);
    de_src.ClearEmpty();
    BDate date_src(28, 1, 1974);
    de_src.SetDate(date_src);
    BTime time_src(15, 33, 2);
    de_src.SetTime(time_src);
    de_src.SetTotalFileName("file_cpy.tst");

    auto de_tgt(de_src);
    int tracks;
    int sectors;

    EXPECT_EQ(de_tgt.GetFileSize(), 12877U);
    EXPECT_EQ(de_tgt.GetAttributes(), WRITE_PROTECT | CATALOG_PROTECT);
    EXPECT_EQ(de_tgt.GetSectorMap(), IS_RANDOM_FILE);
    de_tgt.GetStartTrkSec(tracks, sectors);
    EXPECT_EQ(tracks, 44);
    EXPECT_EQ(sectors, 2);
    de_tgt.GetEndTrkSec(tracks, sectors);
    EXPECT_EQ(tracks, 45);
    EXPECT_EQ(sectors, 3);
    EXPECT_EQ(de_tgt.IsEmpty(), 0);
    auto date_tgt = de_tgt.GetDate();
    EXPECT_EQ(date_tgt, date_src);
    auto time_tgt = de_tgt.GetTime();
    EXPECT_EQ(time_tgt, time_src);
    EXPECT_EQ(de_tgt.GetTotalFileName(), "file_cpy.tst");

    // Source still available.
    EXPECT_EQ(de_src.GetTotalFileName(), "file_cpy.tst");
}

TEST(test_fdirent, move_ctor)
{
    FlexDirEntry de_src;

    de_src.SetFileSize(128);
    de_src.SetAttributes(WRITE_PROTECT);
    de_src.SetSectorMap(IS_RANDOM_FILE);
    de_src.SetStartTrkSec(3, 55);
    de_src.SetEndTrkSec(4, 2);
    de_src.ClearEmpty();
    BDate date_src(1, 4, 2074);
    de_src.SetDate(date_src);
    BTime time_src(3, 28, 56);
    de_src.SetTime(time_src);
    de_src.SetTotalFileName("file_mov.tst");

    FlexDirEntry de_tgt(std::move(de_src));
    int tracks;
    int sectors;

    EXPECT_EQ(de_tgt.GetFileSize(), 128U);
    EXPECT_EQ(de_tgt.GetAttributes(), WRITE_PROTECT);
    EXPECT_EQ(de_tgt.GetSectorMap(), IS_RANDOM_FILE);
    de_tgt.GetStartTrkSec(tracks, sectors);
    EXPECT_EQ(tracks, 3);
    EXPECT_EQ(sectors, 55);
    de_tgt.GetEndTrkSec(tracks, sectors);
    EXPECT_EQ(tracks, 4);
    EXPECT_EQ(sectors, 2);
    EXPECT_EQ(de_tgt.IsEmpty(), 0);
    auto date_tgt = de_tgt.GetDate();
    EXPECT_EQ(date_tgt, date_src);
    auto time_tgt = de_tgt.GetTime();
    EXPECT_EQ(time_tgt, time_src);
    EXPECT_EQ(de_tgt.GetTotalFileName(), "file_mov.tst");

    // Source is reset.
    /* Intentionally test object after move. */
    /* NOLINTNEXTLINE(bugprone-use-after-move) */
    EXPECT_EQ(de_src.GetTotalFileName(), "");
}

TEST(test_fdirent, copy_assignment)
{
    FlexDirEntry de_src;

    de_src.SetFileSize(5690223);
    de_src.SetAttributes(CATALOG_PROTECT);
    de_src.SetSectorMap(IS_RANDOM_FILE);
    de_src.SetStartTrkSec(56, 22);
    de_src.SetEndTrkSec(57, 4);
    de_src.ClearEmpty();
    BDate date_src(3, 12, 1999);
    de_src.SetDate(date_src);
    BTime time_src(13, 21, 3);
    de_src.SetTime(time_src);
    de_src.SetTotalFileName("ass_cpy.tst");

    FlexDirEntry de_tgt = de_src;
    int tracks;
    int sectors;

    EXPECT_EQ(de_tgt.GetFileSize(), 5690223U);
    EXPECT_EQ(de_tgt.GetAttributes(), CATALOG_PROTECT);
    EXPECT_EQ(de_tgt.GetSectorMap(), IS_RANDOM_FILE);
    de_tgt.GetStartTrkSec(tracks, sectors);
    EXPECT_EQ(tracks, 56);
    EXPECT_EQ(sectors, 22);
    de_tgt.GetEndTrkSec(tracks, sectors);
    EXPECT_EQ(tracks, 57);
    EXPECT_EQ(sectors, 4);
    EXPECT_EQ(de_tgt.IsEmpty(), 0);
    auto date_tgt = de_tgt.GetDate();
    EXPECT_EQ(date_tgt, date_src);
    auto time_tgt = de_tgt.GetTime();
    EXPECT_EQ(time_tgt, time_src);
    EXPECT_EQ(de_tgt.GetTotalFileName(), "ass_cpy.tst");

    // Source still available.
    EXPECT_EQ(de_src.GetTotalFileName(), "ass_cpy.tst");
}

TEST(test_fdirent, move_assignment)
{
    FlexDirEntry de_src;

    de_src.SetFileSize(829);
    de_src.SetAttributes(DELETE_PROTECT | CATALOG_PROTECT);
    de_src.SetSectorMap(IS_RANDOM_FILE);
    de_src.SetStartTrkSec(34, 55);
    de_src.SetEndTrkSec(89, 99);
    de_src.ClearEmpty();
    BDate date_src(9, 11, 2004);
    de_src.SetDate(date_src);
    BTime time_src(7, 16, 24);
    de_src.SetTime(time_src);
    de_src.SetTotalFileName("ass_mov.tst");

    FlexDirEntry de_tgt = std::move(de_src);
    int tracks;
    int sectors;

    EXPECT_EQ(de_tgt.GetFileSize(), 829U);
    EXPECT_EQ(de_tgt.GetAttributes(), DELETE_PROTECT | CATALOG_PROTECT);
    EXPECT_EQ(de_tgt.GetSectorMap(), IS_RANDOM_FILE);
    de_tgt.GetStartTrkSec(tracks, sectors);
    EXPECT_EQ(tracks, 34);
    EXPECT_EQ(sectors, 55);
    de_tgt.GetEndTrkSec(tracks, sectors);
    EXPECT_EQ(tracks, 89);
    EXPECT_EQ(sectors, 99);
    EXPECT_EQ(de_tgt.IsEmpty(), 0);
    auto date_tgt = de_tgt.GetDate();
    EXPECT_EQ(date_tgt, date_src);
    auto time_tgt = de_tgt.GetTime();
    EXPECT_EQ(time_tgt, time_src);
    EXPECT_EQ(de_tgt.GetTotalFileName(), "ass_mov.tst");

    // Source is reset.
    /* Intentionally test object after move. */
    /* NOLINTNEXTLINE(bugprone-use-after-move) */
    EXPECT_EQ(de_src.GetTotalFileName(), "");
}

TEST(test_fdirent, get_set)
{
    FlexDirEntry de;
    int tracks;
    int sectors;

    de.SetFileSize(18888);
    de.SetAttributes(DELETE_PROTECT | WRITE_PROTECT);
    de.SetSectorMap(IS_RANDOM_FILE);
    de.SetStartTrkSec(34, 3);
    de.SetEndTrkSec(255, 255);
    de.ClearEmpty();
    BDate date(15, 8, 1989);
    de.SetDate(date);
    BTime time(19, 23, 5);
    de.SetTime(time);
    de.SetTotalFileName("get_set.tst");

    EXPECT_EQ(de.GetFileSize(), 18888U);
    EXPECT_EQ(de.GetAttributes(), DELETE_PROTECT | WRITE_PROTECT);
    de.SetAttributes(CATALOG_PROTECT, WRITE_PROTECT);
    EXPECT_EQ(de.GetAttributes(), DELETE_PROTECT | CATALOG_PROTECT);
    EXPECT_EQ(de.GetAttributesString(), "DC");
    EXPECT_EQ(de.GetSectorMap(), IS_RANDOM_FILE);
    de.SetSectorMap(0);
    EXPECT_EQ(de.GetSectorMap(), 0);
    de.GetStartTrkSec(tracks, sectors);
    EXPECT_EQ(tracks, 34);
    EXPECT_EQ(sectors, 3);
    de.GetEndTrkSec(tracks, sectors);
    EXPECT_EQ(tracks, 255);
    EXPECT_EQ(sectors, 255);
    EXPECT_EQ(de.IsEmpty(), 0);
    de.SetEmpty();
    EXPECT_NE(de.IsEmpty(), 0);
    EXPECT_EQ(de.GetDate(), date);
    EXPECT_EQ(de.GetTime(), time);
    EXPECT_EQ(de.GetTotalFileName(), "get_set.tst");
    EXPECT_EQ(de.GetFileName(), "get_set");
    EXPECT_EQ(de.GetFileExt(), "tst");
    de.SetTotalFileName("get_set");
    EXPECT_EQ(de.GetFileName(), "get_set");
    EXPECT_EQ(de.GetFileExt(), "");
    de.SetTotalFileName(".tst");
    EXPECT_EQ(de.GetFileName(), "");
    EXPECT_EQ(de.GetFileExt(), "tst");
    de.SetTotalFileName("file.tar.gz");
    EXPECT_EQ(de.GetFileName(), "file.tar");
    EXPECT_EQ(de.GetFileExt(), "tar.gz");
}

