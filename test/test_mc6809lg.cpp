/*
    test_mc6809lg.cpp


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
#include "mc6809lg.h"
#include "scpulog.h"
#include <cstring>
#include <vector>
#include <map>
#include <filesystem>


namespace fs = std::filesystem;

struct ParseResult
{
    bool isValid{};
    int lineCount{};
    int doCount{};
    int repeatCount{};
    std::vector<int> repeatValues;
    std::map<std::string, int> countForMnemonic;
};

static ParseResult parseFile(const fs::path &fileName)
{
    ParseResult result;
    std::string line;
    std::ifstream ifs(fileName, std::ios::in);
    result.isValid = ifs.is_open();
    while (std::getline(ifs, line))
    {
        ++result.lineCount;
        if (line.find("DO") != std::string::npos)
        {
            ++result.doCount;
        }
        else if (line.find("REPEAT") != std::string::npos)
        {
            std::stringstream stream{line};
            std::string mnemonic;
            char ch;
            int value;
            ++result.repeatCount;
            result.isValid &= !(stream >> mnemonic >> ch >> value).fail();
            result.repeatValues.push_back(value);
        }
        else
        {
            std::stringstream stream{line};
            std::string hex_addr;
            std::string mnemonic;
            result.isValid &= !(stream >> hex_addr >> mnemonic).fail();
            if (result.countForMnemonic.find(mnemonic) !=
                result.countForMnemonic.end())
            {
                ++result.countForMnemonic[mnemonic];
            }
            else
            {
                result.countForMnemonic[mnemonic] = 1;
            }

        }
    }
    ifs.close();

    return result;
}

static Mc6809CpuStatus setState(const std::vector<Byte> &instruction, Word pc,
        const char *mnemonic, const char *operands = "")
{
    Mc6809CpuStatus state{};

    assert(instruction.size() <= sizeof(state.instruction));
    assert(strlen(mnemonic) < sizeof(state.mnemonic));
    assert(strlen(operands) < sizeof(state.operands));

    state.pc = pc;
    std::copy(instruction.cbegin(),
            instruction.cbegin() + static_cast<Word>(instruction.size()),
            std::begin(state.instruction));
    state.insn_size = static_cast<Word>(instruction.size());
    strcpy(state.mnemonic, mnemonic);
    strcpy(state.operands, operands);
    return state;
}

TEST(test_mc6809logger, fct_doLogging)
{
    Mc6809Logger logger;
    EXPECT_FALSE(logger.doLogging(0x0000));
    EXPECT_FALSE(logger.doLogging(0x4000));
    EXPECT_FALSE(logger.doLogging(0x8000));
    EXPECT_FALSE(logger.doLogging(0xFFFF));
    Mc6809LoggerConfig config;
    config.logFilePath = fs::u8path(u8"test.log");
    config.isEnabled = true;
    logger.setLoggerConfig(config);
    EXPECT_TRUE(logger.doLogging(0x0000));
    EXPECT_TRUE(logger.doLogging(0x4000));
    EXPECT_TRUE(logger.doLogging(0x8000));
    EXPECT_TRUE(logger.doLogging(0xFFFF));
    config.minAddr = 0x4000;
    config.maxAddr = 0x8000;
    logger.setLoggerConfig(config);
    EXPECT_FALSE(logger.doLogging(0x3FFF));
    EXPECT_TRUE(logger.doLogging(0x4000));
    EXPECT_TRUE(logger.doLogging(0x8000));
    EXPECT_FALSE(logger.doLogging(0x8001));
    config.startAddr = 0x5000;
    config.stopAddr = 0x7000;
    logger.setLoggerConfig(config);
    EXPECT_FALSE(logger.doLogging(0x4000));
    EXPECT_FALSE(logger.doLogging(0x4100));
    EXPECT_TRUE(logger.doLogging(0x5000));
    EXPECT_FALSE(logger.doLogging(0x3FFF));
    EXPECT_TRUE(logger.doLogging(0x4000));
    EXPECT_TRUE(logger.doLogging(0x8000));
    EXPECT_FALSE(logger.doLogging(0x8001));
    EXPECT_FALSE(logger.doLogging(0x7000));
    EXPECT_FALSE(logger.doLogging(0x3FFF));
    EXPECT_FALSE(logger.doLogging(0x4000));
    EXPECT_FALSE(logger.doLogging(0x8000));
    EXPECT_FALSE(logger.doLogging(0x8001));
    config.logFilePath.clear();
    config.isEnabled = false;
    logger.setLoggerConfig(config);

    fs::remove(config.logFilePath);
}

TEST(test_mc6809logger, fct_logCpuState_loopOptimized)
{
    Mc6809LoggerConfig config;
    config.logFilePath = fs::u8path(u8"test1.log");
    config.isEnabled = true;
    config.isLoopOptimization = true;
    {
        // Testcase:
        // 0100 BSR $0102
        // 0102 BSR $0104
        // 0104 RTS
        // 0105 JMP $CD03
        // Expectation: no DO - REPEAT, loop only executed once
        Mc6809Logger logger;
        logger.setLoggerConfig(config);
        logger.logCpuState(setState({0x8D, 0x00}, 0x0100, "BSR", "$0102"));
        logger.logCpuState(setState({0x8D, 0x00}, 0x0102, "BSR", "$0104"));
        logger.logCpuState(setState({0x39}, 0x0104, "RTS"));
        logger.logCpuState(setState({0x39}, 0x0104, "RTS"));
        logger.logCpuState(setState({0x8D, 0x00}, 0x0102, "BSR", "$0104"));
        logger.logCpuState(setState({0x39}, 0x0104, "RTS"));
        logger.logCpuState(setState({0x39}, 0x0104, "RTS"));
        auto state = setState({0x7E, 0xCD, 0x03}, 0x0105, "JMP", "$CD03");
        logger.logCpuState(state);
    }
    auto result = parseFile(config.logFilePath);
    ASSERT_TRUE(result.isValid);
    EXPECT_EQ(result.doCount, 0);
    EXPECT_EQ(result.repeatCount, 0);
    EXPECT_EQ(result.lineCount, 8);
    EXPECT_EQ(result.countForMnemonic["BSR"], 3);
    EXPECT_EQ(result.countForMnemonic["RTS"], 4);
    EXPECT_EQ(result.countForMnemonic["JMP"], 1);
    fs::remove(config.logFilePath);

    {
        config.logFilePath = fs::u8path(u8"test2.log");
        // Testcase:
        // 0100 LDA #6
        // 0102 DECA
        // 0103 BNE $0102
        // 0105 JMP $CD03
        // Expectation: one DO - REPEAT, 5 full loop repetitions
        Mc6809Logger logger;
        logger.setLoggerConfig(config);
        logger.logCpuState(setState({0x86, 0x06}, 0x0100, "LDA", "#6"));
        for (int i = 0; i < 6; ++i)
        {
            logger.logCpuState(setState({0x4A}, 0x0102, "DECA"));
            logger.logCpuState(setState({0x26, 0xFD}, 0x0103, "BNE", "$0102"));
        }
        auto state = setState({0x7E, 0xCD, 0x03}, 0x0105, "JMP", "$CD03");
        logger.logCpuState(state);
    }
    result = parseFile(config.logFilePath);
    ASSERT_TRUE(result.isValid);
    EXPECT_EQ(result.doCount, 1);
    EXPECT_EQ(result.repeatCount, 1);
    EXPECT_EQ(result.lineCount, 8);
    EXPECT_EQ(result.repeatValues.size(), 1U);
    EXPECT_EQ(result.repeatValues[0], 5);
    EXPECT_EQ(result.countForMnemonic["LDA"], 1);
    EXPECT_EQ(result.countForMnemonic["DECA"], 2);
    EXPECT_EQ(result.countForMnemonic["BNE"], 2);
    EXPECT_EQ(result.countForMnemonic["JMP"], 1);
    fs::remove(config.logFilePath);

    {
        config.logFilePath = fs::u8path(u8"test3.log");
        // Testcase:
        // 0100 LDA #2
        // 0102 DECA
        // 0103 BNE $102
        // 0105 JMP $CD03
        // Expectation: no DO - REPEAT, loop only executed once
        Mc6809Logger logger;
        logger.setLoggerConfig(config);
        logger.logCpuState(setState({}, 0x0100, "LDA", "#2"));
        for (int i = 0; i < 2; ++i)
        {
            logger.logCpuState(setState({0x4A}, 0x0102, "DECA"));
            logger.logCpuState(setState({0x26, 0xFD}, 0x0103, "BNE", "$0102"));
        }
        logger.logCpuState(setState({}, 0x0105, "JMP", "$CD03"));
    }
    result = parseFile(config.logFilePath);
    ASSERT_TRUE(result.isValid);
    EXPECT_EQ(result.doCount, 0);
    EXPECT_EQ(result.repeatCount, 0);
    EXPECT_EQ(result.lineCount, 6);
    EXPECT_TRUE(result.repeatValues.empty());
    EXPECT_EQ(result.countForMnemonic["LDA"], 1);
    EXPECT_EQ(result.countForMnemonic["DECA"], 2);
    EXPECT_EQ(result.countForMnemonic["BNE"], 2);
    EXPECT_EQ(result.countForMnemonic["JMP"], 1);
    fs::remove(config.logFilePath);

    {
        config.logFilePath = fs::u8path(u8"test4.log");
        // Testcase:
        // 0100 LDA #8
        // 0102 LDX #$FFFC
        // 0105 LEAX A,X
        // 0107 BEQ $010C
        // 0109 DECA
        // 010A BNE $0102
        // 010C JMP $CD03
        // Expectation: DO - REPEAT, 3 full loop repetitions,
        //                           jump out from midth of loop.
        Mc6809Logger logger;
        logger.setLoggerConfig(config);
        logger.logCpuState(setState({0x86, 0x08}, 0x0100, "LDA", "#8"));
        for (int i = 0; i < 4; ++i)
        {
            auto state = setState({0x8E, 0xFF, 0xFC}, 0x0102, "LDX", "#$FFFC");
            logger.logCpuState(state);
            logger.logCpuState(setState({0x30, 0x86}, 0x0105, "LEAX", "A,X"));
            logger.logCpuState(setState({0x27, 0x03}, 0x0107, "BEQ", "$010C"));
            logger.logCpuState(setState({0x4A}, 0x0109, "DECA"));
            logger.logCpuState(setState({0x26, 0xF6}, 0x010A, "BNE", "$0102"));
        }
        auto state = setState({0x8E, 0xFF, 0xFC}, 0x0102, "LDX", "#$FFFC");
        logger.logCpuState(state);
        logger.logCpuState(setState({0x30, 0x86}, 0x0105, "LEAX", "A,X"));
        logger.logCpuState(setState({0x27, 0x03}, 0x0107, "BEQ", "$010C"));
        state = setState({0x7E, 0xCD, 0x03}, 0x010C, "JMP", "$CD03");
        logger.logCpuState(state);
    }

    result = parseFile(config.logFilePath);
    ASSERT_TRUE(result.isValid);
    EXPECT_EQ(result.doCount, 1);
    EXPECT_EQ(result.repeatCount, 1);
    EXPECT_EQ(result.lineCount, 17);
    ASSERT_FALSE(result.repeatValues.empty());
    EXPECT_EQ(result.repeatValues[0], 3);
    EXPECT_EQ(result.countForMnemonic["LDA"], 1);
    EXPECT_EQ(result.countForMnemonic["DECA"], 2);
    EXPECT_EQ(result.countForMnemonic["LDX"], 3);
    EXPECT_EQ(result.countForMnemonic["LEAX"], 3);
    EXPECT_EQ(result.countForMnemonic["BEQ"], 3);
    EXPECT_EQ(result.countForMnemonic["BNE"], 2);
    EXPECT_EQ(result.countForMnemonic["JMP"], 1);
    fs::remove(config.logFilePath);
}

TEST(test_mc6809logger, fct_logCpuState_selfModifyCode)
{
    Mc6809LoggerConfig config;
    config.logFilePath = fs::u8path(u8"test5.log");
    config.isEnabled = true;
    config.isLoopOptimization = true;
    {
        // Testcase:
        // 0100 LDX #0118
        // 0103 LDA #$39  // Opcode for RTS
        // 0105 LDB #20
        // 0107 BSR $0120
        // 0109 JMP $CD03
        // ...
        // 0120 STA ,X+
        // 0122 DECB
        // 0123 BNE $0120
        // 0125 RTS
        // Expectation: no DO - REPEAT, loop executed 8 times
        // then executing RTS which has been overwritten on $0120
        Mc6809Logger logger;
        logger.setLoggerConfig(config);
        auto state = setState({0x8E, 0x01, 0x18}, 0x0100, "LDX","$0118");
        logger.logCpuState(state);
        logger.logCpuState(setState({0x86, 0x39}, 0x0103, "LDA", "#$39"));
        logger.logCpuState(setState({0xC6, 0x10}, 0x0105, "LDB", "#$10"));
        logger.logCpuState(setState({0x8D, 0x17}, 0x0107, "BSR", "$0120"));
        for (int i = 0; i < 9; ++i)
        {
            logger.logCpuState(setState({0xA7, 0x80}, 0x0120, "STA", ",X+"));
            logger.logCpuState(setState({0x5A}, 0x0122, "DECB"));
            logger.logCpuState(setState({0x26, 0xFB}, 0x0123, "BNE", "$0120"));
        }
        logger.logCpuState(setState({0x39}, 0x0120, "RTS"));
        logger.logCpuState(setState({0x7E, 0xCD, 0x03}, 0x0109, "JMP", "$CD03"));
    }
    auto result = parseFile(config.logFilePath);
    ASSERT_TRUE(result.isValid);
    EXPECT_EQ(result.doCount, 1);
    EXPECT_EQ(result.repeatCount, 1);
    EXPECT_EQ(result.lineCount, 14);
    EXPECT_EQ(result.countForMnemonic["LDX"], 1);
    EXPECT_EQ(result.countForMnemonic["LDA"], 1);
    EXPECT_EQ(result.countForMnemonic["LDB"], 1);
    EXPECT_EQ(result.countForMnemonic["BSR"], 1);
    EXPECT_EQ(result.countForMnemonic["STA"], 2);
    EXPECT_EQ(result.countForMnemonic["DECB"], 2);
    EXPECT_EQ(result.countForMnemonic["BNE"], 2);
    EXPECT_EQ(result.countForMnemonic["RTS"], 1);
    EXPECT_EQ(result.countForMnemonic["JMP"], 1);
    fs::remove(config.logFilePath);
}

TEST(test_mc6809logger, fct_asCCString)
{
    EXPECT_EQ(Mc6809Logger::asCCString(0x01), "-------C");
    EXPECT_EQ(Mc6809Logger::asCCString(0x02), "------V-");
    EXPECT_EQ(Mc6809Logger::asCCString(0x04), "-----Z--");
    EXPECT_EQ(Mc6809Logger::asCCString(0x08), "----N---");
    EXPECT_EQ(Mc6809Logger::asCCString(0x10), "---I----");
    EXPECT_EQ(Mc6809Logger::asCCString(0x20), "--H-----");
    EXPECT_EQ(Mc6809Logger::asCCString(0x40), "-F------");
    EXPECT_EQ(Mc6809Logger::asCCString(0x80), "E-------");
    EXPECT_EQ(Mc6809Logger::asCCString(0x55), "-F-I-Z-C");
    EXPECT_EQ(Mc6809Logger::asCCString(0xAA), "E-H-N-V-");
}

