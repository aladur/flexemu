/*
    test_brcfile.cpp


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
#include "misc1.h"
#include "brcfile.h"
#include <string>
#include <map>
#include <sstream>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

TEST(test_brcfile, fct_Initialize)
{
    auto path = fs::temp_directory_path() / u8"test1.rc";
    std::ofstream ofs(path);

    ASSERT_TRUE(ofs.is_open());
    ofs << "Generate some file content\n";
    ofs.close();

    BRcFile rcf(path);
    // Check the Initialize truncates the existing file.
    EXPECT_EQ(rcf.Initialize(), BRC_NO_ERROR);

    std::ifstream ifs(path);
    std::string contents;
    ifs >> contents;
    ifs.close();
    EXPECT_TRUE(contents.empty());
    fs::remove(path);

    BRcFile rcnof(fs::temp_directory_path() / "non_existent_dir" / "file.rc");
    EXPECT_EQ(rcnof.Initialize(), BRC_FILE_ERROR);
}

TEST(test_brcfile, fct_GetValue)
{
    std::string svalue;
    fs::path pvalue;
    int ivalue;
    auto path = fs::temp_directory_path() / u8"test2.rc";
    std::string tempdir = fs::temp_directory_path().u8string();
    std::ofstream ofs(path);

    ASSERT_TRUE(ofs.is_open());
    ofs << "Key" << ' ' << "\"Value1\"\n";
    ofs << "Key.postfix" << "\t" << "\"Value2\"\n";
    ofs << "Key_has_underscore" << "\t\t" << "\"Value3\"\n";
    ofs << "KeyWithIndex01" << "\t \t" << "\"Value4\"\n";
    ofs << "key-with-dash" << " " << "\"Value5\"\n";
    ofs << "KeyWithEmptyValue" << " " << "\"\"\n";
    ofs << "KeyForPath" << "      " << std::quoted(tempdir) << "\n";
    ofs << "KeyForNegIntValue" << "      " << -22 << "\n";
    ofs << "KeyForPosIntValue" << "      " << 577901267 << "\n";
    ofs.close();

    BRcFile rcf(path);
    EXPECT_EQ(rcf.GetValue("Key", svalue), BRC_NO_ERROR);
    EXPECT_EQ(svalue, "Value1");
    EXPECT_EQ(rcf.GetValue("Key.postfix", svalue), BRC_NO_ERROR);
    EXPECT_EQ(svalue, "Value2");
    EXPECT_EQ(rcf.GetValue("Key_has_underscore", svalue), BRC_NO_ERROR);
    EXPECT_EQ(svalue, "Value3");
    EXPECT_EQ(rcf.GetValue("KeyWithIndex01", svalue), BRC_NO_ERROR);
    EXPECT_EQ(svalue, "Value4");
    EXPECT_EQ(rcf.GetValue("key-with-dash", svalue), BRC_NO_ERROR);
    EXPECT_EQ(svalue, "Value5");
    EXPECT_EQ(rcf.GetValue("KeyWithEmptyValue", svalue), BRC_NO_ERROR);
    EXPECT_EQ(svalue, "");
    EXPECT_EQ(rcf.GetValue("KeyForPath", pvalue), BRC_NO_ERROR);
    EXPECT_EQ(pvalue, fs::temp_directory_path());
    EXPECT_EQ(rcf.GetValue("KeyForNegIntValue", ivalue), BRC_NO_ERROR);
    EXPECT_EQ(ivalue, -22);
    EXPECT_EQ(rcf.GetValue("KeyForPosIntValue", ivalue), BRC_NO_ERROR);
    EXPECT_EQ(ivalue, 577901267);
    EXPECT_EQ(rcf.GetValue("Key", ivalue), BRC_NO_INTEGER);
    EXPECT_EQ(ivalue, 577901267);
    EXPECT_EQ(rcf.GetValue("Key", ivalue), BRC_NO_INTEGER);
    EXPECT_EQ(ivalue, 577901267);
    svalue = "abc";
    EXPECT_EQ(rcf.GetValue("InvalidKey", svalue), BRC_NOT_FOUND);
    EXPECT_EQ(svalue, "abc");
    pvalue = fs::u8path("my_path");
    EXPECT_EQ(rcf.GetValue("InvalidKey", pvalue), BRC_NOT_FOUND);
    EXPECT_EQ(pvalue, "my_path");
    EXPECT_EQ(rcf.GetValue("InvalidKey", ivalue), BRC_NOT_FOUND);
    EXPECT_EQ(ivalue, 577901267);
    fs::remove(path);

    BRcFile rcnof(fs::temp_directory_path() / "non_existent_file.rc");
    svalue = "abc";
    EXPECT_EQ(rcnof.GetValue("Key", svalue), BRC_FILE_ERROR);
    EXPECT_EQ(svalue, "abc");
    pvalue = fs::u8path("my_path");
    EXPECT_EQ(rcnof.GetValue("Key", pvalue), BRC_FILE_ERROR);
    EXPECT_EQ(pvalue, "my_path");
    ivalue = 123;
    EXPECT_EQ(rcnof.GetValue("Key", ivalue), BRC_FILE_ERROR);
    EXPECT_EQ(ivalue, 123);
}

TEST(test_brcfile, fct_GetValues)
{
    auto path = fs::temp_directory_path() / u8"test3.rc";
    std::ofstream ofs(path);

    ASSERT_TRUE(ofs.is_open());
    ofs << "KeyPrefixIndividualKey1" << ' ' << "\"Value1\"" << "\n";
    ofs << "KeyPrefixIndividualKey5" << "\t" << "\"Value2\"" << "\n";
    ofs << "KeyPrefixIndividualKey9" << "\t\t" << "\"Value3\"" << "\n";
    ofs << "KeyPrefixIndividualKey22" << "\t \t" << "\"Value4\"" << "\n";
    ofs << "KeyPrefixIndividualKey1999" << "   " << "\"Value5\"" << "\n";
    ofs.close();

    std::map<std::string, std::string> result_map;
    BRcFile rcf(path);
    EXPECT_EQ(rcf.GetValues("KeyPrefix", result_map), BRC_NO_ERROR);
    EXPECT_EQ(result_map.size(), 5U);
    EXPECT_EQ(result_map.at("IndividualKey1"), "Value1");
    EXPECT_EQ(result_map.at("IndividualKey5"), "Value2");
    EXPECT_EQ(result_map.at("IndividualKey9"), "Value3");
    EXPECT_EQ(result_map.at("IndividualKey22"), "Value4");
    EXPECT_EQ(result_map.at("IndividualKey1999"), "Value5");
    fs::remove(path);

    BRcFile rcnof(fs::temp_directory_path() / "non_existent_file.rc");
    EXPECT_EQ(rcnof.GetValues("KeyPrefix", result_map), BRC_FILE_ERROR);
    EXPECT_EQ(result_map.size(), 5U);
    EXPECT_EQ(result_map.at("IndividualKey1"), "Value1");
}

TEST(test_brcfile, fct_SetValue)
{
    auto path = fs::temp_directory_path() / u8"test4.rc";
    BRcFile rcf(path);

    EXPECT_EQ(rcf.SetValue("Key", std::string("Value1")), BRC_NO_ERROR);
    int result = rcf.SetValue("Key.postfix", std::string("Value2"));
    EXPECT_EQ(result, BRC_NO_ERROR);
    result = rcf.SetValue("Key_has_underscore", std::string("Value3"));
    EXPECT_EQ(result, BRC_NO_ERROR);
    result = rcf.SetValue("KeyWithIndex01", std::string("Value4"));
    EXPECT_EQ(result, BRC_NO_ERROR);
    result = rcf.SetValue("key-with-dash", std::string("Value5"));
    EXPECT_EQ(result, BRC_NO_ERROR);
    result = rcf.SetValue("KeyWithEmptyValue", std::string(""));
    EXPECT_EQ(result, BRC_NO_ERROR);
    EXPECT_EQ(rcf.SetValue("KeyForNegIntValue", -22), BRC_NO_ERROR);
    EXPECT_EQ(rcf.SetValue("KeyForPosIntValue", 577901267), BRC_NO_ERROR);
    result = rcf.SetValue("KeyForPath", fs::temp_directory_path());
    EXPECT_EQ(result, BRC_NO_ERROR);

    // Now parse and check the created rc-file.
    std::map<std::string, std::string> result_map;
    std::ifstream ifs(path);
    ASSERT_TRUE(ifs.is_open());
    while (!ifs.eof())
    {
        std::stringbuf strbuf;
        std::string key;

        ifs >> key;
        ifs.get(strbuf);
        ifs.get(); // skip delimiter char.
        const auto value = flx::trim(strbuf.str());
        if (!key.empty())
        {
            result_map.emplace(key, value);
        }
    }
    ifs.close();

    EXPECT_EQ(result_map.size(), 9U);
    EXPECT_EQ(result_map.at("Key"), "\"Value1\"");
    EXPECT_EQ(result_map.at("Key.postfix"), "\"Value2\"");
    EXPECT_EQ(result_map.at("Key_has_underscore"), "\"Value3\"");
    EXPECT_EQ(result_map.at("KeyWithIndex01"), "\"Value4\"");
    EXPECT_EQ(result_map.at("key-with-dash"), "\"Value5\"");
    EXPECT_EQ(result_map.at("KeyWithEmptyValue"), "\"\"");
    std::stringstream stream;
    stream << fs::temp_directory_path();
    EXPECT_EQ(result_map.at("KeyForPath"), stream.str());
    EXPECT_EQ(result_map.at("KeyForNegIntValue"), "-22");
    EXPECT_EQ(result_map.at("KeyForPosIntValue"), "577901267");
    fs::remove(path);

    BRcFile rcnof(fs::temp_directory_path() / "non_existent_dir" / "file.rc");
    EXPECT_EQ(rcnof.SetValue("Key", std::string("Value1")), BRC_FILE_ERROR);
    EXPECT_EQ(rcnof.SetValue("Key", 577901267), BRC_FILE_ERROR);
    EXPECT_EQ(rcnof.SetValue("Key", fs::u8path("my_path")), BRC_FILE_ERROR);
}

