#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "fcnffile.h"
#include "flexerr.h"
#include <fstream>
#include <filesystem>

using ::testing::Throws;
namespace fs = std::filesystem;

bool createCnfFile(const std::string &path)
{
    std::fstream ofs(path, std::ios::out | std::ios::trunc);
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

TEST(test_bcnffile, ctor)
{
    const std::string path1{"/tmp/cnf_file1.ini"};
    EXPECT_TRUE(createCnfFile(path1));
    FlexemuConfigFile cnfFile1(path1);
    EXPECT_TRUE(cnfFile1.IsValid());
    EXPECT_EQ(cnfFile1.GetFileName(), path1);
    fs::remove(path1);

    const std::string path2{R"(\\/\\/\\///\\/)"};
    EXPECT_THAT([&](){ FlexemuConfigFile cnfFile2(path2); },
            testing::Throws<FlexException>());

    const std::string path3 = "/tmp/not_existent_file.conf";
    EXPECT_THAT([&](){ FlexemuConfigFile cnfFile3(path3); },
            testing::Throws<FlexException>());
}

TEST(test_bcnffile, move_ctor)
{
    const std::string path1{"/tmp/cnf_file1.ini"};
    EXPECT_TRUE(createCnfFile(path1));
    FlexemuConfigFile cnfFile1(path1);
    auto cnfFile2(std::move(cnfFile1));

    EXPECT_TRUE(cnfFile1.GetFileName().empty());
    EXPECT_FALSE(cnfFile1.IsValid());
    EXPECT_TRUE(cnfFile2.IsValid());
    EXPECT_EQ(cnfFile2.ReadIoDevices().size(), 5U);
    EXPECT_EQ(cnfFile2.GetFileName(), path1);
    fs::remove(path1);
}

TEST(test_bcnffile, move_assignment)
{
    const std::string path1{"/tmp/cnf_file1.ini"};
    EXPECT_TRUE(createCnfFile(path1));
    FlexemuConfigFile cnfFile1(path1);
    auto cnfFile2 = std::move(cnfFile1);

    EXPECT_TRUE(cnfFile1.GetFileName().empty());
    EXPECT_FALSE(cnfFile1.IsValid());
    EXPECT_TRUE(cnfFile2.IsValid());
    EXPECT_EQ(cnfFile2.GetFileName(), path1);
    EXPECT_EQ(cnfFile2.ReadIoDevices().size(), 5U);
    fs::remove(path1);
}

TEST(test_bcnffile, fct_ReadIoDevices)
{
    const std::string path{"/tmp/cnf_file1.ini"};
    ASSERT_TRUE(createCnfFile(path));
    FlexemuConfigFile cnfFile(path);
    ASSERT_TRUE(cnfFile.IsValid());
    const auto deviceMappings = cnfFile.ReadIoDevices();
    EXPECT_EQ(deviceMappings.size(), 5U);
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
        else
        {
            EXPECT_TRUE(false) << "got unexpected device name";
        }
    }
    fs::remove(path);
}

TEST(test_bcnffile, fct_GetDebugSupportOption)
{
    const std::string path{"/tmp/cnf_file2.ini"};
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

TEST(test_bcnffile, fct_ReadIoDevices_exceptions)
{
    const std::string path{"/tmp/cnf_file3.ini"};
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
        "mmu=FBFF\n";
    ofs.close();
    FlexemuConfigFile cnfFile2(path);
    ASSERT_TRUE(cnfFile2.IsValid());
    EXPECT_THAT([&](){ cnfFile2.ReadIoDevices(); },
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
        "mmu=FD00,41\n";
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

TEST(test_bcnffile, fct_GetDebugSupportOption_exceptions)
{
    const std::string path{"/tmp/cnf_file3.ini"};
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

