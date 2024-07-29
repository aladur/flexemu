#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "binifile.h"
#include "flexerr.h"
#include <fstream>
#include <filesystem>

using ::testing::Throws;
namespace fs = std::filesystem;

void print(std::ostream &os, const std::string &section,
           const std::map<std::string, std::string> &map)
{
    os << "Section=\"" << section << "\"\n";
    for (const auto &[key, value] : map)
    {
        os << "  key=" << key << " value=" << value << "\n";
    }
}

bool createIniFile(const std::string &path)
{
    std::fstream ofs(path, std::ios::out | std::ios::trunc);
    bool retval = false;

    if (ofs.is_open())
    {
        ofs <<
            "key11=value11\n"
            "key12=value12\n"
            "[ ]\n"
            "key13=value13\n"
            "[SECTION2]\n"
            "key21=value21\n"
            "key22=value22\n"
            "[SECTION2]\n"
            "key23=value23\n"
            "[  Section._-~@ 3    ]   \n"
            "#  key30=value30\n"
            ";  key30=value30\n"
            "#\n"
            ";\n"
            "key31=value31\n"
            "key32 = value32\n"
            "key33     =     value 33     \n"
            "key34\t=\tvalue34    \n"
            "key35     \t=\t   value35\n"
            "key36=\n"
            "key37\n"
            "k=v\n"
            "[SECTION2]\n"
            "key24=value24\n"
            "[ ]\n"
            "key14=value14\n";
        retval = ofs.good();
        ofs.close();
        return retval;
    }

    return retval;
}

TEST(test_binifile, ctor)
{
    const std::string path{"/tmp/ini_file1.ini"};
    EXPECT_TRUE(createIniFile(path));
    BIniFile iniFile(path);
    EXPECT_TRUE(iniFile.IsValid());
    EXPECT_EQ(iniFile.GetFileName(), path);
    fs::remove(path);
}

TEST(test_binifile, move_ctor)
{
    const std::string path{"/tmp/ini_file1.ini"};
    EXPECT_TRUE(createIniFile(path));
    BIniFile iniFile1(path);
    EXPECT_TRUE(iniFile1.IsValid());
    EXPECT_EQ(iniFile1.GetFileName(), path);
    auto iniFile2(std::move(iniFile1));
    EXPECT_FALSE(iniFile1.IsValid());
    EXPECT_TRUE(iniFile1.GetFileName().empty());
    const auto map1 = iniFile1.ReadSection("SECTION2");
    EXPECT_TRUE(map1.empty());
    EXPECT_EQ(iniFile2.GetFileName(), path);
    const auto map2 = iniFile2.ReadSection("SECTION2");
    EXPECT_FALSE(map2.empty());
    fs::remove(path);
}

TEST(test_binifile, move_assignment)
{
    const std::string path{"/tmp/ini_file1.ini"};
    EXPECT_TRUE(createIniFile(path));
    BIniFile iniFile1(path);
    EXPECT_TRUE(iniFile1.IsValid());
    EXPECT_EQ(iniFile1.GetFileName(), path);
    auto iniFile2 = std::move(iniFile1);
    EXPECT_FALSE(iniFile1.IsValid());
    EXPECT_TRUE(iniFile1.GetFileName().empty());
    const auto map1 = iniFile1.ReadSection("SECTION2");
    EXPECT_TRUE(map1.empty());
    EXPECT_EQ(iniFile2.GetFileName(), path);
    const auto map2 = iniFile2.ReadSection("SECTION2");
    EXPECT_FALSE(map2.empty());
    fs::remove(path);
}

TEST(test_binifile, fct_ReadSection)
{
    const std::string path{"/tmp/ini_file1.ini"};
    EXPECT_TRUE(createIniFile(path));
    BIniFile iniFile(path);
    std::string section;
    const auto map1 = iniFile.ReadSection(section);
    EXPECT_EQ(map1.size(), 2U);
    EXPECT_EQ(map1.at("key11"), "value11");
    EXPECT_EQ(map1.at("key12"), "value12");
    section = "SECTION2";
    const auto map2 = iniFile.ReadSection(section);
    EXPECT_EQ(map2.size(), 2U);
    EXPECT_EQ(map2.at("key21"), "value21");
    EXPECT_EQ(map2.at("key22"), "value22");
    section = "Section._-~@ 3";
    const auto map3 = iniFile.ReadSection(section);
    EXPECT_EQ(map3.size(), 8U);
    EXPECT_EQ(map3.at("key31"), "value31");
    EXPECT_EQ(map3.at("key32"), "value32");
    EXPECT_EQ(map3.at("key33"), "value 33");
    EXPECT_EQ(map3.at("key34"), "value34");
    EXPECT_EQ(map3.at("key35"), "value35");
    EXPECT_TRUE(map3.at("key36").empty());
    EXPECT_TRUE(map3.at("key37").empty());
    EXPECT_EQ(map3.at("k"), "v");
    section = "NotExistingSection";
    const auto map4 = iniFile.ReadSection(section);
    EXPECT_TRUE(map4.empty());
    fs::remove(path);
}
TEST(test_binifile, fct_ReadSection_exceptions)
{
    const std::string path1{"/tmp/ini_file2.ini"};
    std::fstream ofs(path1, std::ios::out | std::ios::trunc);
    ASSERT_TRUE(ofs.is_open());
    ofs <<
        " [SectionWithIndentation]\n"
        "key11=value11\n";
    ofs.close();
    BIniFile iniFile1(path1);
    EXPECT_THAT([&](){ iniFile1.ReadSection("SectionWithIndentation"); },
            testing::Throws<FlexException>());
    fs::remove(path1);

    const std::string path2{"/tmp/ini_file3.ini"};
    ofs.open(path2, std::ios::out | std::ios::trunc);
    ASSERT_TRUE(ofs.is_open());
    ofs <<
        "[SectionWithIndentedKey]\n"
        " key11=value11\n";
    ofs.close();
    BIniFile iniFile2(path2);
    EXPECT_THAT([&](){ iniFile2.ReadSection("SectionWithIndentedKey"); },
            testing::Throws<FlexException>());
    fs::remove(path2);

    const std::string path3{"/tmp/ini_file4.ini"};
    ofs.open(path3, std::ios::out | std::ios::trunc);
    ASSERT_TRUE(ofs.is_open());
    ofs <<
        "[SectionWithIdenticalKeys]\n"
        "key11=value11a\n"
        "key11=value11b\n";
    ofs.close();
    BIniFile iniFile3(path3);
    EXPECT_THAT([&](){ iniFile3.ReadSection("SectionWithIdenticalKeys"); },
            testing::Throws<FlexException>());
    fs::remove(path3);

    const std::string path4{"/tmp/ini_file5.ini"};
    ofs.open(path4, std::ios::out | std::ios::trunc);
    ASSERT_TRUE(ofs.is_open());
    ofs <<
        "[SectionWithWhitespaceLine]\n"
        "   \t\t    \t  \t   \t\t\t\n";
    ofs.close();
    BIniFile iniFile4(path4);
    EXPECT_THAT([&](){ iniFile4.ReadSection("SectionWithWhitespaceLine"); },
            testing::Throws<FlexException>());
    fs::remove(path4);

    const std::string path5{"/tmp/ini_file6.ini"};
    ofs.open(path5, std::ios::out | std::ios::trunc);
    ASSERT_TRUE(ofs.is_open());
    ofs <<
        "[SectionWithEmptyLine]\n"
        "\n";
    ofs.close();
    BIniFile iniFile5(path5);
    EXPECT_THAT([&](){ iniFile5.ReadSection("SectionWithEmptyLine"); },
            testing::Throws<FlexException>());
    fs::remove(path5);
}
