/*
    mc6809.h

    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2025  W. Schwotzer

    This file is based on usim-0.91 which is
    Copyright (C) 1994 by R. B. Bellis
*/



#ifndef MC6809_INCLUDED
#define MC6809_INCLUDED

#include "misc1.h"
#include "typedefs.h"
#include "memory.h"
#include "schedcpu.h"
#include "scpulog.h"
#include "cpustate.h"
#include "flexemu.h"
#include "absdisas.h"
#include "bobserv.h"
#include "mc6809lg.h"
#include <atomic>
#include <array>
#include "warnoff.h"
#include <optional>
#include "warnon.h"

using OptionalWord = std::optional<Word>;

/* Keep macro to be controlable from command line */
/* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
#define USE_ASM 1

// Test if GCC inline assembler can be used
#if defined(__amd64__)
    #define IX86
#endif
#if defined(__GNUC__) && defined (USE_ASM) && defined(IX86) && \
    defined(__LP64__) && !defined(FASTFLEX)
    #define USE_GCCASM
#endif

#ifdef BITFIELDS_LSB_FIRST
constexpr auto CC_BIT_C{0x01U};
constexpr auto CC_BIT_V{0x02U};
constexpr auto CC_BIT_Z{0x04U};
constexpr auto CC_BIT_N{0x08U};
constexpr auto CC_BIT_I{0x10U};
constexpr auto CC_BIT_H{0x20U};
constexpr auto CC_BIT_F{0x40U};
constexpr auto CC_BIT_E{0x80U};
#else
constexpr auto CC_BIT_C{0x80U};
constexpr auto CC_BIT_V{0x40U};
constexpr auto CC_BIT_Z{0x20U};
constexpr auto CC_BIT_N{0x10U};
constexpr auto CC_BIT_I{0x08U};
constexpr auto CC_BIT_H{0x04U};
constexpr auto CC_BIT_F{0x02U};
constexpr auto CC_BIT_E{0x01U};
#endif
constexpr auto CC_BITS_HNZVC{CC_BIT_H | CC_BIT_N | CC_BIT_Z | CC_BIT_V |
    CC_BIT_C};
constexpr auto CC_BITS_NZVC{CC_BIT_N | CC_BIT_Z | CC_BIT_V | CC_BIT_C};
constexpr auto CC_BITS_NZV{CC_BIT_N | CC_BIT_Z | CC_BIT_V};
constexpr auto CC_BITS_NZC{CC_BIT_N | CC_BIT_Z | CC_BIT_C};

#ifdef FASTFLEX
    #define PC ipcreg
    #define CC_BITI (iccreg & 0x10)
    #define CC_BITF (iccreg & 0x40)
    #include "engine.h"
#else
    #define PC pc
    #define CC_BITI (cc.bit.i)
    #define CC_BITF (cc.bit.f)
#endif

// uncomment this for debug output of cpu
//#define   DEBUG_FILE  "cpu.txt"


union uacc
{
    Word d; // Combined accumulator
    struct
    {
#ifdef WORDS_BIGENDIAN
        Byte a; // Accumulator a
        Byte b; // Accumulator b
#else
        Byte b; // Accumulator b
        Byte a; // Accumulator a
#endif
    } byte;
};

union udpreg
{
    Word dp16;
    struct
    {
#ifdef WORDS_BIGENDIAN
        Byte h;
        Byte l;
#else
        Byte l;
        Byte h;
#endif
    } byte;
};

union ucc
{
    Byte all; // Condition code register
    struct
    {
#ifdef BITFIELDS_LSB_FIRST
        bool c : 1; // Carry
        bool v : 1; // Overflow
        bool z : 1; // Zero
        bool n : 1; // Negative
        bool i : 1; // IRQ disable
        bool h : 1; // Half carry
        bool f : 1; // FIRQ disable
        bool e : 1; // Entire
#else
        bool e : 1; // Entire
        bool f : 1; // FIRQ disable
        bool h : 1; // Half carry
        bool i : 1; // IRQ disable
        bool n : 1; // Negative
        bool z : 1; // Zero
        bool v : 1; // Overflow
        bool c : 1; // Carry
#endif
    } bit;
};

class Da6809;
struct Mc6809CpuStatus;


class Mc6809 : public ScheduledCpu, public BObserver
{
public:
    enum class Event : Word
    {
        NONE = 0U,
        Nmi = (1U << 0U),
        Firq = (1U << 1U),
        Irq = (1U << 2U),
        Invalid = (1U << 3U),
        BreakPoint = (1U << 4U),
        SingleStep = (1U << 5U),
        SingleStepFinished = (1U << 6U),
        SyncExec = (1U << 7U),
        Timer = (1U << 8U),
        SetStatus = (1U << 9U),
        FrequencyControl = (1U << 10U),
        DoSchedule = (1U << 11U),
        Cwai = (1U << 13U),
        Sync = (1U << 14U),
        IgnoreBP = (1U << 15U),
    };

protected:
    class atomic_event
    {
        using T = std::underlying_type_t<Mc6809::Event>;
        std::atomic<T> event;

    public:
        atomic_event() = delete;
        explicit atomic_event(Event p_event)
            : event(static_cast<T>(p_event))
        {
        }
        ~atomic_event() = default;
        atomic_event(const atomic_event&) = delete;
        atomic_event& operator= (const atomic_event&) = delete;
        atomic_event &operator= (Event p_event)
        {
            event = static_cast<T>(p_event);
            return *this;
        }
        Event operator& (Event p_event)
        {
            return static_cast<Event>(static_cast<T>(event) &
                                      static_cast<T>(p_event));
        }
        Event operator|= (Event p_event)
        {
            event |= static_cast<T>(p_event);
            return static_cast<Event>(static_cast<T>(event));
        }
        Event operator&= (Event p_event)
        {
            event &= static_cast<T>(p_event);
            return static_cast<Event>(static_cast<T>(event));
        }
        bool operator!= (Event p_event)
        {
            return static_cast<T>(event) != static_cast<T>(p_event);
        }
    };

    // Processor registers
protected:

    std::array<Byte, 256> indexed_cycles{}; // add. cycles for
    // indexed addr.
    std::array<Byte, 256> psh_pul_cycles{}; // add. cycles for psh
    // and pull-instr.
    Byte nmi_armed{0}; // for handling
    // interrupts
    atomic_event events{Event::NONE}; // event status flags (atomic access)
    tInterruptStatus interrupt_status{};
#ifdef FASTFLEX
    std::atomic<Word> ipcreg{0};
    Word iureg{0};
    Word isreg{0};
    Word ixreg{0};
    Word iyreg{0};
    Byte iareg{0};
    Byte ibreg{0};
    Byte iccreg{0};
    Byte idpreg{0};
    Word eaddr{0};
    Byte ireg{0};
    Byte iflag{0};
    Byte tb{0};
    Word tw{0};
    Byte k{0};
    Byte *pMem{nullptr}; // needed for memory access
#else
    std::atomic<Word> pc{0};
    Word s{0}; // Stack pointer
    Word u{0}; // Alternative Stack pointer
    Word x{0}; // Index Register
    Word y{0}; // Index register
    union uacc acc{0};
    union udpreg dpreg{0};
    Byte &a;
    Byte &b;
    Word &d;
    Byte &dp;
    union ucc cc{0};
#endif // #ifndef FASTFLEX

protected:

    Da6809 *disassembler{nullptr};

    // funcitons for instruction execution:

private:

    void init();
    void init_indexed_cycles();
    void init_psh_pul_cycles();
    void illegal();

#ifndef FASTFLEX
    //***********************************
    // Addressing mode fetch Instructions
    //***********************************

    // fetch immediate 16-Bit operand
    inline Word fetch_imm_16()
    {
        Word addr = memory.read_word(pc);
        pc += 2;
        return addr;
    }

    // fetch extended 16-Bit operand
    inline Word fetch_ext_16()
    {
        Word addr = memory.read_word(pc);
        pc += 2;
        return memory.read_word(addr);
    }

    // fetch direct 16-Bit operand
    inline Word fetch_dir_16()
    {
        Word addr = dpreg.dp16 | memory.read_byte(pc++);
        return memory.read_word(addr);
    }

    // fetch indexed 16-Bit operand
    inline Word fetch_idx_16(cycles_t &cycle_count)
    {
        Word addr;
        Byte post;

        post = memory.read_byte(pc++);
        addr = do_effective_address(post);
        cycle_count += indexed_cycles[post];
        return memory.read_word(addr);
    }

    // fetch immediate 8-Bit operand
    inline Byte fetch_imm_08()
    {
        return memory.read_byte(pc++);
    }

    // fetch extended 8-Bit operand
    inline Byte fetch_ext_08()
    {
        Word addr = memory.read_word(pc);
        pc += 2;
        return memory.read_byte(addr);
    }

    // fetch direct 8-Bit operand
    inline Byte fetch_dir_08()
    {
        Word addr = dpreg.dp16 | memory.read_byte(pc++);
        return memory.read_byte(addr);
    }

    // fetch indexed 8-Bit operand
    inline Byte fetch_idx_08(cycles_t &cycle_count)
    {
        Word addr;
        Byte post;

        post = memory.read_byte(pc++);
        addr = do_effective_address(post);
        cycle_count += indexed_cycles[post];
        return memory.read_byte(addr);
    }

    // fetch effective address extended
    inline Word fetch_ea_ext()
    {
        Word addr = memory.read_word(pc);
        pc += 2;
        return addr;
    }

    // fetch effective address direct
    inline Word fetch_ea_dir()
    {
        Word addr = dpreg.dp16 | memory.read_byte(pc++);
        return addr;
    }

    // fetch indexed address
    inline Word fetch_ea_idx(cycles_t &cycle_count)
    {
        Byte post = memory.read_byte(pc++);
        cycle_count += indexed_cycles[post];
        return do_effective_address(post);
    }

    inline void tst(Word addr)
    {
        Byte m = memory.read_byte(addr);
        tst(m);
    }

    inline void bit(Byte reg, Byte operand)
    {
        Byte m = reg & operand;
        tst(m);
    }

    //**********************************
    // Arithmetic Instructions
    //**********************************

    inline void mul()
    {
        d = a * b;
        cc.bit.c = BTST<Byte>(b, 7U);
        cc.bit.z = !d;
    }

    inline void abx()
    {
        x += b;
    }

    inline void sex()
    {
        cc.bit.n = BTST<Byte>(b, 7U);
        cc.bit.z = !b;
        a = cc.bit.n ? 255 : 0;
    }

    inline void dec(Word addr)
    {
        Byte m = memory.read_byte(addr);
        dec(m);
        memory.write_byte(addr, m);
    }

    inline void inc(Word addr)
    {
        Byte m = memory.read_byte(addr);
        inc(m);
        memory.write_byte(addr, m);
    }

    inline void neg(Word addr)
    {
        Byte m = memory.read_byte(addr);
        neg(m);
        memory.write_byte(addr, m);
    }

    //**********************************
    // Logical Instructions
    //**********************************

    inline void and_(Byte &reg, Byte operand)
    {
        reg &= operand;
        tst(reg);
    }

    inline void or_(Byte &reg, Byte operand)
    {
        reg |= operand;
        tst(reg);
    }

    inline void eor(Byte &reg, Byte operand)
    {
        reg ^= operand;
        tst(reg);
    }

    inline void orcc(Byte operand)
    {
        cc.all |= operand;
    }

    inline void andcc(Byte operand)
    {
        cc.all &= operand;
    }

    //**********************************************
    // (Un)Conditional Jump/Call/Return Instructions
    //**********************************************

    inline void jmp(Word addr)
    {
        pc = addr;
    }

    inline void jsr(Word addr)
    {
        s -= 2;
        memory.write_word(s, pc);
        pc = addr;
    }

    inline void rts()
    {
        pc = memory.read_word(s);
        s += 2;
    }

    inline void do_br(bool condition)
    {
        if (condition)
        {
            pc += EXTEND8(memory.read_byte(pc)) + 1;
        }
        else
        {
            pc++;
        }
    }

    inline cycles_t do_lbr(bool condition)
    {
        if (condition)
        {
            pc += 2 + memory.read_word(pc);
            return 6;
        }

        pc += 2;
        return 5;
    }

    inline void bcc()
    {
        do_br(!cc.bit.c);
    }

    inline cycles_t lbcc()
    {
        return do_lbr(!cc.bit.c);
    }

    inline void bcs()
    {
        do_br(cc.bit.c);
    }

    inline cycles_t lbcs()
    {
        return do_lbr(cc.bit.c);
    }

    inline void beq()
    {
        do_br(cc.bit.z);
    }

    inline cycles_t lbeq()
    {
        return do_lbr(cc.bit.z);
    }

    inline void bge()
    {
        do_br(!(cc.bit.n ^ cc.bit.v));
    }

    inline cycles_t lbge()
    {
        return do_lbr(!(cc.bit.n ^ cc.bit.v));
    }

    inline void bgt()
    {
        do_br(!(static_cast<unsigned>(cc.bit.n ^ cc.bit.v) | cc.bit.z));
    }

    inline cycles_t lbgt()
    {
        return do_lbr(!(static_cast<unsigned>(cc.bit.n ^ cc.bit.v) | cc.bit.z));
    }

    inline void bhi()
    {
        do_br(!(cc.bit.c | cc.bit.z));
    }

    inline cycles_t lbhi()
    {
        return do_lbr(!(cc.bit.c | cc.bit.z));
    }

    inline void ble()
    {
        do_br(static_cast<unsigned>(cc.bit.n ^ cc.bit.v) | cc.bit.z);
    }

    inline cycles_t lble()
    {
        return do_lbr(static_cast<unsigned>(cc.bit.n ^ cc.bit.v) | cc.bit.z);
    }

    inline void bls()
    {
        do_br(cc.bit.c | cc.bit.z);
    }

    inline cycles_t lbls()
    {
        return do_lbr(cc.bit.c | cc.bit.z);
    }

    inline void blt()
    {
        do_br(cc.bit.n ^ cc.bit.v);
    }

    inline cycles_t lblt()
    {
        return do_lbr(cc.bit.n ^ cc.bit.v);
    }

    inline void bmi()
    {
        do_br(cc.bit.n);
    }

    inline cycles_t lbmi()
    {
        return do_lbr(cc.bit.n);
    }

    inline cycles_t lbne()
    {
        return do_lbr(!cc.bit.z);
    }

    inline cycles_t lbpl()
    {
        return do_lbr(!cc.bit.n);
    }

    inline void bne()
    {
        do_br(!cc.bit.z);
    }

    inline void bpl()
    {
        do_br(!cc.bit.n);
    }

    inline void bra()
    {
        pc += EXTEND8(memory.read_byte(pc)) + 1;
    }

    inline cycles_t lbra()
    {
        pc += memory.read_word(pc) + 2;
        return 0;
    }

    inline void brn()
    {
        pc++;
    }

    inline cycles_t lbrn()
    {
        pc += 2;
        return 5;
    }

    inline void bsr()
    {
        Byte offset = memory.read_byte(pc++);
        s -= 2;
        memory.write_word(s, pc);
        pc += EXTEND8(offset);
    }

    inline cycles_t lbsr()
    {
        Word offset = memory.read_word(pc);
        pc += 2;
        s -= 2;
        memory.write_word(s, pc);
        pc += offset;
        return 0;
    }

    inline void bvc()
    {
        do_br(!cc.bit.v);
    }

    inline cycles_t lbvc()
    {
        return do_lbr(!cc.bit.v);
    }

    inline void bvs()
    {
        do_br(cc.bit.v);
    }

    inline cycles_t lbvs()
    {
        return do_lbr(cc.bit.v);
    }

    //**********************************
    // Misc Instructions
    //**********************************

    inline void clr(Word addr)
    {
        Byte m;
        clr(m);
        memory.write_byte(addr, m);
    }

    inline void clr(Byte &reg)
    {
        cc.bit.c = false;
        cc.bit.v = false;
        cc.bit.n = false;
        cc.bit.z = true;
        reg = 0;
    }

    inline void com(Word addr)
    {
        Byte m = memory.read_byte(addr);
        com(m);
        memory.write_byte(addr, m);
    }

    inline void com(Byte &reg)
    {
        reg = ~reg;
        cc.bit.c = true;
        tst(reg);
    }

    inline void nop()
    {
    }

    // additional undocumented instructions
    //
    // undocumented instruction 0x3d:
    // force an internal reset
    inline void rst()
    {
        reset();
    }

    // undocumented instruction 0x01:
    //   If cc.c = -1 then NEG <$xx (op $00)
    //   If cc.c = 0 then COM <$xx (op $03)
    inline void negcom(Word addr)
    {
        if (cc.bit.c)
        {
            com(addr);
        }
        else
        {
            neg(addr);
        }
    }

    // undocumented instruction 0x4E, 0x5E
    // same as CLRA,CLRB but CC.Carry is
    // unchanged
    inline void clr1(Byte &reg)
    {
        cc.bit.v = false;
        cc.bit.n = false;
        cc.bit.z = true;
        reg = 0;
    }

    //**********************************
    // Shift Instructions
    //**********************************
    inline void lsl(Word addr)
    {
        Byte m = memory.read_byte(addr);
        lsl(m);
        memory.write_byte(addr, m);
    }

    inline void asr(Word addr)
    {
        Byte m = memory.read_byte(addr);
        asr(m);
        memory.write_byte(addr, m);
    }

    inline void lsr(Word addr)
    {
        Byte m = memory.read_byte(addr);
        lsr(m);
        memory.write_byte(addr, m);
    }

    inline void lsr(Byte &reg)
    {
        cc.bit.c = BTST<Byte>(reg, 0U);
        reg >>= 1U;
        cc.bit.n = false;
        cc.bit.z = !reg;
    }

    inline void rol(Word addr)
    {
        Byte m = memory.read_byte(addr);
        rol(m);
        memory.write_byte(addr, m);
    }

    inline void rol(Byte &reg)
    {
        bool oc = cc.bit.c;
        cc.bit.c = BTST<Byte>(reg, 7U);
        reg <<= 1U;

        if (oc)
        {
            BSET<Byte>(reg, 0U);
        }

        cc.bit.n = BTST<Byte>(reg, 7U);
        cc.bit.v = cc.bit.c ^ cc.bit.n;
        cc.bit.z = !reg;
    }

    inline void ror(Word addr)
    {
        Byte m = memory.read_byte(addr);
        ror(m);
        memory.write_byte(addr, m);
    }

    inline void ror(Byte &reg)
    {
        bool oc = cc.bit.c;
        cc.bit.c = BTST<Byte>(reg, 0U);
        reg = reg >> 1U;

        if (oc)
        {
            BSET<Byte>(reg, 7U);
        }

        cc.bit.n = BTST<Byte>(reg, 7U);
        cc.bit.z = !reg;
    }

    //**********************************
    // Register Load/Save Instructions
    //**********************************
    inline void ld(Byte &reg, Byte operand)
    {
        reg = operand;
        tst(reg);
    }

    inline void ld(Word &reg, Word operand)
    {
        reg = operand;
        cc.bit.n = BTST<Word>(d, 15U);
        cc.bit.v = false;
        cc.bit.z = !reg;
    }

    inline void st(Byte &reg, Word addr)
    {
        memory.write_byte(addr, reg);
        tst(reg);
    }

    inline void st(Word &reg, Word addr)
    {
        memory.write_word(addr, reg);
        cc.bit.n = BTST<Word>(reg, 15U);
        cc.bit.v = false;
        cc.bit.z = !reg;
    }

    inline void lea(Word &reg, Word addr)
    {
        reg = addr;
        cc.bit.z = !reg;
    }

    static inline void lea_nocc(Word &reg, Word addr)
    {
        reg = addr;
    }

    inline void daa();
    inline void dec(Byte &reg);
    inline void inc(Byte &reg);
    inline void lsl(Byte &reg);
    inline void neg(Byte &reg);
    inline void tst(Byte reg);
    inline void cmp(Byte reg, Byte operand);
    inline void cmp(Word reg, Word operand);
    inline void adc(Byte &reg, Byte operand);
    inline void add(Byte &reg, Byte operand);
    inline void add(Word &reg, Word operand);
    inline void sbc(Byte &reg, Byte operand);
    inline void sub(Byte &reg, Byte operand);
    inline void sub(Word &reg, Word operand);
    inline void swi(), swi2(), swi3();
    inline void cwai(), sync();
    inline cycles_t rti();
    inline void asr(Byte &reg);
    void tfr();
    void exg();
    cycles_t psh(Byte what, Word &s, Word &u);
    cycles_t pul(Byte what, Word &s, Word &u);
    Word do_effective_address(Byte post);
#endif
    void EXEC_NMI(bool save_state);
    void EXEC_IRQ(bool save_state);
    void EXEC_FIRQ(bool save_state);
    void nmi(bool save_state);
    void firq(bool save_state);
    void irq(bool save_state);
    void invalid(const char *pmessage);
    bool use_undocumented{false};
public:
    void set_use_undocumented(bool value);
    bool is_use_undocumented() const
    {
        return use_undocumented;
    };

    // Scheduler Interface implemenation
public:
    void do_reset() override;
    CpuState run(RunMode mode) override;
    void exit_run() override;
    QWord get_cycles(bool reset = false) override;
    void get_status(CpuStatus *p_cpu_status) override;
    CpuStatusPtr create_status_object() override;
    void get_interrupt_status(tInterruptStatus &s) override;
    void set_required_cyclecount(cycles_t p_cycles) override;

    // test support
    void set_status(CpuStatus *p_cpu_status);
protected:
    CpuState runloop();

    // interrupt handling:
public:
    void reset(); // CPU reset
    inline cycles_t exec_irqs(bool save_state = true);
    void set_nmi();
    void set_firq();
    void set_irq();

protected:
    std::atomic<QWord> total_cycles{}; // total cycle count with 64 Bit resolution
    cycles_t cycles{}; // cycle cnt for one timer tick
    std::atomic<cycles_t> required_cyclecount{};//cycle count for freq ctrl

    // breakpoint support
protected:
    std::array<OptionalWord, 3> bp;
public:
    void set_bp(int which, Word address);
    OptionalWord get_bp(int which);
    bool is_bp_set(int which);
    void reset_bp(int which);

    // interface to other classes
public:
    void set_disassembler(Da6809 *p_disassembler);
    bool setLoggerConfig(const Mc6809LoggerConfig &loggerConfig);
    Word get_pc()
    {
        return PC;
    }

public:
    // BObserver interface
    void UpdateFrom(NotifyId id, void *param = nullptr) override;

protected:
    unsigned Disassemble(Word address, InstFlg &p_flags,
                    std::string &code, std::string &mnemonic,
                    std::string &operands);
    Mc6809Logger logger;
    Memory &memory;

    // Public constructor and destructor
public:
    explicit Mc6809(Memory &p_memory);
    ~Mc6809() override = default;
};

//*******************************************************************
// Instruction execution
//*******************************************************************

#ifndef FASTFLEX

#ifdef USE_GCCASM
inline void Mc6809::add(Byte &reg, Byte operand)
{
    long x86flags = 0;
    Byte mask = 0;

    asm volatile(\
                 "addb %5,%0;"    // Execute 8-bit addition
                 "pushf;"         // Push x86 condition code register
                 "pop %2;"        // read condition code into x86flags
                 "bt $4,%2;"      // Copy bit AF into carry (6809: H bit)
                 "rclb %3;"       // rotate carry into mask
                 "shlb %3;"       // rotate 0 into mask
                 "bt $7,%2;"      // copy bit SF into carry (6809: N bit)
                 "rclb %3;"       // rotate carry into mask
                 "bt $6,%2;"      // copy bit ZF into carry (6809: Z bit)
                 "rclb %3;"       // rotate carry into mask
                 "bt $11,%2;"     // copy bit OF into carry (6809: V bit)
                 "rclb %3;"       // rotate carry into mask
                 "rcr %2;"        // rotate CF into carry (6809: C bit)
                 "rclb %3;"       // rotate carry into mask
                 "andb $0xd0,%1;" // keep E, F and I bit unchanged
                 "orb %3,%1;"     // merge Bits H, N, Z, V and C into cc.all
                 : "=qm"(reg), "+m"(cc.all), "+rm"(x86flags), "+r"(mask)
                 : "0"(reg), "qm"(operand)
                 : "cc");
}

inline void Mc6809::add(Word &reg, Word operand)
{
    long x86flags = 0;
    Byte mask = 0;

    asm volatile(\
                 "addw %5,%0;"    // Execute 16-bit addition
                 "pushf;"         // Push x86 condition code register
                 "pop %2;"        // read condition code into x86flags
                 "bt $7,%2;"      // copy bit SF into carry (6809: N bit)
                 "rclb %3;"       // rotate carry into mask
                 "bt $6,%2;"      // copy bit ZF into carry (6809: Z bit)
                 "rclb %3;"       // rotate carry into mask
                 "bt $11,%2;"     // copy bit OF into carry (6809: V bit)
                 "rclb %3;"       // rotate carry into mask
                 "rcr %2;"        // rotate CF into carry (6809: C bit)
                 "rclb %3;"       // rotate carry into mask
                 "andb $0xf0,%1;" // keep E, F, H and I bit unchanged
                 "orb %3,%1;"     // merge Bits N, Z, V and C into cc.all
                 : "=qm"(reg), "+m"(cc.all), "+rm"(x86flags), "+r"(mask)
                 : "0"(reg), "qm"(operand)
                 : "cc");
}

inline void Mc6809::sub(Byte &reg, Byte operand)
{
    long x86flags = 0;
    Byte mask = 0;

    asm volatile(\
                 "subb %5,%0;"    // Execute 16-bit subtraction
                 "pushf;"         // Push x86 condition code register
                 "pop %2;"        // read condition code into x86flags
                 "bt $7,%2;"      // copy bit SF into carry (6809: N bit)
                 "rclb %3;"       // rotate carry into mask
                 "bt $6,%2;"      // copy bit ZF into carry (6809: Z bit)
                 "rclb %3;"       // rotate carry into mask
                 "bt $11,%2;"     // copy bit OF into carry (6809: V bit)
                 "rclb %3;"       // rotate carry into mask
                 "rcr %2;"        // rotate CF into carry (6809: C bit)
                 "rclb %3;"       // rotate carry into mask
                 "andb $0xf0,%1;" // keep E, F, H and I bit unchanged
                 "orb %3,%1;"     // merge Bits N, Z, V and C into cc.all
                 : "=qm"(reg), "+m"(cc.all), "+rm"(x86flags), "+r"(mask)
                 : "0"(reg), "qm"(operand)
                 : "cc");
}

inline void Mc6809::sub(Word &reg, Word operand)
{
    long x86flags = 0;
    Byte mask = 0;

    asm volatile(\
                 "subw %5,%0;"    // Execute 16-bit subtraction
                 "pushf;"         // Push x86 condition code register
                 "pop %2;"        // read condition code into x86flags
                 "bt $7,%2;"      // copy bit SF into carry (6809: N bit)
                 "rclb %3;"       // rotate carry into mask
                 "bt $6,%2;"      // copy bit ZF into carry (6809: Z bit)
                 "rclb %3;"       // rotate carry into mask
                 "bt $11,%2;"     // copy bit OF into carry (6809: V bit)
                 "rclb %3;"       // rotate carry into mask
                 "rcr %2;"        // rotate CF into carry (6809: C bit)
                 "rclb %3;"       // rotate carry into mask
                 "andb $0xf0,%1;" // keep E, F, H and I bit unchanged
                 "orb %3,%1;"     // merge Bits N, Z, V and C into cc.all
                 : "=qm"(reg), "+m"(cc.all), "+rm"(x86flags), "+r"(mask)
                 : "0"(reg), "qm"(operand)
                 : "cc");
}
#else
inline void Mc6809::add(Byte &reg, Byte operand)
{
    Word sum = reg + operand;
    cc.bit.n = BTST<Word>(sum, 7U);
    cc.bit.v = BTST<Word>(reg ^ operand ^ sum ^ (sum >> 1), 7U);
    cc.bit.c = BTST<Word>(sum, 8U);
    cc.bit.h = BTST<Word>(reg ^ operand ^ sum, 4U);
    reg = (Byte)sum;
    cc.bit.z = !reg;
}

inline void Mc6809::add(Word &reg, Word operand)
{
    DWord sum = (DWord)reg + operand;
    cc.bit.n = BTST<DWord>(sum, 15U);
    cc.bit.v = BTST<DWord>(reg ^ operand ^ sum ^ (sum >> 1), 15U);
    cc.bit.c = BTST<DWord>(sum, 16U);
    reg = (Word)sum;
    cc.bit.z = !reg;
}

inline void Mc6809::sub(Byte &reg, Byte operand)
{
    Word diff = reg - operand;
    cc.bit.n = BTST<Word>(diff, 7U);
    cc.bit.v = BTST<Word>(reg ^ operand ^ diff ^ (diff >> 1), 7U);
    cc.bit.c = BTST<Word>(diff, 8U);
    reg = (Byte)diff;
    cc.bit.z = !reg;
}

inline void Mc6809::sub(Word &reg, Word operand)
{
    DWord diff = reg - operand;
    cc.bit.n = BTST<DWord>(diff, 15U);
    cc.bit.v = BTST<DWord>(reg ^ operand ^ diff ^ (diff >> 1), 15U);
    cc.bit.c = BTST<DWord>(diff, 16U);
    reg = (Word)diff;
    cc.bit.z = !reg;
}
#endif

#ifdef USE_GCCASM
inline void Mc6809::adc(Byte &reg, Byte operand)
{
    long x86flags = 0;
    Byte mask = 0;

    asm volatile(\
                 "btw $0,%1;"     // Copy 6809 carry bit into condition flags
                 "adcb %5,%0;"    // Execute 8-bit addition
                 "pushf;"         // Push x86 condition code register
                 "pop %2;"        // read condition code into x86flags
                 "bt $4,%2;"      // Copy bit AF into carry (6809: H bit)
                 "rclb %3;"       // rotate carry into mask
                 "shlb %3;"       // rotate 0 into mask
                 "bt $7,%2;"      // copy bit SF into carry (6809: N bit)
                 "rclb %3;"       // rotate carry into mask
                 "bt $6,%2;"      // copy bit ZF into carry (6809: Z bit)
                 "rclb %3;"       // rotate carry into mask
                 "bt $11,%2;"     // copy bit OF into carry (6809: V bit)
                 "rclb %3;"       // rotate carry into mask
                 "rcr %2;"        // rotate CF into carry (6809: C bit)
                 "rclb %3;"       // rotate carry into mask
                 "andb $0xd0,%1;" // keep E, F and I bit unchanged
                 "orb %3,%1;"     // merge Bits H, N, Z, V and C into cc.all
                 : "=qm"(reg), "+m"(cc.all), "+rm"(x86flags), "+r"(mask)
                 : "0"(reg), "qm"(operand)
                 : "cc");
}

inline void Mc6809::sbc(Byte &reg, Byte operand)
{
    long x86flags = 0;
    Byte mask = 0;

    asm volatile(\
                 "btw $0,%1;"     // Copy 6809 carry bit into condition flags
                 "sbbb %5,%0;"    // Execute 8-bit subtraction with borrow
                 "pushf;"         // Push x86 condition code register
                 "pop %2;"        // read condition code into x86flags
                 "bt $7,%2;"      // copy bit SF into carry (6809: N bit)
                 "rclb %3;"       // rotate carry into mask
                 "bt $6,%2;"      // copy bit ZF into carry (6809: Z bit)
                 "rclb %3;"       // rotate carry into mask
                 "bt $11,%2;"     // copy bit OF into carry (6809: V bit)
                 "rclb %3;"       // rotate carry into mask
                 "rcr %2;"        // rotate CF into carry (6809: C bit)
                 "rclb %3;"       // rotate carry into mask
                 "andb $0xf0,%1;" // keep E, F, H and I bit unchanged
                 "orb %3,%1;"     // merge Bits H, N, Z, V and C into cc.all
                 : "=qm"(reg), "+m"(cc.all), "+rm"(x86flags), "+r"(mask)
                 : "0"(reg), "qm"(operand)
                 : "cc");
}
#else
inline void Mc6809::adc(Byte &reg, Byte operand)
{
    Word sum = reg + operand + cc.bit.c;
    cc.bit.n = BTST<Word>(sum, 7U);
    cc.bit.v = BTST<Word>(reg ^ operand ^ sum ^ (sum >> 1), 7U);
    cc.bit.c = BTST<Word>(sum, 8U);
    cc.bit.h = BTST<Word>(reg ^ operand ^ sum, 4U);
    reg = (Byte)sum;
    cc.bit.z = !reg;
}

inline void Mc6809::sbc(Byte &reg, Byte operand)
{
    Word diff = reg - operand - cc.bit.c;
    cc.bit.n = BTST<Word>(diff, 7U);
    cc.bit.v = BTST<Word>(reg ^ operand ^ diff ^ (diff >> 1), 7U);
    cc.bit.c = BTST<Word>(diff, 8U);
    reg = (Byte)diff;
    cc.bit.z = !reg;
}
#endif

inline void Mc6809::daa()
{
    Word t;
    Word c = 0;
    Byte lsn = a & 0x0FU;
    Byte msn = a & 0xF0U;

    if (cc.bit.h || (lsn > 9))
    {
        c |= 0x06U;
    }

    if (cc.bit.c    ||
        (msn > 0x90) ||
        ((msn > 0x80) && (lsn > 9)))
    {
        c |= 0x60U;
    }

    t = c + a;
    a = static_cast<Byte>(t);

    if (BTST<Word>(t, 8U))
    {
        cc.bit.c = true;
    }

    cc.bit.n = BTST<Byte>(a, 7U);
    cc.bit.z = !a;
}

#ifdef USE_GCCASM
inline void Mc6809::dec(Byte &reg)
{
    long x86flags = 0;
    Byte mask = 0;

    asm volatile(\
                 "decb %0;"       // Execute 8-bit increment
                 "pushf;"         // Push x86 condition code register
                 "pop %2;"        // read condition code into x86flags
                 "bt $7,%2;"      // copy bit SF into carry (6809: N bit)
                 "rclb %3;"       // rotate carry into mask
                 "bt $6,%2;"      // copy bit ZF into carry (6809: Z bit)
                 "rclb %3;"       // rotate carry into mask
                 "bt $11,%2;"     // copy bit OF into carry (6809: V bit)
                 "rclb %3;"       // rotate carry into mask
                 "shlb %3;"       // shift 0 into mask
                 "andb $0xf1,%1;" // keep E, F, H, I and C bit unchanged
                 "orb %3,%1;"     // merge Bits N, Z and V into cc.all
                 : "=qm"(reg), "+m"(cc.all), "+rm"(x86flags), "+r"(mask)
                 : "0"(reg)
                 : "cc");
}

inline void Mc6809::inc(Byte &reg)
{
    long x86flags = 0;
    Byte mask = 0;

    asm volatile(\
                 "incb %0;"       // Execute 8-bit increment
                 "pushf;"         // Push x86 condition code register
                 "pop %2;"        // read condition code into x86flags
                 "bt $7,%2;"      // copy bit SF into carry (6809: N bit)
                 "rclb %3;"       // rotate carry into mask
                 "bt $6,%2;"      // copy bit ZF into carry (6809: Z bit)
                 "rclb %3;"       // rotate carry into mask
                 "bt $11,%2;"     // copy bit OF into carry (6809: V bit)
                 "rclb %3;"       // rotate carry into mask
                 "shlb %3;"       // shift 0 into mask
                 "andb $0xf1,%1;" // keep E, F, H, I and C bit unchanged
                 "orb %3,%1;"     // merge Bits N, Z and V into cc.all
                 : "=qm"(reg), "+m"(cc.all), "+rm"(x86flags), "+r"(mask)
                 : "0"(reg)
                 : "cc");
}

inline void Mc6809::neg(Byte &reg)
{
    long x86flags = 0;
    Byte mask = 0;

    asm volatile(\
                 "negb %0;"       // Execute 8-bit negation
                 "pushf;"         // Push x86 condition code register
                 "pop %2;"        // read condition code into x86flags
                 "bt $7,%2;"      // copy SF into carry (6809: N bit)
                 "rclb %3;"       // rotate carry into mask
                 "bt $6,%2;"      // copy ZF into carry (6809: Z bit)
                 "rclb %3;"       // rotate carry into mask
                 "bt $11,%2;"     // copy OF into carry (6809: V bit)
                 "rclb %3;"       // rotate carry into mask
                 "rcr %2;"        // rotate CF into carry (6809: C bit)
                 "rclb %3;"       // rotate carry into mask
                 "andb $0xf0,%1;" // keep E, F, H and I bit unchanged
                 "orb %3,%1;"     // merge Bits N, Z, V and C into cc.all
                 : "=qm"(reg), "+m"(cc.all), "+rm"(x86flags), "+r"(mask)
                 : "0"(reg)
                 : "cc");
}
#else
inline void Mc6809::dec(Byte &reg)
{
    reg--;
    cc.bit.n = BTST<Byte>(reg, 7U);
    cc.bit.z = !reg;
    cc.bit.v = (reg == 0x7f);
}

inline void Mc6809::inc(Byte &reg)
{
    reg++;
    cc.bit.n = BTST<Byte>(reg, 7U);
    cc.bit.z = !reg;
    cc.bit.v = (reg == 0x80);
}

inline void Mc6809::neg(Byte &reg)
{
    // Sw: fixed carry bug
    cc.bit.v = (reg == 0x80);
    cc.bit.c = (reg != 0);
    reg = (~reg) + 1;
    cc.bit.n = BTST<Byte>(reg, 7U);
    cc.bit.z = !reg;
}
#endif

//**********************************
// Compare Instructions
//**********************************

#ifdef USE_GCCASM
inline void Mc6809::cmp(Byte reg, Byte operand)
{
    long x86flags = 0;
    Byte mask = 0;

    asm volatile(\
                 "cmpb %4,%3;"    // Execute 8-bit comparison
                 "pushf;"         // Push x86 condition code register
                 "pop %1;"        // read condition code into x86flags
                 "bt $7,%1;"      // copy SF into carry (6809: N bit)
                 "rclb %2;"       // rotate carry into mask
                 "bt $6,%1;"      // copy ZF into carry (6809: Z bit)
                 "rclb %2;"       // rotate carry into mask
                 "bt $11,%1;"     // copy OF into carry (6809: V bit)
                 "rclb %2;"       // rotate carry into mask
                 "rcr %1;"        // rotate CF into carry (6809: C bit)
                 "rclb %2;"       // rotate carry into mask
                 "andb $0xf0,%0;" // keep E, F, H and I bit unchanged
                 "orb %2,%0;"     // merge Bits N, Z, V and C into cc.all
                 : "+m"(cc.all), "+rm"(x86flags), "+r"(mask)
                 : "q"(reg), "m"(operand)
                 : "cc");
}

inline void Mc6809::cmp(Word reg, Word operand)
{
    long x86flags = 0;
    Byte mask = 0;

    asm volatile(\
                 "cmpw %4,%3;"    // Execute 8-bit comparison
                 "pushf;"         // Push x86 condition code register
                 "pop %1;"        // read condition code into x86flags
                 "bt $7,%1;"      // copy SF into carry (6809: N bit)
                 "rclb %2;"       // rotate carry into mask
                 "bt $6,%1;"      // copy ZF into carry (6809: Z bit)
                 "rclb %2;"       // rotate carry into mask
                 "bt $11,%1;"     // copy OF into carry (6809: V bit)
                 "rclb %2;"       // rotate carry into mask
                 "rcr %1;"        // rotate CF into carry (6809: C bit)
                 "rclb %2;"       // rotate carry into mask
                 "andb $0xf0,%0;" // keep E, F, H and I bit unchanged
                 "orb %2,%0;"     // merge Bits N, Z, V and C into cc.all
                 : "+m"(cc.all), "+rm"(x86flags), "+r"(mask)
                 : "q"(reg), "m"(operand)
                 : "cc");
}

inline void Mc6809::tst(Byte reg)
{
    long x86flags = 0;
    Byte mask = 0;

    asm volatile(\
                 "testb $255,%3;" // Execute 8-bit logic and with 0xFF
                 // and skip result
                 "pushf;"         // Push x86 condition code register
                 "pop %1;"        // read condition code into x86flags
                 "bt $7,%1;"      // copy bit SF into carry (6809: N bit)
                 "rclb %2;"       // rotate carry into mask
                 "bt $6,%1;"      // copy bit ZF into carry (6809: Z bit)
                 "rclb %2;"       // rotate carry into mask
                 "shlb $2,%2;"    // rotate 00 into mask (6809: V always 0)
                 "andb $0xf1,%0;" // keep E, F, H, I and C bit unchanged
                 "orb %2,%0;"     // merge Bits N, Z and V into cc.all
                 : "+m"(cc.all), "+rm"(x86flags), "+r"(mask)
                 : "rm"(reg)
                 : "cc");
}
#else
inline void Mc6809::cmp(Byte reg, Byte operand)
{
    Word diff = reg - operand;
    cc.bit.n = BTST<Word>(diff, 7U);
    cc.bit.v = BTST<Word>(reg ^ operand ^ diff ^ (diff >> 1), 7U);
    cc.bit.z = !(diff & 0xFF);
    cc.bit.c = BTST<Word>(diff, 8U);
}

inline void Mc6809::cmp(Word reg, Word operand)
{
    DWord diff = reg - operand;
    cc.bit.n = BTST<DWord>(diff, 15U);
    cc.bit.v = BTST<DWord>(reg ^ operand ^ diff ^ (diff >> 1), 15U);
    cc.bit.z = !(diff & 0xFFFF);
    cc.bit.c = BTST<DWord>(diff, 16U);
}

inline void Mc6809::tst(Byte reg)
{
    cc.bit.n = BTST<Byte>(reg, 7U);
    cc.bit.v = 0;
    cc.bit.z = !reg;
}

#endif

#ifdef USE_GCCASM
inline void Mc6809::lsl(Byte &reg)
{
    long x86flags = 0;
    Byte mask = 0;

    asm volatile(\
                 "shlb %0;"       // Execute 8-bit logical shift left
                 "pushf;"         // Push x86 condition code register
                 "pop %2;"        // read condition code into x86flags
                 "bt $7,%2;"      // copy bit SF into carry (6809: N bit)
                 "rclb %3;"       // rotate carry into mask
                 "bt $6,%2;"      // copy bit ZF into carry (6809: Z bit)
                 "rclb %3;"       // rotate carry into mask
                 "bt $11,%2;"     // copy bit OF into carry (6809: V bit)
                 "rclb %3;"       // rotate carry into mask
                 "rcr %2;"        // rotate CF into carry (6809: C bit)
                 "rclb %3;"       // rotate carry into mask
                 "andb $0xf0,%1;" // keep E, F, H, and I bit unchanged
                 "orb %3,%1;"     // merge Bits N, Z, V and C into cc.all
                 : "=qm"(reg), "+m"(cc.all), "+rm"(x86flags), "+r"(mask)
                 : "0"(reg)
                 : "cc");
}

inline void Mc6809::asr(Byte &reg)
{
    long x86flags = 0;
    Byte mask = 0;

    asm volatile(\
                 "sarb %0;"       // Execute 8-bit arithmetic shift right
                 "pushf;"         // Push x86 condition code register
                 "pop %2;"        // read condition code into x86flags
                 "bt $7,%2;"      // copy bit SF into carry (6809: N bit)
                 "rclb %3;"       // rotate carry into mask
                 "bt $6,%2;"      // copy bit ZF into carry (6809: Z bit)
                 "rclb %3;"       // rotate carry into mask
                 "shlb %3;"       // rotate 0 into mask
                 "rcr %2;"        // rotate CF into carry (6809: C bit)
                 "rclb %3;"       // rotate carry into mask
                 "andb $0xf2,%1;" // keep E, F, H, I and V bit unchanged
                 "orb %3,%1;"     // merge Bits N, Z and C into cc.all
                 : "=qm"(reg), "+m"(cc.all), "+rm"(x86flags), "+r"(mask)
                 : "0"(reg)
                 : "cc");
}
#else
inline void Mc6809::lsl(Byte &reg)
{
    cc.bit.c = BTST<Byte>(reg, 7U);
    cc.bit.v = BTST<Byte>(reg ^ (reg << 1), 7U);
    reg <<= 1;
    cc.bit.n = BTST<Byte>(reg, 7U);
    cc.bit.z = !reg;
}

inline void Mc6809::asr(Byte &reg)
{
    cc.bit.c = BTST<Byte>(reg, 0U);
    reg >>= 1;
    cc.bit.n = BTST<Byte>(reg, 6U);

    if (cc.bit.n)
    {
        BSET<Byte>(reg, 7U);
    }

    cc.bit.z = !reg;
}
#endif

//**********************************
// Interrupt related Instructions
//**********************************
inline cycles_t Mc6809::rti()
{
    cc.all = memory.read_byte(s++);

    if (cc.bit.e)
    {
        pul(0xfe, s, u);
        return 15;
    }

    pc = memory.read_word(s);
    s += 2;
    return 6;
}

inline void Mc6809::sync()
{
    pc--; // processor loops in sync instruction until interrupt
    events |= Event::Sync;
}

inline void Mc6809::cwai()
{
    cc.all &= memory.read_byte(pc++);
    cc.bit.e = true;
    psh(0xff, s, u);
    events |= Event::Cwai;
    pc -= 2; // processor loops in cwai instruction until interrupt
}

inline void Mc6809::swi()
{
    cc.bit.e = true;
    psh(0xff, s, u);
    cc.bit.f = cc.bit.i = true;
    pc = memory.read_word(0xfffa);
}

inline void Mc6809::swi2()
{
    cc.bit.e = true;
    psh(0xff, s, u);
    pc = memory.read_word(0xfff4);
}

inline void Mc6809::swi3()
{
    cc.bit.e = true;
    psh(0xff, s, u);
    pc = memory.read_word(0xfff2);
}


#endif // ifndef FASTFLEX

inline Mc6809::Event operator| (Mc6809::Event lhs, Mc6809::Event rhs)
{
    using T1 = std::underlying_type_t<Mc6809::Event>;

    return static_cast<Mc6809::Event>(static_cast<T1>(lhs) |
                                      static_cast<T1>(rhs));
}

inline Mc6809::Event operator& (Mc6809::Event lhs, Mc6809::Event rhs)
{
    using T1 = std::underlying_type_t<Mc6809::Event>;

    return static_cast<Mc6809::Event>(static_cast<T1>(lhs) &
                                      static_cast<T1>(rhs));
}

inline Mc6809::Event operator~ (Mc6809::Event rhs)
{
    using T1 = std::underlying_type_t<Mc6809::Event>;

    return static_cast<Mc6809::Event>(~static_cast<T1>(rhs));
}

inline bool operator! (Mc6809::Event rhs)
{
    using T1 = std::underlying_type_t<Mc6809::Event>;

    return static_cast<T1>(rhs) == 0;
}

#endif // MC6809_INCLUDED
