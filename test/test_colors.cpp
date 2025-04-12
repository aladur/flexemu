/*
    test_colors.cpp


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
#include "colors.h"


TEST(test_colors, fct_getRGBForName)
{
    Word red;
    Word green;
    Word blue;

    EXPECT_TRUE(flx::getRGBForName("blue", red, green, blue));
    EXPECT_EQ(red, 0U);
    EXPECT_EQ(green, 0U);
    EXPECT_EQ(blue, 255U);
    EXPECT_TRUE(flx::getRGBForName("orange", red, green, blue));
    EXPECT_EQ(red, 255U);
    EXPECT_EQ(green, 165U);
    EXPECT_EQ(blue, 0U);
    EXPECT_TRUE(flx::getRGBForName("violet", red, green, blue));
    EXPECT_EQ(red, 238U);
    EXPECT_EQ(green, 130U);
    EXPECT_EQ(blue, 238U);
    EXPECT_FALSE(flx::getRGBForName("invalid", red, green, blue));
}

TEST(test_colors, fct_getColorForName)
{
    DWord rgbColor;

    EXPECT_TRUE(flx::getColorForName("blue", rgbColor));
    EXPECT_EQ(rgbColor, 0xFF0000U);
    EXPECT_TRUE(flx::getColorForName("orange", rgbColor));
    EXPECT_EQ(rgbColor, 0x00A5FFU);
    EXPECT_TRUE(flx::getColorForName("violet", rgbColor));
    EXPECT_EQ(rgbColor, 0xEE82EEU);
    EXPECT_FALSE(flx::getColorForName("invalid", rgbColor));
}

