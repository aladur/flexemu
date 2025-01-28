/*
    test_ffilecnts.cpp


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


// Test free functions defined in filecnts.cpp
//
#include "gtest/gtest.h"
#include "filecnts.h"


TEST(test_filecnts, fct_getTrack0SectorCount)
{
    struct TestSample_t
    {
        int tracks;
        int sectors;
        Word expected_sectors0;
    };
    std::vector<TestSample_t> samples{{
        { 35, 5, 5U },
        { 35, 10, 10U },
        { 35, 18, 10U },
        { 35, 19, 19U },
        { 35, 20, 20U },
        { 35, 25, 20U },
        { 40, 5, 5U },
        { 40, 10, 10U },
        { 40, 18, 10U },
        { 40, 19, 19U },
        { 40, 20, 20U },
        { 40, 25, 20U },
        { 40, 36, 20U },
        { 77, 5, 5U },
        { 77, 10, 10U },
        { 77, 15, 15U },
        { 77, 26, 15U },
        { 77, 27, 27U },
        { 77, 30, 30U },
        { 77, 35, 30U },
        { 77, 52, 30U },
        { 254, 5, 5U },
        { 254, 10, 10U },
        { 254, 50, 50U },
        { 254, 100, 100U },
        { 254, 254, 254U },
        { 254, 255, 255U },
        { 255, 5, 5U },
        { 255, 10, 10U },
        { 255, 50, 50U },
        { 255, 100, 100U },
        { 255, 254, 254U },
        { 255, 255, 255U },
        { 1000, 5, 5U },
        { 1000, 255, 255U },
    }};
    auto index = 0;

    for (const auto &sample : samples)
    {
        auto sectors0 = getTrack0SectorCount(sample.tracks, sample.sectors);
        EXPECT_EQ(sectors0, sample.expected_sectors0) << "index=" << index;
        ++index;
    }
}

TEST(test_filecnts, fct_getSides)
{
    struct TestSample_t
    {
        int tracks;
        int sectors;
        Word expected_sides;
    };
    std::vector<TestSample_t> samples{{
        { 35, 5, 1U },
        { 35, 10, 1U },
        { 35, 18, 1U },
        { 35, 19, 1U },
        { 35, 20, 2U },
        { 35, 25, 1U },
        { 40, 5, 1U },
        { 40, 10, 1U },
        { 40, 18, 1U },
        { 40, 19, 1U },
        { 40, 20, 2U },
        { 40, 25, 1U },
        { 40, 36, 2U },
        { 77, 5, 1U },
        { 77, 10, 1U },
        { 77, 15, 1U },
        { 77, 26, 1U },
        { 77, 27, 1U },
        { 77, 30, 2U },
        { 77, 35, 1U },
        { 77, 52, 2U },
        { 254, 5, 1U },
        { 254, 10, 1U },
        { 254, 50, 1U },
        { 254, 100, 1U },
        { 254, 254, 1U },
        { 254, 255, 1U },
        { 255, 5, 1U },
        { 255, 10, 1U },
        { 255, 50, 1U },
        { 255, 100, 1U },
        { 255, 254, 1U },
        { 255, 255, 1U },
        { 1000, 5, 1U },
        { 1000, 255, 1U },
    }};
    auto index = 0;

    for (const auto &sample : samples)
    {
        auto sides = getSides(sample.tracks, sample.sectors);
        EXPECT_EQ(sides, sample.expected_sides) << "index=" << index;
        ++index;
    }
}

TEST(test_filecnts, fct_getBytesPerSector)
{
    EXPECT_EQ(getBytesPerSector(0U), 128U);
    EXPECT_EQ(getBytesPerSector(1U), 256U);
    EXPECT_EQ(getBytesPerSector(2U), 512U);
    EXPECT_EQ(getBytesPerSector(3U), 1024U);
    EXPECT_EQ(getBytesPerSector(4U), 128U);
    EXPECT_EQ(getBytesPerSector(80U), 128U);
}

TEST(test_filecnts, fct_getFileSize)
{
    struct s_flex_header header{};

    header.sectors0 = 10;
    header.sides0 = 1;
    header.tracks = 40;
    header.sectors = 18;
    header.sides = 1;
    header.sizecode = 1;
    EXPECT_EQ(getFileSize(header), 182288U);
    header.sides0 = 2;
    header.sides = 2;
    EXPECT_EQ(getFileSize(header), 364560U);
    header.tracks = 80;
    EXPECT_EQ(getFileSize(header), 733200U);
    header.tracks = 77;
    header.sectors0 = 15;
    header.sectors = 26;
    EXPECT_EQ(getFileSize(header), 1019408U);
}
