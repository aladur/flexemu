#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "misc1.h"
#include "ffilebuf.h"
#include <filesystem>
#include <fstream>
#include <string.h>


namespace fs = std::filesystem;

TEST(test_ffilebuf, fct_default_ctor)
{
    // Execute default ctor.
    FlexFileBuffer ffb;

    const auto fileHeader = ffb.GetHeader();
    EXPECT_EQ(fileHeader.fileSize, 0U);
    EXPECT_EQ(fileHeader.attributes, 0U);
    EXPECT_EQ(fileHeader.sectorMap, 0U);
    EXPECT_EQ(fileHeader.day, 0U);
    EXPECT_EQ(fileHeader.month, 0U);
    EXPECT_EQ(fileHeader.year, 0U);
    EXPECT_EQ(fileHeader.hour, 0U);
    EXPECT_EQ(fileHeader.minute, 0U);
    EXPECT_EQ(fileHeader.fileName[0], '\0');
    EXPECT_EQ(ffb.GetFileSize(), 0U);
    const auto *p = static_cast<const Byte *>(ffb);
    EXPECT_EQ(p, nullptr);
    EXPECT_TRUE(ffb.IsEmpty());
}

TEST(test_ffilebuf, fct_ReadFromFile)
{
    std::string test_file("test.txt");
    std::filesystem::path path =
        std::filesystem::temp_directory_path() / test_file;
    std::fstream ofs(path, std::ios::out | std::ios::trunc);

    EXPECT_TRUE(ofs.is_open());
    ofs << "testfile content";
    ofs.close();

    FlexFileBuffer ffb_src;
    EXPECT_TRUE(ffb_src.ReadFromFile(path.c_str()));

    const auto fileHeader = ffb_src.GetHeader();
    EXPECT_EQ(fileHeader.fileSize, 16U);
    EXPECT_EQ(fileHeader.attributes, 0U);
    EXPECT_EQ(fileHeader.sectorMap, 0U);
    std::string str(fileHeader.fileName, FLEX_FILENAME_LENGTH);
    auto uc(test_file);
    strupper(uc);
    EXPECT_EQ(
        strncmp(fileHeader.fileName, uc.c_str(), FLEX_FILENAME_LENGTH), 0);
    EXPECT_EQ(ffb_src.GetFileSize(), 16U);
    EXPECT_FALSE(ffb_src.IsEmpty());
}

TEST(test_ffilebuf, fct_move_ctor)
{
    std::string test_file("test.txt");
    std::filesystem::path path =
        std::filesystem::temp_directory_path() / test_file;
    std::fstream ofs(path, std::ios::out | std::ios::trunc);

    EXPECT_TRUE(ofs.is_open());
    ofs << "testfile content move_ctor";
    ofs.close();

    FlexFileBuffer ffb_src;
    EXPECT_TRUE(ffb_src.ReadFromFile(path.c_str()));

    // Execute move ctor.
    FlexFileBuffer ffb_tgt(std::move(ffb_src));

    const auto fileHeader_tgt = ffb_tgt.GetHeader();
    EXPECT_EQ(fileHeader_tgt.fileSize, 26U);
    EXPECT_EQ(fileHeader_tgt.attributes, 0U);
    EXPECT_EQ(fileHeader_tgt.sectorMap, 0U);
    auto uc(test_file);
    strupper(uc);
    EXPECT_EQ(
        strncmp(fileHeader_tgt.fileName, uc.c_str(), FLEX_FILENAME_LENGTH), 0);
    EXPECT_EQ(ffb_tgt.GetFileSize(), 26U);
    EXPECT_FALSE(ffb_tgt.IsEmpty());
    const auto *p_tgt = static_cast<const Byte *>(ffb_tgt);
    EXPECT_NE(p_tgt, nullptr);

    const auto fileHeader_src = ffb_src.GetHeader();
    EXPECT_EQ(fileHeader_src.fileSize, 0U);
    EXPECT_EQ(fileHeader_src.attributes, 0U);
    EXPECT_EQ(fileHeader_src.sectorMap, 0U);
    EXPECT_EQ(fileHeader_src.day, 0U);
    EXPECT_EQ(fileHeader_src.month, 0U);
    EXPECT_EQ(fileHeader_src.year, 0U);
    EXPECT_EQ(fileHeader_src.hour, 0U);
    EXPECT_EQ(fileHeader_src.minute, 0U);
    EXPECT_EQ(fileHeader_src.fileName[0], '\0');
    EXPECT_EQ(ffb_src.GetFileSize(), 0U);
    EXPECT_TRUE(ffb_src.IsEmpty());
    const auto *p_src = static_cast<const Byte *>(ffb_src);
    EXPECT_EQ(p_src, nullptr);
}

TEST(test_ffilebuf, fct_copy_ctor)
{
    std::string test_file("test.txt");
    std::filesystem::path path =
        std::filesystem::temp_directory_path() / test_file;
    std::fstream ofs(path, std::ios::out | std::ios::trunc);

    EXPECT_TRUE(ofs.is_open());
    ofs << "testfile content copy_ctor";
    ofs.close();

    FlexFileBuffer ffb_src;
    EXPECT_TRUE(ffb_src.ReadFromFile(path.c_str()));

    // Execute copy ctor.
    FlexFileBuffer ffb_tgt(ffb_src);

    const auto fileHeader_tgt = ffb_tgt.GetHeader();
    EXPECT_EQ(fileHeader_tgt.fileSize, 26U);
    EXPECT_EQ(fileHeader_tgt.attributes, 0U);
    EXPECT_EQ(fileHeader_tgt.sectorMap, 0U);

    auto uc(test_file);
    strupper(uc);
    EXPECT_EQ(
        strncmp(fileHeader_tgt.fileName, uc.c_str(), FLEX_FILENAME_LENGTH), 0);
    EXPECT_EQ(ffb_tgt.GetFileSize(), 26U);
    EXPECT_FALSE(ffb_tgt.IsEmpty());
    const auto *p_tgt = static_cast<const Byte *>(ffb_src);
    EXPECT_NE(p_tgt, nullptr);

    const auto fileHeader_src = ffb_src.GetHeader();
    EXPECT_EQ(fileHeader_src.fileSize, 26U);
    EXPECT_EQ(fileHeader_src.attributes, 0U);
    EXPECT_EQ(fileHeader_src.sectorMap, 0U);
    EXPECT_EQ(
        strncmp(fileHeader_src.fileName, uc.c_str(), FLEX_FILENAME_LENGTH), 0);
    EXPECT_EQ(ffb_src.GetFileSize(), 26U);
    EXPECT_FALSE(ffb_src.IsEmpty());
    const auto *p_src = static_cast<const Byte *>(ffb_src);
    EXPECT_NE(p_src, nullptr);
}

