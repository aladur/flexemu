#include "gtest/gtest.h"
#include "misc1.h"


TEST(test_misc1, fct_getFileName)
{
    auto result = getFileName("");
    EXPECT_EQ(result, "");
    result = getFileName("/");
    EXPECT_EQ(result, "/");
    result = getFileName("/dir/.");
    EXPECT_EQ(result, ".");
    result = getFileName("/dir/..");
    EXPECT_EQ(result, "..");
    result = getFileName("/dir/dir2/");
    EXPECT_EQ(result, ".");
    result = getFileName("/dir/dir2");
    EXPECT_EQ(result, "dir2");
    result = getFileName("/dir/filename.ext");
    EXPECT_EQ(result, "filename.ext");
    result = getFileName("filename.ext");
    EXPECT_EQ(result, "filename.ext");
    result = getFileName("filename");
    EXPECT_EQ(result, "filename");
    result = getFileName("/dir/.");
    EXPECT_EQ(result, ".");
    result = getFileName("/dir/..");
    EXPECT_EQ(result, "..");
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}

