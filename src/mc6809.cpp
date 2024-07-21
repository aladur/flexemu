/*
    mc6809.cpp

    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2024  W. Schwotzer

    This file is based on usim-0.91 which is
    Copyright (C) 1994 by R. B. Bellis
*/


#include "misc1.h"

#include "mc6809.h"
#include "da6809.h"
#include "inout.h"
#include <cstring>

#ifdef FASTFLEX
    #define PC ipcreg
#else
    #define PC pc
#endif

Mc6809::Mc6809(Memory &p_memory) : events(Event::NONE),
#ifndef FASTFLEX
    a(acc.byte.a), b(acc.byte.b), d(acc.d), dp(dpreg.byte.h),
#endif
     memory(p_memory)
{
#ifndef FASTFLEX
    dpreg.byte.l = 0;
#endif
    std::memset(&interrupt_status, 0, sizeof(interrupt_status));
    init();
}

void Mc6809::set_disassembler(Da6809 *p_disassembler)
{
    disassembler = p_disassembler;

    if (disassembler != nullptr)
    {
        disassembler->set_use_undocumented(use_undocumented);
    }
}

void Mc6809::set_use_undocumented(bool value)
{
    use_undocumented = value;

    if (disassembler != nullptr)
    {
        disassembler->set_use_undocumented(use_undocumented);
    }
}

void Mc6809::init()
{
    int i;

    events = Event::NONE;

    // all breakpoints are reset
    for (i = 0; i < static_cast<int>(bp.size()); ++i)
    {
        bp[i].reset();
    }

    init_indexed_cycles();
    init_psh_pul_cycles();
}

void Mc6809::init_indexed_cycles()
{
    Word i;

    for (i = 0; i < 128; i++)
    {
        indexed_cycles[i] = 1;
    }

    for (i = 128; i < 256; i++)
    {
        switch (static_cast<Byte>(i & 0x1FU))
        {
            case 0x05:
            case 0x06:
            case 0x08:
            case 0x0c:
                indexed_cycles[i] = 1;
                break;

            case 0x00:
            case 0x02:
                indexed_cycles[i] = 2;
                break;

            case 0x01:
            case 0x03:
            case 0x14:
                indexed_cycles[i] = 3;
                break;

            case 0x09:
            case 0x0b:
            case 0x18:
            case 0x15:
            case 0x16:
            case 0x1c:
                indexed_cycles[i] = 4;
                break;

            case 0x0d:
            case 0x1f:
                indexed_cycles[i] = 5;
                break;

            case 0x11:
            case 0x13:
                indexed_cycles[i] = 6;
                break;

            case 0x19:
            case 0x1b:
                indexed_cycles[i] = 7;
                break;

            case 0x1d:
                indexed_cycles[i] = 8;
                break;

            default:
                indexed_cycles[i] = 0;
                break;
        }
    }
}

void Mc6809::init_psh_pul_cycles()
{
    Word i;
    Byte cycle_count;

    for (i = 0; i < 256; i++)
    {
        cycle_count = 5;

        if (i & 0x01U)
        {
            cycle_count++;
        }

        if (i & 0x02U)
        {
            cycle_count++;
        }

        if (i & 0x04U)
        {
            cycle_count++;
        }

        if (i & 0x08U)
        {
            cycle_count++;
        }

        if (i & 0x10U)
        {
            cycle_count += 2;
        }

        if (i & 0x20U)
        {
            cycle_count += 2;
        }

        if (i & 0x40U)
        {
            cycle_count += 2;
        }

        if (i & 0x80U)
        {
            cycle_count += 2;
        }

        psh_pul_cycles[i] = cycle_count;
    }
}

void Mc6809::set_nmi()
{
    events |= Event::Nmi;
}

void Mc6809::set_firq()
{
    events |= Event::Firq;
}

void Mc6809::set_irq()
{
    events |= Event::Irq;
}

#ifndef FASTFLEX
cycles_t Mc6809::psh(Byte what, Word &stack, Word &reg_s_or_u)
{
    switch (static_cast<Byte>(what & 0xF0U))
    {
        case 0xf0:
            stack -= 2;
            memory.write_word(stack, pc);
            FALLTHROUGH;

        case 0x70:
            stack -= 2;
            memory.write_word(stack, reg_s_or_u);
            FALLTHROUGH;

        case 0x30:
            stack -= 2;
            memory.write_word(stack, y);
            FALLTHROUGH;

        case 0x10:
            stack -= 2;
            memory.write_word(stack, x);
            FALLTHROUGH;

        case 0x00:
            break;

        case 0xe0:
            stack -= 2;
            memory.write_word(stack, pc);
            FALLTHROUGH;

        case 0x60:
            stack -= 2;
            memory.write_word(stack, reg_s_or_u);
            FALLTHROUGH;

        case 0x20:
            stack -= 2;
            memory.write_word(stack, y);
            break;

        case 0xd0:
            stack -= 2;
            memory.write_word(stack, pc);
            FALLTHROUGH;

        case 0x50:
            stack -= 2;
            memory.write_word(stack, reg_s_or_u);
            stack -= 2;
            memory.write_word(stack, x);
            break;

        case 0xc0:
            stack -= 2;
            memory.write_word(stack, pc);
            FALLTHROUGH;

        case 0x40:
            stack -= 2;
            memory.write_word(stack, reg_s_or_u);
            break;

        case 0xb0:
            stack -= 2;
            memory.write_word(stack, pc);
            stack -= 2;
            memory.write_word(stack, y);
            stack -= 2;
            memory.write_word(stack, x);
            break;

        case 0xa0:
            stack -= 2;
            memory.write_word(stack, pc);
            stack -= 2;
            memory.write_word(stack, y);
            break;

        case 0x90:
            stack -= 2;
            memory.write_word(stack, pc);
            stack -= 2;
            memory.write_word(stack, x);
            break;

        case 0x80:
            stack -= 2;
            memory.write_word(stack, pc);
            break;
    }

    switch (static_cast<Byte>(what & 0x0FU))
    {
        case 0x0f:
            memory.write_byte(--stack, dp);
            FALLTHROUGH;

        case 0x07:
            memory.write_byte(--stack, b);
            FALLTHROUGH;

        case 0x03:
            memory.write_byte(--stack, a);
            FALLTHROUGH;

        case 0x01:
            memory.write_byte(--stack, cc.all);
            FALLTHROUGH;

        case 0x00:
            break;

        case 0x0e:
            memory.write_byte(--stack, dp);
            FALLTHROUGH;

        case 0x06:
            memory.write_byte(--stack, b);
            FALLTHROUGH;

        case 0x02:
            memory.write_byte(--stack, a);
            break;

        case 0x0d:
            memory.write_byte(--stack, dp);
            FALLTHROUGH;

        case 0x05:
            memory.write_byte(--stack, b);
            memory.write_byte(--stack, cc.all);
            break;

        case 0x0c:
            memory.write_byte(--stack, dp);
            FALLTHROUGH;

        case 0x04:
            memory.write_byte(--stack, b);
            break;

        case 0x0b:
            memory.write_byte(--stack, dp);
            memory.write_byte(--stack, a);
            memory.write_byte(--stack, cc.all);
            break;

        case 0x09:
            memory.write_byte(--stack, dp);
            memory.write_byte(--stack, cc.all);
            break;

        case 0x0a:
            memory.write_byte(--stack, dp);
            memory.write_byte(--stack, a);
            break;

        case 0x08:
            memory.write_byte(--stack, dp);
            break;
    }

    return psh_pul_cycles[what];
}

cycles_t Mc6809::pul(Byte what, Word &stack, Word &reg_s_or_u)
{
    switch (static_cast<Byte>(what & 0x0FU))
    {
        case 0x0f:
            cc.all = memory.read_byte(stack++);
            FALLTHROUGH;

        case 0x0e:
            a = memory.read_byte(stack++);
            FALLTHROUGH;

        case 0x0c:
            b = memory.read_byte(stack++);
            FALLTHROUGH;

        case 0x08:
            dp = memory.read_byte(stack++);
            FALLTHROUGH;

        case 0x00:
            break;

        case 0x07:
            cc.all = memory.read_byte(stack++);
            FALLTHROUGH;

        case 0x06:
            a = memory.read_byte(stack++);
            FALLTHROUGH;

        case 0x04:
            b = memory.read_byte(stack++);
            break;

        case 0x0b:
            cc.all = memory.read_byte(stack++);
            FALLTHROUGH;

        case 0x0a:
            a = memory.read_byte(stack++);
            dp = memory.read_byte(stack++);
            break;

        case 0x03:
            cc.all = memory.read_byte(stack++);
            FALLTHROUGH;

        case 0x02:
            a = memory.read_byte(stack++);
            break;

        case 0x0d:
            cc.all = memory.read_byte(stack++);
            b = memory.read_byte(stack++);
            dp = memory.read_byte(stack++);
            break;

        case 0x09:
            cc.all = memory.read_byte(stack++);
            dp = memory.read_byte(stack++);
            break;

        case 0x05:
            cc.all = memory.read_byte(stack++);
            b = memory.read_byte(stack++);
            break;

        case 0x01:
            cc.all = memory.read_byte(stack++);
            break;
    }

    switch (static_cast<Byte>(what & 0xF0U))
    {
        case 0xf0:
            x = memory.read_word(stack);
            stack += 2;
            FALLTHROUGH;

        case 0xe0:
            y = memory.read_word(stack);
            stack += 2;
            FALLTHROUGH;

        case 0xc0:
            reg_s_or_u = memory.read_word(stack);
            stack += 2;
            FALLTHROUGH;

        case 0x80:
            pc = memory.read_word(stack);
            stack += 2;
            FALLTHROUGH;

        case 0x00:
            break;

        case 0x70:
            x = memory.read_word(stack);
            stack += 2;
            FALLTHROUGH;

        case 0x60:
            y = memory.read_word(stack);
            stack += 2;
            FALLTHROUGH;

        case 0x40:
            reg_s_or_u = memory.read_word(stack);
            stack += 2;
            break;

        case 0xb0:
            x = memory.read_word(stack);
            stack += 2;
            FALLTHROUGH;

        case 0xa0:
            y = memory.read_word(stack);
            stack += 2;
            pc = memory.read_word(stack);
            stack += 2;
            break;

        case 0x30:
            x = memory.read_word(stack);
            stack += 2;
            FALLTHROUGH;

        case 0x20:
            y = memory.read_word(stack);
            stack += 2;
            break;

        case 0xd0:
            x = memory.read_word(stack);
            stack += 2;
            reg_s_or_u = memory.read_word(stack);
            stack += 2;
            pc = memory.read_word(stack);
            stack += 2;
            break;

        case 0x90:
            x = memory.read_word(stack);
            stack += 2;
            pc = memory.read_word(stack);
            stack += 2;
            break;

        case 0x50:
            x = memory.read_word(stack);
            stack += 2;
            reg_s_or_u = memory.read_word(stack);
            stack += 2;
            break;

        case 0x10:
            x = memory.read_word(stack);
            stack += 2;
            break;
    }

    return psh_pul_cycles[what];
}

void Mc6809::exg()
{
    Word t1;
    Word t2;
    Byte w = memory.read_byte(pc++);
    bool r1_is_byte = false;
    bool r2_is_byte = false;

    // decode source
    switch (w >> 4U)
    {
        case 0x00:
            t1 = d;
            break;

        case 0x01:
            t1 = x;
            break;

        case 0x02:
            t1 = y;
            break;

        case 0x03:
            t1 = u;
            break;

        case 0x04:
            t1 = s;
            break;

        case 0x05:
            t1 = pc;
            break;

        case 0x08:
            t1 = a | (a * 256U);
            r1_is_byte = true;
            break;

        case 0x09:
            t1 = b | (b * 256U);
            r1_is_byte = true;
            break;

        case 0x0a:
            t1 = cc.all | (cc.all * 256U);
            r1_is_byte = true;
            break;

        case 0x0b:
            t1 = dp | (dp * 256U);
            r1_is_byte = true;
            break;

        default:
            if (!use_undocumented)
            {
                pc -= 2;
                invalid("transfer register");
                return;
            }

            t1 = 0xFFFF;
    }

    switch (w & 0x0FU)
    {
        case 0x00:
            t2 = d;
            break;

        case 0x01:
            t2 = x;
            break;

        case 0x02:
            t2 = y;
            break;

        case 0x03:
            t2 = u;
            break;

        case 0x04:
            t2 = s;
            break;

        case 0x05:
            t2 = pc;
            break;

        case 0x08:
            t2 = a | 0xFF00U;
            r2_is_byte = true;
            break;

        case 0x09:
            t2 = b | 0xFF00U;
            r2_is_byte = true;
            break;

        case 0x0a:
            t2 = cc.all | 0xFF00U;
            r2_is_byte = true;
            break;

        case 0x0b:
            t2 = dp | 0xFF00U;
            r2_is_byte = true;
            break;

        default:
            if (!use_undocumented)
            {
                pc -= 2;
                invalid("transfer register");
                return;
            }

            t2 = 0xFFFF;
    }

    if (!use_undocumented && (r1_is_byte ^ r2_is_byte))
    {
        pc -= 2;
        invalid("transfer register");
        return;
    }

    switch (w >> 4U)
    {
        case 0x00:
            d = t2;
            break;

        case 0x01:
            x = t2;
            break;

        case 0x02:
            y = t2;
            break;

        case 0x03:
            u = t2;
            break;

        case 0x04:
            s = t2;
            break;

        case 0x05:
            pc = t2;
            break;

        case 0x08:
            a = static_cast<Byte>(t2);
            break;

        case 0x09:
            b = static_cast<Byte>(t2);
            break;

        case 0x0a:
            cc.all = static_cast<Byte>(t2);
            break;

        case 0x0b:
            dp = static_cast<Byte>(t2);
            break;
    }

    switch (w & 0x0FU)
    {
        case 0x00:
            d = t1;
            break;

        case 0x01:
            x = t1;
            break;

        case 0x02:
            y = t1;
            break;

        case 0x03:
            u = t1;
            break;

        case 0x04:
            s = t1;
            break;

        case 0x05:
            pc = t1;
            break;

        case 0x08:
            a = static_cast<Byte>(t1);
            break;

        case 0x09:
            b = static_cast<Byte>(t1);
            break;

        case 0x0a:
            cc.all = static_cast<Byte>(t1);
            break;

        case 0x0b:
            dp = static_cast<Byte>(t1);
            break;
    }
}

void Mc6809::tfr()
{
    Word t;
    Byte w = memory.read_byte(pc++);
    bool is_byte = false;

    // decode source
    switch (w >> 4U)
    {
        case 0x00:
            t = d;
            break;

        case 0x01:
            t = x;
            break;

        case 0x02:
            t = y;
            break;

        case 0x03:
            t = u;
            break;

        case 0x04:
            t = s;
            break;

        case 0x05:
            t = pc;
            break;

        case 0x08:
            t = a | 0xFF00U;
            is_byte = true;
            break;

        case 0x09:
            t = b | 0xFF00U;
            is_byte = true;
            break;

        case 0x0a:
            t = cc.all | (cc.all * 256U);
            is_byte = true;
            break;

        case 0x0b:
            t = dp | (dp * 256U);
            is_byte = true;
            break;

        default:
            if (!use_undocumented)
            {
                pc -= 2;
                invalid("transfer register");
                return;
            }

            t = 0xFFFF;
    }

    // decode destination
    switch (w & 0x0FU)
    {
        case 0x00:
            if (!use_undocumented && is_byte)
            {
                break;
            }

            d = t;
            return;

        case 0x01:
            if (!use_undocumented && is_byte)
            {
                break;
            }

            x = t;
            return;

        case 0x02:
            if (!use_undocumented && is_byte)
            {
                break;
            }

            y = t;
            return;

        case 0x03:
            if (!use_undocumented && is_byte)
            {
                break;
            }

            u = t;
            return;

        case 0x04:
            if (!use_undocumented && is_byte)
            {
                break;
            }

            s = t;
            return;

        case 0x05:
            if (!use_undocumented && is_byte)
            {
                break;
            }

            pc = t;
            return;

        case 0x08:
            if (!use_undocumented && !is_byte)
            {
                break;
            }

            a = static_cast<Byte>(t);
            return;

        case 0x09:
            if (!use_undocumented && !is_byte)
            {
                break;
            }

            b = static_cast<Byte>(t);
            return;

        case 0x0a:
            if (!use_undocumented && !is_byte)
            {
                break;
            }

            cc.all = static_cast<Byte>(t);
            return;

        case 0x0b:
            if (!use_undocumented && !is_byte)
            {
                break;
            }

            dp = static_cast<Byte>(t);
            return;
    }

    pc -= 2;
    invalid("transfer register");
}
#endif

void Mc6809::invalid(const char * /*msg*/)
{
    events |= Event::Invalid;
}

#ifndef FASTFLEX
Word Mc6809::do_effective_address(Byte post)
{
    Word addr = 0;

    if (!BTST<Byte>(post, 7U))
    {
        Word offset = post & 0x1FU;

        if (offset & 0x10U)
        {
            offset |= 0xFFE0U;
        }

        switch (post & 0x60U)
        {
            case 0x00 :
                addr = x + offset;
                break;

            case 0x20 :
                addr = y + offset;
                break;

            case 0x40 :
                addr = u + offset;
                break;

            case 0x60 :
                addr = s + offset;
                break;
        }
    }
    else
    {
        switch (post)
        {
            // ,X+ ,X++ ,-X ,--X ,X
            case 0x80:
                addr = x++;
                break;

            case 0x81:
                addr = x;
                x += 2;
                break;

            case 0x91:
                addr = memory.read_word(x);
                x += 2;
                break;

            case 0x82:
                addr = --x;
                break;

            case 0x83:
                x -= 2;
                addr = x;
                break;

            case 0x93:
                x -= 2;
                addr = memory.read_word(x);
                break;

            case 0x84:
                addr = x;
                break;

            case 0x94:
                addr = memory.read_word(x);
                break;

            // ,Y+ ,Y++ ,-Y ,--Y ,Y
            case 0xa0:
                addr = y++;
                break;

            case 0xa1:
                addr = y;
                y += 2;
                break;

            case 0xb1:
                addr = memory.read_word(y);
                y += 2;
                break;

            case 0xa2:
                addr = --y;
                break;

            case 0xa3:
                y -= 2;
                addr = y;
                break;

            case 0xb3:
                y -= 2;
                addr = memory.read_word(y);
                break;

            case 0xa4:
                addr = y;
                break;

            case 0xb4:
                addr = memory.read_word(y);
                break;

            // ,U+ ,U++ ,-U ,--U ,U
            case 0xc0:
                addr = u++;
                break;

            case 0xc1:
                addr = u;
                u += 2;
                break;

            case 0xd1:
                addr = memory.read_word(u);
                u += 2;
                break;

            case 0xc2:
                addr = --u;
                break;

            case 0xc3:
                u -= 2;
                addr = u;
                break;

            case 0xd3:
                u -= 2;
                addr = memory.read_word(u);
                break;

            case 0xc4:
                addr = u;
                break;

            case 0xd4:
                addr = memory.read_word(u);
                break;

            // ,S+ ,S++ ,-S ,--S ,S
            case 0xe0:
                addr = s++;
                break;

            case 0xe1:
                addr = s;
                s += 2;
                break;

            case 0xf1:
                addr = memory.read_word(s);
                s += 2;
                break;

            case 0xe2:
                addr = --s;
                break;

            case 0xe3:
                s -= 2;
                addr = s;
                break;

            case 0xf3:
                s -= 2;
                addr = memory.read_word(s);
                break;

            case 0xe4:
                addr = s;
                break;

            case 0xf4:
                addr = memory.read_word(s);
                break;

            // (+/- B),R
            case 0x85:
                addr = EXTEND8(b) + x;
                break;

            case 0x95:
                addr = EXTEND8(b) + x;
                addr = memory.read_word(addr);
                break;

            case 0xa5:
                addr = EXTEND8(b) + y;
                break;

            case 0xb5:
                addr = EXTEND8(b) + y;
                addr = memory.read_word(addr);
                break;

            case 0xc5:
                addr = EXTEND8(b) + u;
                break;

            case 0xd5:
                addr = EXTEND8(b) + u;
                addr = memory.read_word(addr);
                break;

            case 0xe5:
                addr = EXTEND8(b) + s;
                break;

            case 0xf5:
                addr = EXTEND8(b) + s;
                addr = memory.read_word(addr);
                break;

            // (+/- A),R
            case 0x86:
                addr = EXTEND8(a) + x;
                break;

            case 0x96:
                addr = EXTEND8(a) + x;
                addr = memory.read_word(addr);
                break;

            case 0xa6:
                addr = EXTEND8(a) + y;
                break;

            case 0xb6:
                addr = EXTEND8(a) + y;
                addr = memory.read_word(addr);
                break;

            case 0xc6:
                addr = EXTEND8(a) + u;
                break;

            case 0xd6:
                addr = EXTEND8(a) + u;
                addr = memory.read_word(addr);
                break;

            case 0xe6:
                addr = EXTEND8(a) + s;
                break;

            case 0xf6:
                addr = EXTEND8(a) + s;
                addr = memory.read_word(addr);
                break;

            // (+/- 7 bit offset),R
            case 0x88:
                addr = x + EXTEND8(memory.read_byte(pc++));
                break;

            case 0x98:
                addr = x + EXTEND8(memory.read_byte(pc++));
                addr = memory.read_word(addr);
                break;

            case 0xa8:
                addr = y + EXTEND8(memory.read_byte(pc++));
                break;

            case 0xb8:
                addr = y + EXTEND8(memory.read_byte(pc++));
                addr = memory.read_word(addr);
                break;

            case 0xc8:
                addr = u + EXTEND8(memory.read_byte(pc++));
                break;

            case 0xd8:
                addr = u + EXTEND8(memory.read_byte(pc++));
                addr = memory.read_word(addr);
                break;

            case 0xe8:
                addr = s + EXTEND8(memory.read_byte(pc++));
                break;

            case 0xf8:
                addr = s + EXTEND8(memory.read_byte(pc++));
                addr = memory.read_word(addr);
                break;

            // (+/- 15 bit offset),R
            case 0x89:
                addr = x + memory.read_word(pc);
                pc += 2;
                break;

            case 0x99:
                addr = x + memory.read_word(pc);
                pc += 2;
                addr = memory.read_word(addr);
                break;

            case 0xa9:
                addr = y + memory.read_word(pc);
                pc += 2;
                break;

            case 0xb9:
                addr = y + memory.read_word(pc);
                pc += 2;
                addr = memory.read_word(addr);
                break;

            case 0xc9:
                addr = u + memory.read_word(pc);
                pc += 2;
                break;

            case 0xd9:
                addr = u + memory.read_word(pc);
                pc += 2;
                addr = memory.read_word(addr);
                break;

            case 0xe9:
                addr = s + memory.read_word(pc);
                pc += 2;
                break;

            case 0xf9:
                addr = s + memory.read_word(pc);
                pc += 2;
                addr = memory.read_word(addr);
                break;

            // (+/- D),R
            case 0x8b:
                addr = d + x;
                break;

            case 0x9b:
                addr = memory.read_word(d + x);
                break;

            case 0xab:
                addr = d + y;
                break;

            case 0xbb:
                addr = memory.read_word(d + y);
                break;

            case 0xcb:
                addr = d + u;
                break;

            case 0xdb:
                addr = memory.read_word(d + u);
                break;

            case 0xeb:
                addr = d + s;
                break;

            case 0xfb:
                addr = memory.read_word(d + s);
                break;

            // (+/- 7 bit offset), PC
            case 0x8c:
            case 0xac:
            case 0xcc:
            case 0xec:
                addr = EXTEND8(memory.read_byte(pc++));
                addr += pc;
                break;

            case 0x9c:
            case 0xbc:
            case 0xdc:
            case 0xfc:
                addr = EXTEND8(memory.read_byte(pc++));
                addr = memory.read_word(addr + pc);
                break;

            // (+/- 15 bit offset), PC
            case 0x8d:
            case 0xad:
            case 0xcd:
            case 0xed:
                addr = memory.read_word(pc);
                pc += 2;
                addr += pc;
                break;

            case 0x9d:
            case 0xbd:
            case 0xdd:
            case 0xfd:
                addr = memory.read_word(pc);
                pc += 2;
                addr = memory.read_word(addr + pc);
                break;

            // [address]
            case 0x9f:
                addr = memory.read_word(pc);
                addr = memory.read_word(addr);
                pc += 2;
                break;

            default:
                --pc;
                invalid("indirect addressing postbyte");
                break;
        }
    }

    return addr;
}

//**********************************
// Interrupt execution
//**********************************
void Mc6809::nmi(bool save_state)
{
    if (save_state)
    {
        cc.bit.e = true;
        psh(0xff, s, u);
    }

    cc.bit.f = cc.bit.i = true;
    pc = memory.read_word(0xfffc);
}

void Mc6809::firq(bool save_state)
{
    if (save_state)
    {
        cc.bit.e = false;
        psh(0x81, s, u);
    }

    cc.bit.f = cc.bit.i = true;
    pc = memory.read_word(0xfff6);
}

void Mc6809::irq(bool save_state)
{
    if (save_state)
    {
        cc.bit.e = true;
        psh(0xff, s, u);
    }

    cc.bit.i = true; // Don't set flag f!
    pc = memory.read_word(0xfff8);
}
#endif

void Mc6809::set_bp(int which, Word address)
{
    if (which < 0 || which > static_cast<int>(bp.size()))
    {
        throw FlexException(FERR_WRONG_PARAMETER);
    }

    bp[which] = address;
    events |= Event::BreakPoint;
}

BOptionalWord Mc6809::get_bp(int which)
{
    if (which < 0 || which > static_cast<int>(bp.size()))
    {
        throw FlexException(FERR_WRONG_PARAMETER);
    }

    return bp[which];
}

bool Mc6809::is_bp_set(int which)
{
    if (which < 0 || which > static_cast<int>(bp.size()))
    {
        throw FlexException(FERR_WRONG_PARAMETER);
    }

    return bp[which].has_value();
}

void Mc6809::reset_bp(int which)
{
    if (which < 0 || which > static_cast<int>(bp.size()))
    {
        throw FlexException(FERR_WRONG_PARAMETER);
    }

    bp[which].reset();

    if (!bp[0].has_value() && !bp[1].has_value() && !bp[2].has_value())
    {
        events &= ~Event::BreakPoint;
    }

    // if a bp has been set in another thread check again
    if (bp[0].has_value() || bp[1].has_value() || bp[2].has_value())
    {
        events |= Event::BreakPoint;
    }
}

// If logFileName is empty the current log file is closed.
bool Mc6809::setLoggerConfig(const Mc6809LoggerConfig &loggerConfig)
{
    return logger.setLoggerConfig(loggerConfig);
}

void Mc6809::get_interrupt_status(tInterruptStatus &stat)
{
    std::memcpy(&stat, &interrupt_status, sizeof(tInterruptStatus));
}

void Mc6809::UpdateFrom(NotifyId id, void * /*param*/)
{
    if (id == NotifyId::SetIrq)
    {
        set_irq();
    }
    else if (id == NotifyId::SetFirq)
    {
        set_firq();
    }
    else if (id == NotifyId::SetNmi)
    {
        set_nmi();
    }
}
