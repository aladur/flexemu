#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "boption.h"
#include <string>


using ::testing::Throws;

using BIntOpt = BOptional<int>;
using BStringOpt = BOptional<std::string>;

TEST(test_boption, int_ctor)
{
    BIntOpt opt1;
    EXPECT_EQ(opt1.has_value(), false);
    EXPECT_EQ(opt1.value_or(4711), 4711);
    BIntOpt opt2(opt1);
    EXPECT_EQ(opt2.has_value(), false);
    EXPECT_EQ(opt2.value_or(4711), 4711);
    BIntOpt opt3(2203);
    EXPECT_EQ(opt3.has_value(), true);
    EXPECT_EQ(opt3.value_or(4711), 2203);
}

TEST(test_boption, int_get_set)
{
    const BIntOpt opt1(2203);
    EXPECT_EQ(opt1.has_value(), true);
    EXPECT_EQ(opt1.value_or(4711), 2203);
    EXPECT_EQ(opt1.value(), 2203);
    BIntOpt opt2(2203);
    EXPECT_EQ(opt2.has_value(), true);
    EXPECT_EQ(opt2.value_or(4711), 2203);
    EXPECT_EQ(opt2.value(), 2203);
    opt2 = -5506;
    EXPECT_EQ(opt2.has_value(), true);
    EXPECT_EQ(opt2.value_or(4711), -5506);
    EXPECT_EQ(opt2.value(), -5506);
    opt2 = opt1;
    EXPECT_EQ(opt2.has_value(), true);
    EXPECT_EQ(opt2.value_or(4711), 2203);
    EXPECT_EQ(opt2.value(), 2203);
    opt2 = 2203;
    EXPECT_EQ(opt2.has_value(), true);
    EXPECT_EQ(opt2.value_or(4711), 2203);
    EXPECT_EQ(opt2.value(), 2203);
    auto opt3 = opt2 = 65535;
    EXPECT_EQ(opt3.has_value(), true);
    EXPECT_EQ(opt3.value_or(4711), 65535);
    EXPECT_EQ(opt3.value(), 65535);
    const auto opt4 = opt3;
    EXPECT_EQ(opt4.has_value(), true);
    EXPECT_EQ(opt4.value_or(4711), 65535);
    EXPECT_EQ(opt4.value(), 65535);
    BIntOpt opt5;
    EXPECT_THAT([&]() { opt5.value(); }, testing::Throws<FlexException>());
    const BIntOpt opt6;
    EXPECT_THAT([&]() { opt6.value(); }, testing::Throws<FlexException>());
}

TEST(test_boption, int_reset)
{
    BIntOpt opt1(2203);
    EXPECT_EQ(opt1.has_value(), true);
    EXPECT_EQ(opt1.value_or(4711), 2203);
    opt1.reset();
    EXPECT_EQ(opt1.has_value(), false);
    EXPECT_EQ(opt1.value_or(4711), 4711);
    auto opt2 = opt1;
    opt2.reset();
    EXPECT_EQ(opt2.has_value(), false);
    EXPECT_EQ(opt2.value_or(4711), 4711);
}

TEST(test_boption, int_deref)
{
    BIntOpt opt1;
    EXPECT_THAT([&]() { *opt1 = 2203; }, testing::Throws<FlexException>());
    EXPECT_THAT([&]() { (void)*opt1; }, testing::Throws<FlexException>());
    const BIntOpt opt2;
    EXPECT_THAT([&]() { (void)*opt2; }, testing::Throws<FlexException>());
    BIntOpt opt3(65535);
    *opt3 = 2203;
    auto v = *opt3;
    EXPECT_EQ(v, 2203);
}

TEST(test_boption, string_ctor)
{
    BStringOpt opt1;
    EXPECT_EQ(opt1.has_value(), false);
    EXPECT_EQ(opt1.value_or("4711"), "4711");
    BStringOpt opt2(opt1);
    EXPECT_EQ(opt2.has_value(), false);
    EXPECT_EQ(opt2.value_or("4711"), "4711");
    BStringOpt opt3("2203");
    EXPECT_EQ(opt3.has_value(), true);
    EXPECT_EQ(opt3.value_or("4711"), "2203");
}

TEST(test_boption, string_get_set)
{
    const BStringOpt opt1("2203");
    EXPECT_EQ(opt1.has_value(), true);
    EXPECT_EQ(opt1.value_or("4711"), "2203");
    EXPECT_EQ(opt1.value(), "2203");
    BStringOpt opt2("2203");
    EXPECT_EQ(opt2.has_value(), true);
    EXPECT_EQ(opt2.value_or("4711"), "2203");
    EXPECT_EQ(opt2.value(), "2203");
    opt2 = "-5506";
    EXPECT_EQ(opt2.has_value(), true);
    EXPECT_EQ(opt2.value_or("4711"), "-5506");
    EXPECT_EQ(opt2.value(), "-5506");
    opt2 = opt1;
    EXPECT_EQ(opt2.has_value(), true);
    EXPECT_EQ(opt2.value_or("4711"), "2203");
    EXPECT_EQ(opt2.value(), "2203");
    opt2 = "2203";
    EXPECT_EQ(opt2.has_value(), true);
    EXPECT_EQ(opt2.value_or("4711"), "2203");
    EXPECT_EQ(opt2.value(), "2203");
    auto opt3 = opt2 = "65535";
    EXPECT_EQ(opt3.has_value(), true);
    EXPECT_EQ(opt3.value_or("4711"), "65535");
    EXPECT_EQ(opt3.value(), "65535");
    const auto opt4 = opt3;
    EXPECT_EQ(opt4.has_value(), true);
    EXPECT_EQ(opt4.value_or("4711"), "65535");
    EXPECT_EQ(opt4.value(), "65535");
    BStringOpt opt5;
    EXPECT_THAT([&]() { opt5.value(); }, testing::Throws<FlexException>());
}

TEST(test_boption, string_reset)
{
    BStringOpt opt("2203");
    EXPECT_EQ(opt.has_value(), true);
    EXPECT_EQ(opt.value_or("4711"), "2203");
    opt.reset();
    EXPECT_EQ(opt.has_value(), false);
    EXPECT_EQ(opt.value_or("4711"), "4711");
}

TEST(test_boption, string_deref)
{
    BStringOpt opt1;
    EXPECT_THAT([&]() { *opt1 = "2203"; }, testing::Throws<FlexException>());
    EXPECT_THAT([&]() { (void)*opt1; }, testing::Throws<FlexException>());
    EXPECT_EQ(opt1.has_value(), false);
    EXPECT_EQ(opt1.value_or("4711"), "4711");
    BStringOpt opt2("65535");
    EXPECT_EQ(*opt2, "65535");
    EXPECT_EQ(opt2->size(), 5U);
    opt2->append(" MHz");
    EXPECT_EQ(*opt2, "65535 MHz");
    *opt2 = "2203";
    EXPECT_EQ(opt2.has_value(), true);
    EXPECT_EQ(opt2.value_or("4711"), "2203");
    opt2.reset();
    EXPECT_THAT([&]() { *opt2 = "2203"; }, testing::Throws<FlexException>());
    BStringOpt opt3;
    EXPECT_THAT([&]() { (void)opt3->size(); },
            testing::Throws<FlexException>());
    const BStringOpt opt4;
    EXPECT_THAT([&]() { (void)opt4->size(); },
            testing::Throws<FlexException>());
    EXPECT_THAT([&]() { (void)*opt4; }, testing::Throws<FlexException>());
}

