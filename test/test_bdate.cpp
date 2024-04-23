#include "gtest/gtest.h"
#include "bdate.h"


TEST(test_bdate, default_ctor)
{
    BDate date1;
    EXPECT_EQ(date1.GetDay(), 0);
    EXPECT_EQ(date1.GetMonth(), 0);
    EXPECT_EQ(date1.GetYear(), 2000);
    BDate date2(28, 2, 81);
    EXPECT_EQ(date2.GetDay(), 28);
    EXPECT_EQ(date2.GetMonth(), 2);
    EXPECT_EQ(date2.GetYear(), 1981);
}

TEST(test_bdate, copy_ctor)
{
    BDate date_src(14, 12, 75);
    BDate date_tgt(date_src);
    EXPECT_EQ(date_tgt.GetDay(), 14);
    EXPECT_EQ(date_tgt.GetMonth(), 12);
    EXPECT_EQ(date_tgt.GetYear(), 1975);
}

TEST(test_bdate, copy_assignment)
{
    BDate date_src(14, 12, 77);
    auto date_tgt = date_src;
    EXPECT_EQ(date_tgt.GetDay(), 14);
    EXPECT_EQ(date_tgt.GetMonth(), 12);
    EXPECT_EQ(date_tgt.GetYear(), 1977);
}

TEST(test_bdate, get_set)
{
    BDate date1;
    date1.Assign(14, 12, 77);
    EXPECT_EQ(date1.GetDay(), 14);
    EXPECT_EQ(date1.GetMonth(), 12);
    EXPECT_EQ(date1.GetYear(), 1977);
    int day;
    int month;
    int year;
    date1.GetDate(day, month, year);
    EXPECT_EQ(day, 14);
    EXPECT_EQ(month, 12);
    EXPECT_EQ(date1.GetMonthBounded(), 12);
    EXPECT_EQ(year, 77);
    day = 15;
    month = 11;
    year = 32;
    date1.SetDate(day, month, year);
    EXPECT_EQ(date1.GetDay(), day);
    EXPECT_EQ(date1.GetMonth(), month);
    EXPECT_EQ(date1.GetMonthBounded(), month);
    EXPECT_EQ(date1.GetYear(), year + 2000);
    date1.SetDate(-5, -10, -2024);
    EXPECT_EQ(date1.GetDay(), -5);
    EXPECT_EQ(date1.GetMonth(), -10);
    EXPECT_EQ(date1.GetMonthBounded(), 1);
    EXPECT_EQ(date1.GetYear(), -24);
    BDate date2(31, 1, 1891);
    date1.SetDate(date2);
    EXPECT_EQ(date2.GetDay(), 31);
    EXPECT_EQ(date2.GetMonth(), 1);
    EXPECT_EQ(date2.GetMonthBounded(), 1);
    EXPECT_EQ(date2.GetYear(), 1891);
    date2.Assign(31, 22, 1891);
    EXPECT_EQ(date2.GetMonthBounded(), 12);
    date2 = BDate::Now();
    EXPECT_TRUE(date2.GetDay() >= 1 && date2.GetDay() <= 31);
    EXPECT_TRUE(date2.GetMonth() >= 1 && date2.GetMonth() <= 12);
}

TEST(test_bdate, get_string)
{
    BDate date(4, 2, 998);
    EXPECT_EQ(date.GetDateString(), "04-Feb-0998");
    EXPECT_EQ(date.GetDateString(BDate::Format::D2MSU3Y4), "04-FEB-0998");
    EXPECT_EQ(date.GetDateString(BDate::Format::Iso), "09980204");
    date.Assign(333, 154, -3);
    EXPECT_EQ(date.GetDateString(BDate::Format::Iso), "1997154333");
    std::string date_string("333-?\?\?-1997");
    EXPECT_EQ(date.GetDateString(BDate::Format::D2MS3Y4), date_string);
    EXPECT_EQ(date.GetDateString(BDate::Format::D2MSU3Y4), date_string);
}

TEST(test_bdate, cmp_fct)
{
    BDate date1(4, 2, 1998);
    BDate date2(5, 2, 1998);
    EXPECT_TRUE(date2 > date1);
    EXPECT_TRUE(date2 >= date1);
    EXPECT_TRUE(date1 < date2);
    EXPECT_TRUE(date1 <= date2);
    EXPECT_TRUE(date1 != date2);
    EXPECT_FALSE(date2 < date1);
    EXPECT_FALSE(date1 > date2);
    EXPECT_FALSE(date1 == date2);
    date2 = date1;
    EXPECT_TRUE(date1 == date2);
    EXPECT_FALSE(date1 != date2);
}

