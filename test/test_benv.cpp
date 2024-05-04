#include "gtest/gtest.h"
#include "benv.h"
#include <string>


TEST(test_benv, get_set_int)
{
    int value;

    EXPECT_TRUE(BEnvironment::SetValue("FLEXEMU_TEST_VALUE", 4711));
    EXPECT_TRUE(BEnvironment::SetValue("flexemu_test_value", 3722));
    EXPECT_TRUE(BEnvironment::SetValue("FlexemuTestValue", 5522));
    EXPECT_FALSE(BEnvironment::SetValue("FLEXEMU_=_NOT_ALLOWED", 0));

    EXPECT_TRUE(BEnvironment::GetValue("FLEXEMU_TEST_VALUE", value));
    EXPECT_EQ(value, 4711);
    EXPECT_TRUE(BEnvironment::GetValue("flexemu_test_value", value));
    EXPECT_EQ(value, 3722);
    EXPECT_TRUE(BEnvironment::GetValue("FlexemuTestValue", value));
    EXPECT_EQ(value, 5522);

    BEnvironment::RemoveKey("FLEXEMU_TEST_VALUE");
    BEnvironment::RemoveKey("flexemu_test_value");
    BEnvironment::RemoveKey("FlexemuTestValue");

    EXPECT_FALSE(BEnvironment::GetValue("FLEXEMU_TEST_VALUE", value));
    EXPECT_FALSE(BEnvironment::GetValue("flexemu_test_value", value));
    EXPECT_FALSE(BEnvironment::GetValue("FlexemuTestValue", value));
}

TEST(test_benv, get_set_string)
{
    std::string value;

    EXPECT_TRUE(BEnvironment::SetValue("FLEXEMU_TEST_VALUE", "4711"));
    EXPECT_TRUE(BEnvironment::SetValue("flexemu_test_value", "3722"));
    EXPECT_TRUE(BEnvironment::SetValue("FlexemuTestValue", "5522"));
    EXPECT_FALSE(BEnvironment::SetValue("Flexemu=NotAllowed", "equal_char"));

    EXPECT_TRUE(BEnvironment::GetValue("FLEXEMU_TEST_VALUE", value));
    EXPECT_EQ(value, "4711");
    EXPECT_TRUE(BEnvironment::GetValue("flexemu_test_value", value));
    EXPECT_EQ(value, "3722");
    EXPECT_TRUE(BEnvironment::GetValue("FlexemuTestValue", value));
    EXPECT_EQ(value, "5522");

    BEnvironment::RemoveKey("FLEXEMU_TEST_VALUE");
    BEnvironment::RemoveKey("flexemu_test_value");
    BEnvironment::RemoveKey("FlexemuTestValue");

    EXPECT_FALSE(BEnvironment::GetValue("FLEXEMU_TEST_VALUE", value));
    EXPECT_FALSE(BEnvironment::GetValue("flexemu_test_value", value));
    EXPECT_FALSE(BEnvironment::GetValue("FlexemuTestValue", value));
}

