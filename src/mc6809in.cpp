/*
     mc6809in.cpp

     flexemu, an MC6809 emulator running FLEX
     Copyright (C) 1997-2004  W. Schwotzer

     This file is based on usim-0.91 which is
     Copyright (C) 1994 by R. B. Bellis
*/


#include <limits.h>
#include "misc1.h"
#include "mc6809.h"
#include "mc6809st.h"
#include "da6809.h"
#include "intmem.h"


#ifdef _MSC_VER
    #pragma warning (disable: 4800)
#endif


void Mc6809::set_serpar(Byte b)
{
    WRITE(SERPAR, b);
}

void Mc6809::reset(void)
{
    ++interrupt_status.count[INT_RESET];
    cycles          = 0;
    total_cycles    = 0;
    nmi_armed       = 0;
    /* no interrupts yet */
    events          = 0;
    reset_bp(2);    // remove next-breakpoint

    if (bp[0] > 0xffff && bp[1] > 0xffff && bp[2] > 0xffff)
    {
        events &= ~DO_BREAKPOINT;
    }

#ifdef FASTFLEX
    ipcreg          = READ_WORD(0xfffe);
    idpreg          = 0x00;         /* Direct page register = 0x00 */
    iccreg          = 0x50;         /* set i and f bit              */
    eaddr           = 0;
    iflag           = 0;
    tb              = 0;
    tw              = 0;
    k               = 0;
#else
    pc              = READ_WORD(0xfffe);
    dp              = 0x00;         /* Direct page register = 0x00 */
    cc.all          = 0x00;         /* Clear all flags */
    cc.bit.i        = true;         /* IRQ disabled */
    cc.bit.f        = true;         /* FIRQ disabled */
#endif
}

int Mc6809::Disassemble(Word address, DWord *pFlags,
                        char **pb1, char **pb2)
{
    Byte buffer[6];
    DWord addr;

    if (disassembler == NULL)
    {
        return 0;
    }

    for (int i = 0; i < 6; i++)
    {
        buffer[i] = READ(address + i);
    }

    return disassembler->Disassemble((const Byte *)buffer, address,
                                     pFlags, &addr, pb1, pb2);
}

//*******************************************************************
// Processor state
//*******************************************************************

QWord Mc6809::get_cycles(bool reset /* = false */)
{
    if (reset)
    {
#ifdef FASTFLEX
        total_cycles += cycles / 10;
#else
        total_cycles += cycles;
#endif
        cycles = 0;
        return total_cycles;
    }
    else
    {
#ifdef FASTFLEX
        return total_cycles + (cycles / 10);
#else
        return total_cycles +  cycles;
#endif
    }
}

void Mc6809::get_status(CpuStatus *status)
{

    DWord flags;
    char *pmnem_buf, *pbuffer;
    Word i, mem_addr;
    Mc6809CpuStatus *stat = (Mc6809CpuStatus *)status;

#ifdef FASTFLEX
    stat->a        = iareg;
    stat->b        = ibreg;
    stat->cc       = iccreg;
    stat->dp       = idpreg;
    stat->pc       = ipcreg;
    stat->x        = ixreg;
    stat->y        = iyreg;
    stat->u        = iureg;
    stat->s        = isreg;
#else
    stat->a        = a;
    stat->b        = b;
    stat->cc       = cc.all;
    stat->dp       = dp;
    stat->pc       = pc;
    stat->x        = x;
    stat->y        = y;
    stat->u        = u;
    stat->s        = s;
#endif
    mem_addr = ((stat->s >> 3) << 3) - 16;

    for (i = 0; i < 4; i++)
    {
        stat->instruction[i] = READ(stat->pc + i);
    }

    for (i = 0; i < 48; i++)
    {
        stat->memory[i] = READ(mem_addr + i);
    }

    if (!Disassemble(stat->pc, &flags, &pbuffer, &pmnem_buf))
    {
        stat->mnemonic[0] = '\0';
    }
    else
    {
        strcpy(stat->mnemonic, pmnem_buf);
    }

    stat->total_cycles = get_cycles();
}  // get_status

void Mc6809::set_status(CpuStatus *status)
{
    Mc6809CpuStatus *stat = (Mc6809CpuStatus *)status;

#ifdef FASTFLEX
    iareg = stat->a;
    ibreg = stat->b;
    iccreg = stat->cc;
    idpreg = stat->dp;
    ipcreg = stat->pc;
    ixreg = stat->x;
    iyreg = stat->y;
    iureg = stat->u;
    isreg = stat->s;
#else
    a = stat->a;
    b = stat->b;
    cc.all = stat->cc;
    dp = stat->dp;
    pc = stat->pc;
    x = stat->x;
    y = stat->y;
    u = stat->u;
    s = stat->s;
#endif
}  // set_status

// The caller is responsible for deleting the object
CpuStatus *Mc6809::create_status_object()
{
    return new Mc6809CpuStatus;
}

Byte Mc6809::run(Word mode)
{
    switch (mode)
    {
        case SINGLESTEP_INTO:
            events |= DO_SINGLESTEP | IGNORE_BP;
            events &= ~DO_GO_BACK;
            break;

        case SINGLESTEP_OVER:
        {
            char *pa, *pb;
            DWord flags = 0;

            // Only if disassembler available and
            // Next Instruction is a BSR, LBSR or JSR
            // set a breakpoint after this instruction
            if (disassembler != NULL)
                bp[2] =
                    PC + Disassemble((unsigned int)PC,
                                     &flags, &pa, &pb);

            if (disassembler == NULL || !(flags & DA_SUB))
            {
                events |= DO_SINGLESTEP | IGNORE_BP;
                reset_bp(2);
            }
            else
            {
                events |= DO_BREAKPOINT | IGNORE_BP;
            }
        }

        events &= ~DO_GO_BACK;
        break;

        case START_RUNNING:
            reset_bp(2);

            if (events & DO_BREAKPOINT)
            {
                events |= IGNORE_BP;
            }

            events &= ~DO_GO_BACK;

        case CONTINUE_RUNNING:
            break;
    }

    return runloop();
}

// Enter runloop with state S_RUN, S_STEP or S_NEXT
// Exit  runloop if new state is S_STOP, S_EXIT, S_RESET or S_RESET_RUN
// The performance of the emulated CPU depends very much on this loop
#ifdef _MSC_VER
    #pragma inline_depth(255)
#endif
Byte Mc6809::runloop()
{
    Byte newState = 0;
    bool first_time = true;

    while (1)
    {
        if (events)
        {
            if (events & (DO_BREAKPOINT | DO_INVALID | DO_SINGLESTEP |
                          DO_SINGLESTEPFINISHED | DO_FREQ_CONTROL | DO_LOG |
                          DO_CWAI | DO_SYNC))
            {
                // All non time critical events
                if (events & DO_INVALID)
                {
                    // An invalid instr. occured
                    events &= ~DO_INVALID;
                    newState = S_INVALID; // invalid instruction encountered
                    break;
                }

                if ((events & DO_BREAKPOINT) && !(events & IGNORE_BP))
                {
                    if (PC == bp[0] || PC == bp[1] || PC == bp[2])
                    {
                        // breakpoint encountered
                        if (PC == bp[2])
                        {
                            reset_bp(2);
                        }

                        newState = S_STOP;
                        break;
                    } // if
                }

                events &= ~IGNORE_BP;

                if (events & DO_SINGLESTEPFINISHED)
                {
                    // one single step has been executed
                    events = events & ~DO_SINGLESTEPFINISHED &
                             ~DO_SINGLESTEP;
                    newState = S_STOP;
                    break;
                }

                if (events & DO_SINGLESTEP)
                {
                    events &= ~DO_SINGLESTEP;

                    if (!(events & (DO_CWAI | DO_SYNC)))
                    {
                        events |= DO_SINGLESTEPFINISHED;
                    }
                    else
                    {
                        newState = S_STOP;
                        break;
                    }
                }

                if (events & DO_CWAI)
                {
                    if (((events & DO_IRQ)  && !CC_BITI) ||
                        ((events & DO_FIRQ) && !CC_BITF) ||
                        ((events & DO_NMI)  && !nmi_armed))
                    {
                        events &= ~DO_CWAI;
                        PC += 2;
                        cycles += exec_irqs(false);
                    }
                    else
                    {
                        // set CPU thread asleep until next timer tick
                        newState = S_NO_CHANGE | EXIT_SUSPEND;
                        break;
                    }
                }

                if (events & DO_SYNC)
                {
                    if (events & (DO_IRQ | DO_FIRQ | DO_NMI))
                    {
                        events &= ~DO_SYNC;
                        PC++;
                        cycles += exec_irqs();
                    }
                    else
                    {
                        // set CPU thread asleep until next timer tick
                        newState = S_NO_CHANGE | EXIT_SUSPEND;
                        break;
                    }
                }

                if (events & DO_FREQ_CONTROL)
                {
                    if (cycles >= required_cyclecount)
                    {
                        // set CPU thread asleep until next timer tick
                        newState = S_NO_CHANGE | EXIT_SUSPEND;
                        break;
                    }
                }

                if (log_fp != NULL && (events & DO_LOG))
                {
                    if (lfs.startAddr >= 0x10000 || PC == lfs.startAddr)
                    {
                        do_logging = true;
                    }

                    if (lfs.stopAddr < 0x10000 && PC == lfs.stopAddr)
                    {
                        do_logging = false;
                    }

                    if (do_logging && disassembler != NULL &&
                        PC >= lfs.minAddr && PC <= lfs.maxAddr)
                    {
                        char *pa, *pb;
                        DWord flags = 0;

                        Disassemble(PC, &flags, &pa, &pb);
                        fprintf(log_fp, "%04X %s\n", PC, pb);
                    }
                }
            }

            if (events & (DO_IRQ | DO_FIRQ | DO_NMI))
            {
                cycles += exec_irqs();
            }

            if ((events & DO_GO_BACK) &&
                !first_time &&
                !(events & (DO_SINGLESTEP | DO_SINGLESTEPFINISHED)))
            {
                // request from scheduler to return runloop
                // with state S_NO_CHANGE
                events &= ~DO_GO_BACK;
                newState = S_NO_CHANGE;
                break;
            }
        } // if


        // execute one CPU instruction
#ifdef FASTFLEX
#include "engine.cpp"
#else
#include "mc6809ex.cpp"
#endif
        first_time = false;
    } // while

    return newState;
} // runloop

void Mc6809::do_reset(void)
{
    reset();
    reset_bp(2);
    memory->reset_io();
}

// request to the CPU to exit run loop (thread save)
void Mc6809::exit_run()
{
    events |= DO_GO_BACK;
}

// Optionally a frequency control can be added
// which cyclically resets the cycle count and sets a
// required cycle count. When the cycle count is
// reached the runloop exits automatically with
// the Pseudo state (EXIT_SUSPEND | S_NO_CHANGE)

void Mc6809::set_required_cyclecount(t_cycles x_cycles)
{
#ifdef FASTFLEX
    required_cyclecount = x_cycles * 10;
#else
    required_cyclecount = x_cycles;
#endif

    if (x_cycles == ULONG_MAX)
    {
        events &= ~DO_FREQ_CONTROL;
    }
    else
    {
        events |= DO_FREQ_CONTROL;
    }
}

