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
    EXPECT_EQ(rgbColor, 0xEE82EE);
    EXPECT_FALSE(flx::getColorForName("invalid", rgbColor));
}

