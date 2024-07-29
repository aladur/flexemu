#include "gtest/gtest.h"
#include "breltime.h"


TEST(test_breltime, getTimeUsll)
{
    const auto time1 = BRelativeTime::GetTimeUsll();
    usleep(10000);
    const auto time2 = BRelativeTime::GetTimeUsll();
    EXPECT_TRUE(time2 - time1 > 9000ULL);
    const auto time3 = BRelativeTime::GetTimeUsll();
    EXPECT_TRUE(time3 - time2 < 10000ULL);
}

