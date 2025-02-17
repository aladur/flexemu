/*
    test_fileread.cpp


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
#include <filesystem>
#include <array>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cassert>
#include <locale>
#include "fileread.h"
#include "memsrc.h"
#include "memtgt.h"


using ::testing::StartsWith;
namespace fs = std::filesystem;

struct TestMemory : public MemoryTarget<DWord>, public MemorySource<DWord>
{
    TestMemory() = default;
    ~TestMemory() override = default;
    void CopyFrom(const Byte *source, DWord address, DWord size) override
    {
        auto secureSize = size;
        if (address >= buffer.size())
        {
            throw std::out_of_range("address is out of valid range");
        }
        if (address + secureSize >= buffer.size())
        {
            secureSize -= address + size - buffer.size();
        }
        memcpy(buffer.data() + address, source, secureSize);

        if (secureSize > 0U)
        {
            DWord endAddress = address + secureSize - 1;
            addressRanges.emplace_back(address, endAddress);
            join(addressRanges);
        }
    }
    void CopyTo(Byte *target, DWord address, DWord size) const override
    {
        auto secureSize = size;

        if (address >= buffer.size())
        {
            throw std::out_of_range("address is out of valid range");
        }

        if (address + secureSize >= buffer.size())
        {
            secureSize -= address + size - buffer.size();
        }

        memcpy(target, buffer.data() + address, secureSize);
    }
    const AddressRanges& GetAddressRanges() const override
    {
        return addressRanges;
    }
    AddressRanges addressRanges;
    std::array<Byte, 65536> buffer{};
};

TEST(test_fileread, fct_load_hexfile)
{
    std::vector<std::string> test_files{
        "cat.cmd", "cat.hex", "cat.s19",
    };

    for (const auto &test_file : test_files)
    {
        TestMemory memory{};
        DWord start_addr = 0U;
        const auto path = (fs::current_path() / "data" / test_file).u8string();
        auto result = load_hexfile(path, memory, start_addr);
        ASSERT_EQ(result, 0);
        if (test_file.find("hex") == std::string::npos)
        {
            // Intel-hex does not support start address.
            EXPECT_EQ(start_addr, 0xC100);
        }
        EXPECT_EQ(memory.buffer[0xC0FF], 0x00);
        EXPECT_EQ(memory.buffer[0xC100], 0x20);
        EXPECT_EQ(memory.buffer[0xC1C4], 0xCD);
        EXPECT_EQ(memory.buffer[0xC288], 0xCC);
        EXPECT_EQ(memory.buffer[0xC34C], 0x45);
        EXPECT_EQ(memory.buffer[0xC39E], 0x23);
        EXPECT_EQ(memory.buffer[0xC39F], 0x00);
    }
}

TEST(test_fileread, fct_load_hexfile__non_exists)
{
    TestMemory memory{};
    DWord start_addr = 0U;
    auto result = load_hexfile("non_existend_file", memory, start_addr);
    ASSERT_EQ(result, -1);
}

TEST(test_fileread, fct_load_hexfile__intel)
{
    std::vector<std::string> file_contents{
       "blabla", // Wrong record type.
       ":20010002", // Not 32 data bytes.
       ":Z0010002", // Wrong char 'Z' in size.
       ":2Z010002", // Wrong char 'Z' in size.
       ":20010Z02", // Wrong char 'Z' in address.
       ":2001000Z", // Wrong char 'Z' in type.
       ":08010000010203090A0B0C0FFF", // Wrong checksum.
       ":08010000010203090A0B0C0FB8", // Missing End of File (01).
       ":08010000010203090A0B0C0FB8\n", // Missing End of File (01).
       ":080100000Z0203090A0B0C0FB8\n:00000001FF\n", // Wrong char 'Z'.
       ":08010000010203090A0B0C0FB8\n:00000001FF\n",
       ":08010000010203090A0B0C0FB8\r\n:00000001FF\r\n",
       ":08010000010203090A0B0C0FB8\n:0000000000\n",
       ":08010000010203090A0B0C0FB8\r\n:0000000000\r\n",

    };
    std::vector<int> expected_results{
        -3, -3, -3, -3, -3, -3, -4, -2, -3, -3,
        0, 0, 0, 0,
    };
    assert(file_contents.size() == expected_results.size());

    int index = 0;
    for (const auto &file_content : file_contents)
    {
        const auto path =
            fs::temp_directory_path() / "test_intel_hex_file.hex";
        std::ofstream ofs(path);
        ASSERT_TRUE(ofs.is_open());
        ofs << file_content;
        ofs.close();
        TestMemory memory{};
        DWord start_addr = 0U;
        auto result = load_hexfile(path.u8string(), memory, start_addr);
        ASSERT_EQ(result, expected_results[index++]);
        if (result == 0)
        {
            EXPECT_EQ(start_addr, 0x0000);
            EXPECT_EQ(memory.buffer[0x00FF], 0x00);
            EXPECT_EQ(memory.buffer[0x0100], 0x01);
            EXPECT_EQ(memory.buffer[0x0103], 0x09);
            EXPECT_EQ(memory.buffer[0x0104], 0x0A);
            EXPECT_EQ(memory.buffer[0x0107], 0x0F);
            EXPECT_EQ(memory.buffer[0x0108], 0x00);
        }
        fs::remove(path);
    }
}

TEST(test_fileread, fct_load_hexfile__motorola)
{
    std::array<std::string, 13> file_contents{
       "blabla", // Wrong record field.
       "S1ZB0100010203090A", // Wrong char 'Z' in size.
       "S10Z0100010203090A", // Wrong char 'Z' in size.
       "S10Z010Z010203090A", // Wrong char 'Z' in address.
       "S10B0100010203090A", // Not 11 data bytes.
       "S10B0100010203090A0B0C0FFF", // Wrong checksum.
       "S10B0100010203090A0B0C0FB4", // Missing S9
       "S30B00000100010203090A0B0C0FB4\nS9030100FB\n", // S3 not supported.
       "S10B0100Z00203090A0B0C0FB4\nS9030100FB\n", // Wrong char 'Z'.
       "S10B0100010203090A0B0C0FB4\nS9030100FB\n",
       "S10B0100010203090A0B0C0FB4\r\nS9030100FB\r\n",
       "S10B0100010203090A0B0C0FB4\nS5030100FB\nS9030100FB\n",
       "S10B0100010203090A0B0C0FB4\r\nS5030100FB\r\nS9030100FB\r\n",

    };
    std::array<int, 13> expected_results{
        -3, -3, -3, -3, -3, -4, -2, -3, -3,
        0, 0, 0, 0,
    };

    assert(file_contents.size() == expected_results.size());
    int index = 0;
    for (const auto &file_content : file_contents)
    {
        const auto path =
            fs::temp_directory_path() / "test_motorola_s-rec_file.hex";
        std::ofstream ofs(path);
        ASSERT_TRUE(ofs.is_open());
        ofs << file_content;
        ofs.close();
        TestMemory memory{};
        DWord start_addr = 0U;
        auto result = load_hexfile(path.u8string(), memory, start_addr);
        ASSERT_EQ(result, expected_results[index++]);
        if (result == 0)
        {
            EXPECT_EQ(start_addr, 0x0100);
            EXPECT_EQ(memory.buffer[0x00FF], 0x00);
            EXPECT_EQ(memory.buffer[0x0100], 0x01);
            EXPECT_EQ(memory.buffer[0x0103], 0x09);
            EXPECT_EQ(memory.buffer[0x0104], 0x0A);
            EXPECT_EQ(memory.buffer[0x0107], 0x0F);
            EXPECT_EQ(memory.buffer[0x0108], 0x00);
        }
        fs::remove(path);
    }
}

TEST(test_fileread, fct_load_hexfile__flex_binary)
{
    std::array<std::array<char, 20>, 6> file_contents{{
        { }, // Empty file.
        { 0x03, 0x01, 0x00, 0x10 }, // Wrong record type (0x03).
        { 0x02, 0x01, 0x00, 0x01, 0x55, 0x17, 0x01, 0x00 }, // Wrong rec.type.
        { 0x02, 0x01, 0x00, 0x07, 0x01, 0x02, 0x03, 0x09, 0x0A, 0x0B, '\xFF' },
        { 0x02, 0x01, 0x00, 0x07, 0x01, 0x02, 0x03, 0x09, 0x0A, 0x0B, '\xFF',
          0x16, 0x01, 0x00 },
        { 0x02, 0x01, 0x00, 0x00, 0x16, 0x01, 0x00 },
    }};
    std::array<int, 6> expected_results{
        -3, -3, -3, 0, 0, 0,
    };

    assert(file_contents.size() == expected_results.size());
    int index = 0;
    const auto path =
        fs::temp_directory_path() / "test_flex_binary_file.cmd";
    for (const auto &file_content : file_contents)
    {
        std::ofstream ofs(path, std::ios::out | std::ios::binary);
        ASSERT_TRUE(ofs.is_open());
        ofs.write(file_content.data(),
                static_cast<std::streamsize>(file_content.size()));
        ofs.close();
        TestMemory memory{};
        DWord start_addr = 0U;
        auto result = load_hexfile(path.u8string(), memory, start_addr);
        EXPECT_EQ(result, expected_results[index]) << "index=" << index;
        if (result == 0)
        {
            if (index >= 4)
            {
                EXPECT_EQ(start_addr, 0x0100);
            }
            if (index != 5)
            {
                EXPECT_EQ(memory.buffer[0x00FF], 0x00);
                EXPECT_EQ(memory.buffer[0x0100], 0x01);
                EXPECT_EQ(memory.buffer[0x0101], 0x02);
                EXPECT_EQ(memory.buffer[0x0102], 0x03);
                EXPECT_EQ(memory.buffer[0x0103], 0x09);
                EXPECT_EQ(memory.buffer[0x0104], 0x0A);
                EXPECT_EQ(memory.buffer[0x0105], 0x0B);
                EXPECT_EQ(memory.buffer[0x0106], 0xFF);
            }
        }
        fs::remove(path);
        ++index;
    }
}

TEST(test_fileread, fct_write_flex_binary)
{
    std::array<std::array<Byte, 3>, 3> file_contents{{
        { 0x7E, 0xF0, 0x2D },
        { 0x7E, 0xCD, 0x03 },
        { 0x01, 0x02, 0x03 }
    }};
    std::array<char, 24> expected_buffer{
        '\x02', '\xC1', '\x00', '\x03', '\x7E', '\xF0', '\x2D',
        '\x02', '\xC2', '\x00', '\x03', '\x7E', '\xCD', '\x03',
        '\x02', '\xC3', '\x00', '\x03', '\x01', '\x02', '\x03',
        '\x16', '\xC1', '\x00',
    };

    TestMemory memory{};
    DWord start_addr = 0xC100U;
    for (const auto &file_content : file_contents)
    {
        memory.CopyFrom(file_content.data(), start_addr, file_content.size());
        start_addr += 0x100;
    }
    const auto path = fs::temp_directory_path() / "test_flex_binary_file.cmd";
    auto result = write_flex_binary(path.u8string(), memory, 0xC100);
    ASSERT_EQ(result, 0);
    EXPECT_EQ(fs::file_size(path), 24);
    std::ifstream ifs(path, std::ios::in | std::ios::binary);
    ASSERT_TRUE(ifs.is_open());
    std::array<char, 24> buffer{};
    ifs.read(buffer.data(), buffer.size());
    EXPECT_EQ(buffer, expected_buffer);
    ifs.close();
    fs::remove(path);

}

TEST(test_fileread, fct_write_intel_hex)
{
    std::array<std::array<Byte, 3>, 3> file_contents{{
        { 0x7E, 0xF0, 0x2D },
        { 0x7E, 0xCD, 0x03 },
        { 0x01, 0x02, 0x03 }
    }};
    std::array<std::string, 5> expected_lines{
        ":03C100007EF02DA1",
        ":03C200007ECD03ED",
        ":03C3000001020334",
        ":040000050000C10036",
        ":00000001FF",
    };

    TestMemory memory{};
    DWord start_addr = 0xC100U;
    for (const auto &file_content : file_contents)
    {
        memory.CopyFrom(file_content.data(), start_addr, file_content.size());
        start_addr += 0x100;
    }
    const auto path = fs::temp_directory_path() / "test_intel_hex_file.hex";
    auto result = write_intel_hex(path.u8string(), memory, 0xC100);
    ASSERT_EQ(result, 0);
    std::ifstream ifs(path);
    ASSERT_TRUE(ifs.is_open());
    std::string linebuffer;
    for (const auto &expected_line : expected_lines)
    {
        std::getline(ifs, linebuffer);
        EXPECT_EQ(linebuffer, expected_line);
        EXPECT_EQ(ifs.good(), true);
    }
    ifs.close();
    fs::remove(path);
}

TEST(test_fileread, fct_write_motorola_srecord)
{
    std::array<std::array<Byte, 3>, 3> file_contents{{
        { 0x7E, 0xF0, 0x2D },
        { 0x7E, 0xCD, 0x03 },
        { 0x01, 0x02, 0x03 }
    }};
    std::array<std::string, 5> expected_lines{
        "S018000043726561746564207769746820666C6578326865780D",
        "S106C1007EF02D9D",
        "S106C2007ECD03E9",
        "S106C30001020330",
        "S903C1003B",
    };

    TestMemory memory{};
    DWord start_addr = 0xC100U;
    for (const auto &file_content : file_contents)
    {
        memory.CopyFrom(file_content.data(), start_addr, file_content.size());
        start_addr += 0x100;
    }
    const auto path =
        fs::temp_directory_path() / "test_motorola_srecord_file.hex";
    auto result = write_motorola_srecord(path.u8string(), memory, 0xC100);
    ASSERT_EQ(result, 0);
    std::ifstream ifs(path);
    ASSERT_TRUE(ifs.is_open());
    std::string linebuffer;
    for (const auto &expected_line : expected_lines)
    {
        std::getline(ifs, linebuffer);
        EXPECT_EQ(linebuffer, expected_line);
        EXPECT_EQ(ifs.eof() || ifs.good(), true);
    }
    ifs.close();
    fs::remove(path);
}

TEST(test_fileread, fct_write_raw_binary)
{
    // Define 3 memory ranges + start address.
    std::array<std::array<Byte, 3>, 3> file_contents{{
        { 0x7E, 0xF0, 0x2D },
        { 0x7E, 0xCD, 0x03 },
        { 0x01, 0x02, 0x03 }
    }};

    TestMemory memory{};
    DWord start_addr = 0xC100U;
    for (const auto &file_content : file_contents)
    {
        memory.CopyFrom(file_content.data(), start_addr, file_content.size());
        start_addr += 0x100;
    }
    const auto path = fs::temp_directory_path() / "test_raw_binary_file.dmp";
    auto result = write_raw_binary(path.u8string(), memory, 0xC100);
    ASSERT_EQ(result, 0);
    EXPECT_EQ(fs::file_size(path), 515);
    std::ifstream ifs(path, std::ios::in | std::ios::binary);
    ASSERT_TRUE(ifs.is_open());
    std::array<char, 515> buffer{};
    ifs.read(buffer.data(), buffer.size());
    ifs.close();
    EXPECT_EQ(buffer[0x0000], '\x7E');
    EXPECT_EQ(buffer[0x0001], '\xF0');
    EXPECT_EQ(buffer[0x0002], '\x2D');
    EXPECT_EQ(buffer[0x0003], '\x00');
    EXPECT_EQ(buffer[0x0100], '\x7E');
    EXPECT_EQ(buffer[0x0101], '\xCD');
    EXPECT_EQ(buffer[0x0102], '\x03');
    EXPECT_EQ(buffer[0x0103], '\x00');
    EXPECT_EQ(buffer[0x0200], '\x01');
    EXPECT_EQ(buffer[0x0201], '\x02');
    EXPECT_EQ(buffer[0x0202], '\x03');
    fs::remove(path);
}

TEST(test_fileread, fct_print_hexfile_error)
{
    std::stringstream stream1;
    print_hexfile_error(stream1, -1);
    EXPECT_THAT(stream1.str(), StartsWith("File does not exist or"));
    std::stringstream stream2;
    print_hexfile_error(stream2, -2);
    EXPECT_EQ(stream2.str(), "Error reading from file.");
    std::stringstream stream3;
    print_hexfile_error(stream3, -3);
    EXPECT_EQ(stream3.str(), "Unknown or invalid file format.");
    std::stringstream stream4;
    print_hexfile_error(stream4, -4);
    EXPECT_EQ(stream4.str(), "Wrong checksum.");
    std::stringstream stream5;
    print_hexfile_error(stream5, -5);
    EXPECT_EQ(stream5.str(), "Error writing to file.");
    std::stringstream stream6;
    print_hexfile_error(stream6, -6);
    EXPECT_EQ(stream6.str(), "File can not be opened for writing.");
    std::stringstream stream7;
    print_hexfile_error(stream7, -100);
    EXPECT_EQ(stream7.str(), "Unspecified error.");
    std::stringstream stream8;
    print_hexfile_error(stream8, 100);
    EXPECT_TRUE(stream8.str().empty());
}

