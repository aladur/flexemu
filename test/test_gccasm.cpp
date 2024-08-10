#include <iostream>
#include <iomanip>
#include <functional>
#include "misc1.h"
#include "mc6809.h"
#include "mc6809st.h"
#include "memory.h"
#include "soptions.h"
#include "foptman.h"
#include "test_gccasm.h"


// LINT suppression for global variable.
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
union ucc cc;

using FctRefByteByte_t = std::function<void(Byte &, Byte)>;
using FctByteByte_t = std::function<void(Byte, Byte)>;
using FctByte_t = std::function<void(Byte)>;
using FctRefByte_t = std::function<void(Byte &)>;
using FctRefWordWord_t = std::function<void(Word &, Word)>;
using FctWordWord_t = std::function<void(Word, Word)>;

#if defined(__GNUC__) && defined(__x86_64__)
void addx(Byte &reg, Byte operand)
{
    Word sum = reg + operand;
    cc.bit.n = BTST<Word>(sum, 7U);
    cc.bit.v = BTST<Word>(reg ^ operand ^ sum ^ (sum >> 1), 7U);
    cc.bit.c = BTST<Word>(sum, 8U);
    cc.bit.h = BTST<Word>(reg ^ operand ^ sum, 4U);
    reg = static_cast<Byte>(sum);
    cc.bit.z = !reg;
}

void add(Byte &reg, Byte operand)
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

inline void addw(Word &reg, Word operand)
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

inline void adc(Byte &reg, Byte operand)
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

inline void sub(Byte &reg, Byte operand)
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

inline void subw(Word &reg, Word operand)
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

inline void sbc(Byte &reg, Byte operand)
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
                 "orb %3,%1;"     // merge Bits N, Z, V and C into cc.all
                 : "=qm"(reg), "+m"(cc.all), "+rm"(x86flags), "+r"(mask)
                 : "0"(reg), "qm"(operand)
                 : "cc");
}

inline void inc(Byte &reg)
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

inline void dec(Byte &reg)
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

inline void neg(Byte &reg)
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

inline void tst(Byte reg)
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

inline void cmp(Byte reg, Byte operand)
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

inline void cmpw(Word reg, Word operand)
{
    long x86flags = 0;
    Byte mask = 0;

    asm volatile(\
                 "cmpw %4,%3;"    // Execute 16-bit comparison
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

inline void lsl(Byte &reg)
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

inline void lsr(Byte &reg)
{
    long x86flags = 0;
    Byte mask = 0;

    asm volatile(\
                 "shrb %0;"       // Execute 8-bit logical shift right
                 "pushf;"         // Push x86 condition code register
                 "pop %2;"        // read condition code into x86flags
                 "bt $6,%2;"      // copy bit ZF into carry (6809: Z bit)
                 "rclb %3;"       // rotate carry into mask
                 "shlb %3;"       // rotate 0 into mask
                 "rcr %2;"        // rotate CF into carry (6809: C bit)
                 "rclb %3;"       // rotate carry into mask
                 "andb $0xf2,%1;" // keep E, F, H, I and V bit unchanged
                 "orb %3,%1;"     // merge Bits N, Z and C into cc.all
                 // N is always zero
                 : "=qm"(reg), "+m"(cc.all), "+rm"(x86flags), "+r"(mask)
                 : "0"(reg)
                 : "cc");
}

inline void asl(Byte &reg)
{
    long x86flags = 0;
    Byte mask = 0;

    asm volatile(\
                 "salb %0;"       // Execute 8-bit arithmetic shift left
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

inline void asr(Byte &reg)
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

/* inline assembly for rol and ror. It compiles and tests successfull on
   clang++ and g++ but it has warnings about matching constraints on g++.
   => rejected
inline void ror(Byte &reg)
{
    asm volatile(\
                 "bt $0,%1;"      // copy mc6809 carry into condition code reg.
                 "rcrb %0;"       // Execute 8-bit rotate right with carry
                 "jnc 1f;"        // Jump to 1: if carry is 0
                 "andb $0xf2,%1;" // keep E, F, H, I and V bit unchanged
                 "orb $1,%1;"     // set C
                 "jmp 2f;"        // jump to 2:
                 "1: andb $0xf2,%1;" // keep E, F, H, I and V bit unchanged
                 "2: testb %0,%0;"   // test reg for 0
                 "jne 3f;"        // jump to 3: if not 0
                 "orb $4,%1;"     // set Z
                 "3: bt $7,%1;"   // test bit 7 (sign bit) of reg
                 "jnc 4f;"        // jump to 4: if 0
                 "orb $8,%1;"     // set N
                 "4:;"
                 : "=rm"(reg), "=m"(cc.all)
                 : "0"(reg), "1"(cc.all)
                 : "cc");
}

inline void rol(Byte &reg)
{
    long x86flags = 0;
    Byte mask = 0;

    asm volatile(\
                 "bt $0,%1;"      // copy mc6809 carry into condition code reg.
                 "rclb %0;"       // Execute 8-bit rotate left with carry
                 "pushf;"         // Push x86 condition code register
                 "pop %2;"        // read condition code into x86flags
                 "bt $11,%2;"     // copy bit OF into carry (6809: V bit)
                 "rclb %3;"       // rotate carry into mask
                 "rcr %2;"        // rotate CF into carry (6809: C bit)
                 "rclb %3;"       // rotate carry into mask
                 "andb $0xf0,%1;" // keep E, F, H and I bit unchanged
                 "testb %0,%0;"   // test reg for 0
                 "jne 1f;"        // jump to 1: if not 0
                 "orb $4,%3;"     // set Z in mask
                 "1: testb $128,%0;"  // test bit 7 (sign bit) of reg
                 "je 2f;"         // jump to 2: if 0
                 "orb $8,%3;"     // set N in mask
                 "2: orb %3,%1;"  // merge Bits N, Z, V and C into cc.all
                 : "=qm"(reg), "=m"(cc.all), "+rm"(x86flags), "+rm"(mask)
                 : "0"(reg), "1"(cc.all)
                 : "cc");
}
*/

void init_test_gccasm(int /*argc*/, char ** /*argv*/)
{
}

void err(const std::string& mnemonic, Word op, Byte m6809_cc,
         const std::string& regname, Word cpureg, Word x86reg)
{
    std::cout << std::setw(2) << std::setfill('0') << std::hex
              << mnemonic << "(0x" << op << ")"
              << ", CC=0x" << static_cast<Word>(m6809_cc)
              << ", " << regname
              << " expected 0x" << cpureg
              << " but is 0x" << x86reg
              << '\n';
}

void err(const std::string& mnemonic, Word op1, Word op2, Byte m6809_cc,
         const std::string& regname, Word cpureg, Word x86reg)
{
    std::cout << std::setw(2) << std::setfill('0') << std::hex
              << mnemonic << "(0x" << op1 << ", 0x" << op2 << ")"
              << ", CC=0x" << static_cast<Word>(m6809_cc)
              << ", " << regname
              << " expected 0x" << cpureg
              << " but is 0x" << x86reg
              << '\n';
}

bool test_gccasm_fctByte(const std::string& mnemonic,
                         const FctByte_t &test_function,
                         Byte opcode)
{
    struct sOptions options;
    FlexemuOptions::InitOptions(options);
    Memory memory(options);
    Mc6809 cpu(memory);
    Mc6809CpuStatus status;
    bool success = true;
    Word op;
    Byte rega;
    Byte regcc;

    memory.write_ram_rom(0xfffe, 0x00);
    memory.write_ram_rom(0xffff, 0x00);

    cpu.reset();

    for (regcc = 0; regcc != 254; --regcc)
    {
        for (op = 0; op < 256; ++op)
        {
            cpu.get_status(&status);
            status.a = static_cast<Byte>(op);
            status.pc = 0x0000;
            status.cc = regcc;
            cpu.set_status(&status);
            memory.write_byte(0x0000, opcode);
            cpu.run(RunMode::SingleStepInto);
            cpu.get_status(&status);

            rega = static_cast<Byte>(op);
            cc.all = regcc;
            test_function(rega);

            if (status.cc != cc.all)
            {
                err(mnemonic, op, regcc, "CC", status.cc, cc.all);
                success = false;
            }
        }
    }

    return success;
}

bool test_gccasm_fctRefByte(const std::string& mnemonic,
                            const FctRefByte_t &test_function,
                            Byte opcode)
{
    struct sOptions options;
    FlexemuOptions::InitOptions(options);
    Memory memory(options);
    Mc6809 cpu(memory);
    Mc6809CpuStatus status;
    bool success = true;
    Word op;
    Byte rega;
    Byte regcc;

    memory.write_ram_rom(0xfffe, 0x00);
    memory.write_ram_rom(0xffff, 0x00);

    cpu.reset();

    for (regcc = 0; regcc != 254; --regcc)
    {
        for (op = 0; op < 256; ++op)
        {
            cpu.get_status(&status);
            status.a = static_cast<Byte>(op);
            status.pc = 0x0000;
            status.cc = regcc;
            cpu.set_status(&status);
            memory.write_byte(0x0000, opcode);
            cpu.run(RunMode::SingleStepInto);
            cpu.get_status(&status);

            rega = static_cast<Byte>(op);
            cc.all = regcc;
            test_function(rega);

            if (status.a != rega)
            {
                err(mnemonic, op, regcc, "A", status.a, rega);
                success = false;
            }

            if (status.cc != cc.all)
            {
                err(mnemonic, op, regcc, "CC", status.cc, cc.all);
                success = false;
            }
        }
    }

    return success;
}

bool test_gccasm_fctByteByte(const std::string& mnemonic,
                             const FctByteByte_t &test_function,
                             Byte opcode)
{
    struct sOptions options;
    FlexemuOptions::InitOptions(options);
    Memory memory(options);
    Mc6809 cpu(memory);
    Mc6809CpuStatus status;
    Word op1;
    Word op2;
    Byte rega;
    Byte regcc;
    bool success = true;

    memory.write_ram_rom(0xfffe, 0x00);
    memory.write_ram_rom(0xffff, 0x00);

    cpu.reset();

    for (regcc = 0; regcc != 254; --regcc)
    {
        for (op1 = 0; op1 < 256; ++op1)
        {
            for (op2 = 0; op2 < 256; ++op2)
            {
                cpu.get_status(&status);
                status.a = static_cast<Byte>(op1);
                status.pc = 0x0000;
                status.cc = regcc;
                cpu.set_status(&status);
                memory.write_byte(0x0000, opcode);
                // write immediate value
                memory.write_byte(0x0001, static_cast<Byte>(op2));
                cpu.run(RunMode::SingleStepInto);
                cpu.get_status(&status);

                rega = static_cast<Byte>(op1);
                cc.all = regcc;
                test_function(rega, static_cast<Byte>(op2));

                if (status.cc != cc.all)
                {
                    err(mnemonic, op1, op2, regcc, "CC", status.cc, cc.all);
                    success = false;
                }
            }
        }
    }

    return success;
}

bool test_gccasm_fctRefByteByte(const std::string& mnemonic,
                                const FctRefByteByte_t &test_function,
                                Byte opcode)
{
    struct sOptions options;
    FlexemuOptions::InitOptions(options);
    Memory memory(options);
    Mc6809 cpu(memory);
    Mc6809CpuStatus status;
    Word op1;
    Word op2;
    Byte rega;
    Byte regcc;
    bool success = true;

    memory.write_ram_rom(0xfffe, 0x00);
    memory.write_ram_rom(0xffff, 0x00);

    cpu.reset();

    for (regcc = 0; regcc != 254; --regcc)
    {
        for (op1 = 0; op1 < 256; ++op1)
        {
            for (op2 = 0; op2 < 256; ++op2)
            {
                cpu.get_status(&status);
                status.a = static_cast<Byte>(op1);
                status.pc = 0x0000;
                status.cc = regcc;
                cpu.set_status(&status);
                memory.write_byte(0x0000, opcode);
                // Write immediate value.
                memory.write_byte(0x0001, static_cast<Byte>(op2));
                cpu.run(RunMode::SingleStepInto);
                cpu.get_status(&status);

                rega = static_cast<Byte>(op1);
                cc.all = regcc;
                test_function(rega, static_cast<Byte>(op2));

                if (status.a != rega)
                {
                    err(mnemonic, op1, op2, regcc, "A", status.a, rega);
                    success = false;
                }

                if (status.cc != cc.all)
                {
                    err(mnemonic, op1, op2, regcc, "CC", status.cc, cc.all);
                    success = false;
                }
            }
        }
    }

    return success;
}

bool test_gccasm_fctWordWord(const std::string& mnemonic,
                             const FctWordWord_t &test_function,
                             Byte opcode1,
                             Byte opcode2)
{
    struct sOptions options;
    FlexemuOptions::InitOptions(options);
    Memory memory(options);
    Mc6809 cpu(memory);
    Mc6809CpuStatus status;
    DWord op1;
    DWord op2;
    Word regd;
    Word addr;
    Byte regcc;
    bool success = true;

    memory.write_ram_rom(0xfffe, 0x00);
    memory.write_ram_rom(0xffff, 0x00);

    cpu.reset();

    for (regcc = 0; regcc != 254; --regcc)
    {
        // Skip some combinations to reduce test time
        for (op1 = 0; op1 < 65536; op1 += 31)
        {
            for (op2 = 0; op2 < 65536; op2 += 30)
            {
                cpu.get_status(&status);
                status.a = static_cast<Byte>(op1 >> 8);
                status.b = static_cast<Byte>(op1);
                addr = 0x0000;
                status.pc = addr;
                status.cc = regcc;
                cpu.set_status(&status);
                memory.write_byte(addr++, opcode1);
                memory.write_byte(addr++, opcode2);
                // Write immediate value high.
                memory.write_byte(addr++, static_cast<Byte>(op2 >> 8));
                // Write immediate value low.
                memory.write_byte(addr++, static_cast<Byte>(op2));
                cpu.run(RunMode::SingleStepInto);
                cpu.get_status(&status);

                regd = static_cast<Word>(op1);
                cc.all = regcc;
                test_function(regd, static_cast<Word>(op2));

                if (status.cc != cc.all)
                {
                    err(mnemonic, op1, op2, regcc, "CC", status.cc, cc.all);
                    success = false;
                }
            }
        }
    }

    return success;
}

bool test_gccasm_fctRefWordWord(const std::string& mnemonic,
                                const FctRefWordWord_t &test_function,
                                Byte opcode)
{
    struct sOptions options;
    FlexemuOptions::InitOptions(options);
    Memory memory(options);
    Mc6809 cpu(memory);
    Mc6809CpuStatus status;
    DWord op1;
    DWord op2;
    Word regd;
    Word cpuregd;
    Byte regcc;
    bool success = true;

    memory.write_ram_rom(0xfffe, 0x00);
    memory.write_ram_rom(0xffff, 0x00);

    cpu.reset();

    for (regcc = 0; regcc != 254; --regcc)
    {
        // Skip some combinations to reduce test time
        for (op1 = 0; op1 < 65536; op1 += 31)
        {
            for (op2 = 0; op2 < 65536; op2 += 30)
            {
                cpu.get_status(&status);
                status.a = static_cast<Byte>(op1 >> 8);
                status.b = static_cast<Byte>(op1);
                status.pc = 0x0000;
                status.cc = regcc;
                cpu.set_status(&status);
                memory.write_byte(0x0000, opcode);
                // Write immediate value high.
                memory.write_byte(0x0001, static_cast<Byte>(op2 >> 8));
                // Write immediate value low.
                memory.write_byte(0x0002, static_cast<Byte>(op2));
                cpu.run(RunMode::SingleStepInto);
                cpu.get_status(&status);
                cpuregd = (static_cast<Word>(status.a) << 8) | status.b;

                regd = static_cast<Word>(op1);
                cc.all = regcc;
                test_function(regd, static_cast<Word>(op2));

                if (cpuregd != regd)
                {
                    err(mnemonic, op1, op2, regcc, "D", cpuregd, regd);
                    success = false;
                }

                if (status.cc != cc.all)
                {
                    err(mnemonic, op1, op2, regcc, "CC", status.cc, cc.all);
                    success = false;
                }
            }
        }
    }

    return success;
}

bool test_gccasm_add8()
{
    // Add immediate to A
    return test_gccasm_fctRefByteByte("adda", add, 0x8b);
}

bool test_gccasm_add16()
{
    // Add immediate to D
    return test_gccasm_fctRefWordWord("addd", addw, 0xc3);
}

bool test_gccasm_adc()
{
    // Add immediate with carry to A
    return test_gccasm_fctRefByteByte("adca", adc, 0x89);
}

bool test_gccasm_sub8()
{
    // Subtract immediate from A
    return test_gccasm_fctRefByteByte("suba", sub, 0x80);
}

bool test_gccasm_sub16()
{
    return test_gccasm_fctRefWordWord("subd", subw, 0x83);
}

bool test_gccasm_sbc()
{
    // Subtract immediate with carry from A
    return test_gccasm_fctRefByteByte("sbca", sbc, 0x82);
}

bool test_gccasm_cmp8()
{
    return test_gccasm_fctByteByte("cmpa", cmp, 0x81);
}

bool test_gccasm_cmp16()
{
    return test_gccasm_fctWordWord("cmpd", cmpw, 0x10, 0x83);
}

bool test_gccasm_inc()
{
    return test_gccasm_fctRefByte("inca", inc, 0x4c);
}

bool test_gccasm_dec()
{
    return test_gccasm_fctRefByte("deca", dec, 0x4a);
}

bool test_gccasm_neg()
{
    return test_gccasm_fctRefByte("nega", neg, 0x40);
}

bool test_gccasm_tst()
{
    return test_gccasm_fctByte("tsta", tst, 0x4d);
}

bool test_gccasm_lsl()
{
    // lsl is the same as asl
    return test_gccasm_fctRefByte("lsla", lsl, 0x48);
}

bool test_gccasm_lsr()
{
    return test_gccasm_fctRefByte("lsra", lsr, 0x44);
}

bool test_gccasm_asr()
{
    return test_gccasm_fctRefByte("asra", asr, 0x47);
}

bool test_gccasm_ror()
{
    //    return test_gccasm_fctRefByte("rora", ror, 0x46);
    return true;
}

bool test_gccasm_rol()
{
    //    return test_gccasm_fctRefByte("rola", rol, 0x49);
    return true;
}
#endif

