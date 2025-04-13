/*
    test_bintervl.cpp


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
#include "bintervl.h"
#include "flexerr.h"
#include "typedefs.h"
#include  <vector>


using BWInterval = BInterval<Word>;
using BIntInterval = BInterval<int>;
using BDWInterval = BInterval<DWord>;
using BFInterval = BInterval<float>;

TEST(test_bintervl, default_ctor)
{
    BIntInterval iv1;
    EXPECT_EQ(iv1.lower(), 0);
    EXPECT_EQ(iv1.upper(), 0);
    BWInterval iv2(0, 0xFFFF);
    EXPECT_EQ(iv2.lower(), 0);
    EXPECT_EQ(iv2.upper(), 0xFFFF);
    BIntInterval iv3(0,8);
    EXPECT_EQ(iv3.lower(), 0);
    EXPECT_EQ(iv3.upper(), 8);
    EXPECT_THAT([&]() { BIntInterval iv4(1,0); },
            testing::Throws<std::invalid_argument>());
    BIntInterval iv5(-8,8);
    EXPECT_EQ(iv5.lower(), -8);
    EXPECT_EQ(iv5.upper(), 8);
    BDWInterval iv6(0x10000000L, 0x20000000L);
    EXPECT_EQ(iv6.lower(), 0x10000000LU);
    EXPECT_EQ(iv6.upper(), 0x20000000LU);
    BFInterval iv7(2.5F, 5.5F);
    EXPECT_EQ(iv7.lower(), 2.5F);
    EXPECT_EQ(iv7.upper(), 5.5F);
#ifdef _MSC_VER
// Disable warning to intentionally testing this case.
#pragma warning( push )
#pragma warning( disable : 4244 )
#endif
    EXPECT_THAT([&]() { BIntInterval iv8(1.0F, -1.0F); },
            testing::Throws<std::invalid_argument>());
#ifdef _MSC_VER
#pragma warning( pop )
#endif
}

TEST(test_bintervl, copy_ctor)
{
    BDWInterval iv_src(0L, 0xFFFFFFFF);
    BDWInterval iv_tgt(iv_src);
    EXPECT_EQ(iv_tgt.lower(), 0LU);
    EXPECT_EQ(iv_tgt.upper(), 0xFFFFFFFFLU);
}

TEST(test_bintervl, copy_assignment)
{
    BDWInterval iv_src(0L, 0xFFFFFFFF);
    BDWInterval iv_tgt = iv_src;
    EXPECT_EQ(iv_tgt.lower(), 0LU);
    EXPECT_EQ(iv_tgt.upper(), 0xFFFFFFFFLU);
}

TEST(test_bintervl, get_set)
{
    BWInterval iv1;
    EXPECT_EQ(width(iv1), 0);
    iv1.assign(8, 0xFFFF);
    EXPECT_EQ(iv1.lower(), 8U);
    EXPECT_EQ(iv1.upper(), 0xFFFFU);
    EXPECT_EQ(width(iv1), 65527U);
    EXPECT_THAT([&]() { iv1.assign(1,0); },
            testing::Throws<std::invalid_argument>());
    const auto [lower,upper] = iv1.get();
    EXPECT_EQ(lower, 8);
    EXPECT_EQ(upper, 0xFFFF);
    std::stringstream os;
    os << iv1;
    EXPECT_EQ(os.str(), "[8,65535]");
    BWInterval iv2{4, 32};
    auto iv3 = hull(iv1, iv2);
    EXPECT_EQ(iv3.lower(), 4U);
    EXPECT_EQ(iv3.upper(), 65535U);
}

TEST(test_bintervl, singleton)
{
    EXPECT_TRUE(singleton(BWInterval{}));
    EXPECT_FALSE(singleton(BIntInterval(10000,10001)));
}

TEST(test_bintervl, equal)
{
    EXPECT_TRUE(equal(BDWInterval{}, BDWInterval{}));
    EXPECT_TRUE(equal(BIntInterval(-4,25), BIntInterval(-4,25)));
    EXPECT_FALSE(equal(BIntInterval(-4,25), BIntInterval(-5,25)));
    EXPECT_FALSE(equal(BIntInterval(-4,25), BIntInterval(-5,26)));
}

TEST(test_bintervl, in)
{
    EXPECT_TRUE(in(0U, BDWInterval{}));
    EXPECT_TRUE(in(42U, BDWInterval(42U, 49U)));
    EXPECT_TRUE(in(49U, BDWInterval(42U, 49U)));
    EXPECT_TRUE(in(0xFFFFFFFFU, BDWInterval(0xFFFFFFFEU, 0xFFFFFFFF)));
    EXPECT_FALSE(in(41U, BDWInterval(42U, 49U)));
    EXPECT_FALSE(in(0xFFFFFFFFU, BDWInterval(42U, 49U)));
}

TEST(test_bintervl, subset)
{
    EXPECT_TRUE(subset(BIntInterval(-100,100), BIntInterval(-100,100)));
    EXPECT_TRUE(subset(BIntInterval(-50,50), BIntInterval(-100,100)));
    EXPECT_TRUE(subset(BIntInterval(), BIntInterval(-100,100)));
    EXPECT_FALSE(subset(BIntInterval(-101,100), BIntInterval(-100,100)));
    EXPECT_FALSE(subset(BIntInterval(-100,101), BIntInterval(-100,100)));
    EXPECT_FALSE(proper_subset(BIntInterval(-100,100), BIntInterval(-100,100)));
    EXPECT_TRUE(proper_subset(BIntInterval(-50,50), BIntInterval(-100,100)));
    EXPECT_TRUE(proper_subset(BIntInterval(), BIntInterval(-100,100)));
    EXPECT_FALSE(proper_subset(BIntInterval(-101,100), BIntInterval(-100,100)));
    EXPECT_FALSE(proper_subset(BIntInterval(-100,101), BIntInterval(-100,100)));
}

TEST(test_bintervl, overlap)
{
    EXPECT_TRUE(overlap(BIntInterval(-100,100), BIntInterval(-100,100)));
    EXPECT_TRUE(overlap(BIntInterval(-100,100), BIntInterval(100,150)));
    EXPECT_TRUE(overlap(BIntInterval(100,150), BIntInterval(-100,100)));
    EXPECT_TRUE(overlap(BIntInterval(-100,100), BIntInterval(0,50)));
    EXPECT_TRUE(overlap(BIntInterval(0,50), BIntInterval(-100,100)));
    EXPECT_TRUE(overlap(BIntInterval(-100,100), BIntInterval(0,200)));
    EXPECT_TRUE(overlap(BIntInterval(-100,100), BIntInterval(50,200)));
    EXPECT_TRUE(overlap(BIntInterval(50,200), BIntInterval(-100,100)));
    EXPECT_FALSE(overlap(BIntInterval(-100,100), BIntInterval(110,200)));
    EXPECT_FALSE(overlap(BIntInterval(110,200), BIntInterval(-100,100)));
    EXPECT_FALSE(overlap(BWInterval(), BWInterval(0xFFFF,0xFFFF)));
}

TEST(test_bintervl, cer)
{
    EXPECT_TRUE(cerle(-200, BIntInterval(-100,100)));
    EXPECT_TRUE(cerle(-100, BIntInterval(-100,100)));
    EXPECT_FALSE(cerle(-50, BIntInterval(-100,100)));
    EXPECT_FALSE(cerle(200, BIntInterval(-100,100)));
    EXPECT_TRUE(cerlt(-200, BIntInterval(-100,100)));
    EXPECT_FALSE(cerlt(-100, BIntInterval(-100,100)));
    EXPECT_FALSE(cerlt(-50, BIntInterval(-100,100)));
    EXPECT_FALSE(cerlt(200, BIntInterval(-100,100)));
    EXPECT_TRUE(cerge(BIntInterval(-100,100), -100));
    EXPECT_TRUE(cerge(BIntInterval(-100,100), -200));
    EXPECT_FALSE(cerge(BIntInterval(-100,100), 200));
    EXPECT_FALSE(cergt(BIntInterval(-100,100), -100));
    EXPECT_TRUE(cergt(BIntInterval(-100,100), -200));
    EXPECT_FALSE(cergt(BIntInterval(-100,100), -100));
}

TEST(test_bintervl, cmp_lower)
{
    EXPECT_TRUE(cmp_lower(BIntInterval(-200,-100), BIntInterval(-100,100)));
    EXPECT_FALSE(cmp_lower(BIntInterval(-100,-100), BIntInterval(-100,100)));
    EXPECT_FALSE(cmp_lower(BIntInterval(100,200), BIntInterval(-100,100)));
}

TEST(test_bintervl, sort_lower)
{
    std::vector<BIntInterval> ivs{
        BIntInterval(100,1000),
        BIntInterval(-100,5000),
        BIntInterval(150,200),
        BIntInterval(-200,100),
    };
    sort_lower(ivs);
    EXPECT_EQ(ivs.size(), 4U);
    EXPECT_TRUE(equal(ivs.at(0), BIntInterval(-200,100)));
    EXPECT_TRUE(equal(ivs.at(1), BIntInterval(-100,5000)));
    EXPECT_TRUE(equal(ivs.at(2), BIntInterval(100,1000)));
    EXPECT_TRUE(equal(ivs.at(3), BIntInterval(150,200)));
}

TEST(test_bintervl, join)
{
    std::vector<BIntInterval> ivs{
        BIntInterval(-100,100),
        BIntInterval(0,0),
        BIntInterval(0,200),
        BIntInterval(200,300),
        BIntInterval(400,500),
    };
    join(ivs);
    EXPECT_EQ(ivs.size(), 2U);
    EXPECT_TRUE(equal(ivs.at(0), BIntInterval(-100,300)));
    EXPECT_TRUE(equal(ivs.at(1), BIntInterval(400,500)));
}

