/*
     mc6809in.cc

     flexemu, an MC6809 emulator running FLEX
     Copyright (C) 1997-2004  W. Schwotzer

     This file is based on usim-0.91 which is
     Copyright (C) 1994 by R. B. Bellis
*/


#include <limits.h>
#include "mc6809.h"
#include "mc6809st.h"
#include "da6809.h"
#include "intmem.h"


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
                events &= ~DO_BREAKPOINT;
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
#ifdef USE_GCCASM
	x86flags.all	= 0x00000000;
#endif
#endif
}

int Mc6809::Disassemble(Word address, DWord *pFlags,
        char **pb1, char **pb2)
{
        Byte buffer[6];
        DWord addr;

        if (disassembler == NULL)
                return 0;
        for (int i = 0; i < 6; i++)
                buffer[i] = READ(address+i);
        return disassembler->Disassemble((const Byte *)buffer, address,
                pFlags, &addr, pb1, pb2);
}

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
	asm volatile ("addb %2,%0\n\t" \
	              "pushf\n\t"      \
                      "pop %1"
			: "=g" (reg), "=m"(x86flags.all)
			: "qm" (operand));
	cc.all = (cc.all & ~CC_BITS_HNZVC) |
		cc_nzvc_from_x86flags[x86flags.word.l] ;
}

inline void Mc6809::add(Word &reg, Word operand)
{
	asm volatile ("addw %2,%0\n\t" \
	              "pushf\n\t"      \
                      "pop %1"
			: "=g" (reg), "=m"(x86flags.all)
			: "qm" (operand));
	cc.all = (cc.all & ~CC_BITS_NZVC) |
		cc_nzvc_from_x86flags[x86flags.word.l] ;
}

inline void Mc6809::sub(Byte &reg, Byte operand)
{
	asm volatile ("subb %2,%0\n\t" \
	              "pushf\n\t"      \
                      "pop %1"
			: "=g" (reg), "=m"(x86flags.all)
			: "qm" (operand));
	cc.all = (cc.all & ~CC_BITS_NZVC) |
		cc_nzvc_from_x86flags[x86flags.word.l] ;
}

inline void Mc6809::sub(Word &reg, Word operand)
{
	asm volatile ("subw %2,%0\n\t" \
	              "pushf\n\t"      \
                      "pop %1"
			: "=g" (reg), "=m"(x86flags.all)
			: "qm" (operand));
	cc.all = (cc.all & ~CC_BITS_NZVC) |
		cc_nzvc_from_x86flags[x86flags.word.l] ;
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

inline void Mc6809::daa(void)
{
	Word	t, c = 0;
	Byte	lsn = a & 0x0f;
	Byte	msn = a & 0xf0;

	if (cc.bit.h || (lsn > 9)) {
		c |= 0x06;
	}
	if (  cc.bit.c    ||
	     (msn > 0x90) ||
	    ((msn > 0x80) && (lsn > 9))) {
		c |= 0x60;
	}

	t = c + a;
	a = (Byte)t;
	if(BTST8(t)) cc.bit.c = true;
        cc.bit.n = BTST7(a);
        cc.bit.z = !a;
}

inline void Mc6809::dec(Word addr)
{
	Byte m = READ(addr);
	dec(m);
	WRITE(addr, m);
}

INLINE1 void Mc6809::dec(Byte &reg)
{
	reg--;
	cc.bit.n = BTST7(reg);
	cc.bit.z = !reg;
	cc.bit.v = (reg == 0x7f);
}


inline void Mc6809::inc(Word addr)
{
	Byte m = READ(addr);
	inc(m);
	WRITE(addr, m);
}

inline void Mc6809::inc(Byte& reg)
{
	reg++;
	cc.bit.n = BTST7(reg);
	cc.bit.z = !reg;
	cc.bit.v = (reg == 0x80);
}

inline void Mc6809::neg(Word addr)
{
	Byte m = READ(addr);
	neg(m);
	WRITE(addr, m);
}

inline void Mc6809::neg(Byte& reg)
{
#ifdef USE_GCCASM
	asm volatile ("negb %0\n\t" \
	              "pushf\n\t"   \
                      "pop %1"
			: "=g" (reg), "=m"(x86flags.all));
	cc.all = (cc.all & ~CC_BITS_NZVC) |
		cc_nzvc_from_x86flags[x86flags.word.l] ;
#else
// Sw: fixed carry bug
	cc.bit.v = (reg == 0x80);
	cc.bit.c = reg;
	reg = (~reg) + 1;
	cc.bit.n = BTST7(reg);
	cc.bit.z = !reg;
#endif
}

//**********************************
// Compare Instructions
//**********************************

#ifdef USE_GCCASM
inline void Mc6809::cmp(Byte reg, Byte operand)
{
	asm volatile ("cmpb %2, %1\n\t" \
	              "pushf\n\t"       \
                      "pop %0"
			: "=m"(x86flags.all)
			: "q" (reg), "qm" (operand));
	cc.all = (cc.all & ~CC_BITS_NZVC) |
		cc_nzvc_from_x86flags[x86flags.word.l] ;
}

inline void Mc6809::cmp(Word reg, Word operand)
{
	asm volatile ("cmpw %2,%1\n\t" \
	              "pushf\n\t"      \
                      "pop %0"
			: "=m"(x86flags.all)
			: "q" (reg), "qm" (operand));
	cc.all = (cc.all & ~CC_BITS_NZVC) |
		cc_nzvc_from_x86flags[x86flags.word.l] ;
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
#endif

inline void Mc6809::tst(Word addr)
{
	Byte m = READ(addr);
	tst(m);
}

INLINE1 void Mc6809::tst(Byte &reg)
{
	cc.bit.n = BTST7(reg);
	cc.bit.v = 0;
	cc.bit.z = !reg;
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

inline void Mc6809::or_(Byte& reg, Byte operand)
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
	WRITE(--s, (Byte)pc);
	WRITE(--s, pc >> 8);
	pc = addr;
}

inline void Mc6809::rts(void)
{
	pc = READ_WORD(s);
	s += 2;
}

inline void Mc6809::bra(void)
{
	pc += EXTEND8(READ(pc)) + 1;
}

inline t_cycles  Mc6809::lbra(void)
{
	pc += READ_WORD(pc) + 2;
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
	Byte	o = READ_PI(pc);
	WRITE(--s, (Byte)pc);
	WRITE(--s, (Byte)(pc >> 8));
	pc += EXTEND8(o);
}

inline t_cycles Mc6809::lbsr(void)
{
	Word o = READ_WORD(pc);
	pc += 2;
	WRITE(--s, (Byte)pc);
	WRITE(--s, (Byte)(pc >> 8));
	pc += o;
	return 0;
}

inline void Mc6809::do_br(int test)
{
	if (test) {
		pc += EXTEND8(READ(pc)) + 1;
	} else
		pc++;
}

inline t_cycles Mc6809::do_lbr(int test)
{
	if (test) {
		pc += READ_WORD(pc) + 2;
		return 6;
	} else {
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
	Byte m = READ(addr);
	clr(m);
	WRITE(addr, m);
}

INLINE1 void Mc6809::clr(Byte &reg)
{
	cc.bit.c = false;
	cc.bit.v = false;
	cc.bit.n = false;
	cc.bit.z = true;
	reg = 0;
}

inline void Mc6809::com(Word addr)
{
	Byte m = READ(addr);
	com(m);
	WRITE(addr, m);
}

INLINE1 void Mc6809::com(Byte &reg)
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
void Mc6809::rst(void)
{
	reset();
}

// undocumented instruction 0x02: 
//   If cc.c = 0 then NEG <$xx (op $00)
//   If cc.c = 1 then COM <$xx (op $03)
void Mc6809::negcom(Word addr)
{
	if (cc.bit.c)
		com(addr);
	else
		neg(addr);
}

// undocumented instruction 0x4E, 0x5E
// same as CLRA,CLRB but CC.Carry is
// unchanged
void Mc6809::clr1(Byte &reg)
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
	Byte m = READ(addr);
	lsl(m);
	WRITE(addr, m);
}

inline void Mc6809::asr(Word addr)
{
	Byte	m = READ(addr);
	asr(m);
	WRITE(addr, m);
}

#ifdef USE_GCCASM
INLINE1 void Mc6809::lsl(Byte &reg)
{
	asm volatile ("shlb $1,%0\n\t" \
	              "pushf\n\t"      \
                      "pop %1"
			: "=g" (reg), "=m" (x86flags.all));
	cc.all = (cc.all & ~CC_BITS_NZVC) |
		cc_nzvc_from_x86flags[x86flags.word.l] ;
}

INLINE1 void Mc6809::asr(Byte &reg)
{
	asm volatile ("sarb $1,%0\n\t" \
	              "pushf\n\t"      \
                      "pop %1"
			: "=g" (reg), "=m" (x86flags.all));
	cc.all = (cc.all & ~CC_BITS_NZC) |
		cc_nzvc_from_x86flags[x86flags.word.l] ;
}
#else
INLINE1 void Mc6809::lsl(Byte &reg)
{
	cc.bit.c = BTST7(reg);
	cc.bit.v = BTST7(reg ^ (reg << 1));
	reg <<= 1;
	cc.bit.n = BTST7(reg);
	cc.bit.z = !reg;
}

INLINE1 void Mc6809::asr(Byte &reg)
{
	cc.bit.c = BTST0(reg);
	reg >>= 1;
	cc.bit.n = BTST6(reg);
	if (cc.bit.n)
		BSET7(reg);
	cc.bit.z = !reg;
}
#endif

inline void Mc6809::lsr(Word addr)
{
	Byte m = READ(addr);
	lsr(m);
	WRITE(addr, m);
}

INLINE1 void Mc6809::lsr(Byte &reg)
{
	cc.bit.c = BTST0(reg);
	reg >>= 1;
	cc.bit.n = 0;
	cc.bit.z = !reg;
}

inline void Mc6809::rol(Word addr)
{
	Byte m = READ(addr);
	rol(m);
	WRITE(addr, m);
}

INLINE1 void Mc6809::rol(Byte &reg)
{
	bool oc = cc.bit.c;
	cc.bit.c = BTST7(reg);
	reg <<= 1;
	if (oc) BSET0(reg);
	cc.bit.n = BTST7(reg);
	cc.bit.v = cc.bit.c ^ cc.bit.n;
	cc.bit.z = !reg;
}

inline void Mc6809::ror(Word addr)
{
	Byte m = READ(addr);
	ror(m);
	WRITE(addr, m);
}

INLINE1 void Mc6809::ror(Byte &reg)
{
	bool oc = cc.bit.c;
	cc.bit.c = BTST0(reg);
	reg = reg >> 1;
	if (oc) BSET7(reg);
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
	WRITE(addr, reg);
	tst(reg);
}

inline void Mc6809::st(Word &reg, Word addr)
{
	WRITE_WORD(addr, reg);
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

inline t_cycles Mc6809::psh(Byte what, Word& s, Word& u)
{
	switch ((Byte)(what & 0xf0)) {
		case 0xf0:	WRITE(--s, (Byte)pc);
				WRITE(--s, (Byte)(pc >> 8));
		case 0x70:	WRITE(--s, (Byte)u);
				WRITE(--s, (Byte)(u >> 8));
		case 0x30:	WRITE(--s, (Byte)y);
				WRITE(--s, (Byte)(y >> 8));
		case 0x10:	WRITE(--s, (Byte)x);
				WRITE(--s, (Byte)(x >> 8));
		case 0x00:	break;
		case 0xe0:	WRITE(--s, (Byte)pc);
				WRITE(--s, (Byte)(pc >> 8));
		case 0x60:	WRITE(--s, (Byte)u);
				WRITE(--s, (Byte)(u >> 8));
		case 0x20:	WRITE(--s, (Byte)y);
				WRITE(--s, (Byte)(y >> 8));
				break;
		case 0xd0:	WRITE(--s, (Byte)pc);
				WRITE(--s, (Byte)(pc >> 8));
		case 0x50:	WRITE(--s, (Byte)u);
				WRITE(--s, (Byte)(u >> 8));
				WRITE(--s, (Byte)x);
				WRITE(--s, (Byte)(x >> 8));
				break;
		case 0xc0:	WRITE(--s, (Byte)pc);
				WRITE(--s, (Byte)(pc >> 8));
		case 0x40:	WRITE(--s, (Byte)u);
				WRITE(--s, (Byte)(u >> 8));
				break;
		case 0xb0:	WRITE(--s, (Byte)pc);
				WRITE(--s, (Byte)(pc >> 8));
				WRITE(--s, (Byte)y);
				WRITE(--s, (Byte)(y >> 8));
				WRITE(--s, (Byte)x);
				WRITE(--s, (Byte)(x >> 8));
				break;
		case 0xa0:	WRITE(--s, (Byte)pc);
				WRITE(--s, (Byte)(pc >> 8));
				WRITE(--s, (Byte)y);
				WRITE(--s, (Byte)(y >> 8));
				break;
		case 0x90:	WRITE(--s, (Byte)pc);
				WRITE(--s, (Byte)(pc >> 8));
				WRITE(--s, (Byte)x);
				WRITE(--s, (Byte)(x >> 8));
				break;
		case 0x80:	WRITE(--s, (Byte)pc);
				WRITE(--s, (Byte)(pc >> 8));
				break;
	} // switch
	switch ((Byte)(what & 0x0f)) {
		case 0x0f:	WRITE(--s, dp);
		case 0x07:	WRITE(--s, b);
		case 0x03:	WRITE(--s, a);
		case 0x01:	WRITE(--s, cc.all);
		case 0x00:	break;
		case 0x0e:	WRITE(--s, dp);
		case 0x06:	WRITE(--s, b);
		case 0x02:	WRITE(--s, a);
				break;
		case 0x0d:	WRITE(--s, dp);
		case 0x05:	WRITE(--s, b);
				WRITE(--s, cc.all);
				break;
		case 0x0c:	WRITE(--s, dp);
		case 0x04:	WRITE(--s, b);
				break;
		case 0x0b:	WRITE(--s, dp);
				WRITE(--s, a);
				WRITE(--s, cc.all);
				break;
		case 0x09:	WRITE(--s, dp);
				WRITE(--s, cc.all);
				break;
		case 0x0a:	WRITE(--s, dp);
				WRITE(--s, a);
				break;
		case 0x08:	WRITE(--s, dp);
				break;
	} // switch
	return psh_pul_cycles[what];
}

inline t_cycles Mc6809::pul(Byte what, Word& s, Word& u)
{
	switch ((Byte)(what & 0x0f)) {
		case 0x0f:	cc.all = READ_PI(s);
		case 0x0e:	a      = READ_PI(s);
		case 0x0c:	b      = READ_PI(s);
		case 0x08:	dp     = READ_PI(s);
		case 0x00:	break;
		case 0x07:	cc.all = READ_PI(s);
		case 0x06:	a      = READ_PI(s);
		case 0x04:	b      = READ_PI(s);
				break;
		case 0x0b:	cc.all = READ_PI(s);
		case 0x0a:	a      = READ_PI(s);
				dp     = READ_PI(s);
				break;
		case 0x03:	cc.all = READ_PI(s);
		case 0x02:	a      = READ_PI(s);
				break;
		case 0x0d:	cc.all = READ_PI(s);
				b      = READ_PI(s);
				dp     = READ_PI(s);
				break;
		case 0x09:	cc.all = READ_PI(s);
				dp     = READ_PI(s);
				break;
		case 0x05:	cc.all = READ_PI(s);
				b      = READ_PI(s);
				break;
		case 0x01:	cc.all = READ_PI(s);
				break;
	} // switch
	switch ((Byte)(what & 0xf0)) {
		case 0xf0:	x  = READ_WORD(s); s += 2;
		case 0xe0:	y  = READ_WORD(s); s += 2;
		case 0xc0:	u  = READ_WORD(s); s += 2;
		case 0x80:	pc = READ_WORD(s); s += 2;
		case 0x00:	break;
		case 0x70:	x  = READ_WORD(s); s += 2;
		case 0x60:	y  = READ_WORD(s); s += 2;
		case 0x40:	u  = READ_WORD(s); s += 2;
				break;
		case 0xb0:	x  = READ_WORD(s); s += 2;
		case 0xa0:	y  = READ_WORD(s); s += 2;
				pc = READ_WORD(s); s += 2;
				break;
		case 0x30:	x  = READ_WORD(s); s += 2;
		case 0x20:	y  = READ_WORD(s); s += 2;
				break;
		case 0xd0:	x  = READ_WORD(s); s += 2;
				u  = READ_WORD(s); s += 2;
				pc = READ_WORD(s); s += 2;
				break;
		case 0x90:	x  = READ_WORD(s); s += 2;
				pc = READ_WORD(s); s += 2;
				break;
		case 0x50:	x  = READ_WORD(s); s += 2;
				u  = READ_WORD(s); s += 2;
				break;
		case 0x10:	x  = READ_WORD(s); s += 2;
				break;
	} // switch
	return psh_pul_cycles[what];
}

inline void Mc6809::exg(void)
{
	Word	t1, t2;
	Byte	w = READ_PI(pc);
	bool	r1_is_byte = false;
	bool	r2_is_byte = false;

	// decode source
	switch(w >> 4)
	{
		case 0x00: t1 = d;  break;
		case 0x01: t1 = x;  break;
		case 0x02: t1 = y;  break;
		case 0x03: t1 = u;  break;
		case 0x04: t1 = s;  break;
		case 0x05: t1 = pc; break;
		case 0x08: t1 = a      | (a << 8); r1_is_byte = true; break;
		case 0x09: t1 = b      | (b << 8); r1_is_byte = true; break;
		case 0x0a: t1 = cc.all | (cc.all << 8); r1_is_byte = true; break;
		case 0x0b: t1 = dp     | (dp << 8); r1_is_byte = true; break;
		default:
			if (!use_undocumented)
			{
				pc -= 2;
				invalid("transfer register");
				return;
			 }
			 t1 = 0xFFFF;
	}
	switch(w & 0x0F)
	{
		case 0x00: t2 = d;  break;
		case 0x01: t2 = x;  break;
		case 0x02: t2 = y;  break;
		case 0x03: t2 = u;  break;
		case 0x04: t2 = s;  break;
		case 0x05: t2 = pc; break;
		case 0x08: t2 = a      | 0xFF00; r2_is_byte = true; break;
		case 0x09: t2 = b      | 0xFF00; r2_is_byte = true; break;
		case 0x0a: t2 = cc.all | 0xFF00; r2_is_byte = true; break;
		case 0x0b: t2 = dp     | 0xFF00; r2_is_byte = true; break;
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

	switch(w >> 4)
	{
		case 0x00: d      = t2;       break;
		case 0x01: x      = t2;       break;
		case 0x02: y      = t2;       break;
		case 0x03: u      = t2;       break;
		case 0x04: s      = t2;       break;
		case 0x05: pc     = t2;       break;
		case 0x08: a      = (Byte)t2; break;
		case 0x09: b      = (Byte)t2; break;
		case 0x0a: cc.all = (Byte)t2; break;
		case 0x0b: dp     = (Byte)t2; break;
	}
	switch(w & 0x0F)
	{
		case 0x00: d      = t1;       break;
		case 0x01: x      = t1;       break;
		case 0x02: y      = t1;       break;
		case 0x03: u      = t1;       break;
		case 0x04: s      = t1;       break;
		case 0x05: pc     = t1;       break;
		case 0x08: a      = (Byte)t1; break;
		case 0x09: b      = (Byte)t1; break;
		case 0x0a: cc.all = (Byte)t1; break;
		case 0x0b: dp     = (Byte)t1; break;
	}
}

inline void Mc6809::tfr(void)
{
	Word	t;
	Byte	w = READ_PI(pc);
	bool	is_byte = false;

	// decode source
	switch(w >> 4)
	{
		case 0x00: t = d;  break;
		case 0x01: t = x;  break;
		case 0x02: t = y;  break;
		case 0x03: t = u;  break;
		case 0x04: t = s;  break;
		case 0x05: t = pc; break;
		case 0x08: t = a      | 0xFF00; is_byte = true; break;
		case 0x09: t = b      | 0xFF00; is_byte = true; break;
		case 0x0a: t = cc.all | (cc.all << 8); is_byte = true; break;
		case 0x0b: t = dp     | (dp << 8); is_byte = true; break;
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
	switch(w & 0x0F)
	{
		case 0x00: if (!use_undocumented && is_byte) break;
			   d      = t;       return;
		case 0x01: if (!use_undocumented && is_byte) break;
		           x      = t;       return;
		case 0x02: if (!use_undocumented && is_byte) break;
		           y      = t;       return;
		case 0x03: if (!use_undocumented && is_byte) break;
		           u      = t;       return;
		case 0x04: if (!use_undocumented && is_byte) break;
		           s      = t;       return;
		case 0x05: if (!use_undocumented && is_byte) break;
		           pc     = t;       return;
		case 0x08: if (!use_undocumented && !is_byte) break;
		           a      = (Byte)t; return;
		case 0x09: if (!use_undocumented && !is_byte) break;
		           b      = (Byte)t; return;
		case 0x0a: if (!use_undocumented && !is_byte) break;
		           cc.all = (Byte)t; return;
		case 0x0b: if (!use_undocumented && !is_byte) break;
		           dp     = (Byte)t; return;
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
	cc.all = READ_PI(s);
	if (cc.bit.e) {
		pul(0xfe, s, u);
		return 15;
	} else {
		pc = READ_WORD(s);
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
	cc.all &= READ_PI(pc);
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
	pc = READ_WORD(0xfffa);
}

inline void Mc6809::swi2(void)
{
	cc.bit.e = 1;
	psh(0xff, s, u);
	pc = READ_WORD(0xfff4);
}

inline void Mc6809::swi3(void)
{
	cc.bit.e = 1;
	psh(0xff, s, u);
	pc = READ_WORD(0xfff2);
}

//**********************************
// Addressing modes
//**********************************

// fetch immediate 16-Bit operand
inline Word Mc6809::fetch_imm_16(void)
{
	Word addr = READ_WORD(pc);
	pc += 2;
	return addr;
}

// fetch extended 16-Bit operand
inline Word Mc6809::fetch_ext_16(void)
{
	Word addr = READ_WORD(pc);
	pc += 2;
	return READ_WORD(addr);
}

// fetch direct 16-Bit operand
inline Word Mc6809::fetch_dir_16(void)
{
	Word addr = dpreg.dp16 | READ_PI(pc);
	return READ_WORD(addr);
}

// fetch indexed 16-Bit operand
inline Word Mc6809::fetch_idx_16(t_cycles *c)
{
	Word		addr;
	Byte		post;

	post = READ_PI(pc);
	addr = do_effective_address(post);
	*c += indexed_cycles[post];
	return READ_WORD(addr);
}

// fetch immediate 8-Bit operand
inline Byte Mc6809::fetch_imm_08(void)
{
   return READ_PI(pc);
}

// fetch extended 8-Bit operand
inline Byte Mc6809::fetch_ext_08(void)
{
   Word addr = READ_WORD(pc);
   pc += 2;
   return READ(addr);
}

// fetch direct 8-Bit operand
inline Byte Mc6809::fetch_dir_08(void)
{
    Word addr = dpreg.dp16 | READ_PI(pc);
    return READ(addr);
}

// fetch indexed 8-Bit operand
inline Byte Mc6809::fetch_idx_08(t_cycles *c)
{
	Word		addr;
	Byte		post;

	post = READ_PI(pc);
        addr = do_effective_address(post);
	*c += indexed_cycles[post];
        return READ(addr);
}

// fetch effective address extended
inline Word Mc6809::fetch_ea_ext(void)
{
	Word addr = READ_WORD(pc);
	pc += 2;
	return addr;
}

// fetch effective address direct
inline Word Mc6809::fetch_ea_dir(void)
{
	Word addr = dpreg.dp16 | READ_PI(pc);
	return addr;
}

// fetch indexed address
inline Word Mc6809::fetch_ea_idx(t_cycles *c)
{
	Byte post = READ_PI(pc);
	*c += indexed_cycles[post];
	return do_effective_address(post);
}

Word Mc6809::do_effective_address(Byte post)
{
	register Word addr = 0;

	if (!BTST7(post)) {
		register Word offset = post & 0x1f;
		if (offset & 0x10)
			offset |= 0xffe0;
		switch (post & 0x60) {
			case 0x00 : addr = x + offset; break;
			case 0x20 : addr = y + offset; break;
			case 0x40 : addr = u + offset; break;
			case 0x60 : addr = s + offset; break;
		}
	} else {
		switch (post) {
			// ,X+ ,X++ ,-X ,--X ,X
			case 0x80: addr = x++;                  break;
			case 0x81: addr = x; x += 2;            break;
			case 0x91: addr = READ_WORD(x); x += 2; break;
			case 0x82: addr = --x;                  break;
			case 0x83: x -= 2; addr = x;            break;
			case 0x93: x -= 2; addr = READ_WORD(x); break;
			case 0x84: addr = x;                    break;
			case 0x94: addr = READ_WORD(x);         break;
			// ,Y+ ,Y++ ,-Y ,--Y ,Y
			case 0xa0: addr = y++;                  break;
			case 0xa1: addr = y; y += 2;            break;
			case 0xb1: addr = READ_WORD(y); y += 2; break;
			case 0xa2: addr = --y;                  break;
			case 0xa3: y -= 2; addr = y;            break;
			case 0xb3: y -= 2; addr = READ_WORD(y); break;
			case 0xa4: addr = y;                    break;
			case 0xb4: addr = READ_WORD(y);         break;
			// ,U+ ,U++ ,-U ,--U ,U
			case 0xc0: addr = u++;                  break;
			case 0xc1: addr = u; u += 2;            break;
			case 0xd1: addr = READ_WORD(u); u += 2; break;
			case 0xc2: addr = --u;                  break;
			case 0xc3: u -= 2; addr = u;            break;
			case 0xd3: u -= 2; addr = READ_WORD(u); break;
			case 0xc4: addr = u;                    break;
			case 0xd4: addr = READ_WORD(u);         break;
			// ,S+ ,S++ ,-S ,--S ,S
			case 0xe0: addr = s++;                  break;
			case 0xe1: addr = s; s += 2;            break;
			case 0xf1: addr = READ_WORD(s); s += 2; break;
			case 0xe2: addr = --s;                  break;
			case 0xe3: s -= 2; addr = s;            break;
			case 0xf3: s -= 2; addr = READ_WORD(s); break;
			case 0xe4: addr = s;                    break;
			case 0xf4: addr = READ_WORD(s);         break;
			// (+/- B),R
			case 0x85: addr = EXTEND8(b) + x;       break;
			case 0x95: addr = EXTEND8(b) + x;
                                   addr = READ_WORD(addr);      break;
			case 0xa5: addr = EXTEND8(b) + y;       break;
			case 0xb5: addr = EXTEND8(b) + y;
                                   addr = READ_WORD(addr);      break;
			case 0xc5: addr = EXTEND8(b) + u;       break;
			case 0xd5: addr = EXTEND8(b) + u;
                                   addr = READ_WORD(addr);      break;
			case 0xe5: addr = EXTEND8(b) + s;       break;
			case 0xf5: addr = EXTEND8(b) + s;
                                   addr = READ_WORD(addr);      break;
			// (+/- A),R
			case 0x86: addr = EXTEND8(a) + x;       break;
			case 0x96: addr = EXTEND8(a) + x;
                                   addr = READ_WORD(addr);      break;
			case 0xa6: addr = EXTEND8(a) + y;       break;
			case 0xb6: addr = EXTEND8(a) + y;
                                   addr = READ_WORD(addr);      break;
			case 0xc6: addr = EXTEND8(a) + u;       break;
			case 0xd6: addr = EXTEND8(a) + u;
                                   addr = READ_WORD(addr);      break;
			case 0xe6: addr = EXTEND8(a) + s;       break;
			case 0xf6: addr = EXTEND8(a) + s;
                                   addr = READ_WORD(addr);      break;
			// (+/- 7 bit offset),R
			case 0x88: addr = x + EXTEND8(READ_PI(pc)); break;
			case 0x98: addr = x + EXTEND8(READ_PI(pc));
                                   addr = READ_WORD(addr);          break;
			case 0xa8: addr = y + EXTEND8(READ_PI(pc)); break;
			case 0xb8: addr = y + EXTEND8(READ_PI(pc));
                                   addr = READ_WORD(addr);          break;
			case 0xc8: addr = u + EXTEND8(READ_PI(pc)); break;
			case 0xd8: addr = u + EXTEND8(READ_PI(pc));
                                   addr = READ_WORD(addr);          break;
			case 0xe8: addr = s + EXTEND8(READ_PI(pc)); break;
			case 0xf8: addr = s + EXTEND8(READ_PI(pc));
                                   addr = READ_WORD(addr);          break;
			// (+/- 15 bit offset),R
			case 0x89: addr = x + READ_WORD(pc); pc += 2; break;
			case 0x99: addr = x + READ_WORD(pc); pc += 2;
                                   addr = READ_WORD(addr);            break;
			case 0xa9: addr = y + READ_WORD(pc); pc += 2; break;
			case 0xb9: addr = y + READ_WORD(pc); pc += 2;
                                   addr = READ_WORD(addr);            break;
			case 0xc9: addr = u + READ_WORD(pc); pc += 2; break;
			case 0xd9: addr = u + READ_WORD(pc); pc += 2;
                                   addr = READ_WORD(addr);            break;
			case 0xe9: addr = s + READ_WORD(pc); pc += 2; break;
			case 0xf9: addr = s + READ_WORD(pc); pc += 2;
                                   addr = READ_WORD(addr);            break;
			// (+/- D),R
			case 0x8b: addr = d + x;            break;
			case 0x9b: addr = READ_WORD(d + x); break;
			case 0xab: addr = d + y;            break;
			case 0xbb: addr = READ_WORD(d + y); break;
			case 0xcb: addr = d + u;            break;
			case 0xdb: addr = READ_WORD(d + u); break;
			case 0xeb: addr = d + s;            break;
			case 0xfb: addr = READ_WORD(d + s); break;
			// (+/- 7 bit offset), PC
			case 0x8c: case 0xac: case 0xcc: case 0xec:
				addr = EXTEND8(READ_PI(pc));
				addr += pc;                    break;
			case 0x9c: case 0xbc: case 0xdc: case 0xfc:
				addr = EXTEND8(READ_PI(pc));
				addr = READ_WORD(addr + pc);   break;
			// (+/- 15 bit offset), PC
			case 0x8d: case 0xad: case 0xcd: case 0xed:
				addr = READ_WORD(pc); pc += 2;
				addr += pc;                    break;
			case 0x9d: case 0xbd: case 0xdd: case 0xfd:
				addr = READ_WORD(pc); pc += 2;
				addr = READ_WORD(addr + pc);   break;
			// [address]
			case 0x9f:
				addr = READ_WORD(pc);
				addr = READ_WORD(addr); pc += 2; break;
			default: --pc;
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
	if (save_state) {
		cc.bit.e = 1;
		psh(0xff, s, u);
	}
	cc.bit.f = cc.bit.i = 1;
	pc = READ_WORD(0xfffc);
}

inline void Mc6809::firq(bool save_state)
{
	if (save_state) {
		cc.bit.e = 0;
		psh(0x81, s, u);
	}
	cc.bit.f = cc.bit.i = 1;
	pc = READ_WORD(0xfff6);
}

inline void Mc6809::irq(bool save_state)
{
	if (save_state) {
		cc.bit.e = 1;
		psh(0xff, s, u);
	}
	cc.bit.i = 1;			// Sw: don't set flag f !!
	pc = READ_WORD(0xfff8);
}


#endif // ifndef FASTFLEX

t_cycles Mc6809::exec_irqs(bool save_state)
{
    if (events & (DO_IRQ | DO_FIRQ | DO_NMI)) {
	    if ((events & DO_NMI) && !nmi_armed) {
 			++interrupt_status.count[INT_NMI];
		    EXEC_NMI(save_state);
		    events &= ~DO_NMI;
	    } else if ((events & DO_FIRQ) && !CC_BITF) {
 			++interrupt_status.count[INT_FIRQ];
		    EXEC_FIRQ(save_state);
		    events &= ~DO_FIRQ;
	    } else if ((events & DO_IRQ) && !CC_BITI) {
 			++interrupt_status.count[INT_IRQ];
		    EXEC_IRQ(save_state);
		    events &= ~DO_IRQ;
	    } // else
    }  // if
    return 5; // rounded
}  // exec_irqs

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
	} else
	{
#ifdef FASTFLEX
		return total_cycles + (cycles / 10);
#else
		return total_cycles +  cycles;
#endif
	}
}

#ifdef USE_GCCASM
inline void Mc6809::update_cc()
{
	cc.bit.c = x86flags.bit.cf;
	cc.bit.v = x86flags.bit.of;
	cc.bit.z = x86flags.bit.zf;
	cc.bit.n = x86flags.bit.sf;
	cc.bit.h = x86flags.bit.af;
}

inline void Mc6809::update_x86flags()
{
	x86flags.bit.cf = cc.bit.c;
	x86flags.bit.of = cc.bit.v;
	x86flags.bit.zf = cc.bit.z;
	x86flags.bit.sf = cc.bit.n;
	x86flags.bit.af = cc.bit.h;
}
#endif

// The caller is responsible for deleting the object
CpuStatus *Mc6809::get_status()
{

    DWord flags;
    char *pmnem_buf, *pbuffer;
    Word i, mem_addr;
    Mc6809CpuStatus *stat = new Mc6809CpuStatus;

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
#ifdef USE_GCCASM
    //update_cc();
#endif
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
        stat->instruction[i] = READ(stat->pc + i);
    for (i = 0; i < 48; i++)
        stat->memory[i] = READ(mem_addr + i);
    if (!Disassemble(stat->pc, &flags, &pbuffer, &pmnem_buf))
       stat->mnemonic[0] = '\0';
    else
       strcpy(stat->mnemonic, pmnem_buf);
    stat->total_cycles = get_cycles();
    return stat;
}  // get_status

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
			if (disassembler == NULL || !(flags & DA_SUB)) {
				events |= DO_SINGLESTEP | IGNORE_BP;
				reset_bp(2);
			} else {
				events |= DO_BREAKPOINT | IGNORE_BP;
			}
		}
		events &= ~DO_GO_BACK;
		break;
   case START_RUNNING:
		reset_bp(2);
		if (events & DO_BREAKPOINT)
			events |= IGNORE_BP;
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

   while (1) {
	if (events) {
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
					reset_bp(2);
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
				events |= DO_SINGLESTEPFINISHED;
			else {
				newState = S_STOP;
				break;
			}
		   }
                   if (events & DO_CWAI)
                   {
			if ((events & DO_IRQ)  && !CC_BITI ||
			    (events & DO_FIRQ) && !CC_BITF ||
			    (events & DO_NMI)  && !nmi_armed)
			{
				events &= ~DO_CWAI;
				PC += 2;
				cycles += exec_irqs(false);
			} else
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
			} else
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
				do_logging = true;
			if (lfs.stopAddr < 0x10000 && PC == lfs.stopAddr)
				do_logging = false;
			if (do_logging && disassembler != NULL &&
				PC >= lfs.minAddr && PC <= lfs.maxAddr) {
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
		events &= ~DO_FREQ_CONTROL;
	else
		events |= DO_FREQ_CONTROL;
}

