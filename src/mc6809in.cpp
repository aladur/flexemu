/*
     mc6809in.cpp

     flexemu, an MC6809 emulator running FLEX
     Copyright (C) 1997-2024  W. Schwotzer

     This file is based on usim-0.91 which is
     Copyright (C) 1994 by R. B. Bellis
*/


#include <limits>
#include <cinttypes>
#include <cassert>
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

void Mc6809::reset()
{
    ++interrupt_status.count[INT_RESET];
    cycles          = 0;
    total_cycles    = 0;
    nmi_armed       = 0;
    /* no interrupts yet */
    events = events & Event::FrequencyControl;
    reset_bp(2);    // remove next-breakpoint

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

int Mc6809::Disassemble(Word address, InstFlg *pFlags,
                        char **pCode, char **pMnemonic)
{
    Byte buffer[6];
    DWord jumpAddress = 0;

    if (disassembler == nullptr)
    {
        return 0;
    }

    for (Word i = 0; i < 6; i++)
    {
        buffer[i] = memory.read_byte(address + i);
    }

    return disassembler->Disassemble(buffer, address, pFlags, &jumpAddress,
                                     pCode, pMnemonic);
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

#ifdef FASTFLEX
    return total_cycles + (cycles / 10);
#else
    return total_cycles +  cycles;
#endif
}

void Mc6809::get_status(CpuStatus *cpu_status)
{

    InstFlg flags = InstFlg::NONE;
    char *pmnem_buf;
    char *pbuffer;
    Word i;
    Word mem_addr;
    auto *stat = dynamic_cast<Mc6809CpuStatus *>(cpu_status);
    assert(stat != nullptr);

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

void Mc6809::set_status(CpuStatus *cpu_status)
{
    const auto *stat = dynamic_cast<Mc6809CpuStatus *>(cpu_status);
    assert(stat != nullptr);

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

CpuStatusPtr Mc6809::create_status_object()
{
    return CpuStatusPtr(new Mc6809CpuStatus);
}

CpuState Mc6809::run(RunMode mode)
{
    switch (mode)
    {
        case RunMode::SingleStepInto:
            events |= Event::SingleStep | Event::IgnoreBP;
            events &= ~Event::DoSchedule;
            break;

        case RunMode::SingleStepOver:
        {
            char *pCode;
            char *pMnemonic;
            InstFlg flags = InstFlg::NONE;

            // Only if disassembler available and
            // Next Instruction is a BSR, LBSR or JSR
            // set a breakpoint after this instruction
            if (disassembler != nullptr)
            {
                bp[2] =
                    PC + Disassemble((unsigned int)PC,
                                     &flags, &pCode, &pMnemonic);
            }

            if (disassembler == nullptr ||
                    (flags & InstFlg::Sub) == InstFlg::NONE)
            {
                // Single step into subroutine call.
                events |= Event::SingleStep | Event::IgnoreBP;
                reset_bp(2);
            }
            else
            {
                // Step over subroutine call. Break at next instruction.
                events |= Event::BreakPoint | Event::IgnoreBP;
            }
        }

        events &= ~Event::DoSchedule;
        break;

        case RunMode::RunningStart:
            reset_bp(2);

            if ((events & Event::BreakPoint) != Event::NONE)
            {
                events |= Event::IgnoreBP;
            }

            events &= ~Event::DoSchedule;

        case RunMode::RunningContinue:
        default:
            break;
    }

    return runloop();
}

// Enter runloop with state CpuState::Run, CpuState::Step or CpuState::Next.
// Exit runloop if new state is CpuState::Stop, CpuState::Exit, CpuState::Reset
// or CpuState::ResetRun.
// The performance of the emulated CPU depends very much on this loop.
#ifdef _MSC_VER
    #pragma inline_depth(255)
#endif

// This function is performance critical for the CPU emulation and thus should
// not be split up.
// NOLINTNEXTLINE(readability-function-size)
CpuState Mc6809::runloop()
{
    CpuState new_state = CpuState::NONE;
    bool first_time = true;

    while (1)
    {
        if (events != Event::NONE)
        {
            if ((events & (Event::BreakPoint | Event::Invalid |
                           Event::SingleStep | Event::SingleStepFinished |
                           Event::FrequencyControl |
                           Event::Cwai | Event::Sync)) != Event::NONE)
            {
                // All non time critical events
                if ((events & Event::Invalid) != Event::NONE)
                {
                    // An invalid instr. occured
                    events &= ~Event::Invalid;
                    new_state = CpuState::Invalid; // invalid instruction encountered
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

                        new_state = CpuState::Stop;
                        break;
                    } // if
                }

                events &= ~Event::IgnoreBP;

                if ((events & Event::SingleStepFinished) != Event::NONE)
                {
                    // one single step has been executed
                    events &= (~Event::SingleStepFinished & ~Event::SingleStep);
                    new_state = CpuState::Stop;
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
                        new_state = CpuState::Stop;
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
                        new_state = CpuState::Suspend;
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
                        new_state = CpuState::Suspend;
                        break;
                    }
                }

                if ((events & Event::FrequencyControl) != Event::NONE)
                {
                    if (cycles >= required_cyclecount)
                    {
                        // set CPU thread asleep until next timer tick
                        new_state = CpuState::Suspend;
                        break;
                    }
                }
            }

            if ((events & AnyInterrupt) != Event::NONE)
            {
                cycles += exec_irqs();
            }

            if (((events & Event::DoSchedule) != Event::NONE) &&
                !first_time &&
                !((events & (Event::SingleStep | Event::SingleStepFinished))
                    != Event::NONE))
            {
                // Request from scheduler to return runloop
                // with state CpuState::Schedule.
                events &= ~Event::DoSchedule;
                new_state = CpuState::Schedule;
                break;
            }
        } // if

        if (log_fp != nullptr)
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
                log_current_instruction();
            }
        }

        // execute one CPU instruction
#ifdef FASTFLEX
#include "engine.cpi"
#else
#include "mc6809ex.cpi"
#endif
        first_time = false;
    } // while

    return new_state;
} // runloop

void Mc6809::log_current_instruction()
{
    Mc6809CpuStatus cpu_status;

    get_status(&cpu_status);
    if (lfs.logCycleCount)
    {
        fprintf(log_fp, "%20" PRIu64 " ", cpu_status.total_cycles);
    }
    fprintf(log_fp, "%04X %-23s", PC.load(), cpu_status.mnemonic);

    if (lfs.logRegisters != LogRegister::NONE)
    {
        LogRegister registerBit = LogRegister::CC;
        while (registerBit != LogRegister::NONE)
        {
            registerBit <<= 1;
        }

        if ((lfs.logRegisters & LogRegister::CC) != LogRegister::NONE)
        {
           fprintf(log_fp, " CC=%s", asCCString(cpu_status.cc).c_str());
        }
        if ((lfs.logRegisters & LogRegister::A) != LogRegister::NONE)
        {
           fprintf(log_fp, " A=%02X", (Word)cpu_status.a);
        }
        if ((lfs.logRegisters & LogRegister::B) != LogRegister::NONE)
        {
           fprintf(log_fp, " B=%02X", (Word)cpu_status.b);
        }
        if ((lfs.logRegisters & LogRegister::DP) != LogRegister::NONE)
        {
           fprintf(log_fp, " DP=%02X", (Word)cpu_status.dp);
        }
        if ((lfs.logRegisters & LogRegister::X) != LogRegister::NONE)
        {
           fprintf(log_fp, " X=%04X", cpu_status.x);
        }
        if ((lfs.logRegisters & LogRegister::Y) != LogRegister::NONE)
        {
           fprintf(log_fp, " Y=%04X", cpu_status.y);
        }
        if ((lfs.logRegisters & LogRegister::U) != LogRegister::NONE)
        {
           fprintf(log_fp, " U=%04X", cpu_status.u);
        }
        if ((lfs.logRegisters & LogRegister::S) != LogRegister::NONE)
        {
           fprintf(log_fp, " S=%04X", cpu_status.s);
        }
    }
    fprintf(log_fp, "\n");
    fflush(log_fp);
}

void Mc6809::do_reset()
{
    reset();
    reset_bp(2);
    memory.reset_io();
}

// Request to the CPU to exit runloop (thread save)
// giving control back to scheduler.
void Mc6809::exit_run()
{
    events |= Event::DoSchedule;
}

// Optionally a frequency control can be added
// which cyclically resets the cycle count and sets a
// required cycle count. When the cycle count is
// reached the runloop exits automatically with
// the state CpuState::Suspend.

void Mc6809::set_required_cyclecount(cycles_t x_cycles)
{
#ifdef FASTFLEX
    required_cyclecount = x_cycles * 10;
#else
    required_cyclecount = x_cycles;
#endif

    if (x_cycles == std::numeric_limits<decltype(cycles)>::max())
    {
        events &= ~Event::FrequencyControl;
    }
    else
    {
        events |= Event::FrequencyControl;
    }
}

cycles_t Mc6809::exec_irqs(bool save_state)
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

        if (((events & Event::Firq) != Event::NONE) && !CC_BITF)
        {
            ++interrupt_status.count[INT_FIRQ];
            EXEC_FIRQ(save_state);
            events &= ~Event::Firq;
            return save_state ? 8 : 5;
        }

        if (((events & Event::Irq) != Event::NONE) && !CC_BITI)
        {
            ++interrupt_status.count[INT_IRQ];
            EXEC_IRQ(save_state);
            events &= ~Event::Irq;
            return save_state ? 17 : 5;
        }
    }

    return 0;
}

std::string Mc6809::asCCString(Byte reg)
{
    const static Byte cc_bitmask[8] = {
        CC_BIT_E, CC_BIT_F, CC_BIT_H, CC_BIT_I,
        CC_BIT_N, CC_BIT_Z, CC_BIT_V, CC_BIT_C,
    };
    const static std::string cc_bitnames = "EFHINZVC";
    std::string result = "--------";

    for (int i = 0; i < 8; ++i)
    {
        if (reg & cc_bitmask[i])
        {
            result[i] = cc_bitnames[i];
        }
    }

    return result;
}

