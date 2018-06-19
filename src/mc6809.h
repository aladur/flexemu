/*
    mc6809.h

    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2018  W. Schwotzer

    This file is based on usim-0.91 which is
    Copyright (C) 1994 by R. B. Bellis
*/



#ifndef __mc6809_h__
#define __mc6809_h__

#include <stdio.h>
#include <atomic>
#include "misc1.h"
#include "typedefs.h"
#include "memory.h"
#include "schedcpu.h"
#include "scpulog.h"
#include "cpustate.h"
#include "flexemu.h"

#define USE_ASM 1

// test using GCC inline assembler
#ifdef __CPU
    #if __CPU==i386 || __CPU==i486 || __CPU==i586 || __CPU==i686 || __CPU==i786
        #define __IX86
    #endif
#endif
#if defined(__GNUC__) && defined (USE_ASM) && defined(__IX86) && \
    defined(__LP64__) && !defined(FASTFLEX)
    #define USE_GCCASM
#endif

#define DO_NMI          0x01
#define DO_FIRQ         0x02
#define DO_IRQ          0x04
#define DO_INVALID      0x08
#define DO_BREAKPOINT       0x10
#define DO_SINGLESTEP       0x20
#define DO_SINGLESTEPFINISHED   0x40
#define DO_SYNCEXEC     0x80
#define DO_TIMER        0x100
#define DO_SET_STATUS       0x200
#define DO_FREQ_CONTROL     0x400
#define DO_GO_BACK      0x800
#define DO_LOG          0x1000
#define DO_CWAI         0x2000
#define DO_SYNC         0x4000
#define IGNORE_BP               0x8000

#define SINGLESTEP_OVER     0x54
#define SINGLESTEP_INTO     0x55
#define START_RUNNING       0x56
#define CONTINUE_RUNNING    0x57


#define SYNC_INSTR      1
#define CWAI_INSTR      2

#ifdef BITFIELDS_LSB_FIRST
    #define CC_BIT_C      0x01
    #define CC_BIT_V      0x02
    #define CC_BIT_Z      0x04
    #define CC_BIT_N      0x08
    #define CC_BIT_I      0x10
    #define CC_BIT_H      0x20
    #define CC_BIT_F      0x40
    #define CC_BIT_E      0x80
#else
    #define CC_BIT_C      0x80
    #define CC_BIT_V      0x40
    #define CC_BIT_Z      0x20
    #define CC_BIT_N      0x10
    #define CC_BIT_I      0x08
    #define CC_BIT_H      0x04
    #define CC_BIT_F      0x02
    #define CC_BIT_E      0x01
#endif
#define CC_BITS_HNZVC (CC_BIT_H | CC_BIT_N | CC_BIT_Z | CC_BIT_V | CC_BIT_C)
#define CC_BITS_NZVC  (CC_BIT_N | CC_BIT_Z | CC_BIT_V | CC_BIT_C)
#define CC_BITS_NZV   (CC_BIT_N | CC_BIT_Z | CC_BIT_V)
#define CC_BITS_NZC   (CC_BIT_N | CC_BIT_Z | CC_BIT_C)

#ifdef FASTFLEX
    #define PC ipcreg
    #define CC_BITI (iccreg & 0x10)
    #define CC_BITF (iccreg & 0x40)
    #include "engine.h"
#else
    #define PC pc
    #define CC_BITI (cc.bit.i)
    #define CC_BITF (cc.bit.f)
    #define EXEC_NMI(s)  nmi(s)
    #define EXEC_IRQ(s)  irq(s)
    #define EXEC_FIRQ(s) firq(s)
#endif

// uncomment this for debug output of cpu
//#define   DEBUG_FILE  "cpu.txt"


union uacc
{
    Word            d;  // Combined accumulator
    struct
    {
#ifdef WORDS_BIGENDIAN
        Byte        a;  // Accumulator a
        Byte        b;  // Accumulator b
#else
        Byte        b;  // Accumulator b
        Byte        a;  // Accumulator a
#endif
    } byte;
};

union udpreg
{
    Word            dp16;
    struct
    {
#ifdef WORDS_BIGENDIAN
        Byte        h;
        Byte        l;
#else
        Byte        l;
        Byte        h;
#endif
    } byte;
};

union ucc
{
    Byte            all;    // Condition code register
    struct
    {
#ifdef BITFIELDS_LSB_FIRST
        bool        c : 1;  // Carry
        bool        v : 1;  // Overflow
        bool        z : 1;  // Zero
        bool        n : 1;  // Negative
        bool        i : 1;  // IRQ disable
        bool        h : 1;  // Half carry
        bool        f : 1;  // FIRQ disable
        bool        e : 1;  // Entire
#else
        bool        e : 1;  // Entire
        bool        f : 1;  // FIRQ disable
        bool        h : 1;  // Half carry
        bool        i : 1;  // IRQ disable
        bool        n : 1;  // Negative
        bool        z : 1;  // Zero
        bool        v : 1;  // Overflow
        bool        c : 1;  // Carry
#endif
    } bit;
};

union ux86flags
{
    DWord           all;
    struct
    {
#ifdef WORDS_BIGENDIAN
        Word        h;
        Word        l;
#else
        Word        l;
        Word        h;
#endif
    } word;
    struct
    {
#ifdef BITFIELDS_LSB_FIRST
        bool        cf    : 1; // carry flag
        bool        one   : 1; // always one
        bool        pf    : 1; // parity flag
        bool        zero1 : 1; // always zero
        bool        af    : 1; // auxilliary carry flag
        bool        zero2 : 1; // always zero
        bool        zf    : 1; // zero flag
        bool        sf    : 1; // sign flag
        bool        tf    : 1; // trap flag
        bool        _if   : 1; // interrupt enable flag
        bool        df    : 1; // direction flag
        bool        of    : 1; // overflow flag
        bool        iopl0 : 1; // i/o previlidge level
        bool        iopl1 : 1; // i/o previlidge level
        bool        nt    : 1; // nested task
        bool        zero3 : 1; // always zero
        int         dummy : 16; // fill up to 32-bit
#else
        int         dummy : 16; // fill up to 32-bit
        bool        zero3 : 1; // always zero
        bool        nt    : 1; // nested task
        bool        iopl1 : 1; // i/o previlidge level
        bool        iopl0 : 1; // i/o previlidge level
        bool        of    : 1; // overflow flag
        bool        df    : 1; // direction flag
        bool        _if   : 1; // interrupt enable flag
        bool        tf    : 1; // trap flag
        bool        sf    : 1; // sign flag
        bool        zf    : 1; // zero flag
        bool        zero2 : 1; // always zero
        bool        af    : 1; // auxilliary carry flag
        bool        zero1 : 1; // always zero
        bool        pf    : 1; // parity flag
        bool        one   : 1; // always one
        bool        cf    : 1; // carry flag
#endif
    } bit;
};

class BTime;
class Da6809;
class Mc6809CpuStatus;


class Mc6809 : public ScheduledCpu
{

    // Processor registers
protected:

    Byte            indexed_cycles[256];    // add. cycles for
    // indexed addr.
    Byte            psh_pul_cycles[256];    // add. cycles for psh
    // and pull-instr.
    Byte            nmi_armed;      // for handling
    // interrupts
    std::atomic<Word> events; // event status flags (atomic access)
    tInterruptStatus    interrupt_status;
#ifdef FASTFLEX
    Word            ipcreg, iureg, isreg, ixreg, iyreg;
    Byte            iareg, ibreg, iccreg, idpreg;
    Word            eaddr;
    Byte            ireg;
    Byte            iflag;
    Byte            tb;
    Word            tw;
    Byte            k;
    Byte            *pMem;      // needed for memory access
#else
    Word            pc;
    Word            u, s;       // Stack pointers
    Word            x, y;       // Index registers
    union uacc acc;
    union udpreg dpreg;
    Byte           &a;
    Byte           &b;
    Word           &d;
    Byte           &dp;
    union ucc cc;
#ifdef USE_GCCASM
    union ux86flags x86flags;
#endif
#endif // #ifndef FASTFLEX

protected:

    Da6809     *disassembler;

    // funcitons for instruction execution:

private:

    void            init();
    void            init_indexed_cycles();
    void            init_psh_pul_cycles();
    void            illegal();

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
    inline Word fetch_idx_16(t_cycles *cycles)
    {
        Word addr;
        Byte post;

        post = memory.read_byte(pc++);
        addr = do_effective_address(post);
        *cycles += indexed_cycles[post];
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
    inline Byte fetch_idx_08(t_cycles *cycles)
    {
        Word addr;
        Byte post;

        post = memory.read_byte(pc++);
        addr = do_effective_address(post);
        *cycles += indexed_cycles[post];
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
    inline Word fetch_ea_idx(t_cycles *cycles)
    {
        Byte post = memory.read_byte(pc++);
        *cycles += indexed_cycles[post];
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
        cc.bit.c = BTST7(b);
        cc.bit.z = !d;
    }

    inline void abx()
    {
        x += b;
    }

    inline void sex()
    {
        cc.bit.n = BTST7(b);
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

    inline t_cycles do_lbr(bool condition)
    {
        if (condition)
        {
            pc += 2 + memory.read_word(pc);
            return 6;
        }
        else
        {
            pc += 2;
            return 5;
        }
    }

    inline void bcc()
    {
        do_br(!cc.bit.c);
    }

    inline t_cycles lbcc()
    {
        return do_lbr(!cc.bit.c);
    }

    inline void bcs()
    {
        do_br(cc.bit.c);
    }

    inline t_cycles lbcs()
    {
        return do_lbr(cc.bit.c);
    }

    inline void beq()
    {
        do_br(cc.bit.z);
    }

    inline t_cycles lbeq()
    {
        return do_lbr(cc.bit.z);
    }

    inline void bge()
    {
        do_br(!(cc.bit.n ^ cc.bit.v));
    }

    inline t_cycles lbge()
    {
        return do_lbr(!(cc.bit.n ^ cc.bit.v));
    }

    inline void bgt()
    {
        do_br(!(cc.bit.z | (cc.bit.n ^ cc.bit.v)));
    }

    inline t_cycles lbgt()
    {
        return do_lbr(!(cc.bit.z | (cc.bit.n ^ cc.bit.v)));
    }

    inline void bhi()
    {
        do_br(!(cc.bit.c | cc.bit.z));
    }

    inline t_cycles lbhi()
    {
        return do_lbr(!(cc.bit.c | cc.bit.z));
    }

    inline void ble()
    {
        do_br(cc.bit.z | (cc.bit.n ^ cc.bit.v));
    }

    inline t_cycles lble()
    {
        return do_lbr(cc.bit.z | (cc.bit.n ^ cc.bit.v));
    }

    inline void bls()
    {
        do_br(cc.bit.c | cc.bit.z);
    }

    inline t_cycles lbls()
    {
        return do_lbr(cc.bit.c | cc.bit.z);
    }

    inline void blt()
    {
        do_br(cc.bit.n ^ cc.bit.v);
    }

    inline t_cycles lblt()
    {
        return do_lbr(cc.bit.n ^ cc.bit.v);
    }

    inline void bmi()
    {
        do_br(cc.bit.n);
    }

    inline t_cycles lbmi()
    {
        return do_lbr(cc.bit.n);
    }

    inline t_cycles lbne()
    {
        return do_lbr(!cc.bit.z);
    }

    inline t_cycles lbpl()
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

    inline t_cycles lbra()
    {
        pc += memory.read_word(pc) + 2;
        return 0;
    }

    inline void brn()
    {
        pc++;
    }

    inline t_cycles lbrn()
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

    inline t_cycles lbsr()
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

    inline t_cycles lbvc()
    {
        return do_lbr(!cc.bit.v);
    }

    inline void bvs()
    {
        do_br(cc.bit.v);
    }

    inline t_cycles lbvs()
    {
        return do_lbr(cc.bit.v);
    }

    //**********************************
    // Misc Instructions
    //**********************************

    inline void clr(Word addr)
    {
        Byte m = memory.read_byte(addr);
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
        cc.bit.c = 0;
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

    // undocumented instruction 0x4D, 0x5E
    // same as CLRA,CLRB but CC.Carry is
    // unchanged
    inline void clr1(Byte &reg)
    {
        cc.bit.v = false;
        cc.bit.n = false;
        cc.bit.z = true;
        reg = -1;
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
        cc.bit.c = BTST0(reg);
        reg >>= 1;
        cc.bit.n = 0;
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
        cc.bit.c = BTST7(reg);
        reg <<= 1;

        if (oc)
        {
            BSET0(reg);
        }

        cc.bit.n = BTST7(reg);
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
        cc.bit.c = BTST0(reg);
        reg = reg >> 1;

        if (oc)
        {
            BSET7(reg);
        }

        cc.bit.n = BTST7(reg);
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
        cc.bit.n = BTST15(d);
        cc.bit.v = 0;
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
        cc.bit.n = BTST15(reg);
        cc.bit.v = 0;
        cc.bit.z = !reg;
    }

    inline void lea(Word &reg, Word addr)
    {
        reg = addr;
        cc.bit.z = !reg;
    }

    inline void lea_nocc(Word &reg, Word addr)
    {
        reg = addr;
    }

    inline void     daa();
    inline void     dec(Byte &reg);
    inline void     inc(Byte &reg);
    inline void     lsl(Byte &reg);
    inline void     neg(Byte &reg);
    inline void     tst(Byte reg);
    inline void     cmp(Byte reg, Byte operand);
    inline void     cmp(Word reg, Word operand);
    inline void     adc(Byte &reg, Byte operand);
    inline void     add(Byte &reg, Byte operand);
    inline void     add(Word &reg, Word operand);
    inline void     sbc(Byte &reg, Byte operand);
    inline void     sub(Byte &reg, Byte operand);
    inline void     sub(Word &reg, Word operand);
    inline void     swi(), swi2(), swi3();
    inline void     cwai(), sync();
    inline t_cycles rti();
    inline void     asr(Byte &reg);
    void     tfr();
    void     exg();
    t_cycles psh(Byte what, Word &s, Word &u);
    t_cycles pul(Byte what, Word &s, Word &u);
    Word     do_effective_address(Byte);
#endif
    void     nmi(bool save_state);
    void     firq(bool save_state);
    void     irq(bool save_state);
    void     invalid(const char *pmessage);
    bool     use_undocumented;
public:
    void            set_use_undocumented(bool b);
    bool            is_use_undocumented()
    {
        return use_undocumented;
    };

    // Scheduler Interface implemenation
public:
    void do_reset() override;
    Byte run(Word mode) override;
    void exit_run() override;
    QWord get_cycles(bool reset = false) override;
    void get_status(CpuStatus *stat) override;
    CpuStatus *create_status_object() override;
    void get_interrupt_status(tInterruptStatus &s) override;
    void set_required_cyclecount(t_cycles x_cycles) override;

    // test support
    void            set_status(CpuStatus *stat);
protected:
    Byte            runloop();

    // interrupt handling:
public:
    void            reset();        // CPU reset
    inline t_cycles        exec_irqs(bool save_state = true);
    void            set_nmi();
    void            set_firq();
    void            set_irq();

protected:
    QWord           total_cycles; // total cycle count with 64 Bit resolution
    t_cycles        cycles;     // cycle cnt for one timer tick
    t_cycles        required_cyclecount;//cycle count for freq ctrl

    // breakpoint support
protected:
    unsigned int    bp[3];
public:
    void        set_bp(int which, Word address);
    unsigned int    get_bp(int which);
    int     is_bp_set(int which);
    void        reset_bp(int which);

    // interface to other classes
public:
    void        set_disassembler(Da6809 *x_da);
    void        set_serpar(Byte b);
    bool        set_logfile(const struct s_cpu_logfile *lf);
protected:
    int Disassemble(Word address, DWord *pFlags,
                    char **pCode, char **pMnemonic);
    FILE        *log_fp;
    bool        do_logging;
    struct s_cpu_logfile lfs;

    Memory &memory;

    // Public constructor and destructor
public:
    Mc6809(Memory &x_memory);
    virtual ~Mc6809();
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
    cc.bit.n = BTST7(sum);
    cc.bit.v = BTST7(reg ^ operand ^ sum ^ (sum >> 1));
    cc.bit.c = BTST8(sum);
    cc.bit.h = BTST4(reg ^ operand ^ sum);
    reg = (Byte)sum;
    cc.bit.z = !reg;
}

inline void Mc6809::add(Word &reg, Word operand)
{
    DWord sum = (DWord)reg + operand;
    cc.bit.n = BTST15(sum);
    cc.bit.v = BTST15(reg ^ operand ^ sum ^ (sum >> 1));
    cc.bit.c = BTST16(sum);
    reg = (Word)sum;
    cc.bit.z = !reg;
}

inline void Mc6809::sub(Byte &reg, Byte operand)
{
    Word diff = reg - operand;
    cc.bit.n = BTST7(diff);
    cc.bit.v = BTST7(reg ^ operand ^ diff ^ (diff >> 1));
    cc.bit.c = BTST8(diff);
    reg = (Byte)diff;
    cc.bit.z = !reg;
}

inline void Mc6809::sub(Word &reg, Word operand)
{
    DWord diff = reg - operand;
    cc.bit.n = BTST15(diff);
    cc.bit.v = BTST15(reg ^ operand ^ diff ^ (diff >> 1));
    cc.bit.c = BTST16(diff);
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
                 "bt $0,%1;"      // Copy 6809 carry bit into condition flags
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
                 "bt $0,%1;"      // Copy 6809 carry bit into condition flags
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
    cc.bit.n = BTST7(sum);
    cc.bit.v = BTST7(reg ^ operand ^ sum ^ (sum >> 1));
    cc.bit.c = BTST8(sum);
    cc.bit.h = BTST4(reg ^ operand ^ sum);
    reg = (Byte)sum;
    cc.bit.z = !reg;
}

inline void Mc6809::sbc(Byte &reg, Byte operand)
{
    Word diff = reg - operand - cc.bit.c;
    cc.bit.n = BTST7(diff);
    cc.bit.v = BTST7(reg ^ operand ^ diff ^ (diff >> 1));
    cc.bit.c = BTST8(diff);
    reg = (Byte)diff;
    cc.bit.z = !reg;
}
#endif

inline void Mc6809::daa()
{
    Word    t, c = 0;
    Byte    lsn = a & 0x0f;
    Byte    msn = a & 0xf0;

    if (cc.bit.h || (lsn > 9))
    {
        c |= 0x06;
    }

    if (cc.bit.c    ||
        (msn > 0x90) ||
        ((msn > 0x80) && (lsn > 9)))
    {
        c |= 0x60;
    }

    t = c + a;
    a = (Byte)t;

    if (BTST8(t))
    {
        cc.bit.c = true;
    }

    cc.bit.n = BTST7(a);
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
    cc.bit.n = BTST7(reg);
    cc.bit.z = !reg;
    cc.bit.v = (reg == 0x7f);
}

inline void Mc6809::inc(Byte &reg)
{
    reg++;
    cc.bit.n = BTST7(reg);
    cc.bit.z = !reg;
    cc.bit.v = (reg == 0x80);
}

inline void Mc6809::neg(Byte &reg)
{
    // Sw: fixed carry bug
    cc.bit.v = (reg == 0x80);
    cc.bit.c = (reg != 0);
    reg = (~reg) + 1;
    cc.bit.n = BTST7(reg);
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
    cc.bit.n = BTST7(diff);
    cc.bit.v = BTST7(reg ^ operand ^ diff ^ (diff >> 1));
    cc.bit.z = !(diff & 0xFF);
    cc.bit.c = BTST8(diff);
}

inline void Mc6809::cmp(Word reg, Word operand)
{
    DWord diff = reg - operand;
    cc.bit.n = BTST15(diff);
    cc.bit.v = BTST15(reg ^ operand ^ diff ^ (diff >> 1));
    cc.bit.z = !(diff & 0xFFFF);
    cc.bit.c = BTST16(diff);
}

inline void Mc6809::tst(Byte reg)
{
    cc.bit.n = BTST7(reg);
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
    cc.bit.c = BTST7(reg);
    cc.bit.v = BTST7(reg ^ (reg << 1));
    reg <<= 1;
    cc.bit.n = BTST7(reg);
    cc.bit.z = !reg;
}

inline void Mc6809::asr(Byte &reg)
{
    cc.bit.c = BTST0(reg);
    reg >>= 1;
    cc.bit.n = BTST6(reg);

    if (cc.bit.n)
    {
        BSET7(reg);
    }

    cc.bit.z = !reg;
}
#endif

//**********************************
// Interrupt related Instructions
//**********************************
inline t_cycles Mc6809::rti()
{
    cc.all = memory.read_byte(s++);

    if (cc.bit.e)
    {
        pul(0xfe, s, u);
        return 15;
    }
    else
    {
        pc = memory.read_word(s);
        s += 2;
        return 6;
    }
}

inline void Mc6809::sync()
{
    pc--; // processor loops in sync instruction until interrupt
    events |= DO_SYNC;
}

inline void Mc6809::cwai()
{
    cc.all &= memory.read_byte(pc++);
    cc.bit.e = 1;
    psh(0xff, s, u);
    events |= DO_CWAI;
    pc -= 2; // processor loops in cwai instruction until interrupt
}

inline void Mc6809::swi()
{
    cc.bit.e = 1;
    psh(0xff, s, u);
    cc.bit.f = cc.bit.i = 1;
    pc = memory.read_word(0xfffa);
}

inline void Mc6809::swi2()
{
    cc.bit.e = 1;
    psh(0xff, s, u);
    pc = memory.read_word(0xfff4);
}

inline void Mc6809::swi3()
{
    cc.bit.e = 1;
    psh(0xff, s, u);
    pc = memory.read_word(0xfff2);
}


#endif // ifndef FASTFLEX

inline t_cycles Mc6809::exec_irqs(bool save_state)
{
    if (events & (DO_IRQ | DO_FIRQ | DO_NMI))
    {
        if ((events & DO_NMI) && !nmi_armed)
        {
            ++interrupt_status.count[INT_NMI];
            EXEC_NMI(save_state);
            events &= ~DO_NMI;
        }
        else if ((events & DO_FIRQ) && !CC_BITF)
        {
            ++interrupt_status.count[INT_FIRQ];
            EXEC_FIRQ(save_state);
            events &= ~DO_FIRQ;
        }
        else if ((events & DO_IRQ) && !CC_BITI)
        {
            ++interrupt_status.count[INT_IRQ];
            EXEC_IRQ(save_state);
            events &= ~DO_IRQ;
        } // else
    }  // if

    return 5; // rounded
}  // exec_irqs

#endif // __mc6809_h__
