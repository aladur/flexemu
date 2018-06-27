/*
     mc6809in.cpp

     flexemu, an MC6809 emulator running FLEX
     Copyright (C) 1997-2018  W. Schwotzer

     This file is based on usim-0.91 which is
     Copyright (C) 1994 by R. B. Bellis
*/


#include <limits.h>
#include "misc1.h"
#include "mc6809.h"
#include "mc6809st.h"
#include "da6809.h"


#ifdef _MSC_VER
    #pragma warning (disable: 4800)
#endif

static const Mc6809::Event AnyInterrupt =
                 Mc6809::Event::Irq |
                 Mc6809::Event::Firq |
                 Mc6809::Event::Nmi;

void Mc6809::set_serpar(Byte b)
{
    memory.write_byte(SERPAR, b);
}

void Mc6809::reset()
{
    ++interrupt_status.count[INT_RESET];
    cycles          = 0;
    total_cycles    = 0;
    nmi_armed       = 0;
    /* no interrupts yet */
    events = Event::NONE;
    reset_bp(2);    // remove next-breakpoint

    if (bp[0] > 0xffff && bp[1] > 0xffff && bp[2] > 0xffff)
    {
        events &= ~Event::BreakPoint;
    }

#ifdef FASTFLEX
    ipcreg          = memory.read_word(0xfffe);
    idpreg          = 0x00;         /* Direct page register = 0x00 */
    iccreg          = 0x50;         /* set i and f bit              */
    eaddr           = 0;
    iflag           = 0;
    tb              = 0;
    tw              = 0;
    k               = 0;
#else
    pc              = memory.read_word(0xfffe);
    dp              = 0x00;         /* Direct page register = 0x00 */
    cc.all          = 0x00;         /* Clear all flags */
    cc.bit.i        = true;         /* IRQ disabled */
    cc.bit.f        = true;         /* FIRQ disabled */
#endif
}

int Mc6809::Disassemble(Word address, DWord *pFlags,
                        char **pCode, char **pMnemonic)
{
    Byte buffer[6];
    DWord jumpAddress = 0;

    if (disassembler == nullptr)
    {
        return 0;
    }

    for (int i = 0; i < 6; i++)
    {
        buffer[i] = memory.read_byte(address + i);
    }

    return disassembler->Disassemble((const Byte * const)buffer, address,
                                      pFlags, &jumpAddress, pCode, pMnemonic);
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
        stat->instruction[i] = memory.read_byte(stat->pc + i);
    }

    for (i = 0; i < 48; i++)
    {
        stat->memory[i] = memory.read_byte(mem_addr + i);
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
            events |= Event::SingleStep | Event::IgnoreBP;
            events &= ~Event::GoBack;
            break;

        case SINGLESTEP_OVER:
        {
            char *pCode, *pMnemonic;
            DWord flags = 0;

            // Only if disassembler available and
            // Next Instruction is a BSR, LBSR or JSR
            // set a breakpoint after this instruction
            if (disassembler != nullptr)
                bp[2] =
                    PC + Disassemble((unsigned int)PC,
                                     &flags, &pCode, &pMnemonic);

            if (disassembler == nullptr || !(flags & DA_SUB))
            {
                events |= Event::SingleStep | Event::IgnoreBP;
                reset_bp(2);
            }
            else
            {
                events |= Event::BreakPoint | Event::IgnoreBP;
            }
        }

        events &= ~Event::GoBack;
        break;

        case START_RUNNING:
            reset_bp(2);

            if ((events & Event::BreakPoint) != Event::NONE)
            {
                events |= Event::IgnoreBP;
            }

            events &= ~Event::GoBack;

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
        if (events != Event::NONE)
        {
            if ((events & (Event::BreakPoint | Event::Invalid |
                           Event::SingleStep | Event::SingleStepFinished |
                           Event::FrequencyControl | Event::Log |
                           Event::Cwai | Event::Sync)) != Event::NONE)
            {
                // All non time critical events
                if ((events & Event::Invalid) != Event::NONE)
                {
                    // An invalid instr. occured
                    events &= ~Event::Invalid;
                    newState = S_INVALID; // invalid instruction encountered
                    break;
                }

                if (((events & Event::BreakPoint) != Event::NONE) &&
                    ((events & Event::IgnoreBP) == Event::NONE))
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

                events &= ~Event::IgnoreBP;

                if ((events & Event::SingleStepFinished) != Event::NONE)
                {
                    // one single step has been executed
                    events &= (~Event::SingleStepFinished & ~Event::SingleStep);
                    newState = S_STOP;
                    break;
                }

                if ((events & Event::SingleStep) != Event::NONE)
                {
                    events &= ~Event::SingleStep;

                    if (!(events & (Event::Cwai | Event::Sync)))
                    {
                        events |= Event::SingleStepFinished;
                    }
                    else
                    {
                        newState = S_STOP;
                        break;
                    }
                }

                if ((events & Event::Cwai) != Event::NONE)
                {
                    if ((((events & Event::Irq) != Event::NONE) && !CC_BITI) ||
                        (((events & Event::Firq) != Event::NONE) && !CC_BITF) ||
                        (((events & Event::Nmi) != Event::NONE) && !nmi_armed))
                    {
                        events &= ~Event::Cwai;
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

                if ((events & Event::Sync) != Event::NONE)
                {
                    if ((events & AnyInterrupt) != Event::NONE)
                    {
                        events &= ~Event::Sync;
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

                if ((events & Event::FrequencyControl) != Event::NONE)
                {
                    if (cycles >= required_cyclecount)
                    {
                        // set CPU thread asleep until next timer tick
                        newState = S_NO_CHANGE | EXIT_SUSPEND;
                        break;
                    }
                }

                if (log_fp != nullptr && ((events & Event::Log) != Event::NONE))
                {
                    if (lfs.startAddr >= 0x10000 || PC == lfs.startAddr)
                    {
                        do_logging = true;
                    }

                    if (lfs.stopAddr < 0x10000 && PC == lfs.stopAddr)
                    {
                        do_logging = false;
                    }

                    if (do_logging && disassembler != nullptr &&
                        PC >= lfs.minAddr && PC <= lfs.maxAddr)
                    {
                        char *pCode, *pMnemonic;
                        DWord flags = 0;

                        Disassemble(PC, &flags, &pCode, &pMnemonic);
                        fprintf(log_fp, "%04X %s\n", PC, pMnemonic);
                    }
                }
            }

            if ((events & AnyInterrupt) != Event::NONE)
            {
                cycles += exec_irqs();
            }

            if (((events & Event::GoBack) != Event::NONE) &&
                !first_time &&
                !((events & (Event::SingleStep | Event::SingleStepFinished))
                    != Event::NONE))
            {
                // request from scheduler to return runloop
                // with state S_NO_CHANGE
                events &= ~Event::GoBack;
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

void Mc6809::do_reset()
{
    reset();
    reset_bp(2);
    memory.reset_io();
}

// request to the CPU to exit run loop (thread save)
void Mc6809::exit_run()
{
    events |= Event::GoBack;
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
        events &= ~Event::FrequencyControl;
    }
    else
    {
        events |= Event::FrequencyControl;
    }
}

t_cycles Mc6809::exec_irqs(bool save_state)
{
    if ((events & AnyInterrupt) != Event::NONE)
    {
        if (((events & Event::Nmi) != Event::NONE) && !nmi_armed)
        {
            ++interrupt_status.count[INT_NMI];
            EXEC_NMI(save_state);
            events &= ~Event::Nmi;
            return save_state ? 17 : 5;
        }
        else if (((events & Event::Firq) != Event::NONE) && !CC_BITF)
        {
            ++interrupt_status.count[INT_FIRQ];
            EXEC_FIRQ(save_state);
            events &= ~Event::Firq;
            return save_state ? 8 : 5;
        }
        else if (((events & Event::Irq) != Event::NONE) && !CC_BITI)
        {
            ++interrupt_status.count[INT_IRQ];
            EXEC_IRQ(save_state);
            events &= ~Event::Irq;
            return save_state ? 17 : 5;
        }
    }

    return 0;
}

