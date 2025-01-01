/*
     mc6809in.cpp

     flexemu, an MC6809 emulator running FLEX
     Copyright (C) 1997-2025  W. Schwotzer

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
#include <array>
#include <cstring>
#include "warnoff.h"
#include <fmt/format.h>
#include "warnon.h"


#ifdef _MSC_VER
    #pragma warning (disable: 4800)
#endif

// Looks like a false postitive, this should not throw an exception.
// NOLINTNEXTLINE(cert-err58-cpp)
static const Mc6809::Event AnyInterrupt =
                 Mc6809::Event::Irq |
                 Mc6809::Event::Firq |
                 Mc6809::Event::Nmi;

void Mc6809::reset()
{
    ++interrupt_status.count[INT_RESET];
    cycles = 0;
    total_cycles = 0;
    nmi_armed = 0;
    /* no interrupts yet */
    events = events & Event::FrequencyControl;
    reset_bp(2); // remove next-breakpoint

#ifdef FASTFLEX
    ipcreg = memory.read_word(0xfffe);
    idpreg = 0x00; /* Direct page register = 0x00 */
    iccreg = 0x50; /* set i and f bit */
    eaddr = 0;
    iflag = 0;
    tb = 0;
    tw = 0;
    k = 0;
#else
    pc = memory.read_word(0xfffe);
    dp = 0x00; /* Direct page register = 0x00 */
    cc.all = 0x00; /* Clear all flags */
    cc.bit.i = true; /* IRQ disabled */
    cc.bit.f = true; /* FIRQ disabled */
#endif
}

unsigned Mc6809::Disassemble(Word address, InstFlg &p_flags,
                        std::string &code, std::string &mnemonic,
                        std::string &operands)
{
    std::array<Byte, 6> buffer{};
    DWord jumpAddress = 0;
    Word address_inc = address;

    if (disassembler == nullptr)
    {
        return 0;
    }

    for (Byte &value : buffer)
    {
        value = memory.read_byte(address_inc++);
    }

    p_flags = disassembler->Disassemble(buffer.data(), address, jumpAddress,
            code, mnemonic, operands);
    return disassembler->getByteSize(buffer.data());
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
    std::string mnemonic;
    std::string operands;
    std::string code;
    Word i;
    auto *stat = dynamic_cast<Mc6809CpuStatus *>(cpu_status);
    assert(stat != nullptr);

#ifdef FASTFLEX
    stat->a = iareg;
    stat->b = ibreg;
    stat->cc = iccreg;
    stat->dp = idpreg;
    stat->pc = ipcreg;
    stat->x = ixreg;
    stat->y = iyreg;
    stat->u = iureg;
    stat->s = isreg;
#else
    stat->a = a;
    stat->b = b;
    stat->cc = cc.all;
    stat->dp = dp;
    stat->pc = pc;
    stat->x = x;
    stat->y = y;
    stat->u = u;
    stat->s = s;
#endif
    Word stack_base = ((stat->s / CPU_STACK_BYTES) * CPU_STACK_BYTES) - 16;

    for (i = 0U; i < static_cast<Word>(sizeof(stat->instruction)); ++i)
    {
        stat->instruction[i] = memory.read_byte(stat->pc + i);
    }

    for (i = 0; i < static_cast<Word>(sizeof(stat->memory)); ++i)
    {
        stat->memory[i] = memory.read_byte(stack_base + i);
    }

    auto byte_size = Disassemble(stat->pc, flags, code, mnemonic, operands);
    if (byte_size == 0)
    {
        stat->mnemonic[0] = '\0';
        stat->operands[0] = '\0';
        stat->insn_size = 0U;
    }
    else
    {
        assert(mnemonic.size() < sizeof(stat->mnemonic));
        std::strncpy(stat->mnemonic, mnemonic.c_str(),
                sizeof(stat->mnemonic) - 1);
        stat->mnemonic[sizeof(stat->mnemonic) - 1] = '\0';
        assert(operands.size() < sizeof(stat->operands));
        std::strncpy(stat->operands, operands.c_str(),
                sizeof(stat->operands) - 1);
        stat->operands[sizeof(stat->operands) - 1] = '\0';
        stat->insn_size = static_cast<Word>(byte_size);
    }

    stat->total_cycles = get_cycles();
}

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
}

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
            std::string code;
            std::string mnemonic;
            std::string operands;
            InstFlg flags = InstFlg::NONE;

            // Only if disassembler available and
            // Next Instruction is a BSR, LBSR or JSR
            // set a breakpoint after this instruction
            if (disassembler != nullptr)
            {
                const auto byteSize =
                    Disassemble(PC, flags, code, mnemonic, operands);
                bp[2] = static_cast<Word>(PC + byteSize);
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

    while (true)
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
                    if ((bp[0].has_value() && PC == bp[0].value()) ||
                        (bp[1].has_value() && PC == bp[1].value()) ||
                        (bp[2].has_value() && PC == bp[2].value()))
                    {
                        // breakpoint encountered
                        if (bp[2].has_value() && PC == bp[2].value())
                        {
                            reset_bp(2);
                        }

                        new_state = CpuState::Stop;
                        break;
                    }
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
        }

        if (logger.doLogging(PC) && disassembler != nullptr)
        {
            Mc6809CpuStatus cpuState;

            get_status(&cpuState);
            logger.logCpuState(cpuState);
        }

        // execute one CPU instruction
#ifdef FASTFLEX
#include "engine.cpi"
#else
#include "mc6809ex.cpi"
#endif
        first_time = false;
    }

    return new_state;
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

void Mc6809::set_required_cyclecount(cycles_t p_cycles)
{
#ifdef FASTFLEX
    required_cyclecount = p_cycles * 10;
#else
    required_cyclecount = p_cycles;
#endif

    if (p_cycles == std::numeric_limits<decltype(cycles)>::max())
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

