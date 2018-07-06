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

    switch (addr)
    {
        // FLEX DOS entries:
        case 0xCD00:
            return "COLDS";

        case 0xCD03:
            return "WARMS";

        case 0xCD06:
            return "RENTER";

        case 0xCD09:
            return "INCH";

        case 0xCD0C:
            return "INCH2";

        case 0xCD0F:
            return "OUTCH";

        case 0xCD12:
            return "OUTCH2";

        case 0xCD15:
            return "GETCHR";

        case 0xCD18:
            return "PUTCHR";

        case 0xCD1B:
            return "INBUFF";

        case 0xCD1E:
            return "PSTRNG";

        case 0xCD21:
            return "CLASS";

        case 0xCD24:
            return "PCRLF";

        case 0xCD27:
            return "NXTCH";

        case 0xCD2A:
            return "RSTRIO";

        case 0xCD2D:
            return "GETFIL";

        case 0xCD30:
            return "LOAD";

        case 0xCD33:
            return "SETEXT";

        case 0xCD36:
            return "ADDBX";

        case 0xCD39:
            return "OUTDEC";

        case 0xCD3C:
            return "OUTHEX";

        case 0xCD3F:
            return "RPTERR";

        case 0xCD42:
            return "GETHEX";

        case 0xCD45:
            return "OUTADR";

        case 0xCD48:
            return "INDEC";

        case 0xCD4B:
            return "DOCMND";

        case 0xCD4E:
            return "STAT";

        // FLEX FMS entries:
        case 0xD400:
            return "FMSINI";   // FMS init

        case 0xD403:
            return "FMSCLS";   // FMS close

        case 0xD406:
            return "FMS";

        case 0xC840:
            return "FCB";  // standard system FCB

        // miscellenious:
        case 0xD435:
            return "VFYFLG";   // FMS verify flag

        case 0xC080:
            return "LINBUF";   // line buffer

        case 0xCC00:
            return "TTYBS";

        case 0xCC01:
            return "TTYDEL";

        case 0xCC02:
            return "TTYEOL";

        case 0xCC03:
            return "TTYDPT";

        case 0xCC04:
            return "TTYWDT";

        case 0xCC11:
            return "TTYTRM";

        case 0xCC12:
            return "COMTBL"; // user command table

        case 0xCC14:
            return "LINBFP"; // line buffer pointer

        case 0xCC16:
            return "ESCRET"; // escape return register

        case 0xCC18:
            return "LINCHR"; // current char in linebuffer

        case 0xCC19:
            return "LINPCH"; // previous char in linebuffer

        case 0xCC1A:
            return "LINENR"; //line nr of current page

        case 0xCC1B:
            return "LODOFS"; // loader address offset

        case 0xCC1D:
            return "TFRFLG"; // loader  transfer flag

        case 0xCC1E:
            return "TFRADR"; // transfer address

        case 0xCC20:
            return "FMSERR"; // FMS error type

        case 0xCC21:
            return "IOFLG"; // special I/O flag

        case 0xCC22:
            return "OUTSWT"; // output switch

        case 0xCC23:
            return "INSWT"; // input switch

        case 0xCC24:
            return "OUTADR"; // file output address

        case 0xCC26:
            return "INADR"; // file input address

        case 0xCC28:
            return "COMFLG"; // command flag

        case 0xCC29:
            return "OUTCOL"; // current output column

        case 0xCC2A:
            return "SCRATC"; // system scratch

        case 0xCC2B:
            return "MEMEND"; // memory end

        case 0xCC2D:
            return "ERRVEC"; // error name vector

        case 0xCC2F:
            return "INECHO"; // file input echo flag

        // printer support
        case 0xCCC0:
            return "PRTINI"; // printer initialize

        case 0xCCD8:
            return "PRTCHK"; // printer check

        case 0xCCE4:
            return "PRTOUT"; // printer output

        default:
            return nullptr;
    }

#endif // #ifdef FLEX_LABEL
    return nullptr;
}

const char *Da6809::IndexedRegister(Byte which)
{
    switch (which & 0x03)
    {
        case 0:
            return "X";

        case 1:
            return "Y";

        case 2:
            return "U";

        case 3:
            return "S";
    } // switch

    return "?"; // should never happen
}

const char *Da6809::InterRegister(Byte which)
{
    switch (which & 0x0f)
    {
        case 0x0:
            return "D";

        case 0x1:
            return "X";

        case 0x2:
            return "Y";

        case 0x3:
            return "U";

        case 0x4:
            return "S";

        case 0x5:
            return "PC";

        case 0x8:
            return "A";

        case 0x9:
            return "B";

        case 0xa:
            return "CC";

        case 0xb:
            return "DP";

        default:
            return "??";
    } // switch
}


const char *Da6809::StackRegister(Byte which, const char *not_stack)
{
    switch (which & 0x07)
    {
        case 0x0:
            return "CC";

        case 0x1:
            return "A";

        case 0x2:
            return "B";

        case 0x3:
            return "DP";

        case 0x4:
            return "X";

        case 0x5:
            return "Y";

        case 0x6:
            return not_stack;

        case 0x7:
            return "PC";

        default:
            return "??"; // this case should never happen
    } // switch
}


inline Byte Da6809::D_Illegal(const char *mnemo, Word pc, Byte bytes,
                              const Byte *pMemory)
{
    Byte code;

    code = *pMemory;
    sprintf(code_buf, "%04X: %02X", pc, code);
    sprintf(mnem_buf, "%s ?????", mnemo);
    return bytes;
}


inline Byte Da6809::D_Direct(const char *mnemo, Word pc, Byte bytes,
                             const Byte *pMemory)
{
    Byte code, offset;

    code = *pMemory;
    offset = *(pMemory + 1);
    sprintf(code_buf, "%04X: %02X %02X", pc, code, offset);
    sprintf(mnem_buf, "%s $%02X", mnemo, offset);
    return bytes;
}


inline Byte Da6809::D_Immediat(const char *mnemo, Word pc, Byte bytes,
                               const Byte *pMemory)
{
    Byte code, offset;

    code = *pMemory;
    offset = *(pMemory + 1);
    sprintf(code_buf, "%04X: %02X %02X", pc, code, offset);
    sprintf(mnem_buf, "%s #$%02X", mnemo, offset);
    return bytes;
}


inline Byte Da6809::D_ImmediatL(const char *mnemo, Word pc, Byte bytes,
                                const Byte *pMemory, DWord * /*pAddr*/)
{
    Byte code;
    Word offset;
    const char *label;

    code = *pMemory;
    offset = (*(pMemory + 1) << 8) | *(pMemory + 2);
    sprintf(code_buf, "%04X: %02X %02X %02X", pc, code, *(pMemory + 1),
            *(pMemory + 2));
    label = FlexLabel(offset);

    if (label == nullptr)
    {
        sprintf(mnem_buf, "%s #$%04X", mnemo, offset);
    }
    else
    {
        sprintf(mnem_buf, "%s #%s ; $%04X", mnemo, label, offset);
    }

    return bytes;
}


inline Byte Da6809::D_Inherent(const char *mnemo, Word pc, Byte bytes,
                               const Byte *pMemory)
{
    int code;

    code = *pMemory;
    sprintf(code_buf, "%04X: %02X", pc, code);
    sprintf(mnem_buf, "%s", mnemo);
    return bytes;
}  // D_Inherent


Byte Da6809::D_Indexed(const char *mnemo, Word pc, Byte bytes,
                       const Byte *pMemory)
{
    Byte code, postbyte;
    const char *s, *br1, *br2;
    Byte extrabytes, disp;
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

        sprintf(code_buf, "%04X: %02X %02X", pc, code, postbyte);
        sprintf(mnem_buf, "%s %s$%02X,%s", mnemo, s, disp,
                IndexedRegister(postbyte >> 5));
    }
    else
    {
        switch (postbyte & 0x1f)
        {
            case 0x00 : // ,R+
                sprintf(code_buf, "%04X: %02X %02X", pc, code, postbyte);
                sprintf(mnem_buf, "%s ,%s+", mnemo,
                        IndexedRegister(postbyte >> 5));
                break;

            case 0x11 : // [,R++]
                br1 = "[";
                br2 = "]";

            case 0x01 : // ,R++
                sprintf(code_buf, "%04X: %02X %02X", pc, code, postbyte);
                sprintf(mnem_buf, "%s %s,%s++%s", mnemo,
                        br1, IndexedRegister(postbyte >> 5), br2);
                break;

            case 0x02 : // ,-R
                sprintf(code_buf, "%04X: %02X %02X", pc, code, postbyte);
                sprintf(mnem_buf, "%s ,-%s", mnemo,
                        IndexedRegister(postbyte >> 5));
                break;

            case 0x13 : // [,R--]
                br1 = "[";
                br2 = "]";

            case 0x03 : // ,--R
                sprintf(code_buf, "%04X: %02X %02X", pc, code, postbyte);
                sprintf(mnem_buf, "%s %s,--%s%s", mnemo,
                        br1, IndexedRegister(postbyte >> 5), br2);
                break;

            case 0x14 : // [,R--]
                br1 = "[";
                br2 = "]";

            case 0x04 : // ,R
                sprintf(code_buf, "%04X: %02X %02X", pc, code, postbyte);
                sprintf(mnem_buf, "%s %s,%s%s", mnemo,
                        br1, IndexedRegister(postbyte >> 5), br2);
                break;

            case 0x15 : // [B,R]
                br1 = "[";
                br2 = "]";

            case 0x05 : // B,R
                sprintf(code_buf, "%04X: %02X %02X", pc, code, postbyte);
                sprintf(mnem_buf, "%s %sB,%s%s", mnemo,
                        br1, IndexedRegister(postbyte >> 5), br2);
                break;

            case 0x16 : // [A,R]
                br1 = "[";
                br2 = "]";

            case 0x06 : // A,R
                sprintf(code_buf, "%04X: %02X %02X", pc, code, postbyte);
                sprintf(mnem_buf, "%s %sA,%s%s", mnemo,
                        br1, IndexedRegister(postbyte >> 5), br2);
                break;

            case 0x18 : // [,R + 8 Bit Offset]
                br1 = "[";
                br2 = "]";

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

                sprintf(code_buf, "%04X: %02X %02X %02X", pc, code, postbyte,
                        *(pMemory + 2));
                sprintf(mnem_buf, "%s %s%s$%02X,%s%s", mnemo, br1, s, offset,
                        IndexedRegister(postbyte >> 5), br2);
                extrabytes = 1;
                break;

            case 0x19 : // [,R + 16 Bit Offset]
                br1 = "[";
                br2 = "]";

            case 0x09 : // ,R + 16 Bit Offset
                offset = (*(pMemory + 2) << 8 | *(pMemory + 3));
                s = "";
                sprintf(code_buf, "%04X: %02X %02X %02X %02X", pc, code,
                        postbyte, *(pMemory + 2), *(pMemory + 3));
                sprintf(mnem_buf, "%s %s%s$%04X,%s%s", mnemo,
                        br1, s, offset, IndexedRegister(postbyte >> 5), br2);
                extrabytes = 2;
                break;

            case 0x1b : // [D,R]
                br1 = "[";
                br2 = "]";

            case 0x0b : // D,R
                sprintf(code_buf, "%04X: %02X %02X", pc, code, postbyte);
                sprintf(mnem_buf, "%s %sD,%s%s", mnemo,
                        br1, IndexedRegister(postbyte >> 5), br2);
                break;

            case 0x1c : // [,PC + 8 Bit Offset]
                br1 = "[";
                br2 = "]";

            case 0x0c : // ,PC + 8 Bit Offset
                offset = (EXTEND8(*(pMemory + 2)) + pc + 3) & 0xFFFF;
                s = "<";
                sprintf(code_buf, "%04X: %02X %02X %02X", pc, code, postbyte,
                        *(pMemory + 2));
                sprintf(mnem_buf, "%s %s%s$%02X,PCR%s", mnemo, br1, s, offset,
                        br2);
                extrabytes = 1;
                break;

            case 0x1d : // [,PC + 16 Bit Offset]
                br1 = "[";
                br2 = "]";

            case 0x0d :  // ,PC + 16 Bit Offset
                offset = (((*(pMemory + 2) << 8) | *(pMemory + 3)) + pc + 4) &
                         0xFFFF;
                s = ">";
                sprintf(code_buf, "%04X: %02X %02X %02X %02X", pc, code,
                        postbyte, *(pMemory + 2), *(pMemory + 3));
                sprintf(mnem_buf, "%s %s%s$%04X,PCR%s", mnemo, br1, s, offset,
                        br2);
                extrabytes = 2;
                break;

            case 0x1f : // [n]
                br1 = "[";
                br2 = "]";
                offset = (*(pMemory + 2) << 8) | *(pMemory + 3);
                sprintf(code_buf, "%04X: %02X %02X %02X %02X", pc, code,
                        postbyte, *(pMemory + 2), *(pMemory + 2));
                sprintf(mnem_buf, "%s %s$%04X%s", mnemo, br1, offset, br2);
                extrabytes = 2;
                break;

            default:
                sprintf(code_buf, "%04X: %02X %02X", pc, code, postbyte);
                sprintf(mnem_buf, "%s ????", mnemo);
        }
    }

    return bytes + extrabytes;
} // D_Indexed


inline Byte Da6809::D_Extended(const char *mnemo, Word pc, Byte bytes,
                               const Byte *pMemory, DWord * /*pAddr*/)
{
    Byte code;
    Word offset;
    const char *label;

    code = *pMemory;
    offset = (*(pMemory + 1) << 8) | *(pMemory + 2);
    sprintf(code_buf, "%04X: %02X %02X %02X", pc, code,
            *(pMemory + 1), *(pMemory + 2));
    label = FlexLabel(offset);

    if (label == nullptr)
    {
        sprintf(mnem_buf, "%s $%04X", mnemo, offset);
    }
    else
    {
        sprintf(mnem_buf, "%s %s ; $%04X", mnemo, label, offset);
    }

    return bytes;
}

inline Byte Da6809::D_Relative(const char *mnemo, Word pc, Byte bytes,
                               const Byte *pMemory, DWord * /*pAddr*/)
{
    Byte code;
    Word offset, disp;
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

    sprintf(code_buf, "%04X: %02X %02X", pc, code, offset);
    label = FlexLabel(disp);

    if (label == nullptr)
    {
        sprintf(mnem_buf, "%s $%04X", mnemo, disp);
    }
    else
    {
        sprintf(mnem_buf, "%s %s ; $%04X", mnemo, label, disp);
    }

    return bytes;
}

inline Byte Da6809::D_RelativeL(
    const char *mnemo, Word pc, Byte bytes, const Byte *pMemory, DWord *pAddr)
{
    Byte code;
    Word offset;
    const char *label;

    code = *pMemory;
    offset = (*(pMemory + 1) << 8) | *(pMemory + 2);

    if (offset < 32767)
    {
        *pAddr = pc + 3 + offset;
    }
    else
    {
        *pAddr = pc + 3 - (65536 - offset);
    }

    sprintf(code_buf, "%04X: %02X %02X %02X", pc, code,
            *(pMemory + 1), *(pMemory + 2));
    label = FlexLabel(*pAddr);

    if (label == nullptr)
    {
        sprintf(mnem_buf, "%s $%04X", mnemo, (Word)*pAddr);
    }
    else
    {
        sprintf(mnem_buf, "%s %s ; $%04X", mnemo, label, (Word)*pAddr);
    }

    return bytes;
}

inline Byte Da6809::D_Register0(const char *mnemo, Word pc, Byte bytes,
                                const Byte *pMemory)
{
    Byte code;
    Word postbyte;
    code = *pMemory;
    postbyte = *(pMemory + 1);

    sprintf(code_buf, "%04X: %02X %02X", pc, code, postbyte);
    sprintf(mnem_buf, "%s %s,%s", mnemo, InterRegister(postbyte >> 4),
            InterRegister(postbyte & 0x0f));

    return bytes;
}

inline Byte Da6809::D_Register1(const char *mnemo, Word pc, Byte bytes,
                                const Byte *pMemory)
{
    Byte code, postbyte;
    Byte i, comma;
    size_t index;

    code      = *pMemory;
    postbyte  = *(pMemory + 1);

    comma = 0;
    sprintf(code_buf, "%04X: %02X %02X", pc, code, postbyte);
    sprintf(mnem_buf, "%s ", mnemo);
    index = strlen(mnemo);

    for (i = 0; i < 8; i++)
    {
        if (postbyte & (1 << i))
        {
            sprintf((char *)&mnem_buf[index], "%s%s",
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
    Byte code, postbyte;
    Byte i, comma;
    size_t index;

    code      = *pMemory;
    postbyte  = *(pMemory + 1);

    comma = 0;
    sprintf(code_buf, "%04X: %02X %02X", pc, code, postbyte);
    sprintf(mnem_buf, "%s ", mnemo);
    index = strlen(mnemo);

    for (i = 0; i < 8; i++)
    {
        if (postbyte & (1 << i))
        {
            sprintf((char *)&mnem_buf[index], "%s%s",
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

    *pCode = code_buf;
    *pMnemonic = mnem_buf;
    *pFlags = InstFlg::NONE;
    code = *pMemory;

    switch (code)
    {
        case 0x01:
            if (use_undocumented)
            {
                return D_Direct("neg", pc, 2, pMemory);
            }

            *pFlags |= InstFlg::Illegal;
            return D_Illegal("", pc, 1, pMemory);

        case 0x00:
            return D_Direct("NEG  ", pc, 2, pMemory);

        case 0x02:
            if (use_undocumented)
            {
                return D_Direct("negcom", pc, 2, pMemory);
            }

            *pFlags |= InstFlg::Illegal;
            return D_Illegal("", pc, 1, pMemory);

        case 0x03:
            return D_Direct("COM  ", pc, 2, pMemory);

        case 0x04:
            return D_Direct("LSR  ", pc, 2, pMemory);

        case 0x05:
            if (use_undocumented)
            {
                return D_Direct("lsr  ", pc, 2, pMemory);
            }

            *pFlags |= InstFlg::Illegal;
            return D_Illegal("", pc, 1, pMemory);

        case 0x06:
            return D_Direct("ROR  ", pc, 2, pMemory);

        case 0x07:
            return D_Direct("ASR  ", pc, 2, pMemory);

        case 0x08:
            return D_Direct("LSR  ", pc, 2, pMemory);

        case 0x09:
            return D_Direct("ROR  ", pc, 2, pMemory);

        case 0x0a:
            return D_Direct("DEC  ", pc, 2, pMemory);

        case 0x0b:
            if (use_undocumented)
            {
                return D_Direct("dec  ", pc, 2, pMemory);
            }

            *pFlags |= InstFlg::Illegal;
            return D_Illegal("", pc, 1, pMemory);

        case 0x0c:
            return D_Direct("INC  ", pc, 2, pMemory);

        case 0x0d:
            return D_Direct("TST  ", pc, 2, pMemory);

        case 0x0e:
            *pFlags |= InstFlg::Jump;
            return D_Direct("JMP  ", pc, 2, pMemory);

        case 0x0f:
            return D_Direct("CLR  ", pc, 2, pMemory);

        case 0x10:
            return D_Page10(pFlags, pc, pMemory, pAddr);

        case 0x11:
            return D_Page11(pFlags, pc, pMemory, pAddr);

        case 0x12:
            *pFlags |= InstFlg::Noop;
            return D_Inherent("NOP  ", pc, 1, pMemory);

        case 0x13:
            *pFlags |= InstFlg::Jump;
            return D_Inherent("SYNC ", pc, 1, pMemory);

        // 0x14, 0x15 is illegal
        case 0x16:
            *pFlags |= InstFlg::Jump | InstFlg::JumpAddr;
            return D_RelativeL("LBRA ", pc, 3, pMemory, pAddr);

        case 0x17:
            *pFlags |= InstFlg::Sub | InstFlg::LabelAddr;
            return D_RelativeL("LBSR ", pc, 3, pMemory, pAddr);

        // 0x18 is illegal
        case 0x19:
            return D_Inherent("DAA  ", pc, 1, pMemory);

        case 0x1a:
            return D_Immediat("ORCC ", pc, 2, pMemory);

        // 0x1b is illegal
        case 0x1c:
            return D_Immediat("ANDCC", pc, 2, pMemory);

        case 0x1d:
            return D_Inherent("SEX  ", pc, 1, pMemory);

        case 0x1e:
            return D_Register0("EXG  ", pc, 2, pMemory);

        case 0x1f:
            return D_Register0("TFR  ", pc, 2, pMemory);

        case 0x20:
            *pFlags |= InstFlg::Jump | InstFlg::JumpAddr;
            return D_Relative("BRA  ", pc, 2, pMemory, pAddr);

        case 0x21:
            *pFlags |= InstFlg::JumpAddr;
            return D_Relative("BRN  ", pc, 2, pMemory, pAddr);

        case 0x22:
            *pFlags |= InstFlg::JumpAddr;
            return D_Relative("BHI  ", pc, 2, pMemory, pAddr);

        case 0x23:
            *pFlags |= InstFlg::JumpAddr;
            return D_Relative("BLS  ", pc, 2, pMemory, pAddr);

        case 0x24:
            *pFlags |= InstFlg::JumpAddr;
            return D_Relative("BCC  ", pc, 2, pMemory, pAddr);

        case 0x25:
            *pFlags |= InstFlg::JumpAddr;
            return D_Relative("BCS  ", pc, 2, pMemory, pAddr);

        case 0x26:
            *pFlags |= InstFlg::JumpAddr;
            return D_Relative("BNE  ", pc, 2, pMemory, pAddr);

        case 0x27:
            *pFlags |= InstFlg::JumpAddr;
            return D_Relative("BEQ  ", pc, 2, pMemory, pAddr);

        case 0x28:
            *pFlags |= InstFlg::JumpAddr;
            return D_Relative("BVC  ", pc, 2, pMemory, pAddr);

        case 0x29:
            *pFlags |= InstFlg::JumpAddr;
            return D_Relative("BVS  ", pc, 2, pMemory, pAddr);

        case 0x2a:
            *pFlags |= InstFlg::JumpAddr;
            return D_Relative("BPL  ", pc, 2, pMemory, pAddr);

        case 0x2b:
            *pFlags |= InstFlg::JumpAddr;
            return D_Relative("BMI  ", pc, 2, pMemory, pAddr);

        case 0x2c:
            *pFlags |= InstFlg::JumpAddr;
            return D_Relative("BGE  ", pc, 2, pMemory, pAddr);

        case 0x2d:
            *pFlags |= InstFlg::JumpAddr;
            return D_Relative("BLT  ", pc, 2, pMemory, pAddr);

        case 0x2e:
            *pFlags |= InstFlg::JumpAddr;
            return D_Relative("BGT  ", pc, 2, pMemory, pAddr);

        case 0x2f:
            *pFlags |= InstFlg::JumpAddr;
            return D_Relative("BLE  ", pc, 2, pMemory, pAddr);

        case 0x30:
            return D_Indexed("LEAX ", pc, 2, pMemory);

        case 0x31:
            return D_Indexed("LEAY ", pc, 2, pMemory);

        case 0x32:
            return D_Indexed("LEAS ", pc, 2, pMemory);

        case 0x33:
            return D_Indexed("LEAU ", pc, 2, pMemory);

        case 0x34:
            return D_Register1("PSHS ", pc, 2, pMemory);

        case 0x35:
            return D_Register1("PULS ", pc, 2, pMemory);

        case 0x36:
            return D_Register2("PSHU ", pc, 2, pMemory);

        case 0x37:
            return D_Register2("PULU ", pc, 2, pMemory);

        // 0x38 is illegal
        case 0x39:
            *pFlags |= InstFlg::Jump;
            return D_Inherent("RTS  ", pc, 1, pMemory);

        case 0x3a:
            return D_Inherent("ABX  ", pc, 1, pMemory);

        case 0x3b:
            *pFlags |= InstFlg::Jump;
            return D_Inherent("RTI  ", pc, 1, pMemory);

        case 0x3c:
            *pFlags |= InstFlg::Jump;
            return D_Immediat("CWAI ", pc, 2, pMemory);

        case 0x3d:
            return D_Inherent("MUL  ", pc, 1, pMemory);

        case 0x3e:
            if (use_undocumented)
            {
                return D_Direct("reset", pc, 1, pMemory);
            }

            *pFlags |= InstFlg::Illegal;
            return D_Illegal("", pc, 1, pMemory);

        case 0x3f:
            *pFlags |= InstFlg::Illegal;
            return D_Illegal("SWI  ", pc, 1, pMemory);

        case 0x40:
            return D_Inherent("NEGA ", pc, 1, pMemory);

        case 0x41:
            if (use_undocumented)
            {
                return D_Direct("nega ", pc, 1, pMemory);
            }

            *pFlags |= InstFlg::Illegal;
            return D_Illegal("", pc, 1, pMemory);

        case 0x43:
            return D_Inherent("COMA ", pc, 1, pMemory);

        case 0x44:
            return D_Inherent("LSRA ", pc, 1, pMemory);

        case 0x45:
            if (use_undocumented)
            {
                return D_Direct("lsra ", pc, 1, pMemory);
            }

            *pFlags |= InstFlg::Illegal;
            return D_Illegal("", pc, 1, pMemory);

        case 0x46:
            return D_Inherent("RORA ", pc, 1, pMemory);

        case 0x47:
            return D_Inherent("ASRA ", pc, 1, pMemory);

        case 0x48:
            return D_Inherent("LSLA ", pc, 1, pMemory);

        case 0x49:
            return D_Inherent("ROLA ", pc, 1, pMemory);

        case 0x4a:
            return D_Inherent("DECA ", pc, 1, pMemory);

        case 0x4b:
            if (use_undocumented)
            {
                return D_Direct("deca ", pc, 1, pMemory);
            }

            *pFlags |= InstFlg::Illegal;
            return D_Illegal("", pc, 1, pMemory);

        case 0x4c:
            return D_Inherent("INCA ", pc, 1, pMemory);

        case 0x4d:
            return D_Inherent("TSTA ", pc, 1, pMemory);

        case 0x4e:
            if (use_undocumented)
            {
                return D_Direct("clra ", pc, 1, pMemory);
            }

            *pFlags |= InstFlg::Illegal;
            return D_Illegal("", pc, 1, pMemory);

        case 0x4f:
            return D_Inherent("CLRA ", pc, 1, pMemory);

        case 0x50:
            return D_Inherent("NEGB ", pc, 1, pMemory);

        case 0x51:
            if (use_undocumented)
            {
                return D_Direct("negb ", pc, 1, pMemory);
            }

            *pFlags |= InstFlg::Illegal;
            return D_Illegal("", pc, 1, pMemory);

        case 0x53:
            return D_Inherent("COMB ", pc, 1, pMemory);

        case 0x54:
            return D_Inherent("LSRB ", pc, 1, pMemory);

        case 0x55:
            if (use_undocumented)
            {
                return D_Direct("lsrb ", pc, 1, pMemory);
            }

            *pFlags |= InstFlg::Illegal;
            return D_Illegal("", pc, 1, pMemory);

        case 0x56:
            return D_Inherent("RORB ", pc, 1, pMemory);

        case 0x57:
            return D_Inherent("ASRB ", pc, 1, pMemory);

        case 0x58:
            return D_Inherent("LSLB ", pc, 1, pMemory);

        case 0x59:
            return D_Inherent("ROLB ", pc, 1, pMemory);

        case 0x5a:
            return D_Inherent("DECB ", pc, 1, pMemory);

        case 0x5b:
            if (use_undocumented)
            {
                return D_Direct("decb ", pc, 1, pMemory);
            }

            *pFlags |= InstFlg::Illegal;
            return D_Illegal("", pc, 1, pMemory);

        case 0x5c:
            return D_Inherent("INCB ", pc, 1, pMemory);

        case 0x5d:
            return D_Inherent("TSTB ", pc, 1, pMemory);

        case 0x5e:
            if (use_undocumented)
            {
                return D_Direct("clrb ", pc, 1, pMemory);
            }

            *pFlags |= InstFlg::Illegal;
            return D_Illegal("", pc, 1, pMemory);

        case 0x5f:
            return D_Inherent("CLRB ", pc, 1, pMemory);

        case 0x60:
            return D_Indexed("NEG  ", pc, 2, pMemory);

        case 0x61:
            if (use_undocumented)
            {
                return D_Indexed("neg  ", pc, 2, pMemory);
            }

            *pFlags |= InstFlg::Illegal;
            return D_Illegal("", pc, 1, pMemory);

        case 0x63:
            return D_Indexed("COM  ", pc, 2, pMemory);

        case 0x64:
            return D_Indexed("LSR  ", pc, 2, pMemory);

        case 0x65:
            if (use_undocumented)
            {
                return D_Indexed("lsr  ", pc, 2, pMemory);
            }

            *pFlags |= InstFlg::Illegal;
            return D_Illegal("", pc, 1, pMemory);

        case 0x66:
            return D_Indexed("ROR  ", pc, 2, pMemory);

        case 0x67:
            return D_Indexed("ASR  ", pc, 2, pMemory);

        case 0x68:
            return D_Indexed("LSL  ", pc, 2, pMemory);

        case 0x69:
            return D_Indexed("ROL  ", pc, 2, pMemory);

        case 0x6a:
            return D_Indexed("DEC  ", pc, 2, pMemory);

        case 0x6b:
            if (use_undocumented)
            {
                return D_Indexed("dec  ", pc, 2, pMemory);
            }

            *pFlags |= InstFlg::Illegal;
            return D_Illegal("", pc, 1, pMemory);

        case 0x6c:
            return D_Indexed("INC  ", pc, 2, pMemory);

        case 0x6d:
            return D_Indexed("TST  ", pc, 2, pMemory);

        case 0x6e:
            *pFlags |= InstFlg::Jump | InstFlg::ComputedGoto;
            return D_Indexed("JMP  ", pc, 2, pMemory);

        case 0x6f:
            return D_Indexed("CLR  ", pc, 2, pMemory);

        case 0x70:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("NEG  ", pc, 3, pMemory, pAddr);

        case 0x71:
            if (use_undocumented)
            {
                *pFlags |= InstFlg::LabelAddr;
                return D_Extended("neg  ", pc, 3, pMemory, pAddr);
            };

            *pFlags |= InstFlg::Illegal;

            return D_Illegal("", pc, 1, pMemory);

        case 0x73:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("COM  ", pc, 3, pMemory, pAddr);

        case 0x74:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("LSR  ", pc, 3, pMemory, pAddr);

        case 0x75:
            if (use_undocumented)
            {
                *pFlags |= InstFlg::LabelAddr;
                return D_Extended("lsr  ", pc, 3, pMemory, pAddr);
            };

            *pFlags |= InstFlg::Illegal;

            return D_Illegal("", pc, 1, pMemory);

        case 0x76:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("ROR  ", pc, 3, pMemory, pAddr);

        case 0x77:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("ASR  ", pc, 3, pMemory, pAddr);

        case 0x78:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("LSL  ", pc, 3, pMemory, pAddr);

        case 0x79:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("ROL  ", pc, 3, pMemory, pAddr);

        case 0x7a:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("DEC  ", pc, 3, pMemory, pAddr);

        case 0x7b:
            if (use_undocumented)
            {
                *pFlags |= InstFlg::LabelAddr;
                return D_Extended("dec  ", pc, 3, pMemory, pAddr);
            };

            *pFlags |= InstFlg::Illegal;

            return D_Illegal("", pc, 1, pMemory);

        case 0x7c:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("INC  ", pc, 3, pMemory, pAddr);

        case 0x7d:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("TST  ", pc, 3, pMemory, pAddr);

        case 0x7e:
            *pFlags |= InstFlg::Jump;
            return D_Extended("JMP  ", pc, 3, pMemory, pAddr);

        case 0x7f:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("CLR  ", pc, 3, pMemory, pAddr);

        case 0x80:
            return D_Immediat("SUBA ", pc, 2, pMemory);

        case 0x81:
            return D_Immediat("CMPA ", pc, 2, pMemory);

        case 0x82:
            return D_Immediat("SBCA ", pc, 2, pMemory);

        case 0x83:
            return D_ImmediatL("SUBD ", pc, 3, pMemory, pAddr);

        case 0x84:
            return D_Immediat("ANDA ", pc, 2, pMemory);

        case 0x85:
            return D_Immediat("BITA ", pc, 2, pMemory);

        case 0x86:
            return D_Immediat("LDA  ", pc, 2, pMemory);

        // 0x87 is illegal
        case 0x88:
            return D_Immediat("EORA ", pc, 2, pMemory);

        case 0x89:
            return D_Immediat("ADCA ", pc, 2, pMemory);

        case 0x8a:
            return D_Immediat("ORA  ", pc, 2, pMemory);

        case 0x8b:
            return D_Immediat("ADDA ", pc, 2, pMemory);

        case 0x8c:
            return D_ImmediatL("CMPX ", pc, 3, pMemory, pAddr);

        case 0x8d:
            *pFlags |= InstFlg::Sub;
            return D_Relative("BSR  ", pc, 2, pMemory, pAddr);

        case 0x8e:
            return D_ImmediatL("LDX  ", pc, 2, pMemory, pAddr);

        // 0x8f is illegal

        case 0x90:
            return D_Direct("SUBA ", pc, 2, pMemory);

        case 0x91:
            return D_Direct("CMPA ", pc, 2, pMemory);

        case 0x92:
            return D_Direct("SBCA ", pc, 2, pMemory);

        case 0x93:
            return D_Direct("SUBD ", pc, 2, pMemory);

        case 0x94:
            return D_Direct("ANDA ", pc, 2, pMemory);

        case 0x95:
            return D_Direct("BITA ", pc, 2, pMemory);

        case 0x96:
            return D_Direct("LDA  ", pc, 2, pMemory);

        case 0x97:
            return D_Direct("STA  ", pc, 2, pMemory);

        case 0x98:
            return D_Direct("EORA ", pc, 2, pMemory);

        case 0x99:
            return D_Direct("ADCA ", pc, 2, pMemory);

        case 0x9a:
            return D_Direct("ORA  ", pc, 2, pMemory);

        case 0x9b:
            return D_Direct("ADDA ", pc, 2, pMemory);

        case 0x9c:
            return D_Direct("CMPX ", pc, 2, pMemory);

        case 0x9d:
            *pFlags |= InstFlg::Sub;
            return D_Direct("JSR  ", pc, 2, pMemory);

        case 0x9e:
            return D_Direct("LDX  ", pc, 2, pMemory);

        case 0x9f:
            return D_Direct("STX  ", pc, 2, pMemory);

        case 0xa0:
            return D_Indexed("SUBA ", pc, 2, pMemory);

        case 0xa1:
            return D_Indexed("CMPA ", pc, 2, pMemory);

        case 0xa2:
            return D_Indexed("SBCA ", pc, 2, pMemory);

        case 0xa3:
            return D_Indexed("SUBD ", pc, 2, pMemory);

        case 0xa4:
            return D_Indexed("ANDA ", pc, 2, pMemory);

        case 0xa5:
            return D_Indexed("BITA ", pc, 2, pMemory);

        case 0xa6:
            return D_Indexed("LDA  ", pc, 2, pMemory);

        case 0xa7:
            return D_Indexed("STA  ", pc, 2, pMemory);

        case 0xa8:
            return D_Indexed("EORA ", pc, 2, pMemory);

        case 0xa9:
            return D_Indexed("ADCA ", pc, 2, pMemory);

        case 0xaa:
            return D_Indexed("ORA  ", pc, 2, pMemory);

        case 0xab:
            return D_Indexed("ADDA ", pc, 2, pMemory);

        case 0xac:
            return D_Indexed("CMPX ", pc, 2, pMemory);

        case 0xad:
            *pFlags |= InstFlg::Sub | InstFlg::ComputedGoto;
            return D_Indexed("JSR  ", pc, 2, pMemory);

        case 0xae:
            return D_Indexed("LDX  ", pc, 2, pMemory);

        case 0xaf:
            return D_Indexed("STX  ", pc, 2, pMemory);

        case 0xb0:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("SUBA ", pc, 3, pMemory, pAddr);

        case 0xb1:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("CMPA ", pc, 3, pMemory, pAddr);

        case 0xb2:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("SBCA ", pc, 3, pMemory, pAddr);

        case 0xb3:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("SUBD ", pc, 3, pMemory, pAddr);

        case 0xb4:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("ANDA ", pc, 3, pMemory, pAddr);

        case 0xb5:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("BITA ", pc, 3, pMemory, pAddr);

        case 0xb6:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("LDA  ", pc, 3, pMemory, pAddr);

        case 0xb7:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("STA  ", pc, 3, pMemory, pAddr);

        case 0xb8:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("EORA ", pc, 3, pMemory, pAddr);

        case 0xb9:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("ADCA ", pc, 3, pMemory, pAddr);

        case 0xba:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("ORA  ", pc, 3, pMemory, pAddr);

        case 0xbb:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("ADDA ", pc, 3, pMemory, pAddr);

        case 0xbc:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("CMPX ", pc, 3, pMemory, pAddr);

        case 0xbd:
            *pFlags |= InstFlg::Sub | InstFlg::JumpAddr;
            return D_Extended("JSR  ", pc, 3, pMemory, pAddr);

        case 0xbe:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("LDX  ", pc, 3, pMemory, pAddr);

        case 0xbf:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("STX  ", pc, 3, pMemory, pAddr);

        case 0xc0:
            return D_Immediat("SUBB ", pc, 2, pMemory);

        case 0xc1:
            return D_Immediat("CMPB ", pc, 2, pMemory);

        case 0xc2:
            return D_Immediat("SBCB ", pc, 2, pMemory);

        case 0xc3:
            *pFlags |= InstFlg::LabelAddr;
            return D_ImmediatL("ADDD ", pc, 3, pMemory, pAddr);

        case 0xc4:
            return D_Immediat("ANDB ", pc, 2, pMemory);

        case 0xc5:
            return D_Immediat("BITB ", pc, 2, pMemory);

        case 0xc6:
            return D_Immediat("LDB  ", pc, 2, pMemory);

        // 0xc7 is illegal
        case 0xc8:
            return D_Immediat("EORB ", pc, 2, pMemory);

        case 0xc9:
            return D_Immediat("ADCB ", pc, 2, pMemory);

        case 0xca:
            return D_Immediat("ORB  ", pc, 2, pMemory);

        case 0xcb:
            return D_Immediat("ADDB ", pc, 2, pMemory);

        case 0xcc:
            *pFlags |= InstFlg::LabelAddr;
            return D_ImmediatL("LDD  ", pc, 3, pMemory, pAddr);

        // 0xcd is illegal
        case 0xce:
            *pFlags |= InstFlg::LabelAddr;
            return D_ImmediatL("LDU  ", pc, 3, pMemory, pAddr);

        // 0xcf is illegal

        case 0xd0:
            return D_Direct("SUBB ", pc, 2, pMemory);

        case 0xd1:
            return D_Direct("CMPB ", pc, 2, pMemory);

        case 0xd2:
            return D_Direct("SBCB ", pc, 2, pMemory);

        case 0xd3:
            return D_Direct("ADDD ", pc, 2, pMemory);

        case 0xd4:
            return D_Direct("ANDB ", pc, 2, pMemory);

        case 0xd5:
            return D_Direct("BITB ", pc, 2, pMemory);

        case 0xd6:
            return D_Direct("LDB  ", pc, 2, pMemory);

        case 0xd7:
            return D_Direct("STB  ", pc, 2, pMemory);

        case 0xd8:
            return D_Direct("EORB ", pc, 2, pMemory);

        case 0xd9:
            return D_Direct("ADCB ", pc, 2, pMemory);

        case 0xda:
            return D_Direct("ORB  ", pc, 2, pMemory);

        case 0xdb:
            return D_Direct("ADDB ", pc, 2, pMemory);

        case 0xdc:
            return D_Direct("LDD  ", pc, 2, pMemory);

        case 0xdd:
            return D_Direct("STD  ", pc, 2, pMemory);

        case 0xde:
            return D_Direct("LDU  ", pc, 2, pMemory);

        case 0xdf:
            return D_Direct("STU  ", pc, 2, pMemory);

        case 0xe0:
            return D_Indexed("SUBB ", pc, 2, pMemory);

        case 0xe1:
            return D_Indexed("CMPB ", pc, 2, pMemory);

        case 0xe2:
            return D_Indexed("SBCB ", pc, 2, pMemory);

        case 0xe3:
            return D_Indexed("ADDD ", pc, 2, pMemory);

        case 0xe4:
            return D_Indexed("ANDB ", pc, 2, pMemory);

        case 0xe5:
            return D_Indexed("BITB ", pc, 2, pMemory);

        case 0xe6:
            return D_Indexed("LDB  ", pc, 2, pMemory);

        case 0xe7:
            return D_Indexed("STB  ", pc, 2, pMemory);

        case 0xe8:
            return D_Indexed("EORB ", pc, 2, pMemory);

        case 0xe9:
            return D_Indexed("ADCB ", pc, 2, pMemory);

        case 0xea:
            return D_Indexed("ORB  ", pc, 2, pMemory);

        case 0xeb:
            return D_Indexed("ADDB ", pc, 2, pMemory);

        case 0xec:
            return D_Indexed("LDD  ", pc, 2, pMemory);

        case 0xed:
            return D_Indexed("STD  ", pc, 2, pMemory);

        case 0xee:
            return D_Indexed("LDU  ", pc, 2, pMemory);

        case 0xef:
            return D_Indexed("STU  ", pc, 2, pMemory);

        case 0xf0:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("SUBB ", pc, 3, pMemory, pAddr);

        case 0xf1:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("CMPB ", pc, 3, pMemory, pAddr);

        case 0xf2:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("SBCA ", pc, 3, pMemory, pAddr);

        case 0xf3:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("ADDD ", pc, 3, pMemory, pAddr);

        case 0xf4:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("ANDB ", pc, 3, pMemory, pAddr);

        case 0xf5:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("BITB ", pc, 3, pMemory, pAddr);

        case 0xf6:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("LDB  ", pc, 3, pMemory, pAddr);

        case 0xf7:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("STB  ", pc, 3, pMemory, pAddr);

        case 0xf8:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("EORB ", pc, 3, pMemory, pAddr);

        case 0xf9:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("ADCB ", pc, 3, pMemory, pAddr);

        case 0xfa:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("ORB  ", pc, 3, pMemory, pAddr);

        case 0xfb:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("ADDB ", pc, 3, pMemory, pAddr);

        case 0xfc:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("LDD  ", pc, 3, pMemory, pAddr);

        case 0xfd:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("STD  ", pc, 3, pMemory, pAddr);

        case 0xfe:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("LDU  ", pc, 3, pMemory, pAddr);

        case 0xff:
            *pFlags |= InstFlg::LabelAddr;
            return D_Extended("STU  ", pc, 3, pMemory, pAddr);

        default:
            *pFlags |= InstFlg::Illegal;
            return D_Illegal("", pc, 1, pMemory);
    }  // switch
} // disassemble

