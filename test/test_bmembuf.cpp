#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "bmembuf.h"
#include "typedefs.h"
#include <array>
#include <vector>


TEST(test_bmembuf, ctor)
{
    BMemoryBuffer buffer(0x4000U);
    EXPECT_EQ(buffer.GetSize(), 0x4000U);
    EXPECT_EQ(buffer.GetAddressRanges().size(), 0U);
}

TEST(test_bmembuf, copy_ctor)
{
    BMemoryBuffer buffer_src(0x7FFFU);
    auto buffer_tgt(buffer_src);
    EXPECT_EQ(buffer_tgt.GetSize(), 0x7FFFU);
    EXPECT_EQ(buffer_tgt.GetAddressRanges().size(), 0U);
}

TEST(test_bmembuf, move_ctor)
{
    auto buffer_tgt(BMemoryBuffer(0x10000U));
    EXPECT_EQ(buffer_tgt.GetSize(), 0x10000U);
    EXPECT_EQ(buffer_tgt.GetAddressRanges().size(), 0U);
}

TEST(test_bmembuf, copy_assignment)
{
    BMemoryBuffer buffer_src(0x8000U);
    auto buffer_tgt = buffer_src;
    EXPECT_EQ(buffer_tgt.GetSize(), 0x8000U);
    EXPECT_EQ(buffer_tgt.GetAddressRanges().size(), 0U);
    EXPECT_EQ(buffer_src.GetSize(), 0x8000U);
    EXPECT_EQ(buffer_src.GetAddressRanges().size(), 0U);
}

TEST(test_bmembuf, move_assignment)
{
    BMemoryBuffer buffer_src(0xC000U);
    auto buffer_tgt = std::move(buffer_src);
    EXPECT_EQ(buffer_tgt.GetSize(), 0xC000U);
    EXPECT_EQ(buffer_tgt.GetAddressRanges().size(), 0U);
    EXPECT_EQ(buffer_src.GetSize(), 0U);
    EXPECT_EQ(buffer_src.GetAddressRanges().size(), 0U);
}

TEST(test_bmembuf, member_fct)
{
    BMemoryBuffer buffer(0x1000U);
    EXPECT_EQ(buffer.GetSize(), 0x1000U);
    buffer.Reset();
    EXPECT_EQ(buffer.GetSize(), 0x1000U);
}

TEST(test_bmembuf, member_CopyFrom)
{
    BMemoryBuffer buffer(0x4000U);
    std::vector<Byte> buffer_src1{ 1, 2, 3, 4, 5, 6, 7, 8 };
    buffer.CopyFrom(buffer_src1.data(), 0x1000U, buffer_src1.size());
    std::vector<Byte> buffer_src2{ 250, 251, 252, 252, 253, 254, 255 };
    buffer.CopyFrom(buffer_src2.data(), 0x3FFC, buffer_src2.size());
    EXPECT_EQ(buffer.GetAddressRanges().size(), 2U);
    EXPECT_EQ(buffer.GetAddressRanges().at(0U).lower(), 0x1000U);
    EXPECT_EQ(buffer.GetAddressRanges().at(0U).upper(), 0x1007U);
    EXPECT_EQ(buffer.GetAddressRanges().at(1U).lower(), 0x3FFCU);
    EXPECT_EQ(buffer.GetAddressRanges().at(1U).upper(), 0x3FFFU);

    buffer.CopyFrom(buffer_src2.data(), 0x3FF5, buffer_src2.size());
    EXPECT_EQ(buffer.GetAddressRanges().size(), 2U);
    EXPECT_EQ(buffer.GetAddressRanges().at(1U).lower(), 0x3FF5U);
    EXPECT_EQ(buffer.GetAddressRanges().at(1U).upper(), 0x3FFFU);

    EXPECT_THAT([&]() {
            buffer.CopyFrom(buffer_src2.data(), 0x4000, 1); },
            testing::Throws<std::out_of_range>());
}

TEST(test_bmembuf, member_CopyTo)
{
    BMemoryBuffer buffer(0x4000U);
    std::vector<Byte> buffer_src1{ 1, 2, 3, 4, 5, 6, 7, 8 };
    buffer.CopyFrom(buffer_src1.data(), 0x1000U, buffer_src1.size());
    std::array<Byte, 10> buffer_tgt1;
    buffer.CopyTo(buffer_tgt1.data(), 0x1000U, 10);
    std::array<Byte, 10> buffer_cmp1{ 1, 2, 3, 4, 5, 6, 7, 8, 0, 0 };
    EXPECT_EQ(buffer_tgt1, buffer_cmp1);

    std::vector<Byte> buffer_src2{ 250, 251, 252, 253, 254, 255 };
    buffer.CopyFrom(buffer_src2.data(), 0x3FFC, buffer_src2.size());
    std::array<Byte, 6> buffer_tgt2_1;
    buffer.CopyTo(buffer_tgt2_1.data(), 0x3FFC, 10);
    std::array<Byte, 6> buffer_cmp2{ 250, 251, 252, 253, 0, 0 };
    EXPECT_EQ(buffer_tgt2_1, buffer_cmp2);

    EXPECT_THAT([&]() {
            buffer.CopyTo(buffer_tgt2_1.data(), 0x4000, 1); },
            testing::Throws<std::out_of_range>());
}

