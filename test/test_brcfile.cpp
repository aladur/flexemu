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
//#include <fmt/format.h>

namespace fs = std::filesystem;

/*
auto print_fct = [](const Byte b){
    std::cout << fmt::format("{:02X} ", static_cast<Word>(b));
};
*/

TEST(test_brcfile, fct_Initialize)
{
    auto path = fs::temp_directory_path() / u8"test1.rc";
    std::ofstream ofs(path);

    ASSERT_TRUE(ofs.is_open());
    ofs << "Generate some file content\n";
    ofs.close();

    BRcFile rcf(path.u8string());
    // Check the Initialize truncates the existing file.
    EXPECT_EQ(rcf.Initialize(), BRC_NO_ERROR);

    std::ifstream ifs(path);
    std::string contents;
    ifs >> contents;
    ifs.close();
    EXPECT_TRUE(contents.empty());
    fs::remove(path);
}

TEST(test_brcfile, fct_GetValue)
{
    std::string svalue;
    int ivalue;
    auto path = fs::temp_directory_path() / u8"test2.rc";
    std::ofstream ofs(path);

    ASSERT_TRUE(ofs.is_open());
    ofs << "Key" << ' ' << "\"Value1\"\n";
    ofs << "Key.postfix" << "\t" << "\"Value2\"\n";
    ofs << "Key_has_underscore" << "\t\t" << "\"Value3\"\n";
    ofs << "KeyWithIndex01" << "\t \t" << "\"Value4\"\n";
    ofs << "key-with-dash" << " " << "\"Value5\"\n";
    ofs << "KeyWithEmptyValue" << " " << "\"\"\n";
    ofs << "KeyForNegIntValue" << "      " << -22 << "\n";
    ofs << "KeyForPosIntValue" << "      " << 577901267 << "\n";
    ofs.close();

    BRcFile rcf(path.u8string());
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
    EXPECT_EQ(rcf.GetValue("KeyForNegIntValue", ivalue), BRC_NO_ERROR);
    EXPECT_EQ(ivalue, -22);
    EXPECT_EQ(rcf.GetValue("KeyForPosIntValue", ivalue), BRC_NO_ERROR);
    EXPECT_EQ(ivalue, 577901267);
    EXPECT_EQ(rcf.GetValue("Key", ivalue), BRC_NO_INTEGER);
    fs::remove(path);
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
    BRcFile rcf(path.u8string());
    EXPECT_EQ(rcf.GetValues("KeyPrefix", result_map), BRC_NO_ERROR);
    EXPECT_EQ(result_map.size(), 5U);
    EXPECT_EQ(result_map.at("IndividualKey1"), "Value1");
    EXPECT_EQ(result_map.at("IndividualKey5"), "Value2");
    EXPECT_EQ(result_map.at("IndividualKey9"), "Value3");
    EXPECT_EQ(result_map.at("IndividualKey22"), "Value4");
    EXPECT_EQ(result_map.at("IndividualKey1999"), "Value5");
    fs::remove(path);
}

TEST(test_brcfile, fct_SetValue)
{
    auto path = fs::temp_directory_path() / u8"test4.rc";
    BRcFile rcf(path.u8string());

    EXPECT_EQ(rcf.SetValue("Key", "Value1"), BRC_NO_ERROR);
    EXPECT_EQ(rcf.SetValue("Key.postfix", "Value2"), BRC_NO_ERROR);
    EXPECT_EQ(rcf.SetValue("Key_has_underscore", "Value3"), BRC_NO_ERROR);
    EXPECT_EQ(rcf.SetValue("KeyWithIndex01", "Value4"), BRC_NO_ERROR);
    EXPECT_EQ(rcf.SetValue("key-with-dash", "Value5"), BRC_NO_ERROR);
    EXPECT_EQ(rcf.SetValue("KeyWithEmptyValue", ""), BRC_NO_ERROR);
    EXPECT_EQ(rcf.SetValue("KeyForNegIntValue", -22), BRC_NO_ERROR);
    EXPECT_EQ(rcf.SetValue("KeyForPosIntValue", 577901267), BRC_NO_ERROR);

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

    EXPECT_EQ(result_map.size(), 8U);
    EXPECT_EQ(result_map.at("Key"), "\"Value1\"");
    EXPECT_EQ(result_map.at("Key.postfix"), "\"Value2\"");
    EXPECT_EQ(result_map.at("Key_has_underscore"), "\"Value3\"");
    EXPECT_EQ(result_map.at("KeyWithIndex01"), "\"Value4\"");
    EXPECT_EQ(result_map.at("key-with-dash"), "\"Value5\"");
    EXPECT_EQ(result_map.at("KeyWithEmptyValue"), "\"\"");
    EXPECT_EQ(result_map.at("KeyForNegIntValue"), "-22");
    EXPECT_EQ(result_map.at("KeyForPosIntValue"), "577901267");
    fs::remove(path);
}

