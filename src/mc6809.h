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
#include "misc1.h"
#include "typedefs.h"
#include "memory.h"
#include "schedcpu.h"
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

struct s_cpu_logfile
{
    unsigned int minAddr;
    unsigned int maxAddr;
    unsigned int startAddr;
    unsigned int stopAddr;
    char logFileName[PATH_MAX];
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
    Word            events;         // event status flags
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

    void            init(void);
    void            init_indexed_cycles(void);
    void            init_psh_pul_cycles(void);
    void            illegal();
#ifndef FASTFLEX
    inline Word     fetch_ea_ext(void);
    inline Word     fetch_ea_dir(void);
    inline Word     fetch_ea_idx(t_cycles *c);
    inline Byte     fetch_imm_08(void);
    inline Byte     fetch_ext_08(void);
    inline Byte     fetch_dir_08(void);
    inline Byte     fetch_idx_08(t_cycles *c);
    inline Word     fetch_imm_16(void);
    inline Word     fetch_ext_16(void);
    inline Word     fetch_dir_16(void);
    inline Word     fetch_idx_16(t_cycles *c);

    Word        do_effective_address(Byte);

    inline void     abx();
    inline void     and_(Byte &reg, Byte operand);
    inline void     andcc(Byte operand);
    inline void        asr(Byte &reg);
    inline void     asr(Word addr);
    inline void     bcc();
    inline t_cycles     lbcc();
    inline void     bcs();
    inline t_cycles     lbcs();
    inline void     beq();
    inline t_cycles     lbeq();
    inline void     bge();
    inline t_cycles     lbge();
    inline void     bgt();
    inline t_cycles     lbgt();
    inline void     bhi();
    inline t_cycles     lbhi();
    inline void     ble();
    inline t_cycles     lble();
    inline void     bls();
    inline t_cycles     lbls();
    inline void     blt();
    inline t_cycles     lblt();
    inline void     bmi();
    inline t_cycles     lbmi();
    inline void     bne();
    inline t_cycles     lbne();
    inline void     bpl();
    inline t_cycles     lbpl();
    inline void     bra();
    inline t_cycles     lbra();
    inline void     brn();
    inline t_cycles     lbrn();
    inline void     bsr();
    inline t_cycles     lbsr();
    inline void     bvc();
    inline t_cycles     lbvc();
    inline void     bvs();
    inline t_cycles     lbvs();
    inline void     bit(Byte reg, Byte operand);
    inline void        clr(Byte &reg);
    inline void     clr(Word addr);
    inline void     cmp(Byte reg, Byte operand);
    inline void     cmp(Word reg, Word operand);
    inline void        com(Byte &reg);
    inline void     com(Word addr);
    inline void     daa();
    inline void        dec(Byte &reg);
    inline void     dec(Word addr);
    inline void     eor(Byte &reg, Byte operand);
    inline void     exg();
    inline void     inc(Word addr);
    inline void     inc(Byte &reg);
    inline void     jsr(Word addr);
    inline void     ld(Byte &reg, Byte operand);
    inline void     ld(Word &reg, Word operand);
    inline void     lea(Word &reg, Word addr);
    inline void     lea_nocc(Word &reg, Word addr);
    inline void        lsl(Byte &reg);
    inline void        lsr(Byte &reg);
    inline void     lsr(Word addr);
    inline void     lsl(Word addr);
    inline void     mul();
    inline void     jmp(Word addr);
    inline void     neg(Byte &reg);
    inline void     neg(Word addr);
    inline void        tst(Byte reg);
    inline void     tst(Word addr);
    inline void     nop();
    inline void     or_(Byte &reg, Byte operand);
    inline void     orcc(Byte operand);
    inline t_cycles     psh(Byte what, Word &s, Word &u);
    inline t_cycles     pul(Byte what, Word &s, Word &u);
    inline void        rol(Byte &reg);
    inline void     rol(Word addr);
    inline void        ror(Byte &reg);
    inline void     ror(Word addr);
    inline t_cycles     rti();
    inline void     rts();
    inline void     sex();
    inline void     st(Byte &reg, Word addr);
    inline void     st(Word &reg, Word addr);
    inline void     adc(Byte &reg, Byte operand);
    inline void         add(Byte &reg, Byte operand);
    inline void         add(Word &reg, Word operand);
    inline void     sbc(Byte &reg, Byte operand);
    inline void         sub(Byte &reg, Byte operand);
    inline void         sub(Word &reg, Word operand);
    inline void     swi(), swi2(), swi3();
    inline void     cwai(), sync();
    inline void     tfr();

    inline void     do_br(int);
    inline t_cycles     do_lbr(int);

    // additional undocumented instructions
    inline void            rst();
    inline void            negcom(Word addr);
    inline void            clr1(Byte &reg);

#endif
    inline void     nmi(bool save_state);
    inline void     firq(bool save_state);
    inline void     irq(bool save_state);
    void            invalid(const char *pmessage);
    bool            use_undocumented;
public:
    void            set_use_undocumented(bool b);
    bool            is_use_undocumented()
    {
        return use_undocumented;
    };

    // Scheduler Interface implemenation
public:
    void            do_reset(void);
    Byte            run(Word mode);
    void            exit_run(void);
    QWord           get_cycles(bool reset = false);
    void            get_status(CpuStatus *stat);
    CpuStatus      *create_status_object();
    void            set_required_cyclecount(t_cycles x_cycles);

    // test support
    void            set_status(CpuStatus *stat);
protected:
    Byte            runloop();

    // interrupt handling:
public:
    void            reset(void);        // CPU reset
    inline t_cycles        exec_irqs(bool save_state = true);
    void            set_nmi(void);
    void            set_firq(void);
    void            set_irq(void);

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
    void        get_interrupt_status(tInterruptStatus &s);
    void        set_disassembler(Da6809 *x_da);
    void        set_serpar(Byte b);
    bool        set_logfile(const struct s_cpu_logfile *lf);
protected:
    int     Disassemble(Word address, DWord *pFlags,
                        char **pb1, char **pb2);
    FILE        *log_fp;
    bool        do_logging;
    struct s_cpu_logfile lfs;

    Memory      *memory;

    // Public constructor and destructor
public:
    Mc6809(Memory *x_memory);
    virtual ~Mc6809();
};

//*******************************************************************
// Instruction execution
//*******************************************************************

#ifndef FASTFLEX
//**********************************
// Arithmetic Instructions
//**********************************
inline void Mc6809::mul(void)
{
    d = a * b;
    cc.bit.c = BTST7(b);
    cc.bit.z = !d;
}

inline void Mc6809::abx(void)
{
    x += b;
}

inline void Mc6809::sex(void)
{
    cc.bit.n = BTST7(b);
    cc.bit.z = !b;
    a = cc.bit.n ? 255 : 0;
}

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

inline void Mc6809::daa(void)
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

inline void Mc6809::dec(Word addr)
{
    Byte m = memory->read(addr);
    dec(m);
    memory->write(addr, m);
}

inline void Mc6809::inc(Word addr)
{
    Byte m = memory->read(addr);
    inc(m);
    memory->write(addr, m);
}

inline void Mc6809::neg(Word addr)
{
    Byte m = memory->read(addr);
    neg(m);
    memory->write(addr, m);
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

inline void Mc6809::tst(Word addr)
{
    Byte m = memory->read(addr);
    tst(m);
}

inline void Mc6809::bit(Byte reg, Byte operand)
{
    Byte m = reg & operand;
    tst(m);
}

//**********************************
// Logical Instructions
//**********************************
inline void Mc6809::and_(Byte &reg, Byte operand)
{
    reg &= operand;
    tst(reg);
}

inline void Mc6809::or_(Byte &reg, Byte operand)
{
    reg |= operand;
    tst(reg);
}

inline void Mc6809::eor(Byte &reg, Byte operand)
{
    reg ^= operand;
    tst(reg);
}

inline void Mc6809::orcc(Byte operand)
{
    cc.all |= operand;
}

inline void Mc6809::andcc(Byte operand)
{
    cc.all &= operand;
}

//**********************************
// (Un)Conditional Jump Instructions
//**********************************

inline void Mc6809::jmp(Word addr)
{
    pc = addr;
}

inline void Mc6809::jsr(Word addr)
{
    s -= 2;
    memory->write_word(s, pc);
    pc = addr;
}

inline void Mc6809::rts(void)
{
    pc = memory->read_word(s);
    s += 2;
}

inline void Mc6809::bra(void)
{
    pc += EXTEND8(memory->read(pc)) + 1;
}

inline t_cycles  Mc6809::lbra(void)
{
    pc += memory->read_word(pc) + 2;
    return 0;
}

inline void Mc6809::brn(void)
{
    pc++;
}

inline t_cycles Mc6809::lbrn(void)
{
    pc += 2;
    return 5;
}

inline void Mc6809::bsr(void)
{
    Byte o = memory->read(pc++);
    s -= 2;
    memory->write_word(s, pc);
    pc += EXTEND8(o);
}

inline t_cycles Mc6809::lbsr(void)
{
    Word o = memory->read_word(pc);
    pc += 2;
    s -= 2;
    memory->write_word(s, pc);
    pc += o;
    return 0;
}

inline void Mc6809::do_br(int test)
{
    if (test)
    {
        pc += EXTEND8(memory->read(pc)) + 1;
    }
    else
    {
        pc++;
    }
}

inline t_cycles Mc6809::do_lbr(int test)
{
    if (test)
    {
        pc += memory->read_word(pc) + 2;
        return 6;
    }
    else
    {
        pc += 2;
        return 5;
    }
}

inline void Mc6809::bcc(void)
{
    do_br(!cc.bit.c);
}

inline t_cycles Mc6809::lbcc(void)
{
    return do_lbr(!cc.bit.c);
}

inline void Mc6809::bcs(void)
{
    do_br(cc.bit.c);
}

inline t_cycles Mc6809::lbcs(void)
{
    return do_lbr(cc.bit.c);
}

inline void Mc6809::beq(void)
{
    do_br(cc.bit.z);
}

inline t_cycles Mc6809::lbeq(void)
{
    return do_lbr(cc.bit.z);
}

inline void Mc6809::bge(void)
{
    do_br(!(cc.bit.n ^ cc.bit.v));
}

inline t_cycles Mc6809::lbge(void)
{
    return do_lbr(!(cc.bit.n ^ cc.bit.v));
}

inline void Mc6809::bgt(void)
{
    do_br(!(cc.bit.z | (cc.bit.n ^ cc.bit.v)));
}

inline t_cycles Mc6809::lbgt(void)
{
    return do_lbr(!(cc.bit.z | (cc.bit.n ^ cc.bit.v)));
}

inline void Mc6809::bhi(void)
{
    do_br(!(cc.bit.c | cc.bit.z));
}

inline t_cycles Mc6809::lbhi(void)
{
    return do_lbr(!(cc.bit.c | cc.bit.z));
}

inline void Mc6809::ble(void)
{
    do_br(cc.bit.z | (cc.bit.n ^ cc.bit.v));
}

inline t_cycles Mc6809::lble(void)
{
    return do_lbr(cc.bit.z | (cc.bit.n ^ cc.bit.v));
}

inline void Mc6809::bls(void)
{
    do_br(cc.bit.c | cc.bit.z);
}

inline t_cycles Mc6809::lbls(void)
{
    return do_lbr(cc.bit.c | cc.bit.z);
}

inline void Mc6809::blt(void)
{
    do_br(cc.bit.n ^ cc.bit.v);
}

inline t_cycles Mc6809::lblt(void)
{
    return do_lbr(cc.bit.n ^ cc.bit.v);
}

inline void Mc6809::bmi(void)
{
    do_br(cc.bit.n);
}

inline t_cycles Mc6809::lbmi(void)
{
    return do_lbr(cc.bit.n);
}

inline void Mc6809::bne(void)
{
    do_br(!cc.bit.z);
}

inline t_cycles Mc6809::lbne(void)
{
    return do_lbr(!cc.bit.z);
}

inline void Mc6809::bpl(void)
{
    do_br(!cc.bit.n);
}

inline t_cycles Mc6809::lbpl(void)
{
    return do_lbr(!cc.bit.n);
}

inline void Mc6809::bvc(void)
{
    do_br(!cc.bit.v);
}

inline t_cycles Mc6809::lbvc(void)
{
    return do_lbr(!cc.bit.v);
}

inline void Mc6809::bvs(void)
{
    do_br(cc.bit.v);
}

inline t_cycles Mc6809::lbvs(void)
{
    return do_lbr(cc.bit.v);
}

//**********************************
// Misc Instructions
//**********************************

inline void Mc6809::clr(Word addr)
{
    Byte m = memory->read(addr);
    clr(m);
    memory->write(addr, m);
}

inline void Mc6809::clr(Byte &reg)
{
    cc.bit.c = false;
    cc.bit.v = false;
    cc.bit.n = false;
    cc.bit.z = true;
    reg = 0;
}

inline void Mc6809::com(Word addr)
{
    Byte m = memory->read(addr);
    com(m);
    memory->write(addr, m);
}

inline void Mc6809::com(Byte &reg)
{
    reg = ~reg;
    cc.bit.c = 1;
    tst(reg);
}

inline void Mc6809::nop(void)
{
}

// undocumented instruction 0x3e:
// force an internal reset
inline void Mc6809::rst(void)
{
    reset();
}

// undocumented instruction 0x02:
//   If cc.c = 0 then NEG <$xx (op $00)
//   If cc.c = 1 then COM <$xx (op $03)
inline void Mc6809::negcom(Word addr)
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
inline void Mc6809::clr1(Byte &reg)
{
    cc.bit.v = false;
    cc.bit.n = false;
    cc.bit.z = true;
    reg = 0;
}

//**********************************
// Shift Instructions
//**********************************
inline void Mc6809::lsl(Word addr)
{
    Byte m = memory->read(addr);
    lsl(m);
    memory->write(addr, m);
}

inline void Mc6809::asr(Word addr)
{
    Byte m = memory->read(addr);
    asr(m);
    memory->write(addr, m);
}

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

inline void Mc6809::lsr(Word addr)
{
    Byte m = memory->read(addr);
    lsr(m);
    memory->write(addr, m);
}

inline void Mc6809::lsr(Byte &reg)
{
    cc.bit.c = BTST0(reg);
    reg >>= 1;
    cc.bit.n = 0;
    cc.bit.z = !reg;
}

inline void Mc6809::rol(Word addr)
{
    Byte m = memory->read(addr);
    rol(m);
    memory->write(addr, m);
}

inline void Mc6809::rol(Byte &reg)
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

inline void Mc6809::ror(Word addr)
{
    Byte m = memory->read(addr);
    ror(m);
    memory->write(addr, m);
}

inline void Mc6809::ror(Byte &reg)
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
inline void Mc6809::ld(Byte &reg, Byte operand)
{
    reg = operand;
    tst(reg);
}

inline void Mc6809::ld(Word &reg, Word operand)
{
    reg = operand;
    cc.bit.n = BTST15(d);
    cc.bit.v = 0;
    cc.bit.z = !reg;
}

inline void Mc6809::st(Byte &reg, Word addr)
{
    memory->write(addr, reg);
    tst(reg);
}

inline void Mc6809::st(Word &reg, Word addr)
{
    memory->write_word(addr, reg);
    cc.bit.n = BTST15(reg);
    cc.bit.v = 0;
    cc.bit.z = !reg;
}

inline void Mc6809::lea(Word &reg, Word addr)
{
    reg = addr;
    cc.bit.z = !reg;
}

inline void Mc6809::lea_nocc(Word &reg, Word addr)
{
    reg = addr;
}

inline t_cycles Mc6809::psh(Byte what, Word &s, Word &u)
{
    switch ((Byte)(what & 0xf0))
    {
        case 0xf0:
            s -= 2;
            memory->write_word(s, pc);

        case 0x70:
            s -= 2;
            memory->write_word(s, u);

        case 0x30:
            s -= 2;
            memory->write_word(s, y);

        case 0x10:
            s -= 2;
            memory->write_word(s, x);

        case 0x00:
            break;

        case 0xe0:
            s -= 2;
            memory->write_word(s, pc);

        case 0x60:
            s -= 2;
            memory->write_word(s, u);

        case 0x20:
            s -= 2;
            memory->write_word(s, y);
            break;

        case 0xd0:
            s -= 2;
            memory->write_word(s, pc);

        case 0x50:
            s -= 2;
            memory->write_word(s, u);
            s -= 2;
            memory->write_word(s, x);
            break;

        case 0xc0:
            s -= 2;
            memory->write_word(s, pc);

        case 0x40:
            s -= 2;
            memory->write_word(s, u);
            break;

        case 0xb0:
            s -= 2;
            memory->write_word(s, pc);
            s -= 2;
            memory->write_word(s, y);
            s -= 2;
            memory->write_word(s, x);
            break;

        case 0xa0:
            s -= 2;
            memory->write_word(s, pc);
            s -= 2;
            memory->write_word(s, y);
            break;

        case 0x90:
            s -= 2;
            memory->write_word(s, pc);
            s -= 2;
            memory->write_word(s, x);
            break;

        case 0x80:
            s -= 2;
            memory->write_word(s, pc);
            break;
    } // switch

    switch ((Byte)(what & 0x0f))
    {
        case 0x0f:
            memory->write(--s, dp);

        case 0x07:
            memory->write(--s, b);

        case 0x03:
            memory->write(--s, a);

        case 0x01:
            memory->write(--s, cc.all);

        case 0x00:
            break;

        case 0x0e:
            memory->write(--s, dp);

        case 0x06:
            memory->write(--s, b);

        case 0x02:
            memory->write(--s, a);
            break;

        case 0x0d:
            memory->write(--s, dp);

        case 0x05:
            memory->write(--s, b);
            memory->write(--s, cc.all);
            break;

        case 0x0c:
            memory->write(--s, dp);

        case 0x04:
            memory->write(--s, b);
            break;

        case 0x0b:
            memory->write(--s, dp);
            memory->write(--s, a);
            memory->write(--s, cc.all);
            break;

        case 0x09:
            memory->write(--s, dp);
            memory->write(--s, cc.all);
            break;

        case 0x0a:
            memory->write(--s, dp);
            memory->write(--s, a);
            break;

        case 0x08:
            memory->write(--s, dp);
            break;
    } // switch

    return psh_pul_cycles[what];
}

inline t_cycles Mc6809::pul(Byte what, Word &s, Word &u)
{
    switch ((Byte)(what & 0x0f))
    {
        case 0x0f:
            cc.all = memory->read(s++);

        case 0x0e:
            a      = memory->read(s++);

        case 0x0c:
            b      = memory->read(s++);

        case 0x08:
            dp     = memory->read(s++);

        case 0x00:
            break;

        case 0x07:
            cc.all = memory->read(s++);

        case 0x06:
            a      = memory->read(s++);

        case 0x04:
            b      = memory->read(s++);
            break;

        case 0x0b:
            cc.all = memory->read(s++);

        case 0x0a:
            a      = memory->read(s++);
            dp     = memory->read(s++);
            break;

        case 0x03:
            cc.all = memory->read(s++);

        case 0x02:
            a      = memory->read(s++);
            break;

        case 0x0d:
            cc.all = memory->read(s++);
            b      = memory->read(s++);
            dp     = memory->read(s++);
            break;

        case 0x09:
            cc.all = memory->read(s++);
            dp     = memory->read(s++);
            break;

        case 0x05:
            cc.all = memory->read(s++);
            b      = memory->read(s++);
            break;

        case 0x01:
            cc.all = memory->read(s++);
            break;
    } // switch

    switch ((Byte)(what & 0xf0))
    {
        case 0xf0:
            x  = memory->read_word(s);
            s += 2;

        case 0xe0:
            y  = memory->read_word(s);
            s += 2;

        case 0xc0:
            u  = memory->read_word(s);
            s += 2;

        case 0x80:
            pc = memory->read_word(s);
            s += 2;

        case 0x00:
            break;

        case 0x70:
            x  = memory->read_word(s);
            s += 2;

        case 0x60:
            y  = memory->read_word(s);
            s += 2;

        case 0x40:
            u  = memory->read_word(s);
            s += 2;
            break;

        case 0xb0:
            x  = memory->read_word(s);
            s += 2;

        case 0xa0:
            y  = memory->read_word(s);
            s += 2;
            pc = memory->read_word(s);
            s += 2;
            break;

        case 0x30:
            x  = memory->read_word(s);
            s += 2;

        case 0x20:
            y  = memory->read_word(s);
            s += 2;
            break;

        case 0xd0:
            x  = memory->read_word(s);
            s += 2;
            u  = memory->read_word(s);
            s += 2;
            pc = memory->read_word(s);
            s += 2;
            break;

        case 0x90:
            x  = memory->read_word(s);
            s += 2;
            pc = memory->read_word(s);
            s += 2;
            break;

        case 0x50:
            x  = memory->read_word(s);
            s += 2;
            u  = memory->read_word(s);
            s += 2;
            break;

        case 0x10:
            x  = memory->read_word(s);
            s += 2;
            break;
    } // switch

    return psh_pul_cycles[what];
}

inline void Mc6809::exg(void)
{
    Word    t1, t2;
    Byte    w = memory->read(pc++);
    bool    r1_is_byte = false;
    bool    r2_is_byte = false;

    // decode source
    switch (w >> 4)
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
            t1 = a      | (a << 8);
            r1_is_byte = true;
            break;

        case 0x09:
            t1 = b      | (b << 8);
            r1_is_byte = true;
            break;

        case 0x0a:
            t1 = cc.all | (cc.all << 8);
            r1_is_byte = true;
            break;

        case 0x0b:
            t1 = dp     | (dp << 8);
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

    switch (w & 0x0F)
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
            t2 = a      | 0xFF00;
            r2_is_byte = true;
            break;

        case 0x09:
            t2 = b      | 0xFF00;
            r2_is_byte = true;
            break;

        case 0x0a:
            t2 = cc.all | 0xFF00;
            r2_is_byte = true;
            break;

        case 0x0b:
            t2 = dp     | 0xFF00;
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

    switch (w >> 4)
    {
        case 0x00:
            d      = t2;
            break;

        case 0x01:
            x      = t2;
            break;

        case 0x02:
            y      = t2;
            break;

        case 0x03:
            u      = t2;
            break;

        case 0x04:
            s      = t2;
            break;

        case 0x05:
            pc     = t2;
            break;

        case 0x08:
            a      = (Byte)t2;
            break;

        case 0x09:
            b      = (Byte)t2;
            break;

        case 0x0a:
            cc.all = (Byte)t2;
            break;

        case 0x0b:
            dp     = (Byte)t2;
            break;
    }

    switch (w & 0x0F)
    {
        case 0x00:
            d      = t1;
            break;

        case 0x01:
            x      = t1;
            break;

        case 0x02:
            y      = t1;
            break;

        case 0x03:
            u      = t1;
            break;

        case 0x04:
            s      = t1;
            break;

        case 0x05:
            pc     = t1;
            break;

        case 0x08:
            a      = (Byte)t1;
            break;

        case 0x09:
            b      = (Byte)t1;
            break;

        case 0x0a:
            cc.all = (Byte)t1;
            break;

        case 0x0b:
            dp     = (Byte)t1;
            break;
    }
}

inline void Mc6809::tfr(void)
{
    Word    t;
    Byte    w = memory->read(pc++);
    bool    is_byte = false;

    // decode source
    switch (w >> 4)
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
            t = a      | 0xFF00;
            is_byte = true;
            break;

        case 0x09:
            t = b      | 0xFF00;
            is_byte = true;
            break;

        case 0x0a:
            t = cc.all | (cc.all << 8);
            is_byte = true;
            break;

        case 0x0b:
            t = dp     | (dp << 8);
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
    switch (w & 0x0F)
    {
        case 0x00:
            if (!use_undocumented && is_byte)
            {
                break;
            }

            d      = t;
            return;

        case 0x01:
            if (!use_undocumented && is_byte)
            {
                break;
            }

            x      = t;
            return;

        case 0x02:
            if (!use_undocumented && is_byte)
            {
                break;
            }

            y      = t;
            return;

        case 0x03:
            if (!use_undocumented && is_byte)
            {
                break;
            }

            u      = t;
            return;

        case 0x04:
            if (!use_undocumented && is_byte)
            {
                break;
            }

            s      = t;
            return;

        case 0x05:
            if (!use_undocumented && is_byte)
            {
                break;
            }

            pc     = t;
            return;

        case 0x08:
            if (!use_undocumented && !is_byte)
            {
                break;
            }

            a      = (Byte)t;
            return;

        case 0x09:
            if (!use_undocumented && !is_byte)
            {
                break;
            }

            b      = (Byte)t;
            return;

        case 0x0a:
            if (!use_undocumented && !is_byte)
            {
                break;
            }

            cc.all = (Byte)t;
            return;

        case 0x0b:
            if (!use_undocumented && !is_byte)
            {
                break;
            }

            dp     = (Byte)t;
            return;
    }

    pc -= 2;
    invalid("transfer register");
    return;
}

//**********************************
// Interrupt related Instructions
//**********************************
inline t_cycles Mc6809::rti(void)
{
    cc.all = memory->read(s++);

    if (cc.bit.e)
    {
        pul(0xfe, s, u);
        return 15;
    }
    else
    {
        pc = memory->read_word(s);
        s += 2;
        return 6;
    }
}

inline void Mc6809::sync(void)
{
    pc--; // processor loops in sync instruction until interrupt
    events |= DO_SYNC;
}

inline void Mc6809::cwai(void)
{
    cc.all &= memory->read(pc++);
    cc.bit.e = 1;
    psh(0xff, s, u);
    events |= DO_CWAI;
    pc -= 2; // processor loops in cwai instruction until interrupt
}

inline void Mc6809::swi(void)
{
    cc.bit.e = 1;
    psh(0xff, s, u);
    cc.bit.f = cc.bit.i = 1;
    pc = memory->read_word(0xfffa);
}

inline void Mc6809::swi2(void)
{
    cc.bit.e = 1;
    psh(0xff, s, u);
    pc = memory->read_word(0xfff4);
}

inline void Mc6809::swi3(void)
{
    cc.bit.e = 1;
    psh(0xff, s, u);
    pc = memory->read_word(0xfff2);
}

//**********************************
// Addressing modes
//**********************************

// fetch immediate 16-Bit operand
inline Word Mc6809::fetch_imm_16(void)
{
    Word addr = memory->read_word(pc);
    pc += 2;
    return addr;
}

// fetch extended 16-Bit operand
inline Word Mc6809::fetch_ext_16(void)
{
    Word addr = memory->read_word(pc);
    pc += 2;
    return memory->read_word(addr);
}

// fetch direct 16-Bit operand
inline Word Mc6809::fetch_dir_16(void)
{
    Word addr = dpreg.dp16 | memory->read(pc++);
    return memory->read_word(addr);
}

// fetch indexed 16-Bit operand
inline Word Mc6809::fetch_idx_16(t_cycles *c)
{
    Word        addr;
    Byte        post;

    post = memory->read(pc++);
    addr = do_effective_address(post);
    *c += indexed_cycles[post];
    return memory->read_word(addr);
}

// fetch immediate 8-Bit operand
inline Byte Mc6809::fetch_imm_08(void)
{
    return memory->read(pc++);
}

// fetch extended 8-Bit operand
inline Byte Mc6809::fetch_ext_08(void)
{
    Word addr = memory->read_word(pc);
    pc += 2;
    return memory->read(addr);
}

// fetch direct 8-Bit operand
inline Byte Mc6809::fetch_dir_08(void)
{
    Word addr = dpreg.dp16 | memory->read(pc++);
    return memory->read(addr);
}

// fetch indexed 8-Bit operand
inline Byte Mc6809::fetch_idx_08(t_cycles *c)
{
    Word        addr;
    Byte        post;

    post = memory->read(pc++);
    addr = do_effective_address(post);
    *c += indexed_cycles[post];
    return memory->read(addr);
}

// fetch effective address extended
inline Word Mc6809::fetch_ea_ext(void)
{
    Word addr = memory->read_word(pc);
    pc += 2;
    return addr;
}

// fetch effective address direct
inline Word Mc6809::fetch_ea_dir(void)
{
    Word addr = dpreg.dp16 | memory->read(pc++);
    return addr;
}

// fetch indexed address
inline Word Mc6809::fetch_ea_idx(t_cycles *c)
{
    Byte post = memory->read(pc++);
    *c += indexed_cycles[post];
    return do_effective_address(post);
}

inline Word Mc6809::do_effective_address(Byte post)
{
    register Word addr = 0;

    if (!BTST7(post))
    {
        register Word offset = post & 0x1f;

        if (offset & 0x10)
        {
            offset |= 0xffe0;
        }

        switch (post & 0x60)
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
                addr = memory->read_word(x);
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
                addr = memory->read_word(x);
                break;

            case 0x84:
                addr = x;
                break;

            case 0x94:
                addr = memory->read_word(x);
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
                addr = memory->read_word(y);
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
                addr = memory->read_word(y);
                break;

            case 0xa4:
                addr = y;
                break;

            case 0xb4:
                addr = memory->read_word(y);
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
                addr = memory->read_word(u);
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
                addr = memory->read_word(u);
                break;

            case 0xc4:
                addr = u;
                break;

            case 0xd4:
                addr = memory->read_word(u);
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
                addr = memory->read_word(s);
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
                addr = memory->read_word(s);
                break;

            case 0xe4:
                addr = s;
                break;

            case 0xf4:
                addr = memory->read_word(s);
                break;

            // (+/- B),R
            case 0x85:
                addr = EXTEND8(b) + x;
                break;

            case 0x95:
                addr = EXTEND8(b) + x;
                addr = memory->read_word(addr);
                break;

            case 0xa5:
                addr = EXTEND8(b) + y;
                break;

            case 0xb5:
                addr = EXTEND8(b) + y;
                addr = memory->read_word(addr);
                break;

            case 0xc5:
                addr = EXTEND8(b) + u;
                break;

            case 0xd5:
                addr = EXTEND8(b) + u;
                addr = memory->read_word(addr);
                break;

            case 0xe5:
                addr = EXTEND8(b) + s;
                break;

            case 0xf5:
                addr = EXTEND8(b) + s;
                addr = memory->read_word(addr);
                break;

            // (+/- A),R
            case 0x86:
                addr = EXTEND8(a) + x;
                break;

            case 0x96:
                addr = EXTEND8(a) + x;
                addr = memory->read_word(addr);
                break;

            case 0xa6:
                addr = EXTEND8(a) + y;
                break;

            case 0xb6:
                addr = EXTEND8(a) + y;
                addr = memory->read_word(addr);
                break;

            case 0xc6:
                addr = EXTEND8(a) + u;
                break;

            case 0xd6:
                addr = EXTEND8(a) + u;
                addr = memory->read_word(addr);
                break;

            case 0xe6:
                addr = EXTEND8(a) + s;
                break;

            case 0xf6:
                addr = EXTEND8(a) + s;
                addr = memory->read_word(addr);
                break;

            // (+/- 7 bit offset),R
            case 0x88:
                addr = x + EXTEND8(memory->read(pc++));
                break;

            case 0x98:
                addr = x + EXTEND8(memory->read(pc++));
                addr = memory->read_word(addr);
                break;

            case 0xa8:
                addr = y + EXTEND8(memory->read(pc++));
                break;

            case 0xb8:
                addr = y + EXTEND8(memory->read(pc++));
                addr = memory->read_word(addr);
                break;

            case 0xc8:
                addr = u + EXTEND8(memory->read(pc++));
                break;

            case 0xd8:
                addr = u + EXTEND8(memory->read(pc++));
                addr = memory->read_word(addr);
                break;

            case 0xe8:
                addr = s + EXTEND8(memory->read(pc++));
                break;

            case 0xf8:
                addr = s + EXTEND8(memory->read(pc++));
                addr = memory->read_word(addr);
                break;

            // (+/- 15 bit offset),R
            case 0x89:
                addr = x + memory->read_word(pc);
                pc += 2;
                break;

            case 0x99:
                addr = x + memory->read_word(pc);
                pc += 2;
                addr = memory->read_word(addr);
                break;

            case 0xa9:
                addr = y + memory->read_word(pc);
                pc += 2;
                break;

            case 0xb9:
                addr = y + memory->read_word(pc);
                pc += 2;
                addr = memory->read_word(addr);
                break;

            case 0xc9:
                addr = u + memory->read_word(pc);
                pc += 2;
                break;

            case 0xd9:
                addr = u + memory->read_word(pc);
                pc += 2;
                addr = memory->read_word(addr);
                break;

            case 0xe9:
                addr = s + memory->read_word(pc);
                pc += 2;
                break;

            case 0xf9:
                addr = s + memory->read_word(pc);
                pc += 2;
                addr = memory->read_word(addr);
                break;

            // (+/- D),R
            case 0x8b:
                addr = d + x;
                break;

            case 0x9b:
                addr = memory->read_word(d + x);
                break;

            case 0xab:
                addr = d + y;
                break;

            case 0xbb:
                addr = memory->read_word(d + y);
                break;

            case 0xcb:
                addr = d + u;
                break;

            case 0xdb:
                addr = memory->read_word(d + u);
                break;

            case 0xeb:
                addr = d + s;
                break;

            case 0xfb:
                addr = memory->read_word(d + s);
                break;

            // (+/- 7 bit offset), PC
            case 0x8c:
            case 0xac:
            case 0xcc:
            case 0xec:
                addr = EXTEND8(memory->read(pc++));
                addr += pc;
                break;

            case 0x9c:
            case 0xbc:
            case 0xdc:
            case 0xfc:
                addr = EXTEND8(memory->read(pc++));
                addr = memory->read_word(addr + pc);
                break;

            // (+/- 15 bit offset), PC
            case 0x8d:
            case 0xad:
            case 0xcd:
            case 0xed:
                addr = memory->read_word(pc);
                pc += 2;
                addr += pc;
                break;

            case 0x9d:
            case 0xbd:
            case 0xdd:
            case 0xfd:
                addr = memory->read_word(pc);
                pc += 2;
                addr = memory->read_word(addr + pc);
                break;

            // [address]
            case 0x9f:
                addr = memory->read_word(pc);
                addr = memory->read_word(addr);
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
inline void Mc6809::nmi(bool save_state)
{
    if (save_state)
    {
        cc.bit.e = 1;
        psh(0xff, s, u);
    }

    cc.bit.f = cc.bit.i = 1;
    pc = memory->read_word(0xfffc);
}

inline void Mc6809::firq(bool save_state)
{
    if (save_state)
    {
        cc.bit.e = 0;
        psh(0x81, s, u);
    }

    cc.bit.f = cc.bit.i = 1;
    pc = memory->read_word(0xfff6);
}

inline void Mc6809::irq(bool save_state)
{
    if (save_state)
    {
        cc.bit.e = 1;
        psh(0xff, s, u);
    }

    cc.bit.i = 1;           // Sw: don't set flag f !!
    pc = memory->read_word(0xfff8);
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
