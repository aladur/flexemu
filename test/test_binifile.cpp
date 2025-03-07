/*
    test_binifile.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2024-2025  W. Schwotzer

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "binifile.h"
#include "flexerr.h"
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

static void print(std::ostream &os, const std::string &section,
           const std::map<std::string, std::string> &map)
{
    os << "Section=\"" << section << "\"\n";
    for (const auto &[key, value] : map)
    {
        os << "  key=" << key << " value=" << value << "\n";
    }
}

static bool createIniFile(const fs::path &path)
{
    std::ofstream ofs(path);
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
    const auto path = fs::temp_directory_path() / u8"ini_file1.ini";
    EXPECT_TRUE(createIniFile(path));
    BIniFile iniFile(path.u8string());
    EXPECT_TRUE(iniFile.IsValid());
    EXPECT_EQ(iniFile.GetFileName(), path);
    fs::remove(path);
}

TEST(test_binifile, move_ctor)
{
    const auto path = fs::temp_directory_path() / u8"ini_file1.ini";
    EXPECT_TRUE(createIniFile(path));
    BIniFile iniFile1(path.u8string());
    EXPECT_TRUE(iniFile1.IsValid());
    EXPECT_EQ(iniFile1.GetFileName(), path);
    auto iniFile2(std::move(iniFile1));
    /* Intentionally test object after move. */
    /* NOLINTBEGIN(bugprone-use-after-move) */
    EXPECT_FALSE(iniFile1.IsValid());
    EXPECT_TRUE(iniFile1.GetFileName().empty());
    const auto map1 = iniFile1.ReadSection("SECTION2");
    /* Intentionally test object after move. */
    /* NOLINTEND(bugprone-use-after-move) */
    EXPECT_TRUE(map1.empty());
    EXPECT_EQ(iniFile2.GetFileName(), path);
    const auto map2 = iniFile2.ReadSection("SECTION2");
    EXPECT_FALSE(map2.empty());
    fs::remove(path);
}

TEST(test_binifile, move_assignment)
{
    const auto path = fs::temp_directory_path() / u8"ini_file1.ini";
    EXPECT_TRUE(createIniFile(path));
    BIniFile iniFile1(path.u8string());
    EXPECT_TRUE(iniFile1.IsValid());
    EXPECT_EQ(iniFile1.GetFileName(), path);
    auto iniFile2 = std::move(iniFile1);
    /* Intentionally test object after move. */
    /* NOLINTBEGIN(bugprone-use-after-move) */
    EXPECT_FALSE(iniFile1.IsValid());
    EXPECT_TRUE(iniFile1.GetFileName().empty());
    const auto map1 = iniFile1.ReadSection("SECTION2");
    /* Intentionally test object after move. */
    /* NOLINTEND(bugprone-use-after-move) */
    EXPECT_TRUE(map1.empty());
    EXPECT_EQ(iniFile2.GetFileName(), path);
    const auto map2 = iniFile2.ReadSection("SECTION2");
    EXPECT_FALSE(map2.empty());
    fs::remove(path);
}

TEST(test_binifile, fct_ReadSection)
{
    const auto path = fs::temp_directory_path() / u8"ini_file1.ini";
    EXPECT_TRUE(createIniFile(path));
    BIniFile iniFile(path.u8string());
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
TEST(test_binifile, fct_GetLineNumber)
{
    const auto path = fs::temp_directory_path() / u8"ini_file1.ini";
    EXPECT_TRUE(createIniFile(path));
    BIniFile iniFile(path.u8string());
    auto lineNumber = iniFile.GetLineNumber("", "key12");
    EXPECT_EQ(lineNumber, 2);
    lineNumber = iniFile.GetLineNumber("SECTION2", "key21");
    EXPECT_EQ(lineNumber, 6);
    lineNumber = iniFile.GetLineNumber("Section._-~@ 3", "key31");
    EXPECT_EQ(lineNumber, 15);
    lineNumber = iniFile.GetLineNumber("InvalidSection", "key11");
    EXPECT_EQ(lineNumber, 0);
    lineNumber = iniFile.GetLineNumber("SECTION2", "invalidKey");
    EXPECT_EQ(lineNumber, 0);
    lineNumber = iniFile.GetLineNumber("SECTION2", "key24");
    EXPECT_EQ(lineNumber, 0);
    fs::remove(path);
}
TEST(test_binifile, fct_ReadSection_exceptions)
{
    const auto path1 = fs::temp_directory_path() / u8"ini_file1.ini";
    std::ofstream ofs(path1);
    ASSERT_TRUE(ofs.is_open());
    ofs <<
        " [SectionWithIndentation]\n"
        "key11=value11\n";
    ofs.close();
    BIniFile iniFile1(path1.u8string());
    EXPECT_THAT([&](){ iniFile1.ReadSection("SectionWithIndentation"); },
            testing::Throws<FlexException>());
    fs::remove(path1);

    const auto path2 = fs::temp_directory_path() / u8"ini_file2.ini";
    ofs.open(path2);
    ASSERT_TRUE(ofs.is_open());
    ofs <<
        "[SectionWithIndentedKey]\n"
        " key11=value11\n";
    ofs.close();
    BIniFile iniFile2(path2.u8string());
    EXPECT_THAT([&](){ iniFile2.ReadSection("SectionWithIndentedKey"); },
            testing::Throws<FlexException>());
    fs::remove(path2);

    const auto path3 = fs::temp_directory_path() / u8"ini_file3.ini";
    ofs.open(path3);
    ASSERT_TRUE(ofs.is_open());
    ofs <<
        "[SectionWithIdenticalKeys]\n"
        "key11=value11a\n"
        "key11=value11b\n";
    ofs.close();
    BIniFile iniFile3(path3.u8string());
    EXPECT_THAT([&](){ iniFile3.ReadSection("SectionWithIdenticalKeys"); },
            testing::Throws<FlexException>());
    fs::remove(path3);

    const auto path4 = fs::temp_directory_path() / u8"ini_file4.ini";
    ofs.open(path4);
    ASSERT_TRUE(ofs.is_open());
    ofs <<
        "[SectionWithWhitespaceLine]\n"
        "   \t\t    \t  \t   \t\t\t\n";
    ofs.close();
    BIniFile iniFile4(path4.u8string());
    EXPECT_THAT([&](){ iniFile4.ReadSection("SectionWithWhitespaceLine"); },
            testing::Throws<FlexException>());
    fs::remove(path4);

    const auto path5 = fs::temp_directory_path() / u8"ini_file5.ini";
    ofs.open(path5);
    ASSERT_TRUE(ofs.is_open());
    ofs <<
        "[SectionWithEmptyLine]\n"
        "\n";
    ofs.close();
    BIniFile iniFile5(path5.u8string());
    EXPECT_THAT([&](){ iniFile5.ReadSection("SectionWithEmptyLine"); },
            testing::Throws<FlexException>());
    fs::remove(path5);
}
