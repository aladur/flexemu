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


Da6809::Da6809() : use_undocumented(false)
{
    memset(code_buf, 0, sizeof(code_buf));
    memset(mnem_buf, 0, sizeof(mnem_buf));
}

Da6809::~Da6809()
{
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
        { 0xCC1D, "TFRFLG" }, // loader  transfer flag
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


inline Byte Da6809::D_Illegal(const char *mnemo, Word pc, Byte bytes,
                              const Byte *pMemory)
{
    Byte code;

    code = *pMemory;
    snprintf(code_buf, sizeof(code_buf), "%04X: %02X", pc, code);
    snprintf(mnem_buf, sizeof(mnem_buf), "%s ?????", mnemo);
    return bytes;
}


inline Byte Da6809::D_Direct(const char *mnemo, Word pc, Byte bytes,
                             const Byte *pMemory)
{
    Byte code;
    Byte offset;

    code = *pMemory;
    offset = *(pMemory + 1);
    snprintf(code_buf, sizeof(code_buf), "%04X: %02X %02X", pc, code, offset);
    snprintf(mnem_buf, sizeof(mnem_buf), "%s $%02X", mnemo, offset);
    return bytes;
}


inline Byte Da6809::D_Immediat(const char *mnemo, Word pc, Byte bytes,
                               const Byte *pMemory)
{
    Byte code;
    Byte offset;

    code = *pMemory;
    offset = *(pMemory + 1);
    snprintf(code_buf, sizeof(code_buf), "%04X: %02X %02X", pc, code, offset);
    snprintf(mnem_buf, sizeof(mnem_buf), "%s #$%02X", mnemo, offset);
    return bytes;
}


inline Byte Da6809::D_ImmediatL(const char *mnemo, Word pc, Byte bytes,
                                const Byte *pMemory, DWord * /*pAddr*/)
{
    Byte code;
    const char *label;

    code = *pMemory;
    auto offset = getValueBigEndian<Word>(&pMemory[1]);
    snprintf(code_buf, sizeof(code_buf), "%04X: %02X %02X %02X", pc, code,
                            *(pMemory + 1),
            *(pMemory + 2));
    label = FlexLabel(offset);

    if (label == nullptr)
    {
        snprintf(mnem_buf, sizeof(mnem_buf), "%s #$%04X", mnemo, offset);
    }
    else
    {
        snprintf(mnem_buf, sizeof(mnem_buf), "%s #%s ; $%04X", mnemo, label,
                 offset);
    }

    return bytes;
}


inline Byte Da6809::D_Inherent(const char *mnemo, Word pc, Byte bytes,
                               const Byte *pMemory)
{
    int code;

    code = *pMemory;
    snprintf(code_buf, sizeof(code_buf), "%04X: %02X", pc, code);
    snprintf(mnem_buf, sizeof(mnem_buf), "%s", mnemo);
    return bytes;
}  // D_Inherent


Byte Da6809::D_Indexed(const char *mnemo, Word pc, Byte bytes,
                       const Byte *pMemory)
{
    Byte code;
    Byte postbyte;
    const char *s;
    const char *br1;
    const char *br2;
    Byte extrabytes;
    Byte disp;
    Word offset;

    extrabytes = 0;
    code = *pMemory;
    postbyte = *(pMemory + 1);
    br1        = "";        // bracket on for indirect addressing
    br2        = "";        // bracket off for indirect addressing
    s          = "";        // minus sign for offset

    if ((postbyte & 0x80) == 0x00)
    {
        // ,R + 5 Bit Offset
        disp = postbyte & 0x1f;

        if ((postbyte & 0x10) == 0x10)
        {
            s = "-";
            disp = 0x20 - disp;
        }

        snprintf(code_buf, sizeof(code_buf), "%04X: %02X %02X", pc, code,
                 postbyte);
        snprintf(mnem_buf, sizeof(mnem_buf), "%s %s$%02X,%s", mnemo, s, disp,
                IndexedRegister(postbyte >> 5));
    }
    else
    {
        switch (postbyte & 0x1f)
        {
            case 0x00 : // ,R+
                snprintf(code_buf, sizeof(code_buf), "%04X: %02X %02X", pc,
                         code, postbyte);
                snprintf(mnem_buf, sizeof(mnem_buf), "%s ,%s+", mnemo,
                        IndexedRegister(postbyte >> 5));
                break;

            case 0x11 : // [,R++]
                br1 = "[";
                br2 = "]";
                FALLTHROUGH;

            case 0x01 : // ,R++
                snprintf(code_buf, sizeof(code_buf), "%04X: %02X %02X", pc,
                         code, postbyte);
                snprintf(mnem_buf, sizeof(mnem_buf), "%s %s,%s++%s", mnemo,
                        br1, IndexedRegister(postbyte >> 5), br2);
                break;

            case 0x02 : // ,-R
                snprintf(code_buf, sizeof(code_buf), "%04X: %02X %02X", pc,
                         code, postbyte);
                snprintf(mnem_buf, sizeof(mnem_buf), "%s ,-%s", mnemo,
                        IndexedRegister(postbyte >> 5));
                break;

            case 0x13 : // [,R--]
                br1 = "[";
                br2 = "]";
                FALLTHROUGH;

            case 0x03 : // ,--R
                snprintf(code_buf, sizeof(code_buf), "%04X: %02X %02X", pc,
                         code, postbyte);
                snprintf(mnem_buf, sizeof(mnem_buf), "%s %s,--%s%s", mnemo,
                        br1, IndexedRegister(postbyte >> 5), br2);
                break;

            case 0x14 : // [,R--]
                br1 = "[";
                br2 = "]";
                FALLTHROUGH;

            case 0x04 : // ,R
                snprintf(code_buf, sizeof(code_buf), "%04X: %02X %02X", pc,
                         code, postbyte);
                snprintf(mnem_buf, sizeof(mnem_buf), "%s %s,%s%s", mnemo,
                        br1, IndexedRegister(postbyte >> 5), br2);
                break;

            case 0x15 : // [B,R]
                br1 = "[";
                br2 = "]";
                FALLTHROUGH;

            case 0x05 : // B,R
                snprintf(code_buf, sizeof(code_buf), "%04X: %02X %02X", pc,
                         code, postbyte);
                snprintf(mnem_buf, sizeof(mnem_buf), "%s %sB,%s%s", mnemo,
                        br1, IndexedRegister(postbyte >> 5), br2);
                break;

            case 0x16 : // [A,R]
                br1 = "[";
                br2 = "]";
                FALLTHROUGH;

            case 0x06 : // A,R
                snprintf(code_buf, sizeof(code_buf), "%04X: %02X %02X", pc,
                         code, postbyte);
                snprintf(mnem_buf, sizeof(mnem_buf), "%s %sA,%s%s", mnemo,
                        br1, IndexedRegister(postbyte >> 5), br2);
                break;

            case 0x18 : // [,R + 8 Bit Offset]
                br1 = "[";
                br2 = "]";
                FALLTHROUGH;

            case 0x08 : // ,R + 8 Bit Offset
                offset = *(pMemory + 2);

                if (offset < 128)
                {
                    s = "";
                }
                else
                {
                    s = "-";
                    offset = 0x0100 - offset;
                }

                snprintf(code_buf, sizeof(code_buf), "%04X: %02X %02X %02X", pc,
                         code, postbyte,
                        *(pMemory + 2));
                snprintf(mnem_buf, sizeof(mnem_buf), "%s %s%s$%02X,%s%s", mnemo,
                         br1, s, offset, IndexedRegister(postbyte >> 5), br2);
                extrabytes = 1;
                break;

            case 0x19 : // [,R + 16 Bit Offset]
                br1 = "[";
                br2 = "]";
                FALLTHROUGH;

            case 0x09 : // ,R + 16 Bit Offset
                offset = getValueBigEndian<Word>(&pMemory[2]);
                s = "";
                snprintf(code_buf, sizeof(code_buf),
                         "%04X: %02X %02X %02X %02X", pc, code, postbyte,
                         *(pMemory + 2), *(pMemory + 3));
                snprintf(mnem_buf, sizeof(mnem_buf), "%s %s%s$%04X,%s%s", mnemo,
                        br1, s, offset, IndexedRegister(postbyte >> 5), br2);
                extrabytes = 2;
                break;

            case 0x1b : // [D,R]
                br1 = "[";
                br2 = "]";
                FALLTHROUGH;

            case 0x0b : // D,R
                snprintf(code_buf, sizeof(code_buf), "%04X: %02X %02X", pc,
                         code, postbyte);
                snprintf(mnem_buf, sizeof(mnem_buf), "%s %sD,%s%s", mnemo,
                        br1, IndexedRegister(postbyte >> 5), br2);
                break;

            case 0x1c : // [,PC + 8 Bit Offset]
                br1 = "[";
                br2 = "]";
                FALLTHROUGH;

            case 0x0c : // ,PC + 8 Bit Offset
                offset = (EXTEND8(*(pMemory + 2)) + pc + 3) & 0xFFFF;
                s = "<";
                snprintf(code_buf, sizeof(code_buf), "%04X: %02X %02X %02X", pc,
                         code, postbyte, *(pMemory + 2));
                snprintf(mnem_buf, sizeof(mnem_buf), "%s %s%s$%02X,PCR%s",
                         mnemo, br1, s, offset, br2);
                extrabytes = 1;
                break;

            case 0x1d : // [,PC + 16 Bit Offset]
                br1 = "[";
                br2 = "]";
                FALLTHROUGH;

            case 0x0d :  // ,PC + 16 Bit Offset
                offset = (getValueBigEndian<Word>(&pMemory[2]) + pc + 4) & 0xFFFF;
                s = ">";
                snprintf(code_buf, sizeof(code_buf),
                         "%04X: %02X %02X %02X %02X", pc, code, postbyte,
                         *(pMemory + 2), *(pMemory + 3));
                snprintf(mnem_buf, sizeof(mnem_buf), "%s %s%s$%04X,PCR%s",
                         mnemo, br1, s, offset, br2);
                extrabytes = 2;
                break;

            case 0x1f : // [n]
                br1 = "[";
                br2 = "]";
                offset = getValueBigEndian<Word>(&pMemory[2]);
                snprintf(code_buf, sizeof(code_buf),
                         "%04X: %02X %02X %02X %02X", pc, code, postbyte,
                         *(pMemory + 2), *(pMemory + 2));
                snprintf(mnem_buf, sizeof(mnem_buf), "%s %s$%04X%s", mnemo, br1,
                         offset, br2); extrabytes = 2;
                break;

            default:
                snprintf(code_buf, sizeof(code_buf), "%04X: %02X %02X", pc,
                         code, postbyte);
                snprintf(mnem_buf, sizeof(mnem_buf), "%s ????", mnemo);
        }
    }

    return bytes + extrabytes;
} // D_Indexed


inline Byte Da6809::D_Extended(const char *mnemo, Word pc, Byte bytes,
                               const Byte *pMemory, DWord * /*pAddr*/)
{
    Byte code;
    const char *label;

    code = *pMemory;
    auto offset = getValueBigEndian<Word>(&pMemory[1]);
    snprintf(code_buf, sizeof(code_buf), "%04X: %02X %02X %02X", pc, code,
            *(pMemory + 1), *(pMemory + 2));
    label = FlexLabel(offset);

    if (label == nullptr)
    {
        snprintf(mnem_buf, sizeof(mnem_buf), "%s $%04X", mnemo, offset);
    }
    else
    {
        snprintf(mnem_buf, sizeof(mnem_buf), "%s %s ; $%04X", mnemo, label,
                 offset);
    }

    return bytes;
}

inline Byte Da6809::D_Relative(const char *mnemo, Word pc, Byte bytes,
                               const Byte *pMemory, DWord * /*pAddr*/)
{
    Byte code;
    Word offset;
    Word disp;
    const char *label;

    code = *pMemory;
    offset = *(pMemory + 1);

    if (offset < 127)
    {
        disp   = pc + 2 + offset;
    }
    else
    {
        disp   = pc + 2 - (256 - offset);
    }

    snprintf(code_buf, sizeof(code_buf), "%04X: %02X %02X", pc, code, offset);
    label = FlexLabel(disp);

    if (label == nullptr)
    {
        snprintf(mnem_buf, sizeof(mnem_buf), "%s $%04X", mnemo, disp);
    }
    else
    {
        snprintf(mnem_buf, sizeof(mnem_buf), "%s %s ; $%04X", mnemo, label,
                 disp);
    }

    return bytes;
}

inline Byte Da6809::D_RelativeL(
    const char *mnemo, Word pc, Byte bytes, const Byte *pMemory, DWord *pAddr)
{
    Byte code;
    const char *label;

    code = *pMemory;
    auto offset = getValueBigEndian<Word>(&pMemory[1]);

    if (offset < 32767)
    {
        *pAddr = pc + 3 + offset;
    }
    else
    {
        *pAddr = pc + 3 - (65536 - offset);
    }

    snprintf(code_buf, sizeof(code_buf), "%04X: %02X %02X %02X", pc, code,
            *(pMemory + 1), *(pMemory + 2));
    label = FlexLabel(static_cast<Word>(*pAddr));

    if (label == nullptr)
    {
        snprintf(mnem_buf, sizeof(mnem_buf), "%s $%04X", mnemo, (Word)*pAddr);
    }
    else
    {
        snprintf(mnem_buf, sizeof(mnem_buf), "%s %s ; $%04X", mnemo, label,
                 (Word)*pAddr);
    }

    return bytes;
}

inline Byte Da6809::D_Register0(const char *mnemo, Word pc, Byte bytes,
                                const Byte *pMemory)
{
    Byte code;
    Byte postbyte;
    code = *pMemory;
    postbyte = *(pMemory + 1);

    snprintf(code_buf, sizeof(code_buf), "%04X: %02X %02X", pc, code, postbyte);
    snprintf(mnem_buf, sizeof(mnem_buf), "%s %s,%s", mnemo,
             InterRegister(postbyte >> 4),
            InterRegister(postbyte & 0x0f));

    return bytes;
}

inline Byte Da6809::D_Register1(const char *mnemo, Word pc, Byte bytes,
                                const Byte *pMemory)
{
    Byte code;
    Byte postbyte;
    Byte i;
    Byte comma;
    size_t index;

    code      = *pMemory;
    postbyte  = *(pMemory + 1);

    comma = 0;
    snprintf(code_buf, sizeof(code_buf), "%04X: %02X %02X", pc, code, postbyte);
    snprintf(mnem_buf, sizeof(mnem_buf), "%s ", mnemo);
    index = strlen(mnemo);

    for (i = 0; i < 8; i++)
    {
        if (postbyte & (1 << i))
        {
            snprintf(&mnem_buf[index], sizeof(mnem_buf) - index, "%s%s",
                    comma ? "," : "", StackRegister(i, "U"));
            index += strlen(StackRegister(i, "U")) + (comma ? 1 : 0);
            comma = 1;
        } // if
    } // for

    return bytes;
} // D_Register1


inline Byte Da6809::D_Register2(const char *mnemo, Word pc, Byte bytes,
                                const Byte *pMemory)
{
    Byte code;
    Byte postbyte;
    Byte i;
    Byte comma;
    size_t index;

    code      = *pMemory;
    postbyte  = *(pMemory + 1);

    comma = 0;
    snprintf(code_buf, sizeof(code_buf), "%04X: %02X %02X", pc, code, postbyte);
    snprintf(mnem_buf, sizeof(mnem_buf), "%s ", mnemo);
    index = strlen(mnemo);

    for (i = 0; i < 8; i++)
    {
        if (postbyte & (1 << i))
        {
            snprintf(&mnem_buf[index], sizeof(mnem_buf) - index, "%s%s",
                    comma ? "," : "", StackRegister(i, "S"));
            index += strlen(StackRegister(i, "S")) + (comma ? 1 : 0);
            comma = 1;
        } // if
    } // for

    return bytes;
} // D_Register2


inline Byte Da6809::D_Page10(InstFlg *pFlags, Word pc, const Byte *pMemory,
                             DWord *pAddr)
{
    Byte code;

    code = *(pMemory + 1);

    switch (code)
    {
        case 0x21:
            *pFlags |= InstFlg::JumpAddr;
            return D_RelativeL("LBRN ", pc, 4, pMemory + 1, pAddr);

        case 0x22:
            *pFlags |= InstFlg::JumpAddr;
            return D_RelativeL("LBHI ", pc, 4, pMemory + 1, pAddr);

        case 0x23:
            *pFlags |= InstFlg::JumpAddr;
            return D_RelativeL("LBLS ", pc, 4, pMemory + 1, pAddr);

        case 0x24:
            *pFlags |= InstFlg::JumpAddr;
            return D_RelativeL("LBCC ", pc, 4, pMemory + 1, pAddr);

        case 0x25:
            *pFlags |= InstFlg::JumpAddr;
            return D_RelativeL("LBCS ", pc, 4, pMemory + 1, pAddr);

        case 0x26:
            *pFlags |= InstFlg::JumpAddr;
            return D_RelativeL("LBNE ", pc, 4, pMemory + 1, pAddr);

        case 0x27:
            *pFlags |= InstFlg::JumpAddr;
            return D_RelativeL("LBEQ ", pc, 4, pMemory + 1, pAddr);

        case 0x28:
            *pFlags |= InstFlg::JumpAddr;
            return D_RelativeL("LBVC ", pc, 4, pMemory + 1, pAddr);

        case 0x29:
            *pFlags |= InstFlg::JumpAddr;
            return D_RelativeL("LBVS ", pc, 4, pMemory + 1, pAddr);

        case 0x2a:
            *pFlags |= InstFlg::JumpAddr;
            return D_RelativeL("LBPL ", pc, 4, pMemory + 1, pAddr);

        case 0x2b:
            *pFlags |= InstFlg::JumpAddr;
            return D_RelativeL("LBMI ", pc, 4, pMemory + 1, pAddr);

        case 0x2c:
            *pFlags |= InstFlg::JumpAddr;
            return D_RelativeL("LBGE ", pc, 4, pMemory + 1, pAddr);

        case 0x2d:
            *pFlags |= InstFlg::JumpAddr;
            return D_RelativeL("LBLT ", pc, 4, pMemory + 1, pAddr);

        case 0x2e:
            *pFlags |= InstFlg::JumpAddr;
            return D_RelativeL("LBGT ", pc, 4, pMemory + 1, pAddr);

        case 0x2f:
            *pFlags |= InstFlg::JumpAddr;
            return D_RelativeL("LBLE ", pc, 4, pMemory + 1, pAddr);

        case 0x3f:
            *pFlags |= InstFlg::Sub;
            return D_Inherent("SWI2 ", pc, 2, pMemory + 1);

        case 0x83:
            *pFlags |= InstFlg::LabelAddr;
            return D_ImmediatL("CMPD ", pc, 4, pMemory + 1, pAddr);

        case 0x8c:
            *pFlags |= InstFlg::LabelAddr;
            return D_ImmediatL("CMPY ", pc, 4, pMemory + 1, pAddr);

        case 0x8e:
            *pFlags |= InstFlg::LabelAddr;
            return D_ImmediatL("LDY  ", pc, 4, pMemory + 1, pAddr);

        case 0x93:
            return D_Direct("CMPD ", pc, 3, pMemory + 1);

        case 0x9c:
            return D_Direct("CMPY ", pc, 3, pMemory + 1);

        case 0x9e:
            return D_Direct("LDY  ", pc, 3, pMemory + 1);

        case 0x9f:
            return D_Direct("STY  ", pc, 3, pMemory + 1);

        case 0xa3:
            return D_Indexed("CMPD ", pc, 3, pMemory + 1);

        case 0xac:
            return D_Indexed("CMPY ", pc, 3, pMemory + 1);

        case 0xae:
            return D_Indexed("LDY  ", pc, 3, pMemory + 1);

        case 0xaf:
            return D_Indexed("STY  ", pc, 3, pMemory + 1);

        case 0xb3:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("CMPD ", pc, 4, pMemory + 1, pAddr);

        case 0xbc:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("CMPY ", pc, 4, pMemory + 1, pAddr);

        case 0xbe:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("LDY  ", pc, 4, pMemory + 1, pAddr);

        case 0xbf:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("STY  ", pc, 4, pMemory + 1, pAddr);

        case 0xce:
            *pFlags |= InstFlg::LabelAddr;
            return D_ImmediatL("LDS  ", pc, 4, pMemory + 1, pAddr);

        case 0xde:
            return D_Direct("LDS  ", pc, 3, pMemory + 1);

        case 0xdf:
            return D_Direct("STS  ", pc, 3, pMemory + 1);

        case 0xee:
            return D_Indexed("LDS  ", pc, 3, pMemory + 1);

        case 0xef:
            return D_Indexed("STS  ", pc, 3, pMemory + 1);

        case 0xfe:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("LDS  ", pc, 4, pMemory + 1, pAddr);

        case 0xff:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("STS  ", pc, 4, pMemory + 1, pAddr);

        default:
            *pFlags |= InstFlg::Illegal;
            return D_Illegal("", pc, 1, pMemory + 1);
    } // switch
} // D_Page10


inline Byte Da6809::D_Page11(InstFlg *pFlags, Word pc, const Byte *pMemory,
                             DWord *pAddr)
{
    Byte code;

    code = *(pMemory + 1);

    switch (code)
    {
        case 0x3f:
            *pFlags |= InstFlg::Sub;
            return D_Inherent("SWI3 ", pc, 2, pMemory + 1);

        case 0x83:
            *pFlags |= InstFlg::LabelAddr;
            return D_ImmediatL("CMPU ", pc, 4, pMemory + 1, pAddr);

        case 0x8c:
            *pFlags |= InstFlg::LabelAddr;
            return D_ImmediatL("CMPS ", pc, 4, pMemory + 1, pAddr);

        case 0x93:
            return D_Direct("CMPU ", pc, 3, pMemory + 1);

        case 0x9c:
            return D_Direct("CMPS ", pc, 3, pMemory + 1);

        case 0xa3:
            return D_Indexed("CMPU ", pc, 3, pMemory + 1);

        case 0xac:
            return D_Indexed("CMPS ", pc, 3, pMemory + 1);

        case 0xb3:
            return D_Indexed("CMPU ", pc, 4, pMemory + 1);

        case 0xbc:
            return D_Indexed("CMPS ", pc, 4, pMemory + 1);

        default:
            *pFlags |= InstFlg::Illegal;
            return D_Illegal("", pc, 1, pMemory + 1);
    }  // switch
} // D_Page11


int Da6809::Disassemble(
        const Byte * const pMemory,
        DWord pc,
        InstFlg *pFlags,
        DWord *pAddr,
        char **pCode,
        char **pMnemonic)
{
    Byte code;
    Word pc16 = static_cast<Word>(pc);

    *pCode = code_buf;
    *pMnemonic = mnem_buf;
    *pFlags = InstFlg::NONE;
    code = *pMemory;

    switch (code)
    {
        case 0x01:
            if (use_undocumented)
            {
                return D_Direct("neg", pc16, 2, pMemory);
            }

            *pFlags |= InstFlg::Illegal;
            return D_Illegal("", pc16, 1, pMemory);

        case 0x00:
            return D_Direct("NEG  ", pc16, 2, pMemory);

        case 0x02:
            if (use_undocumented)
            {
                return D_Direct("negcom", pc16, 2, pMemory);
            }

            *pFlags |= InstFlg::Illegal;
            return D_Illegal("", pc16, 1, pMemory);

        case 0x03:
            return D_Direct("COM  ", pc16, 2, pMemory);

        case 0x04:
            return D_Direct("LSR  ", pc16, 2, pMemory);

        case 0x05:
            if (use_undocumented)
            {
                return D_Direct("lsr  ", pc16, 2, pMemory);
            }

            *pFlags |= InstFlg::Illegal;
            return D_Illegal("", pc16, 1, pMemory);

        case 0x06:
            return D_Direct("ROR  ", pc16, 2, pMemory);

        case 0x07:
            return D_Direct("ASR  ", pc16, 2, pMemory);

        case 0x08:
            return D_Direct("LSR  ", pc16, 2, pMemory);

        case 0x09:
            return D_Direct("ROR  ", pc16, 2, pMemory);

        case 0x0a:
            return D_Direct("DEC  ", pc16, 2, pMemory);

        case 0x0b:
            if (use_undocumented)
            {
                return D_Direct("dec  ", pc16, 2, pMemory);
            }

            *pFlags |= InstFlg::Illegal;
            return D_Illegal("", pc16, 1, pMemory);

        case 0x0c:
            return D_Direct("INC  ", pc16, 2, pMemory);

        case 0x0d:
            return D_Direct("TST  ", pc16, 2, pMemory);

        case 0x0e:
            *pFlags |= InstFlg::Jump;
            return D_Direct("JMP  ", pc16, 2, pMemory);

        case 0x0f:
            return D_Direct("CLR  ", pc16, 2, pMemory);

        case 0x10:
            return D_Page10(pFlags, pc16, pMemory, pAddr);

        case 0x11:
            return D_Page11(pFlags, pc16, pMemory, pAddr);

        case 0x12:
            *pFlags |= InstFlg::Noop;
            return D_Inherent("NOP  ", pc16, 1, pMemory);

        case 0x13:
            *pFlags |= InstFlg::Jump;
            return D_Inherent("SYNC ", pc16, 1, pMemory);

        // 0x14, 0x15 is illegal
        case 0x16:
            *pFlags |= InstFlg::Jump | InstFlg::JumpAddr;
            return D_RelativeL("LBRA ", pc16, 3, pMemory, pAddr);

        case 0x17:
            *pFlags |= InstFlg::Sub | InstFlg::LabelAddr;
            return D_RelativeL("LBSR ", pc16, 3, pMemory, pAddr);

        // 0x18 is illegal
        case 0x19:
            return D_Inherent("DAA  ", pc16, 1, pMemory);

        case 0x1a:
            return D_Immediat("ORCC ", pc16, 2, pMemory);

        // 0x1b is illegal
        case 0x1c:
            return D_Immediat("ANDCC", pc16, 2, pMemory);

        case 0x1d:
            return D_Inherent("SEX  ", pc16, 1, pMemory);

        case 0x1e:
            return D_Register0("EXG  ", pc16, 2, pMemory);

        case 0x1f:
            return D_Register0("TFR  ", pc16, 2, pMemory);

        case 0x20:
            *pFlags |= InstFlg::Jump | InstFlg::JumpAddr;
            return D_Relative("BRA  ", pc16, 2, pMemory, pAddr);

        case 0x21:
            *pFlags |= InstFlg::JumpAddr;
            return D_Relative("BRN  ", pc16, 2, pMemory, pAddr);

        case 0x22:
            *pFlags |= InstFlg::JumpAddr;
            return D_Relative("BHI  ", pc16, 2, pMemory, pAddr);

        case 0x23:
            *pFlags |= InstFlg::JumpAddr;
            return D_Relative("BLS  ", pc16, 2, pMemory, pAddr);

        case 0x24:
            *pFlags |= InstFlg::JumpAddr;
            return D_Relative("BCC  ", pc16, 2, pMemory, pAddr);

        case 0x25:
            *pFlags |= InstFlg::JumpAddr;
            return D_Relative("BCS  ", pc16, 2, pMemory, pAddr);

        case 0x26:
            *pFlags |= InstFlg::JumpAddr;
            return D_Relative("BNE  ", pc16, 2, pMemory, pAddr);

        case 0x27:
            *pFlags |= InstFlg::JumpAddr;
            return D_Relative("BEQ  ", pc16, 2, pMemory, pAddr);

        case 0x28:
            *pFlags |= InstFlg::JumpAddr;
            return D_Relative("BVC  ", pc16, 2, pMemory, pAddr);

        case 0x29:
            *pFlags |= InstFlg::JumpAddr;
            return D_Relative("BVS  ", pc16, 2, pMemory, pAddr);

        case 0x2a:
            *pFlags |= InstFlg::JumpAddr;
            return D_Relative("BPL  ", pc16, 2, pMemory, pAddr);

        case 0x2b:
            *pFlags |= InstFlg::JumpAddr;
            return D_Relative("BMI  ", pc16, 2, pMemory, pAddr);

        case 0x2c:
            *pFlags |= InstFlg::JumpAddr;
            return D_Relative("BGE  ", pc16, 2, pMemory, pAddr);

        case 0x2d:
            *pFlags |= InstFlg::JumpAddr;
            return D_Relative("BLT  ", pc16, 2, pMemory, pAddr);

        case 0x2e:
            *pFlags |= InstFlg::JumpAddr;
            return D_Relative("BGT  ", pc16, 2, pMemory, pAddr);

        case 0x2f:
            *pFlags |= InstFlg::JumpAddr;
            return D_Relative("BLE  ", pc16, 2, pMemory, pAddr);

        case 0x30:
            return D_Indexed("LEAX ", pc16, 2, pMemory);

        case 0x31:
            return D_Indexed("LEAY ", pc16, 2, pMemory);

        case 0x32:
            return D_Indexed("LEAS ", pc16, 2, pMemory);

        case 0x33:
            return D_Indexed("LEAU ", pc16, 2, pMemory);

        case 0x34:
            return D_Register1("PSHS ", pc16, 2, pMemory);

        case 0x35:
            return D_Register1("PULS ", pc16, 2, pMemory);

        case 0x36:
            return D_Register2("PSHU ", pc16, 2, pMemory);

        case 0x37:
            return D_Register2("PULU ", pc16, 2, pMemory);

        // 0x38 is illegal
        case 0x39:
            *pFlags |= InstFlg::Jump;
            return D_Inherent("RTS  ", pc16, 1, pMemory);

        case 0x3a:
            return D_Inherent("ABX  ", pc16, 1, pMemory);

        case 0x3b:
            *pFlags |= InstFlg::Jump;
            return D_Inherent("RTI  ", pc16, 1, pMemory);

        case 0x3c:
            *pFlags |= InstFlg::Jump;
            return D_Immediat("CWAI ", pc16, 2, pMemory);

        case 0x3d:
            return D_Inherent("MUL  ", pc16, 1, pMemory);

        case 0x3e:
            if (use_undocumented)
            {
                return D_Direct("reset", pc16, 1, pMemory);
            }

            *pFlags |= InstFlg::Illegal;
            return D_Illegal("", pc16, 1, pMemory);

        case 0x3f:
            *pFlags |= InstFlg::Illegal;
            return D_Illegal("SWI  ", pc16, 1, pMemory);

        case 0x40:
            return D_Inherent("NEGA ", pc16, 1, pMemory);

        case 0x41:
            if (use_undocumented)
            {
                return D_Direct("nega ", pc16, 1, pMemory);
            }

            *pFlags |= InstFlg::Illegal;
            return D_Illegal("", pc16, 1, pMemory);

        case 0x43:
            return D_Inherent("COMA ", pc16, 1, pMemory);

        case 0x44:
            return D_Inherent("LSRA ", pc16, 1, pMemory);

        case 0x45:
            if (use_undocumented)
            {
                return D_Direct("lsra ", pc16, 1, pMemory);
            }

            *pFlags |= InstFlg::Illegal;
            return D_Illegal("", pc16, 1, pMemory);

        case 0x46:
            return D_Inherent("RORA ", pc16, 1, pMemory);

        case 0x47:
            return D_Inherent("ASRA ", pc16, 1, pMemory);

        case 0x48:
            return D_Inherent("LSLA ", pc16, 1, pMemory);

        case 0x49:
            return D_Inherent("ROLA ", pc16, 1, pMemory);

        case 0x4a:
            return D_Inherent("DECA ", pc16, 1, pMemory);

        case 0x4b:
            if (use_undocumented)
            {
                return D_Direct("deca ", pc16, 1, pMemory);
            }

            *pFlags |= InstFlg::Illegal;
            return D_Illegal("", pc16, 1, pMemory);

        case 0x4c:
            return D_Inherent("INCA ", pc16, 1, pMemory);

        case 0x4d:
            return D_Inherent("TSTA ", pc16, 1, pMemory);

        case 0x4e:
            if (use_undocumented)
            {
                return D_Direct("clra ", pc16, 1, pMemory);
            }

            *pFlags |= InstFlg::Illegal;
            return D_Illegal("", pc16, 1, pMemory);

        case 0x4f:
            return D_Inherent("CLRA ", pc16, 1, pMemory);

        case 0x50:
            return D_Inherent("NEGB ", pc16, 1, pMemory);

        case 0x51:
            if (use_undocumented)
            {
                return D_Direct("negb ", pc16, 1, pMemory);
            }

            *pFlags |= InstFlg::Illegal;
            return D_Illegal("", pc16, 1, pMemory);

        case 0x53:
            return D_Inherent("COMB ", pc16, 1, pMemory);

        case 0x54:
            return D_Inherent("LSRB ", pc16, 1, pMemory);

        case 0x55:
            if (use_undocumented)
            {
                return D_Direct("lsrb ", pc16, 1, pMemory);
            }

            *pFlags |= InstFlg::Illegal;
            return D_Illegal("", pc16, 1, pMemory);

        case 0x56:
            return D_Inherent("RORB ", pc16, 1, pMemory);

        case 0x57:
            return D_Inherent("ASRB ", pc16, 1, pMemory);

        case 0x58:
            return D_Inherent("LSLB ", pc16, 1, pMemory);

        case 0x59:
            return D_Inherent("ROLB ", pc16, 1, pMemory);

        case 0x5a:
            return D_Inherent("DECB ", pc16, 1, pMemory);

        case 0x5b:
            if (use_undocumented)
            {
                return D_Direct("decb ", pc16, 1, pMemory);
            }

            *pFlags |= InstFlg::Illegal;
            return D_Illegal("", pc16, 1, pMemory);

        case 0x5c:
            return D_Inherent("INCB ", pc16, 1, pMemory);

        case 0x5d:
            return D_Inherent("TSTB ", pc16, 1, pMemory);

        case 0x5e:
            if (use_undocumented)
            {
                return D_Direct("clrb ", pc16, 1, pMemory);
            }

            *pFlags |= InstFlg::Illegal;
            return D_Illegal("", pc16, 1, pMemory);

        case 0x5f:
            return D_Inherent("CLRB ", pc16, 1, pMemory);

        case 0x60:
            return D_Indexed("NEG  ", pc16, 2, pMemory);

        case 0x61:
            if (use_undocumented)
            {
                return D_Indexed("neg  ", pc16, 2, pMemory);
            }

            *pFlags |= InstFlg::Illegal;
            return D_Illegal("", pc16, 1, pMemory);

        case 0x63:
            return D_Indexed("COM  ", pc16, 2, pMemory);

        case 0x64:
            return D_Indexed("LSR  ", pc16, 2, pMemory);

        case 0x65:
            if (use_undocumented)
            {
                return D_Indexed("lsr  ", pc16, 2, pMemory);
            }

            *pFlags |= InstFlg::Illegal;
            return D_Illegal("", pc16, 1, pMemory);

        case 0x66:
            return D_Indexed("ROR  ", pc16, 2, pMemory);

        case 0x67:
            return D_Indexed("ASR  ", pc16, 2, pMemory);

        case 0x68:
            return D_Indexed("LSL  ", pc16, 2, pMemory);

        case 0x69:
            return D_Indexed("ROL  ", pc16, 2, pMemory);

        case 0x6a:
            return D_Indexed("DEC  ", pc16, 2, pMemory);

        case 0x6b:
            if (use_undocumented)
            {
                return D_Indexed("dec  ", pc16, 2, pMemory);
            }

            *pFlags |= InstFlg::Illegal;
            return D_Illegal("", pc16, 1, pMemory);

        case 0x6c:
            return D_Indexed("INC  ", pc16, 2, pMemory);

        case 0x6d:
            return D_Indexed("TST  ", pc16, 2, pMemory);

        case 0x6e:
            *pFlags |= InstFlg::Jump | InstFlg::ComputedGoto;
            return D_Indexed("JMP  ", pc16, 2, pMemory);

        case 0x6f:
            return D_Indexed("CLR  ", pc16, 2, pMemory);

        case 0x70:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("NEG  ", pc16, 3, pMemory, pAddr);

        case 0x71:
            if (use_undocumented)
            {
                *pFlags |= InstFlg::LabelAddr;
                return D_Extended("neg  ", pc16, 3, pMemory, pAddr);
            };

            *pFlags |= InstFlg::Illegal;

            return D_Illegal("", pc16, 1, pMemory);

        case 0x73:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("COM  ", pc16, 3, pMemory, pAddr);

        case 0x74:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("LSR  ", pc16, 3, pMemory, pAddr);

        case 0x75:
            if (use_undocumented)
            {
                *pFlags |= InstFlg::LabelAddr;
                return D_Extended("lsr  ", pc16, 3, pMemory, pAddr);
            };

            *pFlags |= InstFlg::Illegal;

            return D_Illegal("", pc16, 1, pMemory);

        case 0x76:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("ROR  ", pc16, 3, pMemory, pAddr);

        case 0x77:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("ASR  ", pc16, 3, pMemory, pAddr);

        case 0x78:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("LSL  ", pc16, 3, pMemory, pAddr);

        case 0x79:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("ROL  ", pc16, 3, pMemory, pAddr);

        case 0x7a:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("DEC  ", pc16, 3, pMemory, pAddr);

        case 0x7b:
            if (use_undocumented)
            {
                *pFlags |= InstFlg::LabelAddr;
                return D_Extended("dec  ", pc16, 3, pMemory, pAddr);
            };

            *pFlags |= InstFlg::Illegal;

            return D_Illegal("", pc16, 1, pMemory);

        case 0x7c:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("INC  ", pc16, 3, pMemory, pAddr);

        case 0x7d:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("TST  ", pc16, 3, pMemory, pAddr);

        case 0x7e:
            *pFlags |= InstFlg::Jump;
            return D_Extended("JMP  ", pc16, 3, pMemory, pAddr);

        case 0x7f:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("CLR  ", pc16, 3, pMemory, pAddr);

        case 0x80:
            return D_Immediat("SUBA ", pc16, 2, pMemory);

        case 0x81:
            return D_Immediat("CMPA ", pc16, 2, pMemory);

        case 0x82:
            return D_Immediat("SBCA ", pc16, 2, pMemory);

        case 0x83:
            return D_ImmediatL("SUBD ", pc16, 3, pMemory, pAddr);

        case 0x84:
            return D_Immediat("ANDA ", pc16, 2, pMemory);

        case 0x85:
            return D_Immediat("BITA ", pc16, 2, pMemory);

        case 0x86:
            return D_Immediat("LDA  ", pc16, 2, pMemory);

        // 0x87 is illegal
        case 0x88:
            return D_Immediat("EORA ", pc16, 2, pMemory);

        case 0x89:
            return D_Immediat("ADCA ", pc16, 2, pMemory);

        case 0x8a:
            return D_Immediat("ORA  ", pc16, 2, pMemory);

        case 0x8b:
            return D_Immediat("ADDA ", pc16, 2, pMemory);

        case 0x8c:
            return D_ImmediatL("CMPX ", pc16, 3, pMemory, pAddr);

        case 0x8d:
            *pFlags |= InstFlg::Sub;
            return D_Relative("BSR  ", pc16, 2, pMemory, pAddr);

        case 0x8e:
            return D_ImmediatL("LDX  ", pc16, 2, pMemory, pAddr);

        // 0x8f is illegal

        case 0x90:
            return D_Direct("SUBA ", pc16, 2, pMemory);

        case 0x91:
            return D_Direct("CMPA ", pc16, 2, pMemory);

        case 0x92:
            return D_Direct("SBCA ", pc16, 2, pMemory);

        case 0x93:
            return D_Direct("SUBD ", pc16, 2, pMemory);

        case 0x94:
            return D_Direct("ANDA ", pc16, 2, pMemory);

        case 0x95:
            return D_Direct("BITA ", pc16, 2, pMemory);

        case 0x96:
            return D_Direct("LDA  ", pc16, 2, pMemory);

        case 0x97:
            return D_Direct("STA  ", pc16, 2, pMemory);

        case 0x98:
            return D_Direct("EORA ", pc16, 2, pMemory);

        case 0x99:
            return D_Direct("ADCA ", pc16, 2, pMemory);

        case 0x9a:
            return D_Direct("ORA  ", pc16, 2, pMemory);

        case 0x9b:
            return D_Direct("ADDA ", pc16, 2, pMemory);

        case 0x9c:
            return D_Direct("CMPX ", pc16, 2, pMemory);

        case 0x9d:
            *pFlags |= InstFlg::Sub;
            return D_Direct("JSR  ", pc16, 2, pMemory);

        case 0x9e:
            return D_Direct("LDX  ", pc16, 2, pMemory);

        case 0x9f:
            return D_Direct("STX  ", pc16, 2, pMemory);

        case 0xa0:
            return D_Indexed("SUBA ", pc16, 2, pMemory);

        case 0xa1:
            return D_Indexed("CMPA ", pc16, 2, pMemory);

        case 0xa2:
            return D_Indexed("SBCA ", pc16, 2, pMemory);

        case 0xa3:
            return D_Indexed("SUBD ", pc16, 2, pMemory);

        case 0xa4:
            return D_Indexed("ANDA ", pc16, 2, pMemory);

        case 0xa5:
            return D_Indexed("BITA ", pc16, 2, pMemory);

        case 0xa6:
            return D_Indexed("LDA  ", pc16, 2, pMemory);

        case 0xa7:
            return D_Indexed("STA  ", pc16, 2, pMemory);

        case 0xa8:
            return D_Indexed("EORA ", pc16, 2, pMemory);

        case 0xa9:
            return D_Indexed("ADCA ", pc16, 2, pMemory);

        case 0xaa:
            return D_Indexed("ORA  ", pc16, 2, pMemory);

        case 0xab:
            return D_Indexed("ADDA ", pc16, 2, pMemory);

        case 0xac:
            return D_Indexed("CMPX ", pc16, 2, pMemory);

        case 0xad:
            *pFlags |= InstFlg::Sub | InstFlg::ComputedGoto;
            return D_Indexed("JSR  ", pc16, 2, pMemory);

        case 0xae:
            return D_Indexed("LDX  ", pc16, 2, pMemory);

        case 0xaf:
            return D_Indexed("STX  ", pc16, 2, pMemory);

        case 0xb0:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("SUBA ", pc16, 3, pMemory, pAddr);

        case 0xb1:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("CMPA ", pc16, 3, pMemory, pAddr);

        case 0xb2:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("SBCA ", pc16, 3, pMemory, pAddr);

        case 0xb3:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("SUBD ", pc16, 3, pMemory, pAddr);

        case 0xb4:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("ANDA ", pc16, 3, pMemory, pAddr);

        case 0xb5:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("BITA ", pc16, 3, pMemory, pAddr);

        case 0xb6:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("LDA  ", pc16, 3, pMemory, pAddr);

        case 0xb7:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("STA  ", pc16, 3, pMemory, pAddr);

        case 0xb8:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("EORA ", pc16, 3, pMemory, pAddr);

        case 0xb9:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("ADCA ", pc16, 3, pMemory, pAddr);

        case 0xba:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("ORA  ", pc16, 3, pMemory, pAddr);

        case 0xbb:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("ADDA ", pc16, 3, pMemory, pAddr);

        case 0xbc:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("CMPX ", pc16, 3, pMemory, pAddr);

        case 0xbd:
            *pFlags |= InstFlg::Sub | InstFlg::JumpAddr;
            return D_Extended("JSR  ", pc16, 3, pMemory, pAddr);

        case 0xbe:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("LDX  ", pc16, 3, pMemory, pAddr);

        case 0xbf:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("STX  ", pc16, 3, pMemory, pAddr);

        case 0xc0:
            return D_Immediat("SUBB ", pc16, 2, pMemory);

        case 0xc1:
            return D_Immediat("CMPB ", pc16, 2, pMemory);

        case 0xc2:
            return D_Immediat("SBCB ", pc16, 2, pMemory);

        case 0xc3:
            *pFlags |= InstFlg::LabelAddr;
            return D_ImmediatL("ADDD ", pc16, 3, pMemory, pAddr);

        case 0xc4:
            return D_Immediat("ANDB ", pc16, 2, pMemory);

        case 0xc5:
            return D_Immediat("BITB ", pc16, 2, pMemory);

        case 0xc6:
            return D_Immediat("LDB  ", pc16, 2, pMemory);

        // 0xc7 is illegal
        case 0xc8:
            return D_Immediat("EORB ", pc16, 2, pMemory);

        case 0xc9:
            return D_Immediat("ADCB ", pc16, 2, pMemory);

        case 0xca:
            return D_Immediat("ORB  ", pc16, 2, pMemory);

        case 0xcb:
            return D_Immediat("ADDB ", pc16, 2, pMemory);

        case 0xcc:
            *pFlags |= InstFlg::LabelAddr;
            return D_ImmediatL("LDD  ", pc16, 3, pMemory, pAddr);

        // 0xcd is illegal
        case 0xce:
            *pFlags |= InstFlg::LabelAddr;
            return D_ImmediatL("LDU  ", pc16, 3, pMemory, pAddr);

        // 0xcf is illegal

        case 0xd0:
            return D_Direct("SUBB ", pc16, 2, pMemory);

        case 0xd1:
            return D_Direct("CMPB ", pc16, 2, pMemory);

        case 0xd2:
            return D_Direct("SBCB ", pc16, 2, pMemory);

        case 0xd3:
            return D_Direct("ADDD ", pc16, 2, pMemory);

        case 0xd4:
            return D_Direct("ANDB ", pc16, 2, pMemory);

        case 0xd5:
            return D_Direct("BITB ", pc16, 2, pMemory);

        case 0xd6:
            return D_Direct("LDB  ", pc16, 2, pMemory);

        case 0xd7:
            return D_Direct("STB  ", pc16, 2, pMemory);

        case 0xd8:
            return D_Direct("EORB ", pc16, 2, pMemory);

        case 0xd9:
            return D_Direct("ADCB ", pc16, 2, pMemory);

        case 0xda:
            return D_Direct("ORB  ", pc16, 2, pMemory);

        case 0xdb:
            return D_Direct("ADDB ", pc16, 2, pMemory);

        case 0xdc:
            return D_Direct("LDD  ", pc16, 2, pMemory);

        case 0xdd:
            return D_Direct("STD  ", pc16, 2, pMemory);

        case 0xde:
            return D_Direct("LDU  ", pc16, 2, pMemory);

        case 0xdf:
            return D_Direct("STU  ", pc16, 2, pMemory);

        case 0xe0:
            return D_Indexed("SUBB ", pc16, 2, pMemory);

        case 0xe1:
            return D_Indexed("CMPB ", pc16, 2, pMemory);

        case 0xe2:
            return D_Indexed("SBCB ", pc16, 2, pMemory);

        case 0xe3:
            return D_Indexed("ADDD ", pc16, 2, pMemory);

        case 0xe4:
            return D_Indexed("ANDB ", pc16, 2, pMemory);

        case 0xe5:
            return D_Indexed("BITB ", pc16, 2, pMemory);

        case 0xe6:
            return D_Indexed("LDB  ", pc16, 2, pMemory);

        case 0xe7:
            return D_Indexed("STB  ", pc16, 2, pMemory);

        case 0xe8:
            return D_Indexed("EORB ", pc16, 2, pMemory);

        case 0xe9:
            return D_Indexed("ADCB ", pc16, 2, pMemory);

        case 0xea:
            return D_Indexed("ORB  ", pc16, 2, pMemory);

        case 0xeb:
            return D_Indexed("ADDB ", pc16, 2, pMemory);

        case 0xec:
            return D_Indexed("LDD  ", pc16, 2, pMemory);

        case 0xed:
            return D_Indexed("STD  ", pc16, 2, pMemory);

        case 0xee:
            return D_Indexed("LDU  ", pc16, 2, pMemory);

        case 0xef:
            return D_Indexed("STU  ", pc16, 2, pMemory);

        case 0xf0:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("SUBB ", pc16, 3, pMemory, pAddr);

        case 0xf1:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("CMPB ", pc16, 3, pMemory, pAddr);

        case 0xf2:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("SBCA ", pc16, 3, pMemory, pAddr);

        case 0xf3:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("ADDD ", pc16, 3, pMemory, pAddr);

        case 0xf4:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("ANDB ", pc16, 3, pMemory, pAddr);

        case 0xf5:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("BITB ", pc16, 3, pMemory, pAddr);

        case 0xf6:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("LDB  ", pc16, 3, pMemory, pAddr);

        case 0xf7:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("STB  ", pc16, 3, pMemory, pAddr);

        case 0xf8:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("EORB ", pc16, 3, pMemory, pAddr);

        case 0xf9:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("ADCB ", pc16, 3, pMemory, pAddr);

        case 0xfa:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("ORB  ", pc16, 3, pMemory, pAddr);

        case 0xfb:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("ADDB ", pc16, 3, pMemory, pAddr);

        case 0xfc:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("LDD  ", pc16, 3, pMemory, pAddr);

        case 0xfd:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("STD  ", pc16, 3, pMemory, pAddr);

        case 0xfe:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("LDU  ", pc16, 3, pMemory, pAddr);

        case 0xff:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("STU  ", pc16, 3, pMemory, pAddr);

        default:
            *pFlags |= InstFlg::Illegal;
            return D_Illegal("", pc16, 1, pMemory);
    }  // switch
} // disassemble

