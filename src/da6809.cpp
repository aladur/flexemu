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


#include "misc1.h"
#include <stdio.h>
#include <array>
#include <map>
#include "da6809.h"


Da6809::Da6809()
{
    memset(code_buf, 0, sizeof(code_buf));
    memset(mnem_buf, 0, sizeof(mnem_buf));
}

void Da6809::set_use_undocumented(bool value)
{
    use_undocumented = value;
}

const char *Da6809::FlexLabel(Word addr)
{
#ifdef FLEX_LABEL
    static const std::map<Word, const char *> label_for_address{
        // FLEX DOS entries:
        { 0xCD00, "COLDS" },
        { 0xCD03, "WARMS" },
        { 0xCD06, "RENTER" },
        { 0xCD09, "INCH" },
        { 0xCD0C, "INCH2" },
        { 0xCD0F, "OUTCH" },
        { 0xCD12, "OUTCH2" },
        { 0xCD15, "GETCHR" },
        { 0xCD18, "PUTCHR" },
        { 0xCD1B, "INBUFF" },
        { 0xCD1E, "PSTRNG" },
        { 0xCD21, "CLASS" },
        { 0xCD24, "PCRLF" },
        { 0xCD27, "NXTCH" },
        { 0xCD2A, "RSTRIO" },
        { 0xCD2D, "GETFIL" },
        { 0xCD30, "LOAD" },
        { 0xCD33, "SETEXT" },
        { 0xCD36, "ADDBX" },
        { 0xCD39, "OUTDEC" },
        { 0xCD3C, "OUTHEX" },
        { 0xCD3F, "RPTERR" },
        { 0xCD42, "GETHEX" },
        { 0xCD45, "OUTADR" },
        { 0xCD48,  "INDEC" },
        { 0xCD4B, "DOCMND" },
        { 0xCD4E, "STAT" },
        // FLEX FMS entries:
        { 0xD400, "FMSINI" }, // FMS init
        { 0xD403, "FMSCLS" }, // FMS close
        { 0xD406, "FMS" },
        { 0xC840, "FCB" }, // standard system FCB
        // miscellenious:
        { 0xD435, "VFYFLG" }, // FMS verify flag
        { 0xC080, "LINBUF" }, // line buffer
        { 0xCC00, "TTYBS" },
        { 0xCC01, "TTYDEL" },
        { 0xCC02, "TTYEOL" },
        { 0xCC03, "TTYDPT" },
        { 0xCC04, "TTYWDT" },
        { 0xCC11, "TTYTRM" },
        { 0xCC12, "COMTBL" }, // user command table
        { 0xCC14, "LINBFP" }, // line buffer pointer
        { 0xCC16, "ESCRET" }, // escape return register
        { 0xCC18, "LINCHR" }, // current char in linebuffer
        { 0xCC19, "LINPCH" }, // previous char in linebuffer
        { 0xCC1A, "LINENR" }, //line nr of current page
        { 0xCC1B, "LODOFS" }, // loader address offset
        { 0xCC1D, "TFRFLG" }, // loader transfer flag
        { 0xCC1E, "TFRADR" }, // transfer address
        { 0xCC20, "FMSERR" }, // FMS error type
        { 0xCC21, "IOFLG" }, // special I/O flag
        { 0xCC22, "OUTSWT" }, // output switch
        { 0xCC23, "INSWT" }, // input switch
        { 0xCC24, "OUTADR" }, // file output address
        { 0xCC26, "INADR" }, // file input address
        { 0xCC28, "COMFLG" }, // command flag
        { 0xCC29, "OUTCOL" }, // current output column
        { 0xCC2A, "SCRATC" }, // system scratch
        { 0xCC2B, "MEMEND" }, // memory end
        { 0xCC2D, "ERRVEC" }, // error name vector
        { 0xCC2F, "INECHO" }, // file input echo flag
        // printer support
        { 0xCCC0, "PRTINI" }, // printer initialize
        { 0xCCD8, "PRTCHK" }, // printer check
        { 0xCCE4, "PRTOUT" }, // printer output
    };

    const auto iter = label_for_address.find(addr);

    if (iter != label_for_address.end())
    {
        return iter->second;
    }
#endif // #ifdef FLEX_LABEL

    return nullptr;
}

const char *Da6809::IndexedRegister(Byte which)
{
    static const std::array<const char *, 4> reg_names{ "X", "Y", "U", "S" };

    return reg_names[which & 0x03];
}

const char *Da6809::InterRegister(Byte which)
{
    static const std::array<const char *, 16> reg_names{
        "D", "X", "Y", "U", "S", "PC", "??", "??",
        "A", "B", "CC", "DP", "??", "??", "??", "??"
    };

    return reg_names[which & 0x0f];
}


const char *Da6809::StackRegister(Byte which, const char *not_stack)
{
    static const std::array<const char *, 8> reg_names{
        "CC", "A", "B", "DP", "X", "Y", "??", "PC"
    };

    if ((which & 0x07) == 6)
    {
        return not_stack;
    }

    return reg_names[which & 0x07];
}


inline Byte Da6809::D_Illegal(const char *mnemo, Byte bytes,
        std::string &p_code, std::string &p_mnemonic)
{
    const auto code = *memory;

    snprintf(code_buf, sizeof(code_buf), "%04X: %02X", pc, code);
    snprintf(mnem_buf, sizeof(mnem_buf), "%s ?????", mnemo);
    p_code = code_buf;
    p_mnemonic = mnem_buf;

    return bytes;
}


inline Byte Da6809::D_Direct(const char *mnemo, Byte bytes, std::string &p_code,
                             std::string &p_mnemonic)
{
    const auto offset = *(memory + bytes - 1);

    PrintCode(bytes);
    snprintf(mnem_buf, sizeof(mnem_buf), "%s $%02X", mnemo, offset);
    p_code = code_buf;
    p_mnemonic = mnem_buf;

    return bytes;
}


inline Byte Da6809::D_Immediat(const char *mnemo, Byte bytes,
        std::string &p_code, std::string &p_mnemonic)
{
    const auto offset = *(memory + bytes - 1);

    PrintCode(bytes);
    snprintf(mnem_buf, sizeof(mnem_buf), "%s #$%02X", mnemo, offset);
    p_code = code_buf;
    p_mnemonic = mnem_buf;

    return bytes;
}


inline Byte Da6809::D_ImmediatL(const char *mnemo, Byte bytes,
        std::string &p_code, std::string &p_mnemonic)
{
    const auto offset = getValueBigEndian<Word>(&memory[bytes - 2]);

    PrintCode(bytes);
    p_code = code_buf;

    const auto *label = FlexLabel(offset);

    if (label == nullptr)
    {
        snprintf(mnem_buf, sizeof(mnem_buf), "%s #$%04X", mnemo, offset);
    }
    else
    {
        snprintf(mnem_buf, sizeof(mnem_buf), "%s #%s ; $%04X", mnemo, label,
                 offset);
    }
    p_mnemonic = mnem_buf;

    return bytes;
}

inline std::string Da6809::PrintCode(int bytes)
{
    auto index = snprintf(code_buf, sizeof(code_buf), "%04X:", pc);

    for (int i = 0; i < bytes; ++i)
    {
        index += snprintf(code_buf + index, sizeof(code_buf) - index, " %02X",
                          memory[i]);
    }

    return {code_buf};
}

inline Byte Da6809::D_Inherent(const char *mnemo, Byte bytes,
        std::string &p_code, std::string &p_mnemonic)
{
    p_code = PrintCode(bytes);
    p_mnemonic = mnemo;

    return bytes;
}


Byte Da6809::D_Indexed(const char *mnemo, Byte bytes, std::string &p_code,
        std::string &p_mnemonic)
{
    Byte disp;
    Word offset;
    Byte extrabytes = 0U;
    const auto postbyte = *(memory + bytes - 1);
    const char *br1 = ""; // opening bracket if indirect addressing
    const char *br2 = ""; // closing bracket if indirect addressing
    const char *s = ""; // minus sign for offset

    if ((postbyte & 0x80) == 0x00)
    {
        // ,R + 5 Bit Offset
        disp = postbyte & 0x1f;

        if ((postbyte & 0x10) == 0x10)
        {
            s = "-";
            disp = 0x20 - disp;
        }

        p_code = PrintCode(bytes);
        snprintf(mnem_buf, sizeof(mnem_buf), "%s %s$%02X,%s", mnemo, s, disp,
                 IndexedRegister(postbyte >> 5));
    }
    else
    {
        switch (postbyte & 0x1f)
        {
            case 0x00 : // ,R+
                PrintCode(bytes);
                snprintf(mnem_buf, sizeof(mnem_buf), "%s ,%s+", mnemo,
                         IndexedRegister(postbyte >> 5));
                break;

            case 0x11 : // [,R++]
                br1 = "[";
                br2 = "]";
                FALLTHROUGH;

            case 0x01 : // ,R++
                PrintCode(bytes);
                snprintf(mnem_buf, sizeof(mnem_buf), "%s %s,%s++%s", mnemo,
                        br1, IndexedRegister(postbyte >> 5), br2);
                break;

            case 0x02 : // ,-R
                PrintCode(bytes);
                snprintf(mnem_buf, sizeof(mnem_buf), "%s ,-%s", mnemo,
                        IndexedRegister(postbyte >> 5));
                break;

            case 0x13 : // [,R--]
                br1 = "[";
                br2 = "]";
                FALLTHROUGH;

            case 0x03 : // ,--R
                PrintCode(bytes);
                snprintf(mnem_buf, sizeof(mnem_buf), "%s %s,--%s%s", mnemo,
                        br1, IndexedRegister(postbyte >> 5), br2);
                break;

            case 0x14 : // [,R--]
                br1 = "[";
                br2 = "]";
                FALLTHROUGH;

            case 0x04 : // ,R
                PrintCode(bytes);
                snprintf(mnem_buf, sizeof(mnem_buf), "%s %s,%s%s", mnemo,
                        br1, IndexedRegister(postbyte >> 5), br2);
                break;

            case 0x15 : // [B,R]
                br1 = "[";
                br2 = "]";
                FALLTHROUGH;

            case 0x05 : // B,R
                PrintCode(bytes);
                snprintf(mnem_buf, sizeof(mnem_buf), "%s %sB,%s%s", mnemo,
                        br1, IndexedRegister(postbyte >> 5), br2);
                break;

            case 0x16 : // [A,R]
                br1 = "[";
                br2 = "]";
                FALLTHROUGH;

            case 0x06 : // A,R
                PrintCode(bytes);
                snprintf(mnem_buf, sizeof(mnem_buf), "%s %sA,%s%s", mnemo,
                        br1, IndexedRegister(postbyte >> 5), br2);
                break;

            case 0x18 : // [,R + 8 Bit Offset]
                br1 = "[";
                br2 = "]";
                FALLTHROUGH;

            case 0x08 : // ,R + 8 Bit Offset
                offset = *(memory + 2);

                if (offset < 128)
                {
                    s = "";
                }
                else
                {
                    s = "-";
                    offset = 0x0100 - offset;
                }
                extrabytes = 1;
                PrintCode(bytes + extrabytes);
                snprintf(mnem_buf, sizeof(mnem_buf), "%s %s%s$%02X,%s%s",
                         mnemo, br1, s, offset,
                         IndexedRegister(postbyte >> 5), br2);
                break;

            case 0x19 : // [,R + 16 Bit Offset]
                br1 = "[";
                br2 = "]";
                FALLTHROUGH;

            case 0x09 : // ,R + 16 Bit Offset
                extrabytes = 2;
                PrintCode(bytes + extrabytes);
                offset = getValueBigEndian<Word>(&memory[2]);
                s = "";
                snprintf(mnem_buf, sizeof(mnem_buf), "%s %s%s$%04X,%s%s",
                         mnemo, br1, s, offset,
                         IndexedRegister(postbyte >> 5), br2);
                break;

            case 0x1b : // [D,R]
                br1 = "[";
                br2 = "]";
                FALLTHROUGH;

            case 0x0b : // D,R
                PrintCode(bytes);
                snprintf(mnem_buf, sizeof(mnem_buf), "%s %sD,%s%s", mnemo,
                        br1, IndexedRegister(postbyte >> 5), br2);
                break;

            case 0x1c : // [,PC + 8 Bit Offset]
                br1 = "[";
                br2 = "]";
                FALLTHROUGH;

            case 0x0c : // ,PC + 8 Bit Offset
                offset = (EXTEND8(*(memory + 2)) + pc + 3) & 0xFFFF;
                s = "<";
                extrabytes = 1;
                PrintCode(bytes + extrabytes);
                snprintf(mnem_buf, sizeof(mnem_buf), "%s %s%s$%02X,PCR%s",
                         mnemo, br1, s, offset, br2);
                break;

            case 0x1d : // [,PC + 16 Bit Offset]
                br1 = "[";
                br2 = "]";
                FALLTHROUGH;

            case 0x0d :  // ,PC + 16 Bit Offset
                offset = (getValueBigEndian<Word>(&memory[2]) + pc + 4) & 0xFFFF;
                s = ">";
                extrabytes = 2;
                PrintCode(bytes + extrabytes);
                snprintf(mnem_buf, sizeof(mnem_buf), "%s %s%s$%04X,PCR%s",
                         mnemo, br1, s, offset, br2);
                break;

            case 0x1f : // [n]
                if (postbyte == 0x9f)
                {
                    br1 = "[";
                    br2 = "]";
                    offset = getValueBigEndian<Word>(&memory[2]);
                    extrabytes = 2;
                    PrintCode(bytes + extrabytes);
                    snprintf(mnem_buf, sizeof(mnem_buf), "%s %s$%04X%s", mnemo, br1,
                             offset, br2);
                    break;
                }
                FALLTHROUGH;

            default:
                PrintCode(bytes);
                snprintf(mnem_buf, sizeof(mnem_buf), "%s ????", mnemo);
        }
    }

    p_code = code_buf;
    p_mnemonic = mnem_buf;

    return bytes + extrabytes;
}


inline Byte Da6809::D_Extended(const char *mnemo, Byte bytes,
        std::string &p_code, std::string &p_mnemonic)
{
    const auto offset = getValueBigEndian<Word>(&memory[bytes - 2]);

    p_code = PrintCode(bytes);
    const auto *label = FlexLabel(offset);

    if (label == nullptr)
    {
        snprintf(mnem_buf, sizeof(mnem_buf), "%s $%04X", mnemo, offset);
    }
    else
    {
        snprintf(mnem_buf, sizeof(mnem_buf), "%s %s ; $%04X", mnemo, label,
                 offset);
    }
    p_mnemonic = mnem_buf;

    return bytes;
}

inline Byte Da6809::D_Relative(const char *mnemo, Byte bytes,
        DWord &p_jumpaddr, std::string &p_code, std::string &p_mnemonic)
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

    if (label == nullptr)
    {
        snprintf(mnem_buf, sizeof(mnem_buf), "%s   $%04X", mnemo, disp);
    }
    else
    {
        snprintf(mnem_buf, sizeof(mnem_buf), "%s   %s ; $%04X", mnemo, label,
                 disp);
    }
    p_mnemonic = mnem_buf;

    return bytes;
}

inline Byte Da6809::D_RelativeL(const char *mnemo, Byte bytes,
        DWord &p_jumpaddr, std::string &p_code, std::string &p_mnemonic)
{
    const auto offset = getValueBigEndian<Word>(&memory[bytes - 2]);

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

    if (label == nullptr)
    {
        snprintf(mnem_buf, sizeof(mnem_buf), "%s  $%04X", mnemo, address);
    }
    else
    {
        snprintf(mnem_buf, sizeof(mnem_buf), "%s  %s ; $%04X", mnemo, label,
                 address);
    }
    p_mnemonic = mnem_buf;

    return bytes;
}

inline Byte Da6809::D_Register0(const char *mnemo, Byte bytes,
        std::string &p_code, std::string &p_mnemonic)
{
    const auto postbyte = *(memory + 1);

    p_code = PrintCode(bytes);
    snprintf(mnem_buf, sizeof(mnem_buf), "%s %s,%s", mnemo,
             InterRegister(postbyte >> 4),
             InterRegister(postbyte & 0x0f));
    p_mnemonic = mnem_buf;

    return bytes;
}

inline Byte Da6809::D_Register1(const char *mnemo, const char *ns_reg,
        Byte bytes, std::string &p_code, std::string &p_mnemonic)
{
    const auto postbyte = *(memory + 1);
    bool withComma = false;

    p_code = PrintCode(bytes);
    auto index = snprintf(mnem_buf, sizeof(mnem_buf), "%s ", mnemo);

    if (postbyte == 0)
    {
        snprintf(&mnem_buf[index], sizeof(mnem_buf) - index, "??");
    }
    else
    {
        for (Byte i = 0; i < 8; i++)
        {
            if (postbyte & (1 << i))
            {
                index += snprintf(&mnem_buf[index], sizeof(mnem_buf) - index,
                        "%s%s", withComma ? "," : "", StackRegister(i, ns_reg));
                withComma = true;
            }
        }
    }
    p_mnemonic = mnem_buf;

    return bytes;
}

inline Byte Da6809::D_Page10(InstFlg &p_flags, DWord &p_jumpaddr,
        std::string &p_code, std::string &p_mnemonic)
{
    const auto code = *(memory + 1);

    switch (code)
    {
        case 0x21:
            p_flags |= InstFlg::JumpAddr;
            return D_RelativeL("LBRN", 4, p_jumpaddr, p_code, p_mnemonic);

        case 0x22:
            p_flags |= InstFlg::JumpAddr;
            return D_RelativeL("LBHI", 4, p_jumpaddr, p_code, p_mnemonic);

        case 0x23:
            p_flags |= InstFlg::JumpAddr;
            return D_RelativeL("LBLS", 4, p_jumpaddr, p_code, p_mnemonic);

        case 0x24:
            p_flags |= InstFlg::JumpAddr;
            return D_RelativeL("LBCC", 4, p_jumpaddr, p_code, p_mnemonic);

        case 0x25:
            p_flags |= InstFlg::JumpAddr;
            return D_RelativeL("LBCS", 4, p_jumpaddr, p_code, p_mnemonic);

        case 0x26:
            p_flags |= InstFlg::JumpAddr;
            return D_RelativeL("LBNE", 4, p_jumpaddr, p_code, p_mnemonic);

        case 0x27:
            p_flags |= InstFlg::JumpAddr;
            return D_RelativeL("LBEQ", 4, p_jumpaddr, p_code, p_mnemonic);

        case 0x28:
            p_flags |= InstFlg::JumpAddr;
            return D_RelativeL("LBVC", 4, p_jumpaddr, p_code, p_mnemonic);

        case 0x29:
            p_flags |= InstFlg::JumpAddr;
            return D_RelativeL("LBVS", 4, p_jumpaddr, p_code, p_mnemonic);

        case 0x2a:
            p_flags |= InstFlg::JumpAddr;
            return D_RelativeL("LBPL", 4, p_jumpaddr, p_code, p_mnemonic);

        case 0x2b:
            p_flags |= InstFlg::JumpAddr;
            return D_RelativeL("LBMI", 4, p_jumpaddr, p_code, p_mnemonic);

        case 0x2c:
            p_flags |= InstFlg::JumpAddr;
            return D_RelativeL("LBGE", 4, p_jumpaddr, p_code, p_mnemonic);

        case 0x2d:
            p_flags |= InstFlg::JumpAddr;
            return D_RelativeL("LBLT", 4, p_jumpaddr, p_code, p_mnemonic);

        case 0x2e:
            p_flags |= InstFlg::JumpAddr;
            return D_RelativeL("LBGT", 4, p_jumpaddr, p_code, p_mnemonic);

        case 0x2f:
            p_flags |= InstFlg::JumpAddr;
            return D_RelativeL("LBLE", 4, p_jumpaddr, p_code, p_mnemonic);

        case 0x3f:
            p_flags |= InstFlg::Sub;
            return D_Inherent("SWI2", 2, p_code, p_mnemonic);

        case 0x83:
            p_flags |= InstFlg::LabelAddr;
            return D_ImmediatL("CMPD ", 4, p_code, p_mnemonic);

        case 0x8c:
            p_flags |= InstFlg::LabelAddr;
            return D_ImmediatL("CMPY ", 4, p_code, p_mnemonic);

        case 0x8e:
            p_flags |= InstFlg::LabelAddr;
            return D_ImmediatL("LDY  ", 4, p_code, p_mnemonic);

        case 0x93:
            return D_Direct("CMPD ", 3, p_code, p_mnemonic);

        case 0x9c:
            return D_Direct("CMPY ", 3, p_code, p_mnemonic);

        case 0x9e:
            return D_Direct("LDY  ", 3, p_code, p_mnemonic);

        case 0x9f:
            return D_Direct("STY  ", 3, p_code, p_mnemonic);

        case 0xa3:
            return D_Indexed("CMPD ", 3, p_code, p_mnemonic);

        case 0xac:
            return D_Indexed("CMPY ", 3, p_code, p_mnemonic);

        case 0xae:
            return D_Indexed("LDY  ", 3, p_code, p_mnemonic);

        case 0xaf:
            return D_Indexed("STY  ", 3, p_code, p_mnemonic);

        case 0xb3:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("CMPD ", 4, p_code, p_mnemonic);

        case 0xbc:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("CMPY ", 4, p_code, p_mnemonic);

        case 0xbe:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("LDY  ", 4, p_code, p_mnemonic);

        case 0xbf:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("STY  ", 4, p_code, p_mnemonic);

        case 0xce:
            p_flags |= InstFlg::LabelAddr;
            return D_ImmediatL("LDS  ", 4, p_code, p_mnemonic);

        case 0xde:
            return D_Direct("LDS  ", 3, p_code, p_mnemonic);

        case 0xdf:
            return D_Direct("STS  ", 3, p_code, p_mnemonic);

        case 0xee:
            return D_Indexed("LDS  ", 3, p_code, p_mnemonic);

        case 0xef:
            return D_Indexed("STS  ", 3, p_code, p_mnemonic);

        case 0xfe:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("LDS  ", 4, p_code, p_mnemonic);

        case 0xff:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("STS  ", 4, p_code, p_mnemonic);

        default:
            p_flags |= InstFlg::Illegal;
            return D_Illegal("", 1, p_code, p_mnemonic);
    }
}


inline Byte Da6809::D_Page11(InstFlg &p_flags, std::string &p_code,
        std::string &p_mnemonic)
{
    const auto code = *(memory + 1);

    switch (code)
    {
        case 0x3f:
            p_flags |= InstFlg::Sub;
            return D_Inherent("SWI3", 2, p_code, p_mnemonic);

        case 0x83:
            p_flags |= InstFlg::LabelAddr;
            return D_ImmediatL("CMPU ", 4, p_code,
                               p_mnemonic);

        case 0x8c:
            p_flags |= InstFlg::LabelAddr;
            return D_ImmediatL("CMPS ", 4, p_code,
                               p_mnemonic);

        case 0x93:
            return D_Direct("CMPU ", 3, p_code, p_mnemonic);

        case 0x9c:
            return D_Direct("CMPS ", 3, p_code, p_mnemonic);

        case 0xa3:
            return D_Indexed("CMPU ", 3, p_code, p_mnemonic);

        case 0xac:
            return D_Indexed("CMPS ", 3, p_code, p_mnemonic);

        case 0xb3:
            return D_Extended("CMPU ", 4, p_code, p_mnemonic);

        case 0xbc:
            return D_Extended("CMPS ", 4, p_code, p_mnemonic);

        default:
            p_flags |= InstFlg::Illegal;
            return D_Illegal("", 1, p_code, p_mnemonic);

    }
}

int Da6809::Disassemble(
        const Byte *p_memory,
        DWord p_pc,
        InstFlg &p_flags,
        DWord &p_jumpaddr,
        std::string &p_code,
        std::string &p_mnemonic)
{
    pc = static_cast<Word>(p_pc);
    memory = p_memory;
    p_flags = InstFlg::NONE;
    const auto opcode = *memory;

    switch (opcode)
    {
        case 0x01:
            if (use_undocumented)
            {
                return D_Direct("neg", 2, p_code, p_mnemonic);
            }

            p_flags |= InstFlg::Illegal;
            return D_Illegal("", 1, p_code, p_mnemonic);

        case 0x00:
            return D_Direct("NEG  ", 2, p_code, p_mnemonic);

        case 0x02:
            if (use_undocumented)
            {
                return D_Direct("negcom", 2, p_code, p_mnemonic);
            }

            p_flags |= InstFlg::Illegal;
            return D_Illegal("", 1, p_code, p_mnemonic);

        case 0x03:
            return D_Direct("COM  ", 2, p_code, p_mnemonic);

        case 0x04:
            return D_Direct("LSR  ", 2, p_code, p_mnemonic);

        case 0x05:
            if (use_undocumented)
            {
                return D_Direct("lsr  ", 2, p_code, p_mnemonic);
            }

            p_flags |= InstFlg::Illegal;
            return D_Illegal("", 1, p_code, p_mnemonic);

        case 0x06:
            return D_Direct("ROR  ", 2, p_code, p_mnemonic);

        case 0x07:
            return D_Direct("ASR  ", 2, p_code, p_mnemonic);

        case 0x08:
            return D_Direct("LSL  ", 2, p_code, p_mnemonic);

        case 0x09:
            return D_Direct("ROL  ", 2, p_code, p_mnemonic);

        case 0x0a:
            return D_Direct("DEC  ", 2, p_code, p_mnemonic);

        case 0x0b:
            if (use_undocumented)
            {
                return D_Direct("dec  ", 2, p_code, p_mnemonic);
            }

            p_flags |= InstFlg::Illegal;
            return D_Illegal("", 1, p_code, p_mnemonic);

        case 0x0c:
            return D_Direct("INC  ", 2, p_code, p_mnemonic);

        case 0x0d:
            return D_Direct("TST  ", 2, p_code, p_mnemonic);

        case 0x0e:
            p_flags |= InstFlg::Jump;
            return D_Direct("JMP  ", 2, p_code, p_mnemonic);

        case 0x0f:
            return D_Direct("CLR  ", 2, p_code, p_mnemonic);

        case 0x10:
            return D_Page10(p_flags, p_jumpaddr, p_code, p_mnemonic);

        case 0x11:
            return D_Page11(p_flags, p_code, p_mnemonic);

        case 0x12:
            p_flags |= InstFlg::Noop;
            return D_Inherent("NOP  ", 1, p_code, p_mnemonic);

        case 0x13:
            p_flags |= InstFlg::Jump;
            return D_Inherent("SYNC ", 1, p_code, p_mnemonic);

        // 0x14, 0x15 is illegal
        case 0x16:
            p_flags |= InstFlg::Jump | InstFlg::JumpAddr;
            return D_RelativeL("LBRA", 3, p_jumpaddr, p_code, p_mnemonic);

        case 0x17:
            p_flags |= InstFlg::Sub | InstFlg::LabelAddr;
            return D_RelativeL("LBSR", 3, p_jumpaddr, p_code, p_mnemonic);

        // 0x18 is illegal
        case 0x19:
            return D_Inherent("DAA  ", 1, p_code, p_mnemonic);

        case 0x1a:
            return D_Immediat("ORCC ", 2, p_code, p_mnemonic);

        // 0x1b is illegal
        case 0x1c:
            return D_Immediat("ANDCC", 2, p_code, p_mnemonic);

        case 0x1d:
            return D_Inherent("SEX  ", 1, p_code, p_mnemonic);

        case 0x1e:
            return D_Register0("EXG  ", 2, p_code, p_mnemonic);

        case 0x1f:
            return D_Register0("TFR  ", 2, p_code, p_mnemonic);

        case 0x20:
            p_flags |= InstFlg::Jump | InstFlg::JumpAddr;
            return D_Relative("BRA", 2, p_jumpaddr, p_code, p_mnemonic);

        case 0x21:
            p_flags |= InstFlg::JumpAddr;
            return D_Relative("BRN", 2, p_jumpaddr, p_code, p_mnemonic);

        case 0x22:
            p_flags |= InstFlg::JumpAddr;
            return D_Relative("BHI", 2, p_jumpaddr, p_code, p_mnemonic);

        case 0x23:
            p_flags |= InstFlg::JumpAddr;
            return D_Relative("BLS", 2, p_jumpaddr, p_code, p_mnemonic);

        case 0x24:
            p_flags |= InstFlg::JumpAddr;
            return D_Relative("BCC", 2, p_jumpaddr, p_code, p_mnemonic);

        case 0x25:
            p_flags |= InstFlg::JumpAddr;
            return D_Relative("BCS", 2, p_jumpaddr, p_code, p_mnemonic);

        case 0x26:
            p_flags |= InstFlg::JumpAddr;
            return D_Relative("BNE", 2, p_jumpaddr, p_code, p_mnemonic);

        case 0x27:
            p_flags |= InstFlg::JumpAddr;
            return D_Relative("BEQ", 2, p_jumpaddr, p_code, p_mnemonic);

        case 0x28:
            p_flags |= InstFlg::JumpAddr;
            return D_Relative("BVC", 2, p_jumpaddr, p_code, p_mnemonic);

        case 0x29:
            p_flags |= InstFlg::JumpAddr;
            return D_Relative("BVS", 2, p_jumpaddr, p_code, p_mnemonic);

        case 0x2a:
            p_flags |= InstFlg::JumpAddr;
            return D_Relative("BPL", 2, p_jumpaddr, p_code, p_mnemonic);

        case 0x2b:
            p_flags |= InstFlg::JumpAddr;
            return D_Relative("BMI", 2, p_jumpaddr, p_code, p_mnemonic);

        case 0x2c:
            p_flags |= InstFlg::JumpAddr;
            return D_Relative("BGE", 2, p_jumpaddr, p_code, p_mnemonic);

        case 0x2d:
            p_flags |= InstFlg::JumpAddr;
            return D_Relative("BLT", 2, p_jumpaddr, p_code, p_mnemonic);

        case 0x2e:
            p_flags |= InstFlg::JumpAddr;
            return D_Relative("BGT", 2, p_jumpaddr, p_code, p_mnemonic);

        case 0x2f:
            p_flags |= InstFlg::JumpAddr;
            return D_Relative("BLE", 2, p_jumpaddr, p_code, p_mnemonic);

        case 0x30:
            return D_Indexed("LEAX ", 2, p_code, p_mnemonic);

        case 0x31:
            return D_Indexed("LEAY ", 2, p_code, p_mnemonic);

        case 0x32:
            return D_Indexed("LEAS ", 2, p_code, p_mnemonic);

        case 0x33:
            return D_Indexed("LEAU ", 2, p_code, p_mnemonic);

        case 0x34:
            return D_Register1("PSHS", "U", 2, p_code, p_mnemonic);

        case 0x35:
            return D_Register1("PULS", "U", 2, p_code, p_mnemonic);

        case 0x36:
            return D_Register1("PSHU", "S", 2, p_code, p_mnemonic);

        case 0x37:
            return D_Register1("PULU", "S", 2, p_code, p_mnemonic);

        // 0x38 is illegal
        case 0x39:
            p_flags |= InstFlg::Jump;
            return D_Inherent("RTS  ", 1, p_code, p_mnemonic);

        case 0x3a:
            return D_Inherent("ABX  ", 1, p_code, p_mnemonic);

        case 0x3b:
            p_flags |= InstFlg::Jump;
            return D_Inherent("RTI  ", 1, p_code, p_mnemonic);

        case 0x3c:
            p_flags |= InstFlg::Jump;
            return D_Immediat("CWAI ", 2, p_code, p_mnemonic);

        case 0x3d:
            return D_Inherent("MUL  ", 1, p_code, p_mnemonic);

        case 0x3e:
            if (use_undocumented)
            {
                return D_Inherent("reset", 1, p_code, p_mnemonic);
            }

            p_flags |= InstFlg::Illegal;
            return D_Illegal("", 1, p_code, p_mnemonic);

        case 0x3f:
            p_flags |= InstFlg::Sub;
            return D_Inherent("SWI  ", 1, p_code, p_mnemonic);

        case 0x40:
            return D_Inherent("NEGA ", 1, p_code, p_mnemonic);

        case 0x41:
            if (use_undocumented)
            {
                return D_Inherent("nega", 1, p_code, p_mnemonic);
            }

            p_flags |= InstFlg::Illegal;
            return D_Illegal("", 1, p_code, p_mnemonic);

        case 0x42:
            if (use_undocumented)
            {
                return D_Inherent("negcoma", 1, p_code, p_mnemonic);
            }

            p_flags |= InstFlg::Illegal;
            return D_Illegal("", 1, p_code, p_mnemonic);

        case 0x43:
            return D_Inherent("COMA ", 1, p_code, p_mnemonic);

        case 0x44:
            return D_Inherent("LSRA ", 1, p_code, p_mnemonic);

        case 0x45:
            if (use_undocumented)
            {
                return D_Inherent("lsra", 1, p_code, p_mnemonic);
            }

            p_flags |= InstFlg::Illegal;
            return D_Illegal("", 1, p_code, p_mnemonic);

        case 0x46:
            return D_Inherent("RORA ", 1, p_code, p_mnemonic);

        case 0x47:
            return D_Inherent("ASRA ", 1, p_code, p_mnemonic);

        case 0x48:
            return D_Inherent("LSLA ", 1, p_code, p_mnemonic);

        case 0x49:
            return D_Inherent("ROLA ", 1, p_code, p_mnemonic);

        case 0x4a:
            return D_Inherent("DECA ", 1, p_code, p_mnemonic);

        case 0x4b:
            if (use_undocumented)
            {
                return D_Inherent("deca", 1, p_code, p_mnemonic);
            }

            p_flags |= InstFlg::Illegal;
            return D_Illegal("", 1, p_code, p_mnemonic);

        case 0x4c:
            return D_Inherent("INCA ", 1, p_code, p_mnemonic);

        case 0x4d:
            return D_Inherent("TSTA ", 1, p_code, p_mnemonic);

        case 0x4e:
            if (use_undocumented)
            {
                return D_Inherent("clra", 1, p_code, p_mnemonic);
            }

            p_flags |= InstFlg::Illegal;
            return D_Illegal("", 1, p_code, p_mnemonic);

        case 0x4f:
            return D_Inherent("CLRA ", 1, p_code, p_mnemonic);

        case 0x50:
            return D_Inherent("NEGB ", 1, p_code, p_mnemonic);

        case 0x51:
            if (use_undocumented)
            {
                return D_Inherent("negb", 1, p_code, p_mnemonic);
            }

            p_flags |= InstFlg::Illegal;
            return D_Illegal("", 1, p_code, p_mnemonic);

        case 0x52:
            if (use_undocumented)
            {
                return D_Inherent("negcomb", 1, p_code, p_mnemonic);
            }

            p_flags |= InstFlg::Illegal;
            return D_Illegal("", 1, p_code, p_mnemonic);

        case 0x53:
            return D_Inherent("COMB ", 1, p_code, p_mnemonic);

        case 0x54:
            return D_Inherent("LSRB ", 1, p_code, p_mnemonic);

        case 0x55:
            if (use_undocumented)
            {
                return D_Inherent("lsrb", 1, p_code, p_mnemonic);
            }

            p_flags |= InstFlg::Illegal;
            return D_Illegal("", 1, p_code, p_mnemonic);

        case 0x56:
            return D_Inherent("RORB ", 1, p_code, p_mnemonic);

        case 0x57:
            return D_Inherent("ASRB ", 1, p_code, p_mnemonic);

        case 0x58:
            return D_Inherent("LSLB ", 1, p_code, p_mnemonic);

        case 0x59:
            return D_Inherent("ROLB ", 1, p_code, p_mnemonic);

        case 0x5a:
            return D_Inherent("DECB ", 1, p_code, p_mnemonic);

        case 0x5b:
            if (use_undocumented)
            {
                return D_Inherent("decb", 1, p_code, p_mnemonic);
            }

            p_flags |= InstFlg::Illegal;
            return D_Illegal("", 1, p_code, p_mnemonic);

        case 0x5c:
            return D_Inherent("INCB ", 1, p_code, p_mnemonic);

        case 0x5d:
            return D_Inherent("TSTB ", 1, p_code, p_mnemonic);

        case 0x5e:
            if (use_undocumented)
            {
                return D_Inherent("clrb", 1, p_code, p_mnemonic);
            }

            p_flags |= InstFlg::Illegal;
            return D_Illegal("", 1, p_code, p_mnemonic);

        case 0x5f:
            return D_Inherent("CLRB ", 1, p_code, p_mnemonic);

        case 0x60:
            return D_Indexed("NEG  ", 2, p_code, p_mnemonic);

        case 0x61:
            if (use_undocumented)
            {
                return D_Indexed("neg  ", 2, p_code, p_mnemonic);
            }

            p_flags |= InstFlg::Illegal;
            return D_Illegal("", 1, p_code, p_mnemonic);

        case 0x62:
            if (use_undocumented)
            {
                return D_Indexed("negcom  ", 2, p_code, p_mnemonic);
            }

            p_flags |= InstFlg::Illegal;
            return D_Illegal("", 1, p_code, p_mnemonic);

        case 0x63:
            return D_Indexed("COM  ", 2, p_code, p_mnemonic);

        case 0x64:
            return D_Indexed("LSR  ", 2, p_code, p_mnemonic);

        case 0x65:
            if (use_undocumented)
            {
                return D_Indexed("lsr  ", 2, p_code, p_mnemonic);
            }

            p_flags |= InstFlg::Illegal;
            return D_Illegal("", 1, p_code, p_mnemonic);

        case 0x66:
            return D_Indexed("ROR  ", 2, p_code, p_mnemonic);

        case 0x67:
            return D_Indexed("ASR  ", 2, p_code, p_mnemonic);

        case 0x68:
            return D_Indexed("LSL  ", 2, p_code, p_mnemonic);

        case 0x69:
            return D_Indexed("ROL  ", 2, p_code, p_mnemonic);

        case 0x6a:
            return D_Indexed("DEC  ", 2, p_code, p_mnemonic);

        case 0x6b:
            if (use_undocumented)
            {
                return D_Indexed("dec  ", 2, p_code, p_mnemonic);
            }

            p_flags |= InstFlg::Illegal;
            return D_Illegal("", 1, p_code, p_mnemonic);

        case 0x6c:
            return D_Indexed("INC  ", 2, p_code, p_mnemonic);

        case 0x6d:
            return D_Indexed("TST  ", 2, p_code, p_mnemonic);

        case 0x6e:
            p_flags |= InstFlg::Jump | InstFlg::ComputedGoto;
            return D_Indexed("JMP  ", 2, p_code, p_mnemonic);

        case 0x6f:
            return D_Indexed("CLR  ", 2, p_code, p_mnemonic);

        case 0x70:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("NEG  ", 3, p_code, p_mnemonic);

        case 0x71:
            if (use_undocumented)
            {
                p_flags |= InstFlg::LabelAddr;
                return D_Extended("neg  ", 3, p_code, p_mnemonic);
            };

            p_flags |= InstFlg::Illegal;

            return D_Illegal("", 1, p_code, p_mnemonic);

        case 0x72:
            if (use_undocumented)
            {
                p_flags |= InstFlg::LabelAddr;
                return D_Extended("negcom  ", 3, p_code, p_mnemonic);
            };

            p_flags |= InstFlg::Illegal;

            return D_Illegal("", 1, p_code, p_mnemonic);

        case 0x73:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("COM  ", 3, p_code, p_mnemonic);

        case 0x74:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("LSR  ", 3, p_code, p_mnemonic);

        case 0x75:
            if (use_undocumented)
            {
                p_flags |= InstFlg::LabelAddr;
                return D_Extended("lsr  ", 3, p_code, p_mnemonic);
            };

            p_flags |= InstFlg::Illegal;

            return D_Illegal("", 1, p_code, p_mnemonic);

        case 0x76:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("ROR  ", 3, p_code, p_mnemonic);

        case 0x77:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("ASR  ", 3, p_code, p_mnemonic);

        case 0x78:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("LSL  ", 3, p_code, p_mnemonic);

        case 0x79:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("ROL  ", 3, p_code, p_mnemonic);

        case 0x7a:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("DEC  ", 3, p_code, p_mnemonic);

        case 0x7b:
            if (use_undocumented)
            {
                p_flags |= InstFlg::LabelAddr;
                return D_Extended("dec  ", 3, p_code, p_mnemonic);
            };

            p_flags |= InstFlg::Illegal;

            return D_Illegal("", 1, p_code, p_mnemonic);

        case 0x7c:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("INC  ", 3, p_code, p_mnemonic);

        case 0x7d:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("TST  ", 3, p_code, p_mnemonic);

        case 0x7e:
            p_flags |= InstFlg::Jump;
            return D_Extended("JMP  ", 3, p_code, p_mnemonic);

        case 0x7f:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("CLR  ", 3, p_code, p_mnemonic);

        case 0x80:
            return D_Immediat("SUBA ", 2, p_code, p_mnemonic);

        case 0x81:
            return D_Immediat("CMPA ", 2, p_code, p_mnemonic);

        case 0x82:
            return D_Immediat("SBCA ", 2, p_code, p_mnemonic);

        case 0x83:
            return D_ImmediatL("SUBD ", 3, p_code, p_mnemonic);

        case 0x84:
            return D_Immediat("ANDA ", 2, p_code, p_mnemonic);

        case 0x85:
            return D_Immediat("BITA ", 2, p_code, p_mnemonic);

        case 0x86:
            return D_Immediat("LDA  ", 2, p_code, p_mnemonic);

        // 0x87 is illegal
        case 0x88:
            return D_Immediat("EORA ", 2, p_code, p_mnemonic);

        case 0x89:
            return D_Immediat("ADCA ", 2, p_code, p_mnemonic);

        case 0x8a:
            return D_Immediat("ORA  ", 2, p_code, p_mnemonic);

        case 0x8b:
            return D_Immediat("ADDA ", 2, p_code, p_mnemonic);

        case 0x8c:
            return D_ImmediatL("CMPX ", 3, p_code, p_mnemonic);

        case 0x8d:
            p_flags |= InstFlg::Sub;
            return D_Relative("BSR", 2, p_jumpaddr, p_code,
                               p_mnemonic);

        case 0x8e:
            return D_ImmediatL("LDX  ", 3, p_code, p_mnemonic);

        // 0x8f is illegal

        case 0x90:
            return D_Direct("SUBA ", 2, p_code, p_mnemonic);

        case 0x91:
            return D_Direct("CMPA ", 2, p_code, p_mnemonic);

        case 0x92:
            return D_Direct("SBCA ", 2, p_code, p_mnemonic);

        case 0x93:
            return D_Direct("SUBD ", 2, p_code, p_mnemonic);

        case 0x94:
            return D_Direct("ANDA ", 2, p_code, p_mnemonic);

        case 0x95:
            return D_Direct("BITA ", 2, p_code, p_mnemonic);

        case 0x96:
            return D_Direct("LDA  ", 2, p_code, p_mnemonic);

        case 0x97:
            return D_Direct("STA  ", 2, p_code, p_mnemonic);

        case 0x98:
            return D_Direct("EORA ", 2, p_code, p_mnemonic);

        case 0x99:
            return D_Direct("ADCA ", 2, p_code, p_mnemonic);

        case 0x9a:
            return D_Direct("ORA  ", 2, p_code, p_mnemonic);

        case 0x9b:
            return D_Direct("ADDA ", 2, p_code, p_mnemonic);

        case 0x9c:
            return D_Direct("CMPX ", 2, p_code, p_mnemonic);

        case 0x9d:
            p_flags |= InstFlg::Sub;
            return D_Direct("JSR  ", 2, p_code, p_mnemonic);

        case 0x9e:
            return D_Direct("LDX  ", 2, p_code, p_mnemonic);

        case 0x9f:
            return D_Direct("STX  ", 2, p_code, p_mnemonic);

        case 0xa0:
            return D_Indexed("SUBA ", 2, p_code, p_mnemonic);

        case 0xa1:
            return D_Indexed("CMPA ", 2, p_code, p_mnemonic);

        case 0xa2:
            return D_Indexed("SBCA ", 2, p_code, p_mnemonic);

        case 0xa3:
            return D_Indexed("SUBD ", 2, p_code, p_mnemonic);

        case 0xa4:
            return D_Indexed("ANDA ", 2, p_code, p_mnemonic);

        case 0xa5:
            return D_Indexed("BITA ", 2, p_code, p_mnemonic);

        case 0xa6:
            return D_Indexed("LDA  ", 2, p_code, p_mnemonic);

        case 0xa7:
            return D_Indexed("STA  ", 2, p_code, p_mnemonic);

        case 0xa8:
            return D_Indexed("EORA ", 2, p_code, p_mnemonic);

        case 0xa9:
            return D_Indexed("ADCA ", 2, p_code, p_mnemonic);

        case 0xaa:
            return D_Indexed("ORA  ", 2, p_code, p_mnemonic);

        case 0xab:
            return D_Indexed("ADDA ", 2, p_code, p_mnemonic);

        case 0xac:
            return D_Indexed("CMPX ", 2, p_code, p_mnemonic);

        case 0xad:
            p_flags |= InstFlg::Sub | InstFlg::ComputedGoto;
            return D_Indexed("JSR  ", 2, p_code, p_mnemonic);

        case 0xae:
            return D_Indexed("LDX  ", 2, p_code, p_mnemonic);

        case 0xaf:
            return D_Indexed("STX  ", 2, p_code, p_mnemonic);

        case 0xb0:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("SUBA ", 3, p_code, p_mnemonic);

        case 0xb1:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("CMPA ", 3, p_code, p_mnemonic);

        case 0xb2:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("SBCA ", 3, p_code, p_mnemonic);

        case 0xb3:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("SUBD ", 3, p_code, p_mnemonic);

        case 0xb4:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("ANDA ", 3, p_code, p_mnemonic);

        case 0xb5:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("BITA ", 3, p_code, p_mnemonic);

        case 0xb6:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("LDA  ", 3, p_code, p_mnemonic);

        case 0xb7:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("STA  ", 3, p_code, p_mnemonic);

        case 0xb8:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("EORA ", 3, p_code, p_mnemonic);

        case 0xb9:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("ADCA ", 3, p_code, p_mnemonic);

        case 0xba:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("ORA  ", 3, p_code, p_mnemonic);

        case 0xbb:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("ADDA ", 3, p_code, p_mnemonic);

        case 0xbc:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("CMPX ", 3, p_code, p_mnemonic);

        case 0xbd:
            p_flags |= InstFlg::Sub | InstFlg::JumpAddr;
            return D_Extended("JSR  ", 3, p_code, p_mnemonic);

        case 0xbe:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("LDX  ", 3, p_code, p_mnemonic);

        case 0xbf:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("STX  ", 3, p_code, p_mnemonic);

        case 0xc0:
            return D_Immediat("SUBB ", 2, p_code, p_mnemonic);

        case 0xc1:
            return D_Immediat("CMPB ", 2, p_code, p_mnemonic);

        case 0xc2:
            return D_Immediat("SBCB ", 2, p_code, p_mnemonic);

        case 0xc3:
            p_flags |= InstFlg::LabelAddr;
            return D_ImmediatL("ADDD ", 3, p_code, p_mnemonic);

        case 0xc4:
            return D_Immediat("ANDB ", 2, p_code, p_mnemonic);

        case 0xc5:
            return D_Immediat("BITB ", 2, p_code, p_mnemonic);

        case 0xc6:
            return D_Immediat("LDB  ", 2, p_code, p_mnemonic);

        // 0xc7 is illegal
        case 0xc8:
            return D_Immediat("EORB ", 2, p_code, p_mnemonic);

        case 0xc9:
            return D_Immediat("ADCB ", 2, p_code, p_mnemonic);

        case 0xca:
            return D_Immediat("ORB  ", 2, p_code, p_mnemonic);

        case 0xcb:
            return D_Immediat("ADDB ", 2, p_code, p_mnemonic);

        case 0xcc:
            p_flags |= InstFlg::LabelAddr;
            return D_ImmediatL("LDD  ", 3, p_code, p_mnemonic);

        // 0xcd is illegal
        case 0xce:
            p_flags |= InstFlg::LabelAddr;
            return D_ImmediatL("LDU  ", 3, p_code, p_mnemonic);

        // 0xcf is illegal

        case 0xd0:
            return D_Direct("SUBB ", 2, p_code, p_mnemonic);

        case 0xd1:
            return D_Direct("CMPB ", 2, p_code, p_mnemonic);

        case 0xd2:
            return D_Direct("SBCB ", 2, p_code, p_mnemonic);

        case 0xd3:
            return D_Direct("ADDD ", 2, p_code, p_mnemonic);

        case 0xd4:
            return D_Direct("ANDB ", 2, p_code, p_mnemonic);

        case 0xd5:
            return D_Direct("BITB ", 2, p_code, p_mnemonic);

        case 0xd6:
            return D_Direct("LDB  ", 2, p_code, p_mnemonic);

        case 0xd7:
            return D_Direct("STB  ", 2, p_code, p_mnemonic);

        case 0xd8:
            return D_Direct("EORB ", 2, p_code, p_mnemonic);

        case 0xd9:
            return D_Direct("ADCB ", 2, p_code, p_mnemonic);

        case 0xda:
            return D_Direct("ORB  ", 2, p_code, p_mnemonic);

        case 0xdb:
            return D_Direct("ADDB ", 2, p_code, p_mnemonic);

        case 0xdc:
            return D_Direct("LDD  ", 2, p_code, p_mnemonic);

        case 0xdd:
            return D_Direct("STD  ", 2, p_code, p_mnemonic);

        case 0xde:
            return D_Direct("LDU  ", 2, p_code, p_mnemonic);

        case 0xdf:
            return D_Direct("STU  ", 2, p_code, p_mnemonic);

        case 0xe0:
            return D_Indexed("SUBB ", 2, p_code, p_mnemonic);

        case 0xe1:
            return D_Indexed("CMPB ", 2, p_code, p_mnemonic);

        case 0xe2:
            return D_Indexed("SBCB ", 2, p_code, p_mnemonic);

        case 0xe3:
            return D_Indexed("ADDD ", 2, p_code, p_mnemonic);

        case 0xe4:
            return D_Indexed("ANDB ", 2, p_code, p_mnemonic);

        case 0xe5:
            return D_Indexed("BITB ", 2, p_code, p_mnemonic);

        case 0xe6:
            return D_Indexed("LDB  ", 2, p_code, p_mnemonic);

        case 0xe7:
            return D_Indexed("STB  ", 2, p_code, p_mnemonic);

        case 0xe8:
            return D_Indexed("EORB ", 2, p_code, p_mnemonic);

        case 0xe9:
            return D_Indexed("ADCB ", 2, p_code, p_mnemonic);

        case 0xea:
            return D_Indexed("ORB  ", 2, p_code, p_mnemonic);

        case 0xeb:
            return D_Indexed("ADDB ", 2, p_code, p_mnemonic);

        case 0xec:
            return D_Indexed("LDD  ", 2, p_code, p_mnemonic);

        case 0xed:
            return D_Indexed("STD  ", 2, p_code, p_mnemonic);

        case 0xee:
            return D_Indexed("LDU  ", 2, p_code, p_mnemonic);

        case 0xef:
            return D_Indexed("STU  ", 2, p_code, p_mnemonic);

        case 0xf0:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("SUBB ", 3, p_code, p_mnemonic);

        case 0xf1:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("CMPB ", 3, p_code, p_mnemonic);

        case 0xf2:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("SBCB ", 3, p_code, p_mnemonic);

        case 0xf3:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("ADDD ", 3, p_code, p_mnemonic);

        case 0xf4:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("ANDB ", 3, p_code, p_mnemonic);

        case 0xf5:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("BITB ", 3, p_code, p_mnemonic);

        case 0xf6:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("LDB  ", 3, p_code, p_mnemonic);

        case 0xf7:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("STB  ", 3, p_code, p_mnemonic);

        case 0xf8:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("EORB ", 3, p_code, p_mnemonic);

        case 0xf9:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("ADCB ", 3, p_code, p_mnemonic);

        case 0xfa:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("ORB  ", 3, p_code, p_mnemonic);

        case 0xfb:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("ADDB ", 3, p_code, p_mnemonic);

        case 0xfc:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("LDD  ", 3, p_code, p_mnemonic);

        case 0xfd:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("STD  ", 3, p_code, p_mnemonic);

        case 0xfe:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("LDU  ", 3, p_code, p_mnemonic);

        case 0xff:
            p_flags |= InstFlg::LabelAddr;
            return D_Extended("STU  ", 3, p_code, p_mnemonic);

        default:
            p_flags |= InstFlg::Illegal;
            return D_Illegal("", 1, p_code, p_mnemonic);
    }
}

