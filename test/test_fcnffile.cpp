/*
    test_fcnffile.cpp


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
#include "fcnffile.h"
#include "flexerr.h"
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

static bool createCnfFile(const std::string &path)
{
    std::ofstream ofs(path);
    bool retval = false;

    if (ofs.is_open())
    {
        ofs <<
            "[IoDevices]\n"
            "mmu=FFFF\n"
            "pia1=FE00\n"
            "acia1=FC00\n"
            "vico1=FD20\n"
            "rtc=FDD0,25\n"
            "pia2=0,1000\n"
            "[IoDeviceLogging]\n"
            "logFilePath=/dir/subdir/subdir/file.log\n"
            "devices=mmu,pia1,acia1\n"
            "[SERPARAddress]\n"
            "neumon54.hex=EE09\n"
            "mon54.s19=EE20\n"
            "mon54-6.s19=EE30\n"
            "[DebugSupport]\n"
            "presetRAM=1\n"
            "logMdcr=1\n"
            "logMdcrFilePath=/dir/subdir/subdir/mdcr.log\n";
        retval = ofs.good();
        ofs.close();
        return retval;
    }

    return retval;
}

TEST(test_fcnffile, ctor)
{
    const auto path1 = (fs::temp_directory_path() / "cnf1.conf").u8string();
    EXPECT_TRUE(createCnfFile(path1));
    FlexemuConfigFile cnfFile1(path1);
    EXPECT_TRUE(cnfFile1.IsValid());
    EXPECT_EQ(cnfFile1.GetFileName(), path1);
    fs::remove(path1);

    const std::string path2{R"(\\/\\/\\///\\/)"};
    EXPECT_THAT([&](){ FlexemuConfigFile cnfFile2(path2); },
            testing::Throws<FlexException>());

    const auto path3 =
        (fs::temp_directory_path() / "not_existent_file.conf").u8string();
    EXPECT_THAT([&](){ FlexemuConfigFile cnfFile3(path3); },
            testing::Throws<FlexException>());
}

TEST(test_fcnffile, move_ctor)
{
    const auto path = (fs::temp_directory_path() / "cnf2.conf").u8string();
    EXPECT_TRUE(createCnfFile(path));
    FlexemuConfigFile cnfFile1(path);
    auto cnfFile2(std::move(cnfFile1));

    /* Intentionally test object after move. */
    /* NOLINTBEGIN(bugprone-use-after-move) */
    EXPECT_TRUE(cnfFile1.GetFileName().empty());
    EXPECT_FALSE(cnfFile1.IsValid());
    /* NOLINTEND(bugprone-use-after-move) */
    EXPECT_TRUE(cnfFile2.IsValid());
    EXPECT_EQ(cnfFile2.ReadIoDevices().size(), 6U);
    EXPECT_EQ(cnfFile2.GetFileName(), path);
    fs::remove(path);
}

TEST(test_fcnffile, move_assignment)
{
    const auto path = (fs::temp_directory_path() / "cnf3.conf").u8string();
    EXPECT_TRUE(createCnfFile(path));
    FlexemuConfigFile cnfFile1(path);
    auto cnfFile2 = std::move(cnfFile1);

    /* Intentionally test object after move. */
    /* NOLINTBEGIN(bugprone-use-after-move) */
    EXPECT_TRUE(cnfFile1.GetFileName().empty());
    EXPECT_FALSE(cnfFile1.IsValid());
    /* NOLINTEND(bugprone-use-after-move) */
    EXPECT_TRUE(cnfFile2.IsValid());
    EXPECT_EQ(cnfFile2.GetFileName(), path);
    EXPECT_EQ(cnfFile2.ReadIoDevices().size(), 6U);
    fs::remove(path);
}

TEST(test_fcnffile, fct_ReadIoDevices)
{
    const auto path = (fs::temp_directory_path() / "cnf4.conf").u8string();
    ASSERT_TRUE(createCnfFile(path));
    FlexemuConfigFile cnfFile(path);
    ASSERT_TRUE(cnfFile.IsValid());
    const auto deviceMappings = cnfFile.ReadIoDevices();
    EXPECT_EQ(deviceMappings.size(), 6U);
    for (const auto &deviceParams : deviceMappings)
    {
        if (deviceParams.name == "mmu")
        {
            EXPECT_EQ(deviceParams.baseAddress, 0xFFFF);
            EXPECT_EQ(deviceParams.byteSize, -1);
        }
        else if (deviceParams.name == "pia1")
        {
            EXPECT_EQ(deviceParams.baseAddress, 0xFE00);
            EXPECT_EQ(deviceParams.byteSize, -1);
        }
        else if (deviceParams.name == "acia1")
        {
            EXPECT_EQ(deviceParams.baseAddress, 0xFC00);
            EXPECT_EQ(deviceParams.byteSize, -1);
        }
        else if (deviceParams.name == "vico1")
        {
            EXPECT_EQ(deviceParams.baseAddress, 0xFD20);
            EXPECT_EQ(deviceParams.byteSize, -1);
        }
        else if (deviceParams.name == "rtc")
        {
            EXPECT_EQ(deviceParams.baseAddress, 0xFDD0);
            EXPECT_EQ(deviceParams.byteSize, 0x25);
        }
        else if (deviceParams.name == "pia2")
        {
            EXPECT_EQ(deviceParams.baseAddress, 0x0000);
            EXPECT_EQ(deviceParams.byteSize, 4096);
        }
        else
        {
            EXPECT_TRUE(false) << "got unexpected device name";
        }
    }
    fs::remove(path);
}

TEST(test_fcnffile, fct_GetDebugSupportOption)
{
    const auto path = (fs::temp_directory_path() / "cnf5.conf").u8string();
    ASSERT_TRUE(createCnfFile(path));
    FlexemuConfigFile cnfFile(path);
    ASSERT_TRUE(cnfFile.IsValid());
    auto value = cnfFile.GetDebugSupportOption("presetRAM");
    EXPECT_EQ(value, "1");
    value = cnfFile.GetDebugSupportOption("logMdcr");
    EXPECT_EQ(value, "1");
    value = cnfFile.GetDebugSupportOption("logMdcrFilePath");
    EXPECT_EQ(value, "/dir/subdir/subdir/mdcr.log");
    value = cnfFile.GetDebugSupportOption("invalidKey");
    EXPECT_TRUE(value.empty());
    fs::remove(path);
}

TEST(test_fcnffile, fct_ReadIoDevices_exceptions)
{
    const auto path = (fs::temp_directory_path() / "cnf6.conf").u8string();
    std::fstream ofs(path, std::ios::out | std::ios::trunc);
    ASSERT_TRUE(ofs.is_open());
    ofs <<
        "[IoDevices]\n"
        "invalidDevice=FFE0\n";
    ofs.close();
    FlexemuConfigFile cnfFile1(path);
    ASSERT_TRUE(cnfFile1.IsValid());
    EXPECT_THAT([&](){ cnfFile1.ReadIoDevices(); },
            testing::Throws<FlexException>());
    fs::remove(path);

    ofs.open(path, std::ios::out | std::ios::trunc);
    ASSERT_TRUE(ofs.is_open());
    ofs <<
        "[IoDevices]\n"
        "mmu=10000\n";
    ofs.close();
    FlexemuConfigFile cnfFile3(path);
    ASSERT_TRUE(cnfFile3.IsValid());
    EXPECT_THAT([&](){ cnfFile3.ReadIoDevices(); },
            testing::Throws<FlexException>());
    fs::remove(path);

    ofs.open(path, std::ios::out | std::ios::trunc);
    ASSERT_TRUE(ofs.is_open());
    ofs <<
        "[IoDevices]\n"
        "mmu=FD00,0\n";
    ofs.close();
    FlexemuConfigFile cnfFile4(path);
    ASSERT_TRUE(cnfFile4.IsValid());
    EXPECT_THAT([&](){ cnfFile4.ReadIoDevices(); },
            testing::Throws<FlexException>());
    fs::remove(path);

    ofs.open(path, std::ios::out | std::ios::trunc);
    ASSERT_TRUE(ofs.is_open());
    ofs <<
        "[IoDevices]\n"
        "mmu=FD00,1001\n";
    ofs.close();
    FlexemuConfigFile cnfFile5(path);
    ASSERT_TRUE(cnfFile5.IsValid());
    EXPECT_THAT([&](){ cnfFile5.ReadIoDevices(); },
            testing::Throws<FlexException>());
    fs::remove(path);

    ofs.open(path, std::ios::out | std::ios::trunc);
    ASSERT_TRUE(ofs.is_open());
    ofs <<
        "[IoDevices]\n"
        "mmu=\n";
    ofs.close();
    FlexemuConfigFile cnfFile6(path);
    ASSERT_TRUE(cnfFile6.IsValid());
    EXPECT_THAT([&](){ cnfFile6.ReadIoDevices(); },
            testing::Throws<FlexException>());
    fs::remove(path);
}

TEST(test_fcnffile, fct_GetDebugSupportOption_exceptions)
{
    const auto path = (fs::temp_directory_path() / "cnf7.conf").u8string();
        std::fstream ofs(path, std::ios::out | std::ios::trunc);
    ASSERT_TRUE(ofs.is_open());
    ofs <<
        "[DebugSupport]\n"
        "invalidKey=value\n";
    ofs.close();
    FlexemuConfigFile cnfFile(path);
    ASSERT_TRUE(cnfFile.IsValid());
    EXPECT_THAT([&](){ cnfFile.GetDebugSupportOption("invalidKey"); },
            testing::Throws<FlexException>());
    fs::remove(path);
}

