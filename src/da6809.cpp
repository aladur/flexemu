/***************************************************************************
  Originally posted to comp.sys.m6809 by Didier Derny (didier@aida.remcomp.fr)

  Minor hacks by Alan DeKok

 Fixed: D_Indexed addressing used prog[2] and prog[3] when it meant
        prog[pc+2] and prog[pc+3]:  Would produce flawed disassemblies!

        changed addresses in D_Indexed to be all hex.
    added 2 instances of 'extrabyte' in D_Indexed: would not skip them..
        Added PC offsets to D_Indexed ,PCR formats
    added SWI2 print out as OS9

    converted to a C++ Class
    to use within flexemu by W. Schwotzer

****************************************************************************/


#include "typedefs.h"
#include "misc1.h"
#include "absdisas.h"
#include "da6809.h"
#include "flblfile.h"
#include "warnoff.h"
#include <fmt/format.h>
#include "warnon.h"
#include <cassert>
#include <string>
#include <array>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;


void Da6809::set_use_undocumented(bool value)
{
    use_undocumented = value;
}

void Da6809::SetFlexLabelFile(const fs::path &path)
{
    flexLabelFile = path;
}

const char *Da6809::FlexLabel(Word addr)
{
#ifdef FLEX_LABEL
    if (label_for_address.empty())
    {
        const auto path =
            flexLabelFile.empty() ? flx::getFlexLabelFile() : flexLabelFile;
        label_for_address = FlexLabelFile::ReadFile(std::cerr, path, "LABELS");
    }

    const auto iter = label_for_address.find(addr);

    if (iter != label_for_address.end())
    {
        return iter->second.c_str();
    }
#endif // #ifdef FLEX_LABEL

    return nullptr;
}

const char *Da6809::IndexRegister(Byte which)
{
    constexpr const std::array<const char *, 4> reg_names{ "X", "Y", "U", "S" };

    return reg_names[which & 0x03U];
}

const char *Da6809::InterRegister(Byte which)
{
    constexpr const std::array<const char *, 16> reg_names{
        "D", "X", "Y", "U", "S", "PC", "??", "??",
        "A", "B", "CC", "DP", "??", "??", "??", "??"
    };

    return reg_names[which & 0x0FU];
}


const char *Da6809::StackRegister(Byte which, const char *not_stack)
{
    constexpr const std::array<const char *, 8> reg_names{
        "CC", "A", "B", "DP", "X", "Y", "??", "PC"
    };

    if ((which & 0x07U) == 6U)
    {
        return not_stack;
    }

    return reg_names[which & 0x07U];
}


inline InstFlg Da6809::D_Illegal(const char *mnemo, Byte bytes,
        std::string &p_code, std::string &p_mnemonic, std::string &p_operands)
{
    p_code = PrintCode(bytes);
    p_mnemonic = mnemo;
    p_operands = "?????";

    return InstFlg::Illegal;
}


inline void Da6809::D_Direct(const char *mnemo, Byte bytes, std::string &p_code,
                             std::string &p_mnemonic, std::string &p_operands)
{
    const auto offset = *(memory + bytes - 1);

    p_code = PrintCode(bytes);
    p_mnemonic = mnemo;
    p_operands = fmt::format("${:02X}", offset);
}


inline void Da6809::D_Immediate8(const char *mnemo, Byte bytes,
        std::string &p_code, std::string &p_mnemonic, std::string &p_operands)
{
    const auto offset = *(memory + bytes - 1);

    p_code = PrintCode(bytes);
    p_mnemonic = mnemo;
    p_operands = fmt::format("#${:02X}", offset);
}


inline void Da6809::D_Immediate16(const char *mnemo, Byte bytes,
        std::string &p_code, std::string &p_mnemonic,
        std::string &p_operands)
{
    const auto offset = flx::getValueBigEndian<Word>(&memory[bytes - 2]);

    p_code = PrintCode(bytes);
    const auto *label = FlexLabel(offset);

    p_mnemonic = mnemo;
    if (label == nullptr)
    {
        p_operands = fmt::format("#${:04X}", offset);
    }
    else
    {
        p_operands = "#";
        p_operands += label;
    }
}

inline std::string Da6809::PrintCode(int bytes)
{
    std::string result = fmt::format("{:04X}:", pc);

    for (int i = 0; i < bytes; ++i)
    {
        result += fmt::format(" {:02X}", memory[i]);
    }

    return result;
}

inline void Da6809::D_Inherent(const char *mnemo, Byte bytes,
        std::string &p_code, std::string &p_mnemonic)
{
    p_code = PrintCode(bytes);
    p_mnemonic = mnemo;
}


void Da6809::D_Indexed(const char *mnemo, Byte bytes, std::string &p_code,
        std::string &p_mnemonic, std::string &p_operands)
{
    Byte disp;
    Word offset;
    Byte extrabytes = 0U;
    const auto postbyte = *(memory + bytes - 1);
    const char *br1 = ""; // opening bracket if indirect addressing
    const char *br2 = ""; // closing bracket if indirect addressing
    const char *sign = ""; // minus sign for offset
    const char *addr_mode = "<"; // addressing mode, "<" for direct, ">" for
                                 // extended addressing.

    p_mnemonic = mnemo;
    if ((postbyte & 0x80U) == 0x00U)
    {
        // ,R + 5 Bit Offset
        disp = postbyte & 0x1fU;

        if ((postbyte & 0x10U) == 0x10U)
        {
            sign = "-";
            disp = 0x20 - disp;
        }

        p_code = PrintCode(bytes);
        p_operands = fmt::format("{}${:02X},{}", sign, disp,
                 IndexRegister(postbyte >> 5U));
    }
    else
    {
        switch (postbyte & 0x1FU)
        {
            case 0x00 : // ,R+
                p_code = PrintCode(bytes);
                p_operands = fmt::format(",{}+", IndexRegister(postbyte >> 5U));
                break;

            case 0x11 : // [,R++]
                br1 = "[";
                br2 = "]";
                [[fallthrough]];

            case 0x01 : // ,R++
                p_code = PrintCode(bytes);
                p_operands = fmt::format("{},{}++{}",
                        br1, IndexRegister(postbyte >> 5U), br2);
                break;

            case 0x02 : // ,-R
                p_code = PrintCode(bytes);
                p_operands = fmt::format(",-{}", IndexRegister(postbyte >> 5U));
                break;

            case 0x13 : // [,R--]
                br1 = "[";
                br2 = "]";
                [[fallthrough]];

            case 0x03 : // ,--R
                p_code = PrintCode(bytes);
                p_operands = fmt::format("{},--{}{}",
                        br1, IndexRegister(postbyte >> 5U), br2);
                break;

            case 0x14 : // [,R--]
                br1 = "[";
                br2 = "]";
                [[fallthrough]];

            case 0x04 : // ,R
                p_code = PrintCode(bytes);
                p_operands = fmt::format("{},{}{}",
                        br1, IndexRegister(postbyte >> 5U), br2);
                break;

            case 0x15 : // [B,R]
                br1 = "[";
                br2 = "]";
                [[fallthrough]];

            case 0x05 : // B,R
                p_code = PrintCode(bytes);
                p_operands = fmt::format("{}B,{}{}",
                        br1, IndexRegister(postbyte >> 5U), br2);
                break;

            case 0x16 : // [A,R]
                br1 = "[";
                br2 = "]";
                [[fallthrough]];

            case 0x06 : // A,R
                p_code = PrintCode(bytes);
                p_operands = fmt::format("{}A,{}{}",
                        br1, IndexRegister(postbyte >> 5U), br2);
                break;

            case 0x18 : // [,R + 8 Bit Offset]
                br1 = "[";
                br2 = "]";
                [[fallthrough]];

            case 0x08 : // ,R + 8 Bit Offset
                offset = *(memory + 2);

                if (offset >= 128)
                {
                    sign = "-";
                    offset = 0x0100 - offset;
                }
                extrabytes = 1;
                p_code = PrintCode(bytes + extrabytes);
                p_operands = fmt::format("{}{}${:02X},{}{}",
                         br1, sign, offset, IndexRegister(postbyte >> 5U), br2);
                break;

            case 0x19 : // [,R + 16 Bit Offset]
                br1 = "[";
                br2 = "]";
                [[fallthrough]];

            case 0x09 : // ,R + 16 Bit Offset
                extrabytes = 2;
                p_code = PrintCode(bytes + extrabytes);
                offset = flx::getValueBigEndian<Word>(&memory[2]);
                if (offset >= 32768)
                {
                    sign = "-";
                    offset = 0xFFFF - offset + 1;
                }
                p_operands = fmt::format("{}{}${:04X},{}{}",
                         br1, sign, offset, IndexRegister(postbyte >> 5U), br2);
                break;

            case 0x1b : // [D,R]
                br1 = "[";
                br2 = "]";
                [[fallthrough]];

            case 0x0b : // D,R
                p_code = PrintCode(bytes);
                p_operands = fmt::format("{}D,{}{}",
                        br1, IndexRegister(postbyte >> 5U), br2);
                break;

            case 0x1c : // [,PC + 8 Bit Offset]
                br1 = "[";
                br2 = "]";
                [[fallthrough]];

            case 0x0c : // ,PC + 8 Bit Offset
                offset = (EXTEND8(*(memory + 2)) + pc + 3U) & 0xFFFFU;
                extrabytes = 1;
                p_code = PrintCode(bytes + extrabytes);
                p_operands = fmt::format("{}{}${:02X},PCR{}",
                         br1, addr_mode, offset, br2);
                break;

            case 0x1d : // [,PC + 16 Bit Offset]
                br1 = "[";
                br2 = "]";
                [[fallthrough]];

            case 0x0d :  // ,PC + 16 Bit Offset
                offset = (flx::getValueBigEndian<Word>(&memory[2]) + pc + 4U)
                         & 0xFFFFU;
                addr_mode = ">";
                extrabytes = 2;
                p_code = PrintCode(bytes + extrabytes);
                p_operands = fmt::format("{}{}${:04X},PCR{}",
                         br1, addr_mode, offset, br2);
                break;

            case 0x1f : // [n]
                if (postbyte == 0x9f)
                {
                    br1 = "[";
                    br2 = "]";
                    offset = flx::getValueBigEndian<Word>(&memory[2]);
                    extrabytes = 2;
                    p_code = PrintCode(bytes + extrabytes);
                    p_operands = fmt::format("{}${:04X}{}", br1, offset, br2);
                    break;
                }
                [[fallthrough]];

            default:
                p_code = PrintCode(bytes);
                p_operands = "????";
        }
    }
}


inline void Da6809::D_Extended(const char *mnemo, Byte bytes,
        std::string &p_code, std::string &p_mnemonic, std::string &p_operands)
{
    const auto offset = flx::getValueBigEndian<Word>(&memory[bytes - 2]);

    p_code = PrintCode(bytes);
    const auto *label = FlexLabel(offset);
    p_mnemonic = mnemo;

    if (label == nullptr)
    {
        p_operands = fmt::format("${:04X}", offset);
    }
    else
    {
        p_operands = label;
    }
}

inline void Da6809::D_Relative8(const char *mnemo, Byte bytes,
        DWord &p_jumpaddr, std::string &p_code, std::string &p_mnemonic,
        std::string &p_operands)
{
    Word disp{};
    const auto offset = *(memory + bytes - 1);

    if (offset <= 127)
    {
        disp = pc + 2 + offset;
    }
    else
    {
        disp = pc + 2 - (256 - offset);
    }
    p_jumpaddr = disp;

    p_code = PrintCode(bytes);
    const auto *label = FlexLabel(disp);
    p_mnemonic = mnemo;

    if (label == nullptr)
    {
        p_operands = fmt::format("${:04X}", disp);
    }
    else
    {
        p_operands = label;
    }
}

inline void Da6809::D_Relative16(const char *mnemo, Byte bytes,
        DWord &p_jumpaddr, std::string &p_code, std::string &p_mnemonic,
        std::string &p_operands)
{
    const auto offset = flx::getValueBigEndian<Word>(&memory[bytes - 2]);

    if (offset <= 32767)
    {
        p_jumpaddr = pc + bytes + offset;
    }
    else
    {
        p_jumpaddr = pc + bytes - (65536 - offset);
    }

    p_code = PrintCode(bytes);
    const auto address = static_cast<Word>(p_jumpaddr);
    const auto *label = FlexLabel(address);
    p_mnemonic = mnemo;

    if (label == nullptr)
    {
        p_operands = fmt::format("${:04X}", address);
    }
    else
    {
        p_operands = label;
    }
}

inline void Da6809::D_RegisterRegister(const char *mnemo, Byte bytes,
        std::string &p_code, std::string &p_mnemonic, std::string &p_operands)
{
    const auto postbyte = *(memory + 1);

    p_code = PrintCode(bytes);
    p_mnemonic = mnemo;
    p_operands = fmt::format("{},{}",
             InterRegister(postbyte >> 4U),
             InterRegister(postbyte & 0x0FU));
}

inline void Da6809::D_RegisterList(const char *mnemo, const char *ns_reg,
        Byte bytes, std::string &p_code, std::string &p_mnemonic,
        std::string &p_operands)
{
    const auto postbyte = *(memory + 1);

    p_code = PrintCode(bytes);
    p_mnemonic = mnemo;

    if (postbyte == 0)
    {
        p_operands = "??";
    }
    else
    {
        bool withComma = false;

        p_operands = "";
        for (Byte i = 0; i < 8; i++)
        {
            if (postbyte & (1U << i))
            {
                p_operands.append(withComma ? "," : "");
                p_operands.append(StackRegister(i, ns_reg));
                withComma = true;
            }
        }
    }
}

inline InstFlg Da6809::D_Page2(InstFlg p_flags, DWord &p_jumpaddr,
        std::string &p_code, std::string &p_mnemonic, std::string &p_operands)
{
    const auto code = *(memory + 1);

    switch (code)
    {
        case 0x21:
            p_flags |= InstFlg::JumpAddr;
            D_Relative16("LBRN", 4, p_jumpaddr, p_code, p_mnemonic, p_operands);
            break;

        case 0x22:
            p_flags |= InstFlg::JumpAddr;
            D_Relative16("LBHI", 4, p_jumpaddr, p_code, p_mnemonic, p_operands);
            break;

        case 0x23:
            p_flags |= InstFlg::JumpAddr;
            D_Relative16("LBLS", 4, p_jumpaddr, p_code, p_mnemonic, p_operands);
            break;

        case 0x24:
            p_flags |= InstFlg::JumpAddr;
            D_Relative16("LBCC", 4, p_jumpaddr, p_code, p_mnemonic, p_operands);
            break;

        case 0x25:
            p_flags |= InstFlg::JumpAddr;
            D_Relative16("LBCS", 4, p_jumpaddr, p_code, p_mnemonic, p_operands);
            break;

        case 0x26:
            p_flags |= InstFlg::JumpAddr;
            D_Relative16("LBNE", 4, p_jumpaddr, p_code, p_mnemonic, p_operands);
            break;

        case 0x27:
            p_flags |= InstFlg::JumpAddr;
            D_Relative16("LBEQ", 4, p_jumpaddr, p_code, p_mnemonic, p_operands);
            break;

        case 0x28:
            p_flags |= InstFlg::JumpAddr;
            D_Relative16("LBVC", 4, p_jumpaddr, p_code, p_mnemonic, p_operands);
            break;

        case 0x29:
            p_flags |= InstFlg::JumpAddr;
            D_Relative16("LBVS", 4, p_jumpaddr, p_code, p_mnemonic, p_operands);
            break;

        case 0x2a:
            p_flags |= InstFlg::JumpAddr;
            D_Relative16("LBPL", 4, p_jumpaddr, p_code, p_mnemonic, p_operands);
            break;

        case 0x2b:
            p_flags |= InstFlg::JumpAddr;
            D_Relative16("LBMI", 4, p_jumpaddr, p_code, p_mnemonic, p_operands);
            break;

        case 0x2c:
            p_flags |= InstFlg::JumpAddr;
            D_Relative16("LBGE", 4, p_jumpaddr, p_code, p_mnemonic, p_operands);
            break;

        case 0x2d:
            p_flags |= InstFlg::JumpAddr;
            D_Relative16("LBLT", 4, p_jumpaddr, p_code, p_mnemonic, p_operands);
            break;

        case 0x2e:
            p_flags |= InstFlg::JumpAddr;
            D_Relative16("LBGT", 4, p_jumpaddr, p_code, p_mnemonic, p_operands);
            break;

        case 0x2f:
            p_flags |= InstFlg::JumpAddr;
            D_Relative16("LBLE", 4, p_jumpaddr, p_code, p_mnemonic, p_operands);
            break;

        case 0x3f:
            p_flags |= InstFlg::Sub;
            D_Inherent("SWI2", 2, p_code, p_mnemonic);
            break;

        case 0x83:
            p_flags |= InstFlg::LabelAddr;
            D_Immediate16("CMPD", 4, p_code, p_mnemonic, p_operands);
            break;

        case 0x8c:
            p_flags |= InstFlg::LabelAddr;
            D_Immediate16("CMPY", 4, p_code, p_mnemonic, p_operands);
            break;

        case 0x8e:
            p_flags |= InstFlg::LabelAddr;
            D_Immediate16("LDY", 4, p_code, p_mnemonic, p_operands);
            break;

        case 0x93:
            D_Direct("CMPD", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0x9c:
            D_Direct("CMPY", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0x9e:
            D_Direct("LDY", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0x9f:
            D_Direct("STY", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xa3:
            D_Indexed("CMPD", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xac:
            D_Indexed("CMPY", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xae:
            D_Indexed("LDY", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xaf:
            D_Indexed("STY", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xb3:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("CMPD", 4, p_code, p_mnemonic, p_operands);
            break;

        case 0xbc:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("CMPY", 4, p_code, p_mnemonic, p_operands);
            break;

        case 0xbe:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("LDY", 4, p_code, p_mnemonic, p_operands);
            break;

        case 0xbf:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("STY", 4, p_code, p_mnemonic, p_operands);
            break;

        case 0xce:
            p_flags |= InstFlg::LabelAddr;
            D_Immediate16("LDS", 4, p_code, p_mnemonic, p_operands);
            break;

        case 0xde:
            D_Direct("LDS", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xdf:
            D_Direct("STS", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xee:
            D_Indexed("LDS", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xef:
            D_Indexed("STS", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xfe:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("LDS", 4, p_code, p_mnemonic, p_operands);
            break;

        case 0xff:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("STS", 4, p_code, p_mnemonic, p_operands);
            break;

        default:
            return D_Illegal("", 2, p_code, p_mnemonic, p_operands);
    }

    return p_flags;
}


inline InstFlg Da6809::D_Page3(InstFlg p_flags, std::string &p_code,
        std::string &p_mnemonic, std::string &p_operands)
{
    const auto code = *(memory + 1);

    switch (code)
    {
        case 0x3f:
            p_flags |= InstFlg::Sub;
            D_Inherent("SWI3", 2, p_code, p_mnemonic);
            break;

        case 0x83:
            p_flags |= InstFlg::LabelAddr;
            D_Immediate16("CMPU", 4, p_code, p_mnemonic, p_operands);
            break;

        case 0x8c:
            p_flags |= InstFlg::LabelAddr;
            D_Immediate16("CMPS", 4, p_code, p_mnemonic, p_operands);
            break;

        case 0x93:
            D_Direct("CMPU", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0x9c:
            D_Direct("CMPS", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xa3:
            D_Indexed("CMPU", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xac:
            D_Indexed("CMPS", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xb3:
            D_Extended("CMPU", 4, p_code, p_mnemonic, p_operands);
            break;

        case 0xbc:
            D_Extended("CMPS", 4, p_code, p_mnemonic, p_operands);
            break;

        default:
            return D_Illegal("", 2, p_code, p_mnemonic, p_operands);
    }
    return p_flags;
}

InstFlg Da6809::Disassemble(
        const Byte *p_memory,
        DWord p_pc,
        DWord &p_jumpaddr,
        std::string &p_code,
        std::string &p_mnemonic,
        std::string &p_operands)
{
    pc = static_cast<Word>(p_pc);
    memory = p_memory;
    auto p_flags = InstFlg::NONE;
    const auto opcode = *memory;
    p_operands.clear();

    switch (opcode)
    {
        case 0x01:
            if (use_undocumented)
            {
                D_Direct("neg", 2, p_code, p_mnemonic, p_operands);
                break;
            }

            return D_Illegal("", 2, p_code, p_mnemonic, p_operands);

        case 0x00:
            D_Direct("NEG", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x02:
            if (use_undocumented)
            {
                D_Direct("negcom", 2, p_code, p_mnemonic, p_operands);
                break;
            }

            return D_Illegal("", 2, p_code, p_mnemonic, p_operands);

        case 0x03:
            D_Direct("COM", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x04:
            D_Direct("LSR", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x05:
            if (use_undocumented)
            {
                D_Direct("lsr", 2, p_code, p_mnemonic, p_operands);
                break;
            }

            return D_Illegal("", 2, p_code, p_mnemonic, p_operands);

        case 0x06:
            D_Direct("ROR", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x07:
            D_Direct("ASR", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x08:
            D_Direct("LSL", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x09:
            D_Direct("ROL", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x0a:
            D_Direct("DEC", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x0b:
            if (use_undocumented)
            {
                D_Direct("dec", 2, p_code, p_mnemonic, p_operands);
                break;
            }

            return D_Illegal("", 2, p_code, p_mnemonic, p_operands);

        case 0x0c:
            D_Direct("INC", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x0d:
            D_Direct("TST", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x0e:
            p_flags |= InstFlg::Jump;
            D_Direct("JMP", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x0f:
            D_Direct("CLR", 2, p_code, p_mnemonic, p_operands);
            break;

        case PAGE2:
            return D_Page2(p_flags, p_jumpaddr, p_code, p_mnemonic, p_operands);

        case PAGE3:
            return D_Page3(p_flags, p_code, p_mnemonic, p_operands);

        case 0x12:
            p_flags |= InstFlg::Noop;
            D_Inherent("NOP", 1, p_code, p_mnemonic);
            break;

        case 0x13:
            p_flags |= InstFlg::Jump;
            D_Inherent("SYNC", 1, p_code, p_mnemonic);
            break;

        // 0x14, 0x15 is illegal
        case 0x16:
            p_flags |= InstFlg::Jump | InstFlg::JumpAddr;
            D_Relative16("LBRA", 3, p_jumpaddr, p_code, p_mnemonic, p_operands);
            break;

        case 0x17:
            p_flags |= InstFlg::Sub | InstFlg::LabelAddr;
            D_Relative16("LBSR", 3, p_jumpaddr, p_code, p_mnemonic, p_operands);
            break;

        case 0x19:
            D_Inherent("DAA", 1, p_code, p_mnemonic);
            break;

        case 0x1a:
            D_Immediate8("ORCC", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x1c:
            D_Immediate8("ANDCC", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x1d:
            D_Inherent("SEX", 1, p_code, p_mnemonic);
            break;

        case 0x1e:
            D_RegisterRegister("EXG", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x1f:
            D_RegisterRegister("TFR", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x20:
            p_flags |= InstFlg::Jump | InstFlg::JumpAddr;
            D_Relative8("BRA", 2, p_jumpaddr, p_code, p_mnemonic, p_operands);
            break;

        case 0x21:
            p_flags |= InstFlg::JumpAddr;
            D_Relative8("BRN", 2, p_jumpaddr, p_code, p_mnemonic, p_operands);
            break;

        case 0x22:
            p_flags |= InstFlg::JumpAddr;
            D_Relative8("BHI", 2, p_jumpaddr, p_code, p_mnemonic, p_operands);
            break;

        case 0x23:
            p_flags |= InstFlg::JumpAddr;
            D_Relative8("BLS", 2, p_jumpaddr, p_code, p_mnemonic, p_operands);
            break;

        case 0x24:
            p_flags |= InstFlg::JumpAddr;
            D_Relative8("BCC", 2, p_jumpaddr, p_code, p_mnemonic, p_operands);
            break;

        case 0x25:
            p_flags |= InstFlg::JumpAddr;
            D_Relative8("BCS", 2, p_jumpaddr, p_code, p_mnemonic, p_operands);
            break;

        case 0x26:
            p_flags |= InstFlg::JumpAddr;
            D_Relative8("BNE", 2, p_jumpaddr, p_code, p_mnemonic, p_operands);
            break;

        case 0x27:
            p_flags |= InstFlg::JumpAddr;
            D_Relative8("BEQ", 2, p_jumpaddr, p_code, p_mnemonic, p_operands);
            break;

        case 0x28:
            p_flags |= InstFlg::JumpAddr;
            D_Relative8("BVC", 2, p_jumpaddr, p_code, p_mnemonic, p_operands);
            break;

        case 0x29:
            p_flags |= InstFlg::JumpAddr;
            D_Relative8("BVS", 2, p_jumpaddr, p_code, p_mnemonic, p_operands);
            break;

        case 0x2a:
            p_flags |= InstFlg::JumpAddr;
            D_Relative8("BPL", 2, p_jumpaddr, p_code, p_mnemonic, p_operands);
            break;

        case 0x2b:
            p_flags |= InstFlg::JumpAddr;
            D_Relative8("BMI", 2, p_jumpaddr, p_code, p_mnemonic, p_operands);
            break;

        case 0x2c:
            p_flags |= InstFlg::JumpAddr;
            D_Relative8("BGE", 2, p_jumpaddr, p_code, p_mnemonic, p_operands);
            break;

        case 0x2d:
            p_flags |= InstFlg::JumpAddr;
            D_Relative8("BLT", 2, p_jumpaddr, p_code, p_mnemonic, p_operands);
            break;

        case 0x2e:
            p_flags |= InstFlg::JumpAddr;
            D_Relative8("BGT", 2, p_jumpaddr, p_code, p_mnemonic, p_operands);
            break;

        case 0x2f:
            p_flags |= InstFlg::JumpAddr;
            D_Relative8("BLE", 2, p_jumpaddr, p_code, p_mnemonic, p_operands);
            break;

        case 0x30:
            D_Indexed("LEAX", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x31:
            D_Indexed("LEAY", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x32:
            D_Indexed("LEAS", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x33:
            D_Indexed("LEAU", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x34:
            D_RegisterList("PSHS", "U", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x35:
            D_RegisterList("PULS", "U", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x36:
            D_RegisterList("PSHU", "S", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x37:
            D_RegisterList("PULU", "S", 2, p_code, p_mnemonic, p_operands);
            break;

        // 0x38 is illegal
        case 0x39:
            p_flags |= InstFlg::Jump;
            D_Inherent("RTS", 1, p_code, p_mnemonic);
            break;

        case 0x3a:
            D_Inherent("ABX", 1, p_code, p_mnemonic);
            break;

        case 0x3b:
            p_flags |= InstFlg::Jump;
            D_Inherent("RTI", 1, p_code, p_mnemonic);
            break;

        case 0x3c:
            p_flags |= InstFlg::Jump;
            D_Immediate8("CWAI", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x3d:
            D_Inherent("MUL", 1, p_code, p_mnemonic);
            break;

        case 0x3e:
            if (use_undocumented)
            {
                D_Inherent("reset", 1, p_code, p_mnemonic);
                break;
            }

            return D_Illegal("", 1, p_code, p_mnemonic, p_operands);

        case 0x3f:
            p_flags |= InstFlg::Sub;
            D_Inherent("SWI", 1, p_code, p_mnemonic);
            break;

        case 0x40:
            D_Inherent("NEGA", 1, p_code, p_mnemonic);
            break;

        case 0x41:
            if (use_undocumented)
            {
                D_Inherent("nega", 1, p_code, p_mnemonic);
                break;
            }

            return D_Illegal("", 1, p_code, p_mnemonic, p_operands);

        case 0x42:
            if (use_undocumented)
            {
                D_Inherent("negcoma", 1, p_code, p_mnemonic);
                break;
            }

            return D_Illegal("", 1, p_code, p_mnemonic, p_operands);

        case 0x43:
            D_Inherent("COMA", 1, p_code, p_mnemonic);
            break;

        case 0x44:
            D_Inherent("LSRA", 1, p_code, p_mnemonic);
            break;

        case 0x45:
            if (use_undocumented)
            {
                D_Inherent("lsra", 1, p_code, p_mnemonic);
                break;
            }

            return D_Illegal("", 1, p_code, p_mnemonic, p_operands);

        case 0x46:
            D_Inherent("RORA", 1, p_code, p_mnemonic);
            break;

        case 0x47:
            D_Inherent("ASRA", 1, p_code, p_mnemonic);
            break;

        case 0x48:
            D_Inherent("LSLA", 1, p_code, p_mnemonic);
            break;

        case 0x49:
            D_Inherent("ROLA", 1, p_code, p_mnemonic);
            break;

        case 0x4a:
            D_Inherent("DECA", 1, p_code, p_mnemonic);
            break;

        case 0x4b:
            if (use_undocumented)
            {
                D_Inherent("deca", 1, p_code, p_mnemonic);
                break;
            }

            return D_Illegal("", 1, p_code, p_mnemonic, p_operands);

        case 0x4c:
            D_Inherent("INCA", 1, p_code, p_mnemonic);
            break;

        case 0x4d:
            D_Inherent("TSTA", 1, p_code, p_mnemonic);
            break;

        case 0x4e:
            if (use_undocumented)
            {
                D_Inherent("clra", 1, p_code, p_mnemonic);
                break;
            }

            return D_Illegal("", 1, p_code, p_mnemonic, p_operands);

        case 0x4f:
            D_Inherent("CLRA", 1, p_code, p_mnemonic);
            break;

        case 0x50:
            D_Inherent("NEGB", 1, p_code, p_mnemonic);
            break;

        case 0x51:
            if (use_undocumented)
            {
                D_Inherent("negb", 1, p_code, p_mnemonic);
                break;
            }

            return D_Illegal("", 1, p_code, p_mnemonic, p_operands);

        case 0x52:
            if (use_undocumented)
            {
                D_Inherent("negcomb", 1, p_code, p_mnemonic);
                break;
            }

            return D_Illegal("", 1, p_code, p_mnemonic, p_operands);

        case 0x53:
            D_Inherent("COMB", 1, p_code, p_mnemonic);
            break;

        case 0x54:
            D_Inherent("LSRB", 1, p_code, p_mnemonic);
            break;

        case 0x55:
            if (use_undocumented)
            {
                D_Inherent("lsrb", 1, p_code, p_mnemonic);
                break;
            }

            return D_Illegal("", 1, p_code, p_mnemonic, p_operands);

        case 0x56:
            D_Inherent("RORB", 1, p_code, p_mnemonic);
            break;

        case 0x57:
            D_Inherent("ASRB", 1, p_code, p_mnemonic);
            break;

        case 0x58:
            D_Inherent("LSLB", 1, p_code, p_mnemonic);
            break;

        case 0x59:
            D_Inherent("ROLB", 1, p_code, p_mnemonic);
            break;

        case 0x5a:
            D_Inherent("DECB", 1, p_code, p_mnemonic);
            break;

        case 0x5b:
            if (use_undocumented)
            {
                D_Inherent("decb", 1, p_code, p_mnemonic);
                break;
            }

            return D_Illegal("", 1, p_code, p_mnemonic, p_operands);

        case 0x5c:
            D_Inherent("INCB", 1, p_code, p_mnemonic);
            break;

        case 0x5d:
            D_Inherent("TSTB", 1, p_code, p_mnemonic);
            break;

        case 0x5e:
            if (use_undocumented)
            {
                D_Inherent("clrb", 1, p_code, p_mnemonic);
                break;
            }

            return D_Illegal("", 1, p_code, p_mnemonic, p_operands);

        case 0x5f:
            D_Inherent("CLRB", 1, p_code, p_mnemonic);
            break;

        case 0x60:
            D_Indexed("NEG", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x61:
            if (use_undocumented)
            {
                D_Indexed("neg", 2, p_code, p_mnemonic, p_operands);
                break;
            }

            return D_Illegal("", 2, p_code, p_mnemonic, p_operands);

        case 0x62:
            if (use_undocumented)
            {
                D_Indexed("negcom", 2, p_code, p_mnemonic, p_operands);
                break;
            }

            return D_Illegal("", 2, p_code, p_mnemonic, p_operands);

        case 0x63:
            D_Indexed("COM", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x64:
            D_Indexed("LSR", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x65:
            if (use_undocumented)
            {
                D_Indexed("lsr", 2, p_code, p_mnemonic, p_operands);
                break;
            }

            return D_Illegal("", 2, p_code, p_mnemonic, p_operands);

        case 0x66:
            D_Indexed("ROR", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x67:
            D_Indexed("ASR", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x68:
            D_Indexed("LSL", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x69:
            D_Indexed("ROL", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x6a:
            D_Indexed("DEC", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x6b:
            if (use_undocumented)
            {
                D_Indexed("dec", 2, p_code, p_mnemonic, p_operands);
                break;
            }

            return D_Illegal("", 2, p_code, p_mnemonic, p_operands);

        case 0x6c:
            D_Indexed("INC", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x6d:
            D_Indexed("TST", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x6e:
            p_flags |= InstFlg::Jump | InstFlg::ComputedGoto;
            D_Indexed("JMP", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x6f:
            D_Indexed("CLR", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x70:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("NEG", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0x71:
            if (use_undocumented)
            {
                p_flags |= InstFlg::LabelAddr;
                D_Extended("neg", 3, p_code, p_mnemonic, p_operands);
                break;
            };

            return D_Illegal("", 3, p_code, p_mnemonic, p_operands);

        case 0x72:
            if (use_undocumented)
            {
                p_flags |= InstFlg::LabelAddr;
                D_Extended("negcom", 3, p_code, p_mnemonic, p_operands);
                break;
            };

            return D_Illegal("", 3, p_code, p_mnemonic, p_operands);

        case 0x73:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("COM", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0x74:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("LSR", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0x75:
            if (use_undocumented)
            {
                p_flags |= InstFlg::LabelAddr;
                D_Extended("lsr", 3, p_code, p_mnemonic, p_operands);
                break;
            };

            return D_Illegal("", 3, p_code, p_mnemonic, p_operands);

        case 0x76:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("ROR", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0x77:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("ASR", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0x78:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("LSL", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0x79:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("ROL", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0x7a:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("DEC", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0x7b:
            if (use_undocumented)
            {
                p_flags |= InstFlg::LabelAddr;
                D_Extended("dec", 3, p_code, p_mnemonic, p_operands);
                break;
            };

            return D_Illegal("", 3, p_code, p_mnemonic, p_operands);

        case 0x7c:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("INC", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0x7d:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("TST", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0x7e:
            p_flags |= InstFlg::Jump;
            D_Extended("JMP", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0x7f:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("CLR", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0x80:
            D_Immediate8("SUBA", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x81:
            D_Immediate8("CMPA", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x82:
            D_Immediate8("SBCA", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x83:
            D_Immediate16("SUBD", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0x84:
            D_Immediate8("ANDA", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x85:
            D_Immediate8("BITA", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x86:
            D_Immediate8("LDA", 2, p_code, p_mnemonic, p_operands);
            break;

        // 0x87 is illegal
        case 0x88:
            D_Immediate8("EORA", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x89:
            D_Immediate8("ADCA", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x8a:
            D_Immediate8("ORA", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x8b:
            D_Immediate8("ADDA", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x8c:
            D_Immediate16("CMPX", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0x8d:
            p_flags |= InstFlg::Sub;
            D_Relative8("BSR", 2, p_jumpaddr, p_code, p_mnemonic, p_operands);
            break;

        case 0x8e:
            D_Immediate16("LDX", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0x90:
            D_Direct("SUBA", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x91:
            D_Direct("CMPA", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x92:
            D_Direct("SBCA", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x93:
            D_Direct("SUBD", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x94:
            D_Direct("ANDA", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x95:
            D_Direct("BITA", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x96:
            D_Direct("LDA", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x97:
            D_Direct("STA", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x98:
            D_Direct("EORA", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x99:
            D_Direct("ADCA", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x9a:
            D_Direct("ORA", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x9b:
            D_Direct("ADDA", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x9c:
            D_Direct("CMPX", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x9d:
            p_flags |= InstFlg::Sub;
            D_Direct("JSR", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x9e:
            D_Direct("LDX", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0x9f:
            D_Direct("STX", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xa0:
            D_Indexed("SUBA", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xa1:
            D_Indexed("CMPA", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xa2:
            D_Indexed("SBCA", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xa3:
            D_Indexed("SUBD", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xa4:
            D_Indexed("ANDA", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xa5:
            D_Indexed("BITA", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xa6:
            D_Indexed("LDA", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xa7:
            D_Indexed("STA", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xa8:
            D_Indexed("EORA", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xa9:
            D_Indexed("ADCA", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xaa:
            D_Indexed("ORA", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xab:
            D_Indexed("ADDA", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xac:
            D_Indexed("CMPX", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xad:
            p_flags |= InstFlg::Sub | InstFlg::ComputedGoto;
            D_Indexed("JSR", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xae:
            D_Indexed("LDX", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xaf:
            D_Indexed("STX", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xb0:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("SUBA", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xb1:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("CMPA", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xb2:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("SBCA", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xb3:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("SUBD", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xb4:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("ANDA", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xb5:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("BITA", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xb6:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("LDA", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xb7:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("STA", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xb8:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("EORA", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xb9:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("ADCA", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xba:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("ORA", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xbb:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("ADDA", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xbc:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("CMPX", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xbd:
            p_flags |= InstFlg::Sub | InstFlg::JumpAddr;
            D_Extended("JSR", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xbe:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("LDX", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xbf:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("STX", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xc0:
            D_Immediate8("SUBB", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xc1:
            D_Immediate8("CMPB", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xc2:
            D_Immediate8("SBCB", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xc3:
            p_flags |= InstFlg::LabelAddr;
            D_Immediate16("ADDD", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xc4:
            D_Immediate8("ANDB", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xc5:
            D_Immediate8("BITB", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xc6:
            D_Immediate8("LDB", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xc8:
            D_Immediate8("EORB", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xc9:
            D_Immediate8("ADCB", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xca:
            D_Immediate8("ORB", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xcb:
            D_Immediate8("ADDB", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xcc:
            p_flags |= InstFlg::LabelAddr;
            D_Immediate16("LDD", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xce:
            p_flags |= InstFlg::LabelAddr;
            D_Immediate16("LDU", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xd0:
            D_Direct("SUBB", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xd1:
            D_Direct("CMPB", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xd2:
            D_Direct("SBCB", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xd3:
            D_Direct("ADDD", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xd4:
            D_Direct("ANDB", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xd5:
            D_Direct("BITB", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xd6:
            D_Direct("LDB", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xd7:
            D_Direct("STB", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xd8:
            D_Direct("EORB", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xd9:
            D_Direct("ADCB", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xda:
            D_Direct("ORB", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xdb:
            D_Direct("ADDB", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xdc:
            D_Direct("LDD", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xdd:
            D_Direct("STD", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xde:
            D_Direct("LDU", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xdf:
            D_Direct("STU", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xe0:
            D_Indexed("SUBB", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xe1:
            D_Indexed("CMPB", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xe2:
            D_Indexed("SBCB", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xe3:
            D_Indexed("ADDD", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xe4:
            D_Indexed("ANDB", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xe5:
            D_Indexed("BITB", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xe6:
            D_Indexed("LDB", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xe7:
            D_Indexed("STB", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xe8:
            D_Indexed("EORB", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xe9:
            D_Indexed("ADCB", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xea:
            D_Indexed("ORB", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xeb:
            D_Indexed("ADDB", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xec:
            D_Indexed("LDD", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xed:
            D_Indexed("STD", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xee:
            D_Indexed("LDU", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xef:
            D_Indexed("STU", 2, p_code, p_mnemonic, p_operands);
            break;

        case 0xf0:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("SUBB", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xf1:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("CMPB", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xf2:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("SBCB", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xf3:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("ADDD", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xf4:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("ANDB", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xf5:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("BITB", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xf6:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("LDB", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xf7:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("STB", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xf8:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("EORB", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xf9:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("ADCB", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xfa:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("ORB", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xfb:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("ADDB", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xfc:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("LDD", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xfd:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("STD", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xfe:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("LDU", 3, p_code, p_mnemonic, p_operands);
            break;

        case 0xff:
            p_flags |= InstFlg::LabelAddr;
            D_Extended("STU", 3, p_code, p_mnemonic, p_operands);
            break;

        default:
            return D_Illegal("", 1, p_code, p_mnemonic, p_operands);
    }

    return p_flags;
}

unsigned Da6809::getByteSize(const Byte *p_memory)
{
    constexpr const Byte X = 1;
    constexpr const Byte Y = 2;
    // Bit 0-3: byte size.
    // Bit 4: Flag, if set, add additional bytes for index mode.
    // Bit 5-7: reserved, should be 0.
    // Table includes byte sizes of undocumented instructions.
    constexpr const std::array<Byte, 256> byteSizesPage1
    {//-0 -1 -2 -3 -4 -5 -6 -7 -8 -9 -A -B -C -D -E -F
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // 0-
        X, X, 1, 1, X, X, 3, 3, X, 1, 2, X, 2, 1, 2, 2, // 1-
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // 2-
       18,18,18,18, 2, 2, 2, 2, X, 1, 1, 1, 2, 1, 1, 1, // 3-
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 4-
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 5-
       18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18, // 6-
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, // 7-
        2, 2, 2, 3, 2, 2, 2, X, 2, 2, 2, 2, 3, 2, 3, X, // 8-
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // 9-
       18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18, // A-
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, // B-
        2, 2, 2, 3, 2, 2, 2, X, 2, 2, 2, 2, 3, X, 3, X, // C-
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // D-
       18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18, // E-
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, // F-
    };

    constexpr const std::array<Byte, 256> byteSizesPage2
    {//-0 -1 -2 -3 -4 -5 -6 -7 -8 -9 -A -B -C -D -E -F
        Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, // 0-
        Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, // 1-
        Y, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, // 2-
        Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, 2, // 3-
        Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, // 4-
        Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, // 5-
        Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, // 6-
        Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, // 7-
        Y, Y, Y, 4, Y, Y, Y, Y, Y, Y, Y, Y, 4, Y, 4, Y, // 8-
        Y, Y, Y, 3, Y, Y, Y, Y, Y, Y, Y, Y, 3, Y, 3, 3, // 9-
        Y, Y, Y,19, Y, Y, Y, Y, Y, Y, Y, Y,19, Y,19,19, // A-
        Y, Y, Y, 4, Y, Y, Y, Y, Y, Y, Y, Y, 4, Y, 4, 4, // B-
        Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, 4, Y, // C-
        Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, 3, 3, // D-
        Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y,19,19, // E-
        Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, 4, 4, // F-
    };

    constexpr const std::array<Byte, 256> byteSizesPage3
    {//-0 -1 -2 -3 -4 -5 -6 -7 -8 -9 -A -B -C -D -E -F
        Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, // 0-
        Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, // 1-
        Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, // 2-
        Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, 2, // 3-
        Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, // 4-
        Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, // 5-
        Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, // 6-
        Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, // 7-
        Y, Y, Y, 4, Y, Y, Y, Y, Y, Y, Y, Y, 4, Y, Y, Y, // 8-
        Y, Y, Y, 3, Y, Y, Y, Y, Y, Y, Y, Y, 3, Y, Y, Y, // 9-
        Y, Y, Y,19, Y, Y, Y, Y, Y, Y, Y, Y,19, Y, Y, Y, // A-
        Y, Y, Y, 4, Y, Y, Y, Y, Y, Y, Y, Y, 4, Y, Y, Y, // B-
        Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, // C-
        Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, // D-
        Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, // E-
        Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, Y, // F-
    };

    // Contains the additional bytes using postbyte as index.
    constexpr const std::array<Byte, 256> additionalIndexedByteSize
    {//-0 -1 -2 -3 -4 -5 -6 -7 -8 -9 -A -B -C -D -E -F
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0-
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 1-
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 2-
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 3-
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 4-
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 5-
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 6-
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 7-
        0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 0, 1, 2, 0, 0, // 8-
        0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 0, 1, 2, 0, 2, // 9-
        0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 0, 1, 2, 0, 2, // A-
        2, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 0, 1, 2, 0, 0, // B-
        0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 0, 1, 2, 0, 0, // C-
        0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 0, 1, 2, 0, 0, // D-
        0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 0, 1, 2, 0, 0, // E-
        0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 0, 1, 2, 0, 0, // F-
    };

    assert(p_memory != nullptr);

    unsigned byteSize = 0U;
    Byte opcode = *(p_memory++);
    switch (opcode)
    {
        case PAGE2:
        opcode = *(p_memory++);
        byteSize = byteSizesPage2[opcode];
        break;

        case PAGE3:
        opcode = *(p_memory++);
        byteSize = byteSizesPage3[opcode];
        break;

        default:
        byteSize = byteSizesPage1[opcode];
        break;
    }

    if ((byteSize & 0x10U) != 0U)
    {
        return (byteSize & 0x0FU) + additionalIndexedByteSize[*p_memory];
    }

    return byteSize;
}

