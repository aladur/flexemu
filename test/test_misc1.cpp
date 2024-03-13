#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "misc1.h"
#include <sys/stat.h>


using ::testing::EndsWith;

TEST(test_misc1, fct_strlower)
{
    std::string str("abC012");
    strlower(str);
    EXPECT_EQ(str, "abc012");
    str = "ABC012";
    strlower(str);
    EXPECT_EQ(str, "abc012");
}

TEST(test_misc1, fct_strupper)
{
    std::string str("abC012");
    strupper(str);
    EXPECT_EQ(str, "ABC012");
    str = "ABC012";
    strupper(str);
    EXPECT_EQ(str, "ABC012");
}

TEST(test_misc1, fct_binstr)
{
    auto result = binstr(0x55);
    EXPECT_EQ(result, "01010101");
    result = binstr(0x00);
    EXPECT_EQ(result, "00000000");
    result = binstr(0x01);
    EXPECT_EQ(result, "00000001");
    result = binstr(0xFF);
    EXPECT_EQ(result, "11111111");
}

TEST(test_misc1, fct_hexstr_byte)
{
    auto result = hexstr(static_cast<Byte>(0x55));
    EXPECT_EQ(result, "55");
    result = hexstr(static_cast<Byte>(0x00));
    EXPECT_EQ(result, "00");
    result = hexstr(static_cast<Byte>(0x01));
    EXPECT_EQ(result, "01");
    result = hexstr(static_cast<Byte>(0xFF));
    EXPECT_EQ(result, "ff");
}

TEST(test_misc1, fct_hexstr_word)
{
    auto result = hexstr(static_cast<Word>(0x55AA));
    EXPECT_EQ(result, "55aa");
    result = hexstr(static_cast<Word>(0x0000));
    EXPECT_EQ(result, "0000");
    result = hexstr(static_cast<Word>(0x0001));
    EXPECT_EQ(result, "0001");
    result = hexstr(static_cast<Word>(0xFFFF));
    EXPECT_EQ(result, "ffff");
}

TEST(test_misc1, fct_ascchr)
{
    auto result = ascchr('a');
    EXPECT_EQ(result, "a");
    result = ascchr('3');
    EXPECT_EQ(result, "3");
    result = ascchr('\xa1');
    EXPECT_EQ(result, ".");
}

TEST(test_misc1, fct_stricmp)
{
    auto result = stricmp("AbCDeF", "abcdef");
    EXPECT_EQ(result, 0);
    result = stricmp("0AcdX", "0aCDx");
    EXPECT_EQ(result, 0);
    result = stricmp("abc", "acd");
    EXPECT_NE(result, 0);
}

TEST(test_misc1, fct_matches)
{
    auto result = matches("abcdef", "abc*", false);
    EXPECT_TRUE(result);
    result = matches("file.ext", "f*.?xt", false);
    EXPECT_TRUE(result);
    result = matches("file.a", "f*.???", false);
    EXPECT_FALSE(result);
    result = matches("abcdef", "ABC*", true);
    EXPECT_TRUE(result);
    result = matches("file.ext", "F*.?xt", true);
    EXPECT_TRUE(result);
    result = matches("file.a", "f*L*.???", true);
    EXPECT_FALSE(result);
}

#ifdef _WIN32
TEST(test_misc1, fct_getExecutablePath)
{
    struct stat sbuf;
    auto path = getExecutablePath();
    bool result = !stat(path.c_str(), &sbuf) && S_ISDIR(sbuf.st_mode);

    EXPECT_TRUE(result);
}
#endif

TEST(test_misc1, fct_getHomeDirectory)
{
    struct stat sbuf;
    auto path = getHomeDirectory();
    bool result = !stat(path.c_str(), &sbuf) && S_ISDIR(sbuf.st_mode);

    EXPECT_TRUE(result);
}

#ifdef _WIN32
TEST(test_misc1, fct_getExecutablePath)
{
    struct stat sbuf;
    auto path = getExecutablePath();
    bool result = !stat(path.c_str(), &sbuf) && S_ISDIR(sbuf.st_mode);

    EXPECT_TRUE(result);
}
#endif

TEST(test_misc1, fct_getTempPath)
{
    struct stat sbuf;
    auto path = getTempPath();
    bool result = !stat(path.c_str(), &sbuf) && S_ISDIR(sbuf.st_mode);

    EXPECT_TRUE(result);
}

TEST(test_misc1, fct_getCurrentPath)
{
    struct stat sbuf;
    auto path = getCurrentPath();
    bool result = !stat(path.c_str(), &sbuf) && S_ISDIR(sbuf.st_mode);

    EXPECT_TRUE(result);
}

TEST(test_misc1, fct_toAbsolutePath)
{
#ifdef _WIN32
    auto result = toAbsolutePath("C:\\Temp");
    EXPECT_EQ(result, "C:\\Temp");
    result = toAbsolutePath("tmp");
    EXPECT_THAT(result, EndsWith("\\tmp"));
#endif
#ifdef UNIX
    auto result = toAbsolutePath("/tmp");
    EXPECT_EQ(result, "/tmp");
    result = toAbsolutePath("tmp");
    EXPECT_THAT(result, EndsWith("/tmp"));
#endif
}

TEST(test_misc1, fct_getHostName)
{
    auto result = getHostName();
    EXPECT_FALSE(result.empty());
}

TEST(test_misc1, fct_getFileName)
{
    auto result = getFileName("");
    EXPECT_EQ(result, "");
    result = getFileName("filename.ext");
    EXPECT_EQ(result, "filename.ext");
    result = getFileName("filename.ext");
    EXPECT_EQ(result, "filename.ext");
    result = getFileName("filename");
    EXPECT_EQ(result, "filename");
    result = getFileName(".ext");
    EXPECT_EQ(result, ".ext");
#ifdef _WIN32
    result = getFileName("C:\\");
    EXPECT_EQ(result, "\\");
    result = getFileName("C:\\dir1\\dir2\\.");
    EXPECT_EQ(result, ".");
    result = getFileName("C:\\dir1\\dir2\\..");
    EXPECT_EQ(result, "..");
    result = getFileName("C:\\dir1\\dir2\\");
    EXPECT_EQ(result, ".");
    result = getFileName("C:\\dir1\\dir2");
    EXPECT_EQ(result, "dir2");
    result = getFileName("C:\\dir\\filename.ext");
    EXPECT_EQ(result, "filename.ext");
#endif
#ifdef UNIX
    result = getFileName("/");
    EXPECT_EQ(result, "/");
    result = getFileName("/dir1/dir2/.");
    EXPECT_EQ(result, ".");
    result = getFileName("/dir1/dir2/..");
    EXPECT_EQ(result, "..");
    result = getFileName("/dir1/dir2/");
    EXPECT_EQ(result, ".");
    result = getFileName("/dir1/dir2");
    EXPECT_EQ(result, "dir2");
    result = getFileName("/dir/filename.ext");
    EXPECT_EQ(result, "filename.ext");
#endif
}

TEST(test_misc1, fct_getFileExtension)
{
    auto result = getFileExtension("");
    EXPECT_EQ(result, "");
    result = getFileExtension("filename.ext");
    EXPECT_EQ(result, ".ext");
    result = getFileExtension("filename.tar.gz");
    EXPECT_EQ(result, ".gz");
    result = getFileExtension("filename");
    EXPECT_EQ(result, "");
    result = getFileExtension("filename...ext");
    EXPECT_EQ(result, ".ext");
#ifdef _WIN32
    result = getFileExtension("\\");
    EXPECT_EQ(result, "");
    result = getFileExtension("C:\\dir1\\dir2\\.");
    EXPECT_EQ(result, "");
    result = getFileExtension("C:\\dir1\\dir2\\..");
    EXPECT_EQ(result, "");
    result = getFileExtension("C:\\dir1\\dir2\\");
    EXPECT_EQ(result, "");
    result = getFileExtension("C:\\dir1\\dir2");
    EXPECT_EQ(result, "");
    result = getFileExtension("C:\\dir\\filename.ext");
    EXPECT_EQ(result, ".ext");
#endif
#ifdef UNIX
    result = getFileExtension("/");
    EXPECT_EQ(result, "");
    result = getFileExtension("/dir1/dir2/.");
    EXPECT_EQ(result, "");
    result = getFileExtension("/dir1/dir2/..");
    EXPECT_EQ(result, "");
    result = getFileExtension("/dir1/dir2/");
    EXPECT_EQ(result, "");
    result = getFileExtension("/dir1/dir2");
    EXPECT_EQ(result, "");
    result = getFileExtension("/dir/filename.ext");
    EXPECT_EQ(result, ".ext");
#endif
}

TEST(test_misc1, fct_getFileStem)
{
    auto result = getFileStem("");
    EXPECT_EQ(result, "");
    result = getFileStem("filename.ext");
    EXPECT_EQ(result, "filename");
    result = getFileStem("filename.tar.gz");
    EXPECT_EQ(result, "filename.tar");
    result = getFileStem("filename");
    EXPECT_EQ(result, "filename");
    result = getFileStem("filename...ext");
    EXPECT_EQ(result, "filename..");
#ifdef _WIN32
    result = getFileStem("C:\\");
    EXPECT_EQ(result, "\\");
    result = getFileStem("C:\\dir1\\dir2\\.");
    EXPECT_EQ(result, ".");
    result = getFileStem("C:\\dir1\\dir2\..");
    EXPECT_EQ(result, "..");
    result = getFileStem("C:\\dir1\\dir2\\");
    EXPECT_EQ(result, ".");
    result = getFileStem("C:\\dir1\dir2");
    EXPECT_EQ(result, "dir2");
    result = getFileStem("C:\\dir\\filename.ext");
    EXPECT_EQ(result, "filename");
#endif
#ifdef UNIX
    result = getFileStem("/");
    EXPECT_EQ(result, "/");
    result = getFileStem("/dir1/dir2/.");
    EXPECT_EQ(result, ".");
    result = getFileStem("/dir1/dir2/..");
    EXPECT_EQ(result, "..");
    result = getFileStem("/dir1/dir2/");
    EXPECT_EQ(result, ".");
    result = getFileStem("/dir1/dir2");
    EXPECT_EQ(result, "dir2");
    result = getFileStem("/dir/filename.ext");
    EXPECT_EQ(result, "filename");
#endif
}

TEST(test_misc1, fct_getParentPath)
{
    auto result = getParentPath("");
    EXPECT_EQ(result, "");
    result = getParentPath("filename.ext");
    EXPECT_EQ(result, "");
    result = getParentPath("filename");
    EXPECT_EQ(result, "");
#ifdef _WIN32
    result = getParentPath("C:\\");
    EXPECT_EQ(result, "");
    result = getParentPath("C:\\dir1\\dir2\\.");
    EXPECT_EQ(result, "C:\\dir1\\dir2");
    result = getParentPath("C:\\dir1\\dir2\\..");
    EXPECT_EQ(result, "C:\\dir1\\dir2");
    result = getParentPath("C:\\dir1\\dir2\\");
    EXPECT_EQ(result, "C:\\dir1\\dir2");
    result = getParentPath("C:\\dir1\\dir2");
    EXPECT_EQ(result, "C:\\dir1");
    result = getParentPath("C:\\dir\\filename.ext");
    EXPECT_EQ(result, "C:\\dir");
#endif
#ifdef UNIX
    result = getParentPath("/");
    EXPECT_EQ(result, "");
    result = getParentPath("/dir1/dir2/.");
    EXPECT_EQ(result, "/dir1/dir2");
    result = getParentPath("/dir1/dir2/..");
    EXPECT_EQ(result, "/dir1/dir2");
    result = getParentPath("/dir1/dir2/");
    EXPECT_EQ(result, "/dir1/dir2");
    result = getParentPath("/dir1/dir2");
    EXPECT_EQ(result, "/dir1");
    result = getParentPath("/dir/filename.ext");
    EXPECT_EQ(result, "/dir");
#endif
}

TEST(test_misc1, fct_isAbsolutePath)
{
#ifdef UNIX
    auto result = isAbsolutePath("");
    EXPECT_FALSE(result);
    result = isAbsolutePath("/usr");
    EXPECT_TRUE(result);
    result = isAbsolutePath("//SERVER/SHARE/rootdir");
    EXPECT_TRUE(result);
    result = isAbsolutePath("dir1/dir2");
    EXPECT_FALSE(result);
    result = isAbsolutePath(".");
    EXPECT_FALSE(result);
#endif
#ifdef _WIN32
    auto result = isAbsolutePath("");
    EXPECT_FALSE(result);
    result = isAbsolutePath("C:\\Temp");
    EXPECT_TRUE(result);
    result = isAbsolutePath("//SERVER/SHARE/rootdir");
    EXPECT_TRUE(result);
    result = isAbsolutePath("dir1\\dir2");
    EXPECT_FALSE(result);
    result = isAbsolutePath("\\dir1\\dir2");
    EXPECT_FALSE(result);
    result = isAbsolutePath(".");
    EXPECT_FALSE(result);
#endif
}

TEST(test_misc1, fct_endsWithPathSeparator)
{
#ifdef _WIN32
    auto result = endsWithPathSeparator("C:\\abc\\");
    EXPECT_TRUE(result);
    result = endsWithPathSeparator("C:\\abc");
    EXPECT_FALSE(result);
#endif
#ifdef UNIX
    auto result = endsWithPathSeparator("/abc/");
    EXPECT_TRUE(result);
    result = endsWithPathSeparator("/abc");
    EXPECT_FALSE(result);
#endif
}

TEST(test_misc1, fct_isPathsEqual)
{
#ifdef _WIN32
    auto result = isPathsEqual("C:\\Temp\\test.txt", "C:\\Temp\\Test.txt");
    EXPECT_TRUE(result);
    result = isPathsEqual("C:\\Temp\\test.txt", "C:\\Temp\\Test1.txt");
    EXPECT_FALSE(result);
#endif
#ifdef UNIX
    auto result = isPathsEqual("/usr/lib/libtest.so", "/usr/lib/libtest.so");
    EXPECT_TRUE(result);
    result = isPathsEqual("/usr/lib/libtest.so", "/usr/lib/libTest.so");
    EXPECT_FALSE(result);
#endif
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}

