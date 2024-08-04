#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "misc1.h"
#include "ffilebuf.h"
#include "fdirent.h"
#include "flexerr.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <numeric>
#include <algorithm>
//#include <fmt/format.h>


using ::testing::Throws;
namespace fs = std::filesystem;

/*
auto print_fct = [](const Byte b){
    std::cout << fmt::format("{:02X} ", static_cast<Word>(b));
};
*/

TEST(test_ffilebuf, fct_default_ctor)
{
    FlexFileBuffer ffb;
    EXPECT_TRUE(ffb.IsEmpty());
    EXPECT_EQ(ffb.GetFileSize(), 0U);
    EXPECT_EQ(ffb.GetAttributes(), 0U);
    EXPECT_EQ(ffb.GetSectorMap(), 0);
    EXPECT_FALSE(ffb.IsRandom());
    EXPECT_EQ(ffb.GetDate(), BDate{});
    EXPECT_EQ(ffb.GetTime(), BTime());
    EXPECT_EQ(ffb.GetFilename().empty(), true);
    const auto *p = static_cast<const Byte *>(ffb);
    EXPECT_EQ(p, nullptr);
    EXPECT_NO_THROW({ ffb.FillWith(); });
}

TEST(test_ffilebuf, fct_move_ctor)
{
    std::string test_file("test.txt");
    auto path = fs::temp_directory_path() / test_file;
    std::fstream ofs(path, std::ios::out | std::ios::trunc);

    EXPECT_TRUE(ofs.is_open());
    ofs << "testfile content move_ctor";
    ofs.close();

    FlexFileBuffer ffb_src;
    EXPECT_TRUE(ffb_src.ReadFromFile(path.c_str()));
    ffb_src.SetAttributes(25);
    ffb_src.SetSectorMap(899);
    ffb_src.SetDateTime(BDate{2, 11, 2004}, BTime{18, 26});

    // Execute move ctor.
    FlexFileBuffer ffb_tgt(std::move(ffb_src));

    // Check target.
    EXPECT_FALSE(ffb_tgt.IsEmpty());
    EXPECT_EQ(ffb_tgt.GetFileSize(), 26U);
    EXPECT_EQ(ffb_tgt.GetAttributes(), 25U);
    EXPECT_EQ(ffb_tgt.GetSectorMap(), 899);
    EXPECT_TRUE(ffb_tgt.IsRandom());
    EXPECT_EQ(ffb_tgt.GetDate(), BDate(2, 11, 2004));
    EXPECT_EQ(ffb_tgt.GetTime(), BTime(18, 26));
    const auto *p_tgt = static_cast<const Byte *>(ffb_tgt);
    EXPECT_NE(p_tgt, nullptr);
    const auto uc_test_file(flx::toupper(test_file));
    EXPECT_EQ(uc_test_file.compare(ffb_tgt.GetFilename()), 0);

    // Check source (should be all empty).
    EXPECT_TRUE(ffb_src.IsEmpty());
    EXPECT_EQ(ffb_src.GetFileSize(), 0U);
    EXPECT_EQ(ffb_src.GetAttributes(), 0U);
    EXPECT_EQ(ffb_src.GetSectorMap(), 0);
    EXPECT_FALSE(ffb_src.IsRandom());
    EXPECT_EQ(ffb_src.GetDate(), BDate{});
    EXPECT_EQ(ffb_src.GetTime(), BTime{});
    const auto *p_src = static_cast<const Byte *>(ffb_src);
    EXPECT_EQ(p_src, nullptr);

    fs::remove(path);
}

TEST(test_ffilebuf, fct_copy_ctor)
{
    std::string test_file("test.txt");
    auto path = fs::temp_directory_path() / test_file;
    std::fstream ofs(path, std::ios::out | std::ios::trunc);

    EXPECT_TRUE(ofs.is_open());
    ofs << "testfile content copy_ctor";
    ofs.close();

    FlexFileBuffer ffb_src;
    EXPECT_TRUE(ffb_src.ReadFromFile(path.c_str()));
    ffb_src.SetAttributes(99);
    ffb_src.SetSectorMap(7825);
    ffb_src.SetDateTime(BDate{13, 3, 2244}, BTime{9, 28});

    // Execute copy ctor.
    FlexFileBuffer ffb_tgt(ffb_src);

    // Check target.
    EXPECT_FALSE(ffb_tgt.IsEmpty());
    EXPECT_EQ(ffb_tgt.GetFileSize(), 26U);
    EXPECT_EQ(ffb_tgt.GetAttributes(), 99);
    EXPECT_EQ(ffb_tgt.GetSectorMap(), 7825);
    EXPECT_TRUE(ffb_tgt.IsRandom());
    EXPECT_EQ(ffb_tgt.GetDate(), BDate(13, 3, 2244));
    EXPECT_EQ(ffb_tgt.GetTime(), BTime(9, 28));
    const auto *p_tgt = static_cast<const Byte *>(ffb_tgt);
    EXPECT_NE(p_tgt, nullptr);
    auto uc_tgt(flx::toupper(test_file));
    EXPECT_EQ(uc_tgt.compare(ffb_tgt.GetFilename()), 0);

    // Check source (same as target).
    EXPECT_FALSE(ffb_src.IsEmpty());
    EXPECT_EQ(ffb_src.GetFileSize(), 26U);
    EXPECT_EQ(ffb_src.GetAttributes(), 99);
    EXPECT_EQ(ffb_src.GetSectorMap(), 7825);
    EXPECT_TRUE(ffb_src.IsRandom());
    EXPECT_EQ(ffb_src.GetDate(), BDate(13, 3, 2244));
    EXPECT_EQ(ffb_src.GetTime(), BTime(9, 28));
    const auto *p_src = static_cast<const Byte *>(ffb_tgt);
    EXPECT_NE(p_src, nullptr);
    auto uc_src(flx::toupper(test_file));
    EXPECT_EQ(uc_src.compare(ffb_src.GetFilename()), 0);

    fs::remove(path);
}

TEST(test_ffilebuf, fct_ReadFromFile)
{
    std::string test_file("test.txt");
    auto path = fs::temp_directory_path() / test_file;
    std::fstream ofs(path, std::ios::out | std::ios::trunc);

    EXPECT_TRUE(ofs.is_open());
    ofs << "testfile content ReadFromFile";
    ofs.close();

    FlexFileBuffer ffb;
    EXPECT_TRUE(ffb.ReadFromFile(path.c_str()));

    // Check properties.
    EXPECT_EQ(ffb.GetFileSize(), 29U);
    EXPECT_FALSE(ffb.IsEmpty());
    auto uc_test_file(flx::toupper(test_file));
    EXPECT_EQ(uc_test_file.compare(ffb.GetFilename()), 0);

    fs::remove(path);
}

TEST(test_ffilebuf, fct_WriteToFile)
{
    std::string test_file("test.txt");
    auto path = fs::temp_directory_path() / test_file;
    std::string content("testfile content WriteToFile");
    FlexFileBuffer ffb;
    ffb.Realloc(content.size() + 1);
    ffb.SetDateTime(BDate(15, 3, 1985), BTime(18, 12));
    ffb.CopyFrom(reinterpret_cast<const Byte *>(content.c_str()), content.size() + 1);
    EXPECT_TRUE(ffb.WriteToFile(path.c_str(), FileTimeAccess::NONE));

    // Check file on filesystem.
    EXPECT_TRUE(fs::exists(path));
    EXPECT_FALSE(fs::is_empty(path));
    EXPECT_EQ(fs::file_size(path), ffb.GetFileSize());
    auto status = fs::status(path);
    EXPECT_TRUE(fs::is_regular_file(status));

    fs::remove(path);
}

TEST(test_ffilebuf, fct_get_set)
{
    std::string test_file("test.txt");
    FlexFileBuffer ffb;
    EXPECT_TRUE(ffb.IsEmpty());
    EXPECT_FALSE(ffb.IsRandom());
    EXPECT_EQ(ffb.GetFileSize(), 0U);
    EXPECT_EQ(ffb.GetSectorMap(), 0);
    EXPECT_EQ(ffb.GetAttributes(), 0U);
    EXPECT_EQ(ffb.GetDate(), BDate{});
    EXPECT_EQ(ffb.GetTime(), BTime());
    EXPECT_EQ(ffb.GetFilename().empty(), true);

    ffb.Realloc(33U);
    ffb.SetFilename(test_file.c_str());
    BDate date(15, 12, 1984);
    BTime time(13, 28, 59);
    ffb.SetDateTime(date, time);
    ffb.SetSectorMap(IS_RANDOM_FILE);
    ffb.SetAttributes(FLX_READONLY | FLX_NOCAT);
    EXPECT_EQ(ffb.GetFileSize(), 33U);
    EXPECT_FALSE(ffb.IsEmpty());
    EXPECT_TRUE(ffb.IsRandom());
    EXPECT_EQ(ffb.GetSectorMap(), IS_RANDOM_FILE);
    EXPECT_EQ(ffb.GetAttributes(), FLX_READONLY | FLX_NOCAT);
    EXPECT_EQ(ffb.GetDate(), date);
    EXPECT_NE(ffb.GetTime(), time); // No support of seconds
    time.Set(13, 28, 0);
    EXPECT_EQ(ffb.GetTime(), time);

}

TEST(test_ffilebuf, fct_header_get_set)
{
    FlexFileBuffer ffb;
    tFlexFileHeader header_src{};
    memcpy(header_src.magicNumber,
           flexFileHeaderMagicNumber.data(),
           flexFileHeaderMagicNumber.size());
    header_src.fileSize = flx::toBigEndian(static_cast<DWord>(44U));
    header_src.attributes = flx::toBigEndian(static_cast<Word>(FLX_READONLY));
    header_src.sectorMap = flx::toBigEndian(static_cast<Word>(IS_RANDOM_FILE));
    header_src.day = flx::toBigEndian(static_cast<Word>(28U));
    header_src.month = flx::toBigEndian(static_cast<Word>(5U));
    header_src.year = flx::toBigEndian(static_cast<Word>(1999U));
    header_src.hour = flx::toBigEndian(static_cast<Word>(19U));
    header_src.minute = flx::toBigEndian(static_cast<Word>(27U));
    std::string test_file("test.txt");
    std::copy(test_file.cbegin(), test_file.cend(),
              std::begin(header_src.fileName));

    ffb.CopyHeaderBigEndianFrom(header_src);
    auto header_tgt = ffb.GetHeaderBigEndian();
    EXPECT_EQ(header_tgt.fileSize, flx::toBigEndian(static_cast<DWord>(44U)));
    EXPECT_EQ(header_tgt.attributes,
            flx::toBigEndian(static_cast<Word>(FLX_READONLY)));
    EXPECT_EQ(header_tgt.sectorMap,
            flx::toBigEndian(static_cast<Word>(IS_RANDOM_FILE)));
    EXPECT_EQ(header_tgt.day, flx::toBigEndian(static_cast<Word>(28U)));
    EXPECT_EQ(header_tgt.month, flx::toBigEndian(static_cast<Word>(5U)));
    EXPECT_EQ(header_tgt.year, flx::toBigEndian(static_cast<Word>(1999U)));
    EXPECT_EQ(header_tgt.hour, flx::toBigEndian(static_cast<Word>(19U)));
    EXPECT_EQ(header_tgt.minute, flx::toBigEndian(static_cast<Word>(27U)));
    EXPECT_EQ(test_file.compare(header_tgt.fileName), 0);

    header_src.magicNumber[0] = '\x44';
    EXPECT_THAT([&](){ ffb.CopyHeaderBigEndianFrom(header_src); },
            testing::Throws<FlexException>());
}

TEST(test_ffilebuf, fct_buffer_get_set)
{
    FlexFileBuffer ffb;
    int size = 4;
    ffb.Realloc(size);
    const auto *p = static_cast<const Byte *>(ffb);
    Byte expected = '\0';
    auto compare_fct = [&](const Byte b){ return b == expected; };
    EXPECT_TRUE(std::all_of(p, &p[size], compare_fct));
    expected = '\x55';
    ffb.FillWith(expected);
    EXPECT_TRUE(std::all_of(p, &p[size], compare_fct));
    size = 8;
    ffb.Realloc(size, true);
    p = static_cast<const Byte *>(ffb);
//    std::for_each(p, &p[size], print_fct);
//    std::cout << "\n";
    expected = '\x55'; // Index 0..3 contains '\x55'.
    EXPECT_TRUE(std::all_of(p, &p[4], compare_fct));
    expected = '\0'; // Index 4..7 contains '\0'.
    EXPECT_TRUE(std::all_of(&p[5], &p[size], compare_fct));
    size = 12;
    ffb.Realloc(size, false);
    p = static_cast<const Byte *>(ffb);
    expected = '\0';
    EXPECT_TRUE(std::all_of(p, &p[size], compare_fct));
    expected = '\xAA';
    ffb.FillWith(expected);
    size = 6;
    ffb.Realloc(size, true);
    EXPECT_TRUE(std::all_of(p, &p[size], compare_fct));
}

TEST(test_ffilebuf, fct_buffer_CopyFrom)
{
    FlexFileBuffer ffb;
    std::vector<Byte> data(16U);
    data.resize(16);
    std::iota(data.begin(), data.end(), '\0');

    // Copy from data with no allocated buffer
    EXPECT_FALSE(ffb.CopyFrom(data.data(), 1U));

    ffb.Realloc(16U);

    // Copy from data with size and offset (size + offset too large).
    EXPECT_FALSE(ffb.CopyFrom(data.data(), data.size(), 1U));

    // Copy from data with size.
    EXPECT_TRUE(ffb.CopyFrom(data.data(), data.size()));
    const Byte *p = static_cast<const Byte *>(ffb);
    EXPECT_TRUE(std::equal(p, p+16, data.cbegin()));

    // Copy from data with size and offset.
    EXPECT_TRUE(ffb.CopyFrom(data.data(), 8U, 8U));
    p = static_cast<const Byte *>(ffb);
    EXPECT_TRUE(std::equal(p, p + 8, data.cbegin()));
    EXPECT_TRUE(std::equal(p + 8, p + 16, data.cbegin()));

    EXPECT_THAT([&](){ ffb.CopyFrom(nullptr, 0); },
            testing::Throws<FlexException>());
}

TEST(test_ffilebuf, fct_buffer_CopyTo)
{
    FlexFileBuffer ffb;
    std::string data;
    data.resize(16);
    std::iota(data.begin(), data.end(), '\0');
    std::string test_file("test");
    auto path = fs::temp_directory_path() / test_file;
    std::fstream ofs(path, std::ios::out | std::ios::trunc);
    EXPECT_TRUE(ofs.is_open());
    ofs << data;
    ofs.close();

    // Check host text file.
    EXPECT_TRUE(ffb.ReadFromFile(path.c_str()));
    EXPECT_EQ(ffb.GetFileSize(), static_cast<DWord>(data.size()));
    std::vector<Byte> target(16U);
    std::vector<Byte> expected(16U);
    std::iota(expected.begin(), expected.end(), '\0');

    // Copy to target without offset.
    EXPECT_TRUE(ffb.CopyTo(target.data(), target.size(), 0U));
    EXPECT_TRUE(std::equal(target.cbegin(), target.cend(), expected.cbegin()));

    // Copy to target with size, offset and no stuff byte.
    EXPECT_FALSE(ffb.CopyTo(target.data(), target.size(), 5U));

    // Copy to target with size, offset (too large) and stuff byte.
    EXPECT_FALSE(ffb.CopyTo(target.data(), target.size(), 16U, '\0'));

    // Copy to target with size, offset and stuff byte.
    // (size + offset is too large for buffer).
    EXPECT_TRUE(ffb.CopyTo(target.data(), target.size(), 5U, 0xE5));
    std::iota(expected.begin(), expected.begin() + 11, '\x5');
    std::fill(expected.begin() + 11, expected.end(), '\xE5');
    EXPECT_TRUE(std::equal(target.cbegin(), target.cend(), expected.cbegin()));
    target.resize(11U);
    expected.resize(11U);
    std::iota(expected.begin(), expected.end(), '\x5');
    // Copy to target with size and offset (size + offset fits in buffer).
    EXPECT_TRUE(ffb.CopyTo(target.data(), target.size(), 5));
    EXPECT_TRUE(std::equal(target.cbegin(), target.cend(), expected.cbegin()));
    EXPECT_THAT([&](){ ffb.CopyTo(nullptr, 0); },
            testing::Throws<FlexException>());

    fs::remove(path);
}

TEST(test_ffilebuf, fct_ConvertToFlexTextFile)
{
    FlexFileBuffer ffb;
    std::string test_file("test.txt");
    auto path = fs::temp_directory_path() / test_file;
    std::fstream ofs(path, std::ios::out | std::ios::trunc);
    EXPECT_TRUE(ofs.is_open());
    ofs << "This    is\tline1\nline3\nAnd this is the last line\n";
    ofs.close();

    // Check host text file.
    EXPECT_TRUE(ffb.ReadFromFile(path.c_str()));
    EXPECT_EQ(ffb.GetFileSize(), 49U);
    EXPECT_TRUE(ffb.IsTextFile());
    EXPECT_TRUE(ffb.IsFlexTextFile()); // Could also be a FLEX text file.
    EXPECT_FALSE(ffb.IsFlexExecutableFile());

    // Check conversion from host to FLEX text file.
    ffb.ConvertToFlexTextFile();
    EXPECT_TRUE(ffb.WriteToFile(path.c_str(), FileTimeAccess::NONE));
    EXPECT_TRUE(ffb.ReadFromFile(path.c_str()));
    EXPECT_EQ(ffb.GetFileSize(), 48U);
    EXPECT_FALSE(ffb.IsTextFile());
    EXPECT_TRUE(ffb.IsFlexTextFile());
    EXPECT_FALSE(ffb.IsFlexExecutableFile());

    // Check FLEX space compression (ending without new line).
    ofs.open(path, std::ios::out | std::ios::trunc);
    EXPECT_TRUE(ofs.is_open());
    std::string much_spaces(128, ' ');
    ofs << much_spaces;
    //ofs << much_spaces << "\n";
    ofs.close();
    EXPECT_TRUE(ffb.ReadFromFile(path.c_str()));
    ffb.ConvertToFlexTextFile();
    EXPECT_TRUE(ffb.WriteToFile(path.c_str(), FileTimeAccess::NONE));
    EXPECT_TRUE(ffb.ReadFromFile(path.c_str()));
    EXPECT_EQ(ffb.GetFileSize(), 3U);
    EXPECT_TRUE(ffb.IsTextFile()); // Could also be a host text file.
    EXPECT_TRUE(ffb.IsFlexTextFile());
    EXPECT_FALSE(ffb.IsFlexExecutableFile());

    // Check FLEX space compression (ending with new line).
    ofs.open(path, std::ios::out | std::ios::trunc);
    EXPECT_TRUE(ofs.is_open());
    ofs << "text" << much_spaces << "text\n";
    ofs.close();
    EXPECT_TRUE(ffb.ReadFromFile(path.c_str()));
    ffb.ConvertToFlexTextFile();
    EXPECT_TRUE(ffb.WriteToFile(path.c_str(), FileTimeAccess::NONE));
    EXPECT_TRUE(ffb.ReadFromFile(path.c_str()));
    EXPECT_EQ(ffb.GetFileSize(), 12U);
    EXPECT_TRUE(ffb.IsTextFile()); // Could also be a host text file.
    EXPECT_TRUE(ffb.IsFlexTextFile());
    EXPECT_FALSE(ffb.IsFlexExecutableFile());

    // Check FLEX text file.
    ofs.open(path, std::ios::out | std::ios::trunc);
    EXPECT_TRUE(ofs.is_open());
    ofs << "text" << '\x9' << '\x4' << "text" << "\n" << "next line" <<
        '\r' << '\x18' << '\x0c' << "text\r\x1a";
    ofs.close();
    EXPECT_TRUE(ffb.ReadFromFile(path.c_str()));
    EXPECT_EQ(ffb.GetFileSize(), 29U);
    EXPECT_FALSE(ffb.IsTextFile());
    EXPECT_TRUE(ffb.IsFlexTextFile());
    EXPECT_FALSE(ffb.IsFlexExecutableFile());

    // Check conversion from FLEX to host text file.
    ffb.ConvertToTextFile();
    EXPECT_TRUE(ffb.WriteToFile(path.c_str(), FileTimeAccess::NONE));
    EXPECT_TRUE(ffb.ReadFromFile(path.c_str()));
    EXPECT_EQ(ffb.GetFileSize(), 27U);
    EXPECT_TRUE(ffb.IsTextFile());
    EXPECT_TRUE(ffb.IsFlexTextFile()); // Could also be a FLEX text file
    EXPECT_FALSE(ffb.IsFlexExecutableFile());

    fs::remove(path);
}

TEST(test_ffilebuf, fct_ConvertToDumpFile)
{
    FlexFileBuffer ffb;
    std::string test_file("test");
    auto path = fs::temp_directory_path() / test_file;
    std::fstream ofs(path, std::ios::out | std::ios::trunc);
    EXPECT_TRUE(ofs.is_open());
    ofs << '\x02' << '\xC1' << '\x00' << '\x03' <<
           '\x7E' << '\xF0' << '\x2D' <<
           '\x16' << '\xC1' << '\x00';
    ofs.close();
    // Check FLEX CMD file.
    EXPECT_TRUE(ffb.ReadFromFile(path.c_str()));
    EXPECT_EQ(ffb.GetFileSize(), 10U);
    EXPECT_FALSE(ffb.IsTextFile());
    EXPECT_FALSE(ffb.IsFlexTextFile());
    EXPECT_TRUE(ffb.IsFlexExecutableFile());

    // Check conversion from FLEX CMD file to dump file.
    ffb.ConvertToDumpFile(16U);
    EXPECT_TRUE(ffb.WriteToFile(path.c_str(), FileTimeAccess::NONE));
    EXPECT_TRUE(ffb.ReadFromFile(path.c_str()));
    EXPECT_EQ(ffb.GetFileSize(), 66U);
    EXPECT_TRUE(ffb.IsTextFile());
    EXPECT_TRUE(ffb.IsFlexTextFile());
    EXPECT_FALSE(ffb.IsFlexExecutableFile());
    fs::remove(path);

    test_file = "data/cat.cmd";
    path = fs::current_path() / test_file;
    // Check cat.cmd file.
    EXPECT_TRUE(ffb.ReadFromFile(path.c_str()));
    EXPECT_EQ(ffb.GetFileSize(), 756U);
    EXPECT_FALSE(ffb.IsTextFile());
    EXPECT_FALSE(ffb.IsFlexTextFile());
    EXPECT_TRUE(ffb.IsFlexExecutableFile());

    test_file = "test";
    path = fs::temp_directory_path() / test_file;
    // Check conversion of cat.cmd to dump file.
    ffb.ConvertToDumpFile(16U);
    EXPECT_TRUE(ffb.WriteToFile(path.c_str(), FileTimeAccess::NONE));
    EXPECT_TRUE(ffb.ReadFromFile(path.c_str()));
    EXPECT_EQ(ffb.GetFileSize(), 3444U);
    EXPECT_TRUE(ffb.IsTextFile());
    EXPECT_TRUE(ffb.IsFlexTextFile());
    EXPECT_FALSE(ffb.IsFlexExecutableFile());
    fs::remove(path);
}

TEST(test_ffilebuf, fct_bin_file)
{
    FlexFileBuffer ffb;
    std::string test_file("test");
    auto path = fs::temp_directory_path() / test_file;
    std::fstream ofs(path, std::ios::out | std::ios::trunc);
    EXPECT_TRUE(ofs.is_open());
    ofs << '\x05' << '\x44' << '\x02' << '\x43' <<
           '\xA5' << '\x5A' << '\x9C' << '\x66' <<
           '\x76' << '\x81' << '\x09' << '\x9E';
    ofs.close();
    // Check unspecified FLEX binary file.
    EXPECT_TRUE(ffb.ReadFromFile(path.c_str()));
    EXPECT_EQ(ffb.GetFileSize(), 12U);
    EXPECT_FALSE(ffb.IsTextFile());
    EXPECT_FALSE(ffb.IsFlexTextFile());
    EXPECT_FALSE(ffb.IsFlexExecutableFile());
    fs::remove(path);
}

