/*
    test_misc1.cpp


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
#include "bitops.h"


TEST(test_bitops, fct_BTST)
{
    const Byte val8 = 0x55U;
    const Word val16 = 0xAAAAU;
    const DWord val32 = 0x55555555U;
    const QWord val64 = 0xAAAAAAAAAAAAAAAAU;

    // 8-bit value
    EXPECT_TRUE(BTST<Byte>(val8, 0U));
    EXPECT_FALSE(BTST<Byte>(val8, 1U));
    EXPECT_TRUE(BTST<Byte>(val8, 4U));
    EXPECT_FALSE(BTST<Byte>(val8, 7U));
    // 16-bit value
    EXPECT_FALSE(BTST<Word>(val16, 0U));
    EXPECT_TRUE(BTST<Word>(val16, 1U));
    EXPECT_FALSE(BTST<Word>(val16, 4U));
    EXPECT_TRUE(BTST<Word>(val16, 15U));
    // 32-bit value
    EXPECT_TRUE(BTST<DWord>(val32, 0U));
    EXPECT_FALSE(BTST<DWord>(val32, 1U));
    EXPECT_TRUE(BTST<DWord>(val32, 4U));
    EXPECT_FALSE(BTST<DWord>(val32, 31U));
    // 64-bit value
    EXPECT_FALSE(BTST<QWord>(val64, 0U));
    EXPECT_TRUE(BTST<QWord>(val64, 1U));
    EXPECT_FALSE(BTST<QWord>(val64, 4U));
    EXPECT_TRUE(BTST<QWord>(val64, 63U));
}

TEST(test_bitops, fct_BSET)
{
    Byte val8 = 0U;
    Word val16 = 0U;
    DWord val32 = 0U;
    QWord val64 = 0U;

    // 8-bit value
    BSET<Byte>(val8, 0U);
    EXPECT_EQ(val8, 0x01U);
    BSET<Byte>(val8, 1U);
    EXPECT_EQ(val8, 0x03U);
    BSET<Byte>(val8, 4U);
    EXPECT_EQ(val8, 0x13U);
    BSET<Byte>(val8, 7U);
    EXPECT_EQ(val8, 0x93U);
    BSET<Byte>(val8, 4U);
    EXPECT_EQ(val8, 0x93U);
    // 16-bit value
    BSET<Word>(val16, 0U);
    EXPECT_EQ(val16, 0x01U);
    BSET<Word>(val16, 1U);
    EXPECT_EQ(val16, 0x03U);
    BSET<Word>(val16, 4U);
    EXPECT_EQ(val16, 0x13U);
    BSET<Word>(val16, 15U);
    EXPECT_EQ(val16, 0x8013U);
    BSET<Word>(val16, 4U);
    EXPECT_EQ(val16, 0x8013U);
    // 32-bit value
    BSET<DWord>(val32, 0U);
    EXPECT_EQ(val32, 0x01U);
    BSET<DWord>(val32, 1U);
    EXPECT_EQ(val32, 0x03U);
    BSET<DWord>(val32, 4U);
    EXPECT_EQ(val32, 0x13U);
    BSET<DWord>(val32, 31U);
    EXPECT_EQ(val32, 0x80000013U);
    BSET<DWord>(val32, 4U);
    EXPECT_EQ(val32, 0x80000013U);
    // 64-bit value
    BSET<QWord>(val64, 0U);
    EXPECT_EQ(val64, 0x01U);
    BSET<QWord>(val64, 1U);
    EXPECT_EQ(val64, 0x03U);
    BSET<QWord>(val64, 4U);
    EXPECT_EQ(val64, 0x13U);
    BSET<QWord>(val64, 63U);
    EXPECT_EQ(val64, 0x8000000000000013U);
    BSET<QWord>(val64, 4U);
    EXPECT_EQ(val64, 0x8000000000000013U);
}

TEST(test_bitops, fct_BCLR)
{
    Byte val8 = 0xFFU;
    Word val16 = 0xFFFFU;
    DWord val32 = 0xFFFFFFFFU;
    QWord val64 = 0xFFFFFFFFFFFFFFFFU;

    // 8-bit value
    BCLR<Byte>(val8, 0U);
    EXPECT_EQ(val8, 0xFEU);
    BCLR<Byte>(val8, 1U);
    EXPECT_EQ(val8, 0xFCU);
    BCLR<Byte>(val8, 4U);
    EXPECT_EQ(val8, 0xECU);
    BCLR<Byte>(val8, 7U);
    EXPECT_EQ(val8, 0x6CU);
    BCLR<Byte>(val8, 4U);
    EXPECT_EQ(val8, 0x6CU);
    // 16-bit value
    BCLR<Word>(val16, 0U);
    EXPECT_EQ(val16, 0xFFFEU);
    BCLR<Word>(val16, 1U);
    EXPECT_EQ(val16, 0xFFFCU);
    BCLR<Word>(val16, 4U);
    EXPECT_EQ(val16, 0xFFECU);
    BCLR<Word>(val16, 15U);
    EXPECT_EQ(val16, 0x7FECU);
    BCLR<Word>(val16, 4U);
    EXPECT_EQ(val16, 0x7FECU);
    // 32-bit value
    BCLR<DWord>(val32, 0U);
    EXPECT_EQ(val32, 0xFFFFFFFEU);
    BCLR<DWord>(val32, 1U);
    EXPECT_EQ(val32, 0xFFFFFFFCU);
    BCLR<DWord>(val32, 4U);
    EXPECT_EQ(val32, 0xFFFFFFECU);
    BCLR<DWord>(val32, 31U);
    EXPECT_EQ(val32, 0x7FFFFFECU);
    BCLR<DWord>(val32, 4U);
    EXPECT_EQ(val32, 0x7FFFFFECU);
    // 64-bit value
    BCLR<QWord>(val64, 0U);
    EXPECT_EQ(val64, 0xFFFFFFFFFFFFFFFEU);
    BCLR<QWord>(val64, 1U);
    EXPECT_EQ(val64, 0xFFFFFFFFFFFFFFFCU);
    BCLR<QWord>(val64, 4U);
    EXPECT_EQ(val64, 0xFFFFFFFFFFFFFFECU);
    BCLR<QWord>(val64, 63U);
    EXPECT_EQ(val64, 0x7FFFFFFFFFFFFFECU);
    BCLR<QWord>(val64, 4U);
    EXPECT_EQ(val64, 0x7FFFFFFFFFFFFFECU);
}

TEST(test_bitops, fct_EXTEND8)
{
    const Byte src1{0};
    const auto tgt1 = EXTEND8(src1);
    EXPECT_EQ(0x0U, tgt1);
    EXPECT_EQ(sizeof(tgt1), 2U);
    const Byte src2 = 0x7FU;
    const auto tgt2 = EXTEND8(src2);
    EXPECT_EQ(0x7FU, tgt2);
    EXPECT_EQ(sizeof(tgt2), 2U);
    const Byte src3 = 0x7FU;
    const auto tgt3 = EXTEND8(src3);
    EXPECT_EQ(0x7FU, tgt3);
    EXPECT_EQ(sizeof(tgt3), 2U);
    const Byte src4 = 0xFFU;
    const auto tgt4 = EXTEND8(src4);
    EXPECT_EQ(0xFFFFU, tgt4);
    EXPECT_EQ(sizeof(tgt4), 2U);
}

