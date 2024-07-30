#include "gtest/gtest.h"
#include "blinxsys.h"
#include <string>


#ifdef __LINUX

TEST(test_blinxsys, fct_toString)
{
    BLinuxSysInfo info;

    const auto typeString = BLinuxSysInfo::ToString(BLinuxSysInfoType::LED);
    EXPECT_EQ(typeString, "LED");
}

TEST(test_blinxsys, fct_read)
{
    BLinuxSysInfo info;

    // On linux servers there may be no terminal attached in which case
    // the numlock LED does not exist.
    const auto value1 = info.Read(BLinuxSysInfoType::LED, "numlock");
    EXPECT_TRUE(value1.empty() || value1 == "0" || value1 == "1");
    const auto value2 = info.Read(BLinuxSysInfoType::LED, "capslock");
    EXPECT_TRUE(value2.empty() || value2 == "0" || value1 == "1");
    const auto value3 = info.Read(BLinuxSysInfoType::LED, "notexistent");
    EXPECT_TRUE(value3.empty());
}

#endif

