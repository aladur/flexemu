#include "gtest/gtest.h"
#include "misc1.h"
#include "da6809.h"
#include <string>
#include <iomanip>
#include <sstream>
#include "fmt/format.h"


TEST(test_da6809, dis_illegal)
{
    Da6809 da;
    InstFlg flags;
    DWord pc = 0x0000;
    DWord jumpaddr = 4711U;
    std::string code;
    std::string mnemonic;
    std::vector<Byte> memory{
        0x01, 0x02, 0x05, 0x0B, 0x14, 0x15, 0x18, 0x1B, 0x38,
        0x3E, 0x41, 0x42, 0x45, 0x4B, 0x4E, 0x51, 0x52, 0x5E,
        0x61, 0x62, 0x65, 0x6B, 0x71, 0x72, 0x75, 0x7B, 0x87,
        0x8F, 0xC7, 0xCD, 0xCF,
    };

    while (pc < static_cast<Word>(memory.size()))
    {
        const auto bytes = da.Disassemble(&memory[pc], pc, flags,
                                          jumpaddr, code, mnemonic);

        EXPECT_EQ(bytes, 1);
        EXPECT_EQ(flags, InstFlg::Illegal);
        EXPECT_EQ(jumpaddr, 4711U); // unchanged value
        const auto expected_code = fmt::format("{:04X}: {:02X}", pc,
                                               memory[pc]);
        EXPECT_EQ(code, expected_code);
        EXPECT_EQ(mnemonic, " ?????");
        pc += bytes;
    }
}

TEST(test_da6809, dis_inherent)
{
    Da6809 da;
    InstFlg flags;
    DWord pc = 0x0000;
    DWord jumpaddr = 4711U;
    std::string code;
    std::string mnemonic;
    static const std::vector<Byte> memory{
        0x12, 0x13, 0x19, 0x1D, 0x39, 0x3A, 0x3B, 0x3D, 0x3F,
        0x40, 0x43, 0x44, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4C, 0x4D, 0x4F,
        0x50, 0x53, 0x54, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5C, 0x5D, 0x5F,
        0x10, 0x3F,
        0x11, 0x3F,
    };
    static const std::vector<const char *> expected_mnemonics{
        "NOP", "SYNC", "DAA", "SEX", // 0X
        "RTS", "ABX", "RTI", "MUL", "SWI", // 3X
        "NEGA", "COMA", "LSRA", "RORA", "ASRA", "LSLA", // 4X
        "ROLA", "DECA", "INCA", "TSTA", "CLRA",
        "NEGB", "COMB", "LSRB", "RORB", "ASRB", "LSLB", // 5X
        "ROLB", "DECB", "INCB", "TSTB", "CLRB",
        "SWI2", // 10 3X
        "SWI3" // 11 3X
    };
    auto iexpected_mnemonic = expected_mnemonics.cbegin();

    while (pc < static_cast<Word>(memory.size()))
    {
        const auto bytes = da.Disassemble(&memory[pc], pc, flags,
                                          jumpaddr, code, mnemonic);

        const bool is_page23 = (memory[pc] & 0xFE) == 0x10;
        auto size = is_page23 ? 2 : 1;
        EXPECT_EQ(bytes, size);
        Word opcode = memory[pc];
        opcode = is_page23 ? ((opcode << 8) | memory[pc + 1]) : opcode;
        auto expected_flags = InstFlg::NONE;
        switch (opcode)
        {
            case 0x12: // NOP
                expected_flags = InstFlg::Noop;
                 break;
            case 0x13: // SYNC
            case 0x39: // RTS
            case 0x3B: // RTI
                expected_flags = InstFlg::Jump;
                 break;
            case 0x3F: // SWI
            case 0x103F: // SWI2
            case 0x113F: // SWI3
                expected_flags = InstFlg::Sub;
                 break;
        }
        EXPECT_EQ(flags, expected_flags);
        EXPECT_EQ(jumpaddr, 4711U); // unchanged value
        auto expected_code = fmt::format("{:04X}:", pc);
        for (int i = 0; i < size; ++i)
        {
            expected_code += fmt::format(" {:02X}", memory[pc + i]);
        }
        EXPECT_EQ(code, expected_code);
        const std::string expected_mnemonic{*(iexpected_mnemonic++)};
        rtrim(mnemonic);
        EXPECT_EQ(mnemonic, expected_mnemonic);
        pc += bytes;
    }
}

TEST(test_da6809, dis_immediate)
{
    Da6809 da;
    InstFlg flags;
    DWord pc = 0x0000;
    DWord jumpaddr = 4711U;
    std::string code;
    std::string mnemonic;
    // Remark: Instead of $CC00 use $CC10, $CC00 would print label TTYBS.
    static const std::vector<Byte> memory{
        0x1A, 0x55, 0x1C, 0xAA, // 1X
        0x3C, 0x55, // 3X
        0x80, 0x80, 0x81, 0x81, 0x82, 0x82, 0x83, 0x83, 0x00, 0x84, 0x84,
        0x85, 0x85, 0x86, 0x86, 0x88, 0x88, 0x89, 0x89, 0x8A, 0x8A,
        0x8B, 0x8B, 0x8C, 0x8C, 0x00, 0x8E, 0x8E, 0x00, // 8X
        0xC0, 0xC0, 0xC1, 0xC1, 0xC2, 0xC2, 0xC3, 0xC3, 0x00, 0xC4, 0xC4,
        0xC5, 0xC5, 0xC6, 0xC6, 0xC8, 0xC8, 0xC9, 0xC9, 0xCA, 0xCA,
        0xCB, 0xCB, 0xCC, 0xCC, 0x10, 0xCE, 0xCE, 0x00, // CX
        0x10, 0x83, 0x83, 0x00, 0x10, 0x8C, 0x8C, 0x00,
        0x10, 0x8E, 0x8E, 0x00, // 10 8X
        0x10, 0xCE, 0xCE, 0x00, // 10 CX
        0x11, 0x83, 0x83, 0x00, 0x11, 0x8C, 0x8C, 0x00, // 11 8X
    };
    static const std::vector<const char *> expected_mnemonics{
        "ORCC", "ANDCC",
        "CWAI",
        "SUBA", "CMPA", "SBCA", "SUBD", "ANDA", "BITA", // r89X
        "LDA", "EORA", "ADCA", "ORA", "ADDA",
        "CMPX", "LDX",
        "SUBB", "CMPB", "SBCB", "ADDD", "ANDB", "BITB", // CX
        "LDB", "EORB", "ADCB", "ORB", "ADDB", "LDD", "LDU",
        "CMPD", "CMPY", "LDY", // 10 8X
        "LDS", // 10 CX
        "CMPU", "CMPS", // 11 8X
    };
    static const std::vector<Word> opcode_16bit{
        0x83, 0x8C, 0x8E, 0xC3, 0xCC, 0xCE, 0x1083, 0x108C, 0x108E,
        0x10CE, 0x1183, 0x118C,
    };
    auto iexpected_mnemonic = expected_mnemonics.cbegin();

    while (pc < static_cast<Word>(memory.size()))
    {
        const auto bytes = da.Disassemble(&memory[pc], pc, flags,
                                          jumpaddr, code, mnemonic);

        const bool is_page23 = (memory[pc] & 0xFE) == 0x10;
        auto size = is_page23 ? 3 : 2;
        Word opcode = memory[pc];
        opcode = is_page23 ? ((opcode << 8) | memory[pc + 1]) : opcode;
        const bool is_16bit = (std::find(opcode_16bit.cbegin(),
                               opcode_16bit.cend(),
                                opcode) != opcode_16bit.cend());
        size = is_16bit ? size + 1 : size;
        EXPECT_EQ(bytes, size);
        auto expected_flags = InstFlg::NONE;
        switch (opcode)
        {
            case 0xC3: // ADDD
            case 0xCC: // LDD
            case 0xCE: // LDU
            case 0x1083: // CMPD
            case 0x108C: // CMPY
            case 0x108E: // LDY
            case 0x10CE: // LDS
            case 0x1183: // CMPU
            case 0x118C: // CMPS
                expected_flags = InstFlg::LabelAddr;
                 break;
            case 0x3C: // CWAI
                expected_flags = InstFlg::Jump;
                 break;
        }
        EXPECT_EQ(flags, expected_flags);
        EXPECT_EQ(jumpaddr, 4711U); // unchanged value
        auto expected_code = fmt::format("{:04X}:", pc);
        for (int i = 0; i < size; ++i)
        {
            expected_code += fmt::format(" {:02X}", memory[pc + i]);
        }
        EXPECT_EQ(code, expected_code);
        const std::string mnemo{*(iexpected_mnemonic++)};
        const std::string spaces(6U - mnemo.size(), ' ');
        Word tgtaddr = memory[pc + size - 1];
        tgtaddr = is_16bit ? (memory[pc + size - 2] << 8 | tgtaddr) : tgtaddr;
        auto expected_mnemonic = fmt::format("{}{}#$", mnemo, spaces);
        if (is_16bit)
        {
            expected_mnemonic += fmt::format("{:04X}", tgtaddr);
        }
        else
        {
            expected_mnemonic += fmt::format("{:02X}", tgtaddr);
        }
        rtrim(mnemonic);
        EXPECT_EQ(mnemonic, expected_mnemonic);
        pc += bytes;
    }
}

TEST(test_da6809, dis_direct)
{
    Da6809 da;
    InstFlg flags;
    DWord pc = 0x0000;
    DWord jumpaddr = 4711U;
    std::string code;
    std::string mnemonic;
    static const std::vector<Byte> memory{
        0x00, 0x00, 0x03, 0x03, 0x04, 0x04, 0x06, 0x06, 0x07, 0x07, 0x08, 0x08,
        0x09, 0x09, 0x0A, 0x0A, 0x0C, 0x0C, 0x0D, 0x0D, 0x0E, 0x0E, 0x0F, 0x0F,
        0x90, 0x90, 0x91, 0x91, 0x92, 0x92, 0x93, 0x93, 0x94, 0x94, 0x95, 0x95,
        0x96, 0x96, 0x97, 0x97, 0x98, 0x98, 0x99, 0x99, 0x9A, 0x9A, 0x9B, 0x9B,
        0x9C, 0x9C, 0x9D, 0x9D, 0x9E, 0x9E, 0x9F, 0x9F,
        0xD0, 0xD0, 0xD1, 0xD1, 0xD2, 0xD2, 0xD3, 0xD3, 0xD4, 0xD4, 0xD5, 0xD5,
        0xD6, 0xD6, 0xD7, 0xD7, 0xD8, 0xD8, 0xD9, 0xD9, 0xDA, 0xDA, 0xDB, 0xDB,
        0xDC, 0xDC, 0xDD, 0xDD, 0xDE, 0xDE, 0xDF, 0xDF,
        0x10, 0x93, 0x93, 0x10, 0x9C, 0x9C, 0x10, 0x9E, 0x9E, 0x10, 0x9F, 0x9F,
        0x10, 0xDE, 0xDE, 0x10, 0xDF, 0xDF,
        0x11, 0x93, 0x93,
        0x11, 0x9C, 0x9C,
    };
    static const std::vector<const char *> expected_mnemonics{
        "NEG", "COM", "LSR", "ROR", "ASR", "LSL",  // 0X
        "ROL", "DEC", "INC", "TST", "JMP", "CLR",
        "SUBA", "CMPA", "SBCA", "SUBD", "ANDA", "BITA", // 9X
        "LDA", "STA", "EORA", "ADCA", "ORA", "ADDA",
        "CMPX", "JSR", "LDX", "STX",
        "SUBB", "CMPB", "SBCB", "ADDD", "ANDB", "BITB", // DX
        "LDB", "STB", "EORB", "ADCB", "ORB", "ADDB",
        "LDD", "STD", "LDU", "STU",
        "CMPD", "CMPY", "LDY", "STY", // 10 9X
        "LDS", "STS", // 10 DX
        "CMPU", "CMPS" // 11 9X
    };
    auto iexpected_mnemonic = expected_mnemonics.cbegin();

    while (pc < static_cast<Word>(memory.size()))
    {
        const auto bytes = da.Disassemble(&memory[pc], pc, flags,
                                          jumpaddr, code, mnemonic);

        const bool is_page23 = (memory[pc] & 0xFE) == 0x10;
        const auto size = is_page23 ? 3 : 2;
        EXPECT_EQ(bytes, size);
        Word opcode = memory[pc];
        opcode = is_page23 ? ((opcode << 8) | memory[pc + 1]) : opcode;
        auto expected_flags = InstFlg::NONE;
        switch (opcode)
        {
            case 0x0E: // JMP
                expected_flags = InstFlg::Jump;
                 break;
            case 0x9D: // JSR
                expected_flags = InstFlg::Sub;
                 break;
        }
        EXPECT_EQ(flags, expected_flags);
        EXPECT_EQ(jumpaddr, 4711U); // unchanged value
        auto expected_code = fmt::format("{:04X}:", pc);
        for (int i = 0; i < size; ++i)
        {
            expected_code += fmt::format(" {:02X}", memory[pc + i]);
        }
        EXPECT_EQ(code, expected_code);
        const std::string mnemo{*(iexpected_mnemonic++)};
        const std::string spaces(6U - mnemo.size(), ' ');
        const auto expected_mnemonic =
            fmt::format("{}{}${:02X}", mnemo, spaces, memory[pc + 1]);
        EXPECT_EQ(mnemonic, expected_mnemonic);
        pc += bytes;
    }
}

TEST(test_da6809, dis_branch_relative)
{
    Da6809 da;
    InstFlg flags;
    DWord pc = 0x0000;
    DWord jumpaddr = 4711U;
    std::string code;
    std::string mnemonic;
    static const std::vector<Byte> memory{
        0x20, 0x00, 0x21, 0x00, 0x22, 0x00, 0x23, 0x00, 0x24, 0x00,
        0x25, 0x00, 0x26, 0x00, 0x27, 0x00, 0x28, 0x00, 0x29, 0x00,
        0x2A, 0x00, 0x2B, 0x00, 0x2C, 0x00, 0x2D, 0x00, 0x2E, 0x00,
        0x2F, 0x00,
        0x8D, 0x00,
        0x20, 0x7F, 0x20, 0x80,
    };
    static const std::vector<const char *> expected_mnemonics{
        "BRA", "BRN", "BHI", "BLS", "BCC",
        "BCS", "BNE", "BEQ", "BVC", "BVS",
        "BPL", "BMI", "BGE", "BLT", "BGT",
        "BLE",
        "BSR",
        "BRA", "BRA",

    };
    auto iexpected_mnemonic = expected_mnemonics.cbegin();

    while (pc < static_cast<DWord>(memory.size()))
    {
        const auto bytes = da.Disassemble(&memory[pc], pc, flags,
                                          jumpaddr, code, mnemonic);

        const auto size = 2;
        EXPECT_EQ(bytes, size);
        auto expected_flags = InstFlg::JumpAddr;
        auto opcode = memory[pc];
        switch (opcode)
        {
            case 0x20:
                expected_flags |= InstFlg::Jump;
                break;
            case 0x8D:
                expected_flags = InstFlg::Sub;
                break;
        }
        EXPECT_EQ(flags, expected_flags);
        const auto expected_jumpaddr = pc + 2 +
            (static_cast<int16_t>(static_cast<int8_t>(memory[pc + 1])) &
             0xFFFF);
        EXPECT_EQ(jumpaddr, expected_jumpaddr);
        auto expected_code = fmt::format("{:04X}:", pc);
        for (int i = 0; i < size; ++i)
        {
            expected_code += fmt::format(" {:02X}", memory[pc + i]);
        }
        EXPECT_EQ(code, expected_code);
        const std::string mnemo{*(iexpected_mnemonic++)};
        const std::string spaces(6U - mnemo.size(), ' ');
        const auto expected_mnemonic =
            fmt::format("{}{}${:04X}", mnemo, spaces, expected_jumpaddr);
        EXPECT_EQ(mnemonic, expected_mnemonic);
        pc += bytes;
    }
}

TEST(test_da6809, dis_long_branch_relative)
{
    Da6809 da;
    InstFlg flags;
    DWord pc = 0x0000;
    DWord jumpaddr = 4711U;
    std::string code;
    std::string mnemonic;
    static const std::vector<Byte> memory{
        0x16, 0x00, 0x00, 0x17, 0x00, 0x00,
        0x10, 0x21, 0x00, 0x00, 0x10, 0x22, 0x00, 0x00,
        0x10, 0x23, 0x00, 0x00, 0x10, 0x24, 0x00, 0x00,
        0x10, 0x25, 0x00, 0x00, 0x10, 0x26, 0x00, 0x00,
        0x10, 0x27, 0x00, 0x00, 0x10, 0x28, 0x00, 0x00,
        0x10, 0x29, 0x00, 0x00, 0x10, 0x2A, 0x00, 0x00,
        0x10, 0x2B, 0x00, 0x00, 0x10, 0x2C, 0x00, 0x00,
        0x10, 0x2D, 0x00, 0x00, 0x10, 0x2E, 0x00, 0x00,
        0x10, 0x2F, 0x00, 0x00,
    };
    static const std::vector<const char *> expected_mnemonics{
        "LBRA", "LBSR",
        "LBRN", "LBHI", "LBLS", "LBCC", "LBCS",
        "LBNE", "LBEQ", "LBVC", "LBVS", "LBPL", "LBMI",
        "LBGE", "LBLT", "LBGT", "LBLE",
    };
    auto iexpected_mnemonic = expected_mnemonics.cbegin();

    while (pc < static_cast<Word>(memory.size()))
    {
        const auto bytes = da.Disassemble(&memory[pc], pc, flags,
                                          jumpaddr, code, mnemonic);

        const auto size = (memory[pc] == 0x10) ? 4 : 3;
        EXPECT_EQ(bytes, size);
        auto expected_flags = InstFlg::JumpAddr;
        switch (memory[pc])
        {
            case 0x16:
                expected_flags |= InstFlg::Jump;
                break;
            case 0x17:
                expected_flags = InstFlg::Sub | InstFlg::LabelAddr;
                break;
        }

        EXPECT_EQ(flags, expected_flags);
        const auto expected_jumpaddr = pc + size +
            getValueBigEndian<Word>(&memory[pc + size - 2]);
        EXPECT_EQ(jumpaddr, expected_jumpaddr);
        auto expected_code = fmt::format("{:04X}:", pc);
        for (int i = 0; i < size; ++i)
        {
            expected_code += fmt::format(" {:02X}", memory[pc + i]);
        }
        EXPECT_EQ(code, expected_code);
        const std::string mnemo{*(iexpected_mnemonic++)};
        const std::string spaces(6U - mnemo.size(), ' ');
        const auto expected_mnemonic =
            fmt::format("{}{}${:04X}", mnemo, spaces, jumpaddr);
        EXPECT_EQ(mnemonic, expected_mnemonic);
        pc += bytes;
    }
}

TEST(test_da6809, dis_extended)
{
    Da6809 da;
    InstFlg flags;
    DWord pc = 0x0000;
    DWord jumpaddr = 4711U;
    std::string code;
    std::string mnemonic;
    static const std::vector<Byte> memory{
        0x70, 0x01, 0x70, 0x73, 0x01, 0x73, 0x74, 0x01, 0x74, 0x76, 0x01, 0x76,
        0x77, 0x01, 0x77, 0x78, 0x01, 0x78, 0x79, 0x01, 0x79, 0x7A, 0x01, 0x7A,
        0x7C, 0x01, 0x7C, 0x7D, 0x01, 0x7D, 0x7E, 0x01, 0x7E, 0x7F, 0x01, 0x7F,
        0xB0, 0x01, 0xB0, 0xB1, 0x01, 0xB1, 0xB2, 0x01, 0xB2, 0xB3, 0x01, 0xB3,
        0xB4, 0x01, 0xB4, 0xB5, 0x01, 0xB5, 0xB6, 0x01, 0xB6, 0xB7, 0x01, 0xB7,
        0xB8, 0x01, 0xB8, 0xB9, 0x01, 0xB9, 0xBA, 0x01, 0xBA, 0xBB, 0x01, 0xBB,
        0xBC, 0x01, 0xBC, 0xBD, 0x01, 0xBD, 0xBE, 0x01, 0xBE, 0xBF, 0x01, 0xBF,
        0xF0, 0x01, 0xF0, 0xF1, 0x01, 0xF1, 0xF2, 0x01, 0xF2, 0xF3, 0x01, 0xF3,
        0xF4, 0x01, 0xF4, 0xF5, 0x01, 0xF5, 0xF6, 0x01, 0xF6, 0xF7, 0x01, 0xF7,
        0xF8, 0x01, 0xF8, 0xF9, 0x01, 0xF9, 0xFA, 0x01, 0xFA, 0xFB, 0x01, 0xFB,
        0xFC, 0x01, 0xFC, 0xFD, 0x01, 0xFD, 0xFE, 0x01, 0xFE, 0xFF, 0x01, 0xFF,
        0x10, 0xB3, 0x01, 0xB3, 0x10, 0xBC, 0x01, 0xBC, 0x10, 0xBE, 0x01, 0xBE,
        0x10, 0xBF, 0x01, 0xBF,
        0x10, 0xFE, 0x01, 0xFE, 0x10, 0xFF, 0x01, 0xFF,
        0x11, 0xB3, 0x01, 0xB3, 0x11, 0xBC, 0x01, 0xBC,
    };
    static const std::vector<const char *> expected_mnemonics{
        "NEG", "COM", "LSR", "ROR", "ASR", "LSL",  // 7X
        "ROL", "DEC", "INC", "TST", "JMP", "CLR",
        "SUBA", "CMPA", "SBCA", "SUBD", "ANDA", "BITA", // BX
        "LDA", "STA", "EORA", "ADCA", "ORA", "ADDA",
        "CMPX", "JSR", "LDX", "STX",
        "SUBB", "CMPB", "SBCB", "ADDD", "ANDB", "BITB", // FX
        "LDB", "STB", "EORB", "ADCB", "ORB", "ADDB",
        "LDD", "STD", "LDU", "STU",
        "CMPD", "CMPY", "LDY", "STY", "LDS", "STS", // 10 BX
        "CMPU", "CMPS", // 11 BX
    };
    auto iexpected_mnemonic = expected_mnemonics.cbegin();

    while (pc < static_cast<Word>(memory.size()))
    {
        const auto bytes = da.Disassemble(&memory[pc], pc, flags,
                                          jumpaddr, code, mnemonic);

        const bool is_page23 = (memory[pc] & 0xFE) == 0x10;
        const auto size = is_page23 ? 4 : 3;
        EXPECT_EQ(bytes, size);
        Word opcode = memory[pc];
        opcode = is_page23 ? ((opcode << 8) | memory[pc + 1]) : opcode;
        auto expected_flags = InstFlg::LabelAddr;
        switch (opcode)
        {
            case 0x11B3: // CMPU
            case 0x11BC: // CMPS
                expected_flags = InstFlg::NONE;
                 break;
            case 0x7E: // JMP
                expected_flags = InstFlg::Jump;
                 break;
            case 0xBD: // JSR
                expected_flags = InstFlg::Sub | InstFlg::JumpAddr;
                 break;
        }
        EXPECT_EQ(flags, expected_flags);
        auto tgtaddr = (static_cast<Word>(memory[pc + size - 2] << 8) |
                       static_cast<Word>(memory[pc + size - 1]));
        EXPECT_EQ(jumpaddr, 4711U); // unchanged value
        auto expected_code = fmt::format("{:04X}:", pc);
        for (int i = 0; i < size; ++i)
        {
            expected_code += fmt::format(" {:02X}", memory[pc + i]);
        }
        EXPECT_EQ(code, expected_code);
        const std::string mnemo{*(iexpected_mnemonic++)};
        const std::string spaces(6U - mnemo.size(), ' ');
        const auto expected_mnemonic =
            fmt::format("{}{}${:04X}", mnemo, spaces, tgtaddr);
        EXPECT_EQ(mnemonic, expected_mnemonic);
        pc += bytes;
    }
}

TEST(test_da6809, dis_indexed)
{
    Da6809 da;
    InstFlg flags;
    DWord pc = 0x0000;
    DWord jumpaddr = 4711U;
    std::string code;
    std::string mnemonic;
    static const std::vector<Byte> memory{
        0x30, 0x00, 0x31, 0x00, 0x32, 0x00, 0x33, 0x00, // 3X
        0x60, 0x00, 0x63, 0x00, 0x64, 0x00, 0x66, 0x00, 0x67, 0x00,
        0x68, 0x00, 0x69, 0x00, 0x6A, 0x00, 0x6C, 0x00, 0x6D, 0x00,
        0x6E, 0x00, 0x6F, 0x00, // 6X
        0xA0, 0x00, 0xA1, 0x00, 0xA2, 0x00, 0xA3, 0x00, 0xA4, 0x00,
        0xA5, 0x00, 0xA6, 0x00, 0xA7, 0x00, 0xA8, 0x00, 0xA9, 0x00,
        0xAA, 0x00, 0xAB, 0x00, 0xAC, 0x00, 0xAD, 0x00, 0xAE, 0x00,
        0xAF, 0x00, // AX
        0xE0, 0x00, 0xE1, 0x00, 0xE2, 0x00, 0xE3, 0x00, 0xE4, 0x00,
        0xE5, 0x00, 0xE6, 0x00, 0xE7, 0x00, 0xE8, 0x00, 0xE9, 0x00,
        0xEA, 0x00, 0xEB, 0x00, 0xEC, 0x00, 0xED, 0x00, 0xEE, 0x00,
        0xEF, 0x00, // EX
        0x10, 0xA3, 0x00, 0x10, 0xAC, 0x00, // 10 AX
        0x10, 0xAE, 0x00, 0x10, 0xAF, 0x00,
        0x10, 0xEE, 0x00, 0x10, 0xEF, 0x00, // 10 EX
        0x11, 0xA3, 0x00, 0x11, 0xAC, 0x00, // 11 AX

    };
    static const std::vector<const char *> expected_mnemonics{
        "LEAX", "LEAY", "LEAS", "LEAU",
        "NEG", "COM", "LSR", "ROR", "ASR", "LSL",  // 6X
        "ROL", "DEC", "INC", "TST", "JMP", "CLR",
        "SUBA", "CMPA", "SBCA", "SUBD", "ANDA", "BITA", // AX
        "LDA", "STA", "EORA", "ADCA", "ORA", "ADDA",
        "CMPX", "JSR", "LDX", "STX",
        "SUBB", "CMPB", "SBCB", "ADDD", "ANDB", "BITB", // EX
        "LDB", "STB", "EORB", "ADCB", "ORB", "ADDB",
        "LDD", "STD", "LDU", "STU",
        "CMPD", "CMPY", "LDY", "STY", // 10 AX
        "LDS", "STS",
        "CMPU", "CMPS", // 11 AX
    };
    auto iexpected_mnemonic = expected_mnemonics.cbegin();

    while (pc < static_cast<Word>(memory.size()))
    {
        const auto bytes = da.Disassemble(&memory[pc], pc, flags,
                                          jumpaddr, code, mnemonic);

        const bool is_page23 = (memory[pc] & 0xFE) == 0x10;
        const auto size = is_page23 ? 3 : 2;
        EXPECT_EQ(bytes, size);
        Word opcode = memory[pc];
        opcode = is_page23 ? ((opcode << 8) | memory[pc + 1]) : opcode;
        InstFlg expected_flags = InstFlg::NONE;
        switch (opcode)
        {
            case 0x6E: // JMP
                expected_flags = InstFlg::Jump | InstFlg::ComputedGoto;
                 break;
            case 0xAD: // JSR
                expected_flags = InstFlg::Sub | InstFlg::ComputedGoto;
                 break;
        }
        EXPECT_EQ(flags, expected_flags);
        EXPECT_EQ(jumpaddr, 4711U); // unchanged value
        auto expected_code = fmt::format("{:04X}:", pc);
        for (int i = 0; i < size; ++i)
        {
            expected_code += fmt::format(" {:02X}", memory[pc + i]);
        }
        EXPECT_EQ(code, expected_code);
        const std::string mnemo{*(iexpected_mnemonic++)};
        const std::string spaces(6U - mnemo.size(), ' ');
        const auto expected_mnemonic =
            fmt::format("{}{}$00,X", mnemo, spaces);
        EXPECT_EQ(mnemonic, expected_mnemonic);
        pc += bytes;
    }
}

TEST(test_da6809, dis_indexed_modes)
{
    Da6809 da;
    InstFlg flags;
    DWord pc = 0x0000;
    DWord jumpaddr = 4711U;
    std::string code;
    std::string mnemonic;
    static const std::vector<Byte> memory{
        // Non indirect.
        0x60, 0x84, 0x60, 0xA4, 0x60, 0xC4, 0x60, 0xE4, // No offset
        0x60, 0x10, 0x60, 0x2F, 0x60, 0x50, 0x60, 0x6F, // 5-bit offset
        0x60, 0x88, 0x80, 0x60, 0xA8, 0x7F, // 8-bit offset
        0x60, 0xC8, 0x80, 0x60, 0xE8, 0x7F,
        0x60, 0x89, 0x80, 0x00, 0x60, 0xA9, 0x7F, 0xFF, // 16-bit offset
        0x60, 0xC9, 0x80, 0x00, 0x60, 0xE9, 0x7F, 0xFF,
        0x60, 0x86, 0x60, 0xA5, 0x60, 0xCB, // accumulator offset
        0x60, 0xE0, 0x60, 0xE1, // post increment by 1/2
        0x60, 0x82, 0x60, 0x83, // pre decrement by 1/2
        // Indirect.
        0x60, 0x94, 0x60, 0xB4, 0x60, 0xD4, 0x60, 0xF4, // No offset
        0x60, 0x98, 0x80, 0x60, 0xB8, 0x7F, // 8-bit offset
        0x60, 0xD8, 0x80, 0x60, 0xF8, 0x7F,
        0x60, 0x99, 0x80, 0x00, 0x60, 0xB9, 0x7F, 0xFF, // 16-bit offset
        0x60, 0xD9, 0x80, 0x00, 0x60, 0xF9, 0x7F, 0xFF,
        0x60, 0x96, 0x60, 0xB5, 0x60, 0xDB, // accumulator offset
        0x60, 0xF1, // post increment by 2
        0x60, 0x93, // pre decrement by 2
        0x60, 0x9F, 0x55, 0xAA, // extended indirect
    };
    static const std::vector<const char *> expected_mnemonics{
        // Non indirect.
        ",X", ",Y", ",U", ",S", // No offset
        "-$10,X", "$0F,Y", "-$10,U", "$0F,S", // 5-bit offset
        "-$80,X", "$7F,Y", "-$80,U", "$7F,S", // 8-bit offset
        "$8000,X", "$7FFF,Y", "$8000,U", "$7FFF,S", // 8-bit offset
        "A,X", "B,Y", "D,U", // accumulator offset
        ",S+", ",S++", // post increment by 1/2
        ",-X", ",--X", // pre decrement by 1/2
        // Indirect.
        "[,X]", "[,Y]", "[,U]", "[,S]", // No offset
        "[-$80,X]", "[$7F,Y]", "[-$80,U]", "[$7F,S]", // 8-bit offset
        "[$8000,X]", "[$7FFF,Y]", "[$8000,U]", "[$7FFF,S]", // 8-bit offset
        "[A,X]", "[B,Y]", "[D,U]", // accumulator offset
        "[,S++]", // post increment by 2
        "[,--X]", // pre decrement by 2
        "[$55AA]", // extended indirect
    };
    auto iexpected_mnemonic = expected_mnemonics.cbegin();

    while (pc < static_cast<Word>(memory.size()))
    {
        const auto bytes = da.Disassemble(&memory[pc], pc, flags,
                                          jumpaddr, code, mnemonic);

        const auto mask = memory[pc + 1] & 0x8B;
        const auto is_8bit_offset =(mask == 0x88);
        const bool is_16bit_offset = (mask == 0x89) ||
            (memory[pc + 1] == 0x9F);
        auto size = is_8bit_offset ? 3 : 2;
        size = is_16bit_offset ? size + 2 : size;
        EXPECT_EQ(bytes, size);
        const InstFlg expected_flags = InstFlg::NONE;
        EXPECT_EQ(flags, expected_flags);
        EXPECT_EQ(jumpaddr, 4711U); // unchanged value
        auto expected_code = fmt::format("{:04X}:", pc);
        for (int i = 0; i < size; ++i)
        {
            expected_code += fmt::format(" {:02X}", memory[pc + i]);
        }
        EXPECT_EQ(code, expected_code);
        const auto expected_mnemonic =
            fmt::format("NEG   {}", *(iexpected_mnemonic++));;
        EXPECT_EQ(mnemonic, expected_mnemonic);
        pc += bytes;
    }
}

TEST(test_da6809, dis_indexed_modes_pc_rel)
{
    Da6809 da;
    InstFlg flags;
    DWord pc = 0x8000;
    DWord jumpaddr = 4711U;
    DWord offset = 0;
    std::string code;
    std::string mnemonic;
    static const std::vector<Byte> memory{
        // Non indirect.
        0x60, 0x8C, 0x80, 0x60, 0xAC, 0x80, // PC relative 8-bit offset
        0x60, 0xCC, 0x80, 0x60, 0xEC, 0x80, // PC relative 8-bit offset
        0x60, 0x8C, 0x7F, 0x60, 0xAC, 0x7F, // PC relative 8-bit offset
        0x60, 0xCC, 0x7F, 0x60, 0xEC, 0x7F, // PC relative 8-bit offset
        0x60, 0x8D, 0x40, 0x00, 0x60, 0xAD, 0x40, 0x00, // PC rel. 16-bit offs.
        0x60, 0xCD, 0x40, 0x00, 0x60, 0xED, 0x40, 0x00, // PC rel. 16-bit offs.
        0x60, 0x8D, 0x3F, 0xFF, 0x60, 0xAD, 0x3F, 0xFF, // PC rel. 16-bit offs.
        0x60, 0xCD, 0x3F, 0xFF, 0x60, 0xED, 0x3F, 0xFF, // PC rel. 16-bit offs.
        // Indirect.
        0x60, 0x9C, 0x80, 0x60, 0xBC, 0x80, // PC relative 8-bit offset
        0x60, 0xDC, 0x80, 0x60, 0xFC, 0x80, // PC relative 8-bit offset
        0x60, 0x9C, 0x7F, 0x60, 0xBC, 0x7F, // PC relative 8-bit offset
        0x60, 0xDC, 0x7F, 0x60, 0xFC, 0x7F, // PC relative 8-bit offset
        0x60, 0x9D, 0x40, 0x00, 0x60, 0xBD, 0x40, 0x00, // PC rel. 16-bit offs.
        0x60, 0xDD, 0x40, 0x00, 0x60, 0xFD, 0x40, 0x00, // PC rel. 16-bit offs.
        0x60, 0x9D, 0x3F, 0xFF, 0x60, 0xBD, 0x3F, 0xFF, // PC rel. 16-bit offs.
        0x60, 0xDD, 0x3F, 0xFF, 0x60, 0xFD, 0x3F, 0xFF, // PC rel. 16-bit offs.
    };
    static const std::vector<const char *> expected_mnemonics{
        // Non indirect.
        "<$7F83,PCR", "<$7F86,PCR", // PC relative 8-bit offset
        "<$7F89,PCR", "<$7F8C,PCR",
        "<$808E,PCR", "<$8091,PCR",
        "<$8094,PCR", "<$8097,PCR",
        ">$C01C,PCR", ">$C020,PCR", // PC relative 16-bit offset
        ">$C024,PCR", ">$C028,PCR",
        ">$C02B,PCR", ">$C02F,PCR",
        ">$C033,PCR", ">$C037,PCR",
        // Indirect.
        "[<$7F83,PCR]", "[<$7F86,PCR]", // PC relative 8-bit offset
        "[<$7F89,PCR]", "[<$7F8C,PCR]",
        "[<$808E,PCR]", "[<$8091,PCR]",
        "[<$8094,PCR]", "[<$8097,PCR]",
        "[>$C01C,PCR]", "[>$C020,PCR]", // PC relative 16-bit offset
        "[>$C024,PCR]", "[>$C028,PCR]",
        "[>$C02B,PCR]", "[>$C02F,PCR]",
        "[>$C033,PCR]", "[>$C037,PCR]",
    };
    auto iexpected_mnemonic = expected_mnemonics.cbegin();

    while (offset < static_cast<DWord>(memory.size()))
    {
        const auto bytes = da.Disassemble(memory.data() + offset, pc, flags,
                                          jumpaddr, code, mnemonic);

        const auto mask = memory[offset + 1] & 0x8B;
        const auto is_8bit_offset =(mask == 0x88);
        const bool is_16bit_offset = (mask == 0x89);
        auto size = is_8bit_offset ? 3 : 2;
        size = is_16bit_offset ? size + 2 : size;
        EXPECT_EQ(bytes, size);
        const InstFlg expected_flags = InstFlg::NONE;
        EXPECT_EQ(flags, expected_flags);
        EXPECT_EQ(jumpaddr, 4711U); // unchanged value
        auto expected_code = fmt::format("{:04X}:", pc);
        for (int i = 0; i < size; ++i)
        {
            expected_code += fmt::format(" {:02X}", memory[offset + i]);
        }
        EXPECT_EQ(code, expected_code);
        const auto expected_mnemonic =
            fmt::format("NEG   {}", *(iexpected_mnemonic++));;
        EXPECT_EQ(mnemonic, expected_mnemonic);
        offset += bytes;
        pc += bytes;
        if (pc >= 0x8000 + 56)
        {
            // start indirect addressing with same PC value.
            pc = 0x8000;
        }
    }
}

TEST(test_da6809, dis_indexed_modes_illegal)
{
    Da6809 da;
    InstFlg flags;
    DWord pc = 0x0000;
    DWord jumpaddr = 4711U;
    std::string code;
    std::string mnemonic;
    static const std::vector<Byte> memory{
        // Non indirect.
        0x60, 0x8F, 0x60, 0xAF,
        0x60, 0x87, 0x60, 0xA7, 0x60, 0xC7, 0x60, 0xE7,
        0x60, 0x8A, 0x60, 0xAA, 0x60, 0xCA, 0x60, 0xEA,
        0x60, 0x8E, 0x60, 0xAE, 0x60, 0xCE, 0x60, 0xEE,
        0x60, 0xCF, 0x60, 0xEF,
        // Indirect.
        0x60, 0x90, 0x60, 0xB0, 0x60, 0xD0, 0x60, 0xF0,
        0x60, 0x92, 0x60, 0xB2, 0x60, 0xD2, 0x60, 0xF2,
        0x60, 0x97, 0x60, 0xB7, 0x60, 0xD7, 0x60, 0xF7,
        0x60, 0x9A, 0x60, 0xBA, 0x60, 0xDA, 0x60, 0xFA,
        0x60, 0x9E, 0x60, 0xBE, 0x60, 0xDE, 0x60, 0xFE,
        0x60, 0xBF, 0x60, 0xDF, 0x60, 0xFF,
    };

    while (pc < static_cast<Word>(memory.size()))
    {
        const auto bytes = da.Disassemble(&memory[pc], pc, flags,
                                          jumpaddr, code, mnemonic);

        const auto size = 2;
        EXPECT_EQ(bytes, size);
        const InstFlg expected_flags = InstFlg::NONE;
        EXPECT_EQ(flags, expected_flags);
        EXPECT_EQ(jumpaddr, 4711U); // unchanged value
        auto expected_code = fmt::format("{:04X}:", pc);
        for (int i = 0; i < size; ++i)
        {
            expected_code += fmt::format(" {:02X}", memory[pc + i]);
        }
        EXPECT_EQ(code, expected_code);
        const auto expected_mnemonic{R"(NEG   ????)"};
        EXPECT_EQ(mnemonic, expected_mnemonic);
        pc += bytes;
    }
}

std::string GetRegisterName(Byte code)
{
    static const std::array<const char *, 16> registers{
        "D", "X", "Y", "U", "S", "PC", "??", "??",
        "A", "B", "CC", "DP", "??", "??", "??", "??"
    };

    if (code >= registers.size())
    {
        return {"??"};
    }

    return registers[code];
}


TEST(test_da6809, dis_exg_tfr)
{
    Da6809 da;
    InstFlg flags;
    DWord pc = 0x0000;
    DWord jumpaddr = 4711U;
    std::string code;
    std::string mnemonic;
    std::array<Byte, 2> memory{};

    for (Byte opcode = 0x1E; opcode <= 0x1F; ++opcode)
    {
        memory[0] = opcode;

        for (int postbyte = 0; postbyte < 256; ++postbyte)
        {
            memory[1] = static_cast<Byte>(postbyte);

            const auto bytes = da.Disassemble(memory.data(), pc, flags,
                                              jumpaddr, code, mnemonic);

            const auto size = 2;
            EXPECT_EQ(bytes, size);
            InstFlg expected_flags = InstFlg::NONE;
            EXPECT_EQ(flags, expected_flags);
            EXPECT_EQ(jumpaddr, 4711U); // unchanged value
            auto expected_code = fmt::format("{:04X}:", pc);
            for (int i = 0; i < size; ++i)
            {
                expected_code += fmt::format(" {:02X}", memory[pc + i]);
            }
            EXPECT_EQ(code, expected_code);
            const std::string mnemo = (opcode == 0x1E) ? "EXG" : "TFR";
            const auto r0 = GetRegisterName(postbyte >> 4);
            const auto r1 = GetRegisterName(postbyte & 0x0F);
            const auto expected_mnemonic =
                fmt::format("{}   {},{}", mnemo, r0, r1);
            EXPECT_EQ(mnemonic, expected_mnemonic);
        }
    }
}

std::string GetRegisterList(Byte postbyte, const std::string &nonstack_reg)
{
    static const std::array<const char *, 8> reg_names{
        "CC", "A", "B", "DP", "X", "Y", "", "PC"
    };
    bool is_first = true;
    std::string result;

    if (postbyte == 0)
    {
        return {"??"};
    }

    for (Byte i = 0; i < 8; ++i)
    {
        if (postbyte & (1 << i))
        {
            std::string reg = reg_names[i];
            reg = (reg.empty()) ? nonstack_reg : reg;
            if (!is_first)
            {
                result.append(",");
            }
            result.append(reg);
            is_first = false;
        }
    }

    return result;
}

TEST(test_da6809, dis_psh_pul)
{
    Da6809 da;
    InstFlg flags;
    DWord pc = 0x0000;
    DWord jumpaddr = 4711U;
    std::string code;
    std::string mnemonic;
    std::array<Byte, 2> memory{};
    std::array<const char *, 4> expected_mnemonics{
        "PSHS", "PULS", "PSHU", "PULU",
    };

    for (Byte opcode = 0x34; opcode <= 0x37; ++opcode)
    {
        memory[0] = opcode;

        for (int postbyte = 0; postbyte < 256; ++postbyte)
        {
            memory[1] = static_cast<Byte>(postbyte);

            const auto bytes = da.Disassemble(memory.data(), pc, flags,
                                              jumpaddr, code, mnemonic);

            auto size = 2;
            EXPECT_EQ(bytes, size);
            InstFlg expected_flags = InstFlg::NONE;
            EXPECT_EQ(flags, expected_flags);
            EXPECT_EQ(jumpaddr, 4711U); // unchanged value
            auto expected_code = fmt::format("{:04X}:", pc);
            for (int i = 0; i < size; ++i)
            {
                expected_code += fmt::format(" {:02X}", memory[pc + i]);
            }
            EXPECT_EQ(code, expected_code);
            std::string mnemo = expected_mnemonics[opcode - 0x34];
            const std::string nonstack_reg = (opcode & 0x02) ? "S" : "U";
            std::string reg_list = GetRegisterList(postbyte, nonstack_reg);
            const auto expected_mnemonic =
                fmt::format("{} {}", mnemo, reg_list);
            EXPECT_EQ(mnemonic, expected_mnemonic);
        }
    }
}

TEST(test_da6809, dis_undocumented)
{
    Da6809 da;
    InstFlg flags;
    DWord pc = 0x0000;
    DWord jumpaddr = 4711U;
    std::string code;
    std::string mnemonic;
    static const std::vector<Byte> memory{
        0x01, 0x00, 0x02, 0x00, 0x05, 0x00, 0x0B, 0x00,
        0x3E,
        0x41, 0x42, 0x45, 0x4B, 0x4E,
        0x51, 0x52, 0x55, 0x5B, 0x5E,
        0x61, 0x00, 0x62, 0x00, 0x65, 0x00, 0x6B, 0x00,
        0x71, 0x71, 0x00, 0x72, 0x72, 0x00, 0x75, 0x75, 0x00,
        0x7B, 0x7B, 0x00,
    };
    static const std::vector<const char *> expected_mnemonics{
        "neg   $00", "negcom $00", "lsr   $00", "dec   $00", "reset",
        "nega", "negcoma", "lsra", "deca", "clra",
        "negb", "negcomb", "lsrb", "decb", "clrb",
        "neg   $00,X", "negcom $00,X", "lsr   $00,X", "dec   $00,X",
        "neg   $7100", "negcom $7200", "lsr   $7500", "dec   $7B00",
    };
    auto iexpected_mnemonic = expected_mnemonics.cbegin();

    da.set_use_undocumented(true);
    while (pc < static_cast<Word>(memory.size()))
    {
        const auto bytes = da.Disassemble(&memory[pc], pc, flags,
                                          jumpaddr, code, mnemonic);

        auto opcode = memory[pc];
        int size{};
        auto expected_flags = InstFlg::NONE;
        switch(opcode & 0xF0)
        {
            case 0x00:
            case 0x60:
                size = 2;
                break;
            case 0x70:
                expected_flags = InstFlg::LabelAddr;
                size = 3;
                break;
            default:
                size = 1;
        }
        EXPECT_EQ(bytes, size);
        EXPECT_EQ(flags, expected_flags);
        EXPECT_EQ(jumpaddr, 4711U); // unchanged value
        auto expected_code = fmt::format("{:04X}:", pc);
        for (int i = 0; i < size; ++i)
        {
            expected_code += fmt::format(" {:02X}", memory[pc + i]);
        }
        EXPECT_EQ(code, expected_code);
        const std::string expected_mnemonic{*(iexpected_mnemonic++)};
        EXPECT_EQ(mnemonic, expected_mnemonic);
        pc += bytes;
    }
}

TEST(test_da6809, dis_flex_labels)
{
    Da6809 da;
    InstFlg flags{};
    DWord pc = 0x0000;
    DWord jumpaddr = 4711U;
    std::string code;
    std::string mnemonic;
    static const std::vector<Byte> memory{
        0x7E, 0xCD, 0x03, 0xB6, 0xCC, 0x00,
        0xF7, 0xCC, 0x02, 0xBD, 0xD4, 0x06,
        0x8E, 0xC8, 0x40,
    };
    static const std::vector<const char *> expected_mnemonics{
        "JMP   WARMS ; $CD03", "LDA   TTYBS ; $CC00",
        "STB   TTYEOL ; $CC02", "JSR   FMS ; $D406",
        "LDX   #FCB ; $C840",
    };
    auto iexpected_mnemonic = expected_mnemonics.cbegin();

    while (pc < static_cast<Word>(memory.size()))
    {
        const auto bytes = da.Disassemble(&memory[pc], pc, flags,
                                          jumpaddr, code, mnemonic);

        const std::string expected_mnemonic{*(iexpected_mnemonic++)};
        EXPECT_EQ(mnemonic, expected_mnemonic);
        pc += bytes;
    }
}

