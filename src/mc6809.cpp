/*
    mc6809.cpp

    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2018  W. Schwotzer

    This file is based on usim-0.91 which is
    Copyright (C) 1994 by R. B. Bellis
*/


#include "misc1.h"

#include "mc6809.h"
#include "da6809.h"
#include "inout.h"
#include "btime.h"


#ifdef FASTFLEX
    #define PC ipcreg
#else
    #define PC pc
#endif

Mc6809::Mc6809(Memory *x_memory) : events(0),
#ifdef FASTFLEX
    pMem(NULL),
#else
    a(acc.byte.a), b(acc.byte.b), d(acc.d), dp(dpreg.byte.h),
#endif
    disassembler(NULL), use_undocumented(false),
    log_fp(NULL), do_logging(false), memory(x_memory)
{
#ifndef FASTFLEX
    dpreg.byte.l = 0;
#endif
    memset(&lfs, 0, sizeof(lfs));
    memset(&interrupt_status, 0, sizeof(interrupt_status));
    init();
}

Mc6809::~Mc6809()
{
    delete disassembler;
    delete memory;
    disassembler = NULL;

    if (log_fp != NULL)
    {
        fclose(log_fp);
    }

    log_fp = NULL;
}

void Mc6809::set_disassembler(Da6809 *x_disassembler)
{
    disassembler = x_disassembler;

    if (disassembler != NULL)
    {
        disassembler->set_use_undocumented(use_undocumented);
    }
}

void Mc6809::set_use_undocumented(bool b)
{
    use_undocumented = b;

    if (disassembler != NULL)
    {
        disassembler->set_use_undocumented(use_undocumented);
    }
}

void Mc6809::init()
{
    int i;

    events = 0;

    // all breakpoints are reset
    for (i = 0; i < 3; i++)
    {
        bp[i] = 0x10000;
    }

    init_indexed_cycles();
    init_psh_pul_cycles();
} // init

void Mc6809::init_indexed_cycles()
{
    Word i;

    for (i = 0; i < 128; i++)
    {
        indexed_cycles[i] = 1;
    }

    for (i = 128; i < 256; i++)
        switch ((Byte)(i & 0x1f))
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
        } // switch
}

void Mc6809::init_psh_pul_cycles()
{
    Word i;
    Byte cycles;

    for (i = 0; i < 256; i++)
    {
        cycles = 5;

        if (i & 0x01)
        {
            cycles++;
        }

        if (i & 0x02)
        {
            cycles++;
        }

        if (i & 0x04)
        {
            cycles++;
        }

        if (i & 0x08)
        {
            cycles++;
        }

        if (i & 0x10)
        {
            cycles += 2;
        }

        if (i & 0x20)
        {
            cycles += 2;
        }

        if (i & 0x40)
        {
            cycles += 2;
        }

        if (i & 0x80)
        {
            cycles += 2;
        }

        psh_pul_cycles[i] = cycles;
    } // for
}

void Mc6809::set_nmi()
{
    events |= DO_NMI;
} // set_nmi

void Mc6809::set_firq()
{
    events |= DO_FIRQ;
} // set_firq

void Mc6809::set_irq()
{
    events |= DO_IRQ;
} // set_irq


void Mc6809::invalid(const char * /*msg*/)
{
    events |= DO_INVALID;
}

void Mc6809::set_bp(int which, Word address)
{
    bp[which] = address;
    events |= DO_BREAKPOINT;
}

unsigned int Mc6809::get_bp(int which)
{
    return bp[which];
}

int Mc6809::is_bp_set(int which)
{
    return bp[which] < 0x10000;
}

void Mc6809::reset_bp(int which)
{
    bp[which] = 0x10000;

    if (bp[0] >  0xffff && bp[1] >  0xffff && bp[2] >  0xffff)
    {
        events &= ~DO_BREAKPOINT;
    }

    // if a bp has been set in another thread check again
    if (bp[0] <= 0xffff || (bp[1] <= 0xffff && bp[2] <= 0xffff))
    {
        events |= DO_BREAKPOINT;
    }
}

// if lfs is NULL the current log file is closed
bool Mc6809::set_logfile(const struct s_cpu_logfile *x_lfs)
{
    if (log_fp != NULL)
    {
        fclose(log_fp);
        log_fp = NULL;
    }

    if (x_lfs == NULL || x_lfs->logFileName[0] == '\0')
    {
        // Disable logging
        lfs.logFileName[0] = '\0';
        events &= ~DO_LOG;
        return true;
    }

    if ((log_fp = fopen(x_lfs->logFileName, "w")) != NULL)
    {
        // Enable logging
        lfs.minAddr   = x_lfs->minAddr;
        lfs.maxAddr   = x_lfs->maxAddr;
        lfs.startAddr = x_lfs->startAddr;
        lfs.stopAddr  = x_lfs->stopAddr;
        strcpy(lfs.logFileName, x_lfs->logFileName);
        events |= DO_LOG;
        return true;
    }
    else
    {
        lfs.logFileName[0] = '\0';
        events &= ~DO_LOG;
        return false;
    }
}

void Mc6809::get_interrupt_status(tInterruptStatus &stat)
{
    memcpy(&stat, &interrupt_status, sizeof(tInterruptStatus));
}

