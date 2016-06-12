/*
    mc6809.h

    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2004  W. Schwotzer

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

// test using GCC inline assembler
#ifdef __CPU
  #if __CPU==i386 || __CPU==i486 || __CPU==i586 || __CPU==i686 || __CPU==i786
    #define __IX86
  #endif
#endif
#if defined(__GNUC__) && defined (USE_ASM) && defined(__IX86) && !defined(FASTFLEX)
  #define USE_GCCASM
#endif

#define DO_NMI			0x01
#define DO_FIRQ			0x02
#define DO_IRQ			0x04
#define DO_INVALID		0x08
#define DO_BREAKPOINT		0x10
#define DO_SINGLESTEP		0x20
#define DO_SINGLESTEPFINISHED	0x40
#define DO_SYNCEXEC		0x80
#define DO_TIMER		0x100
#define DO_SET_STATUS		0x200
#define DO_FREQ_CONTROL		0x400
#define DO_GO_BACK		0x800
#define DO_LOG			0x1000
#define DO_CWAI			0x2000
#define DO_SYNC			0x4000
#define IGNORE_BP               0x8000

#define SINGLESTEP_OVER		0x54
#define SINGLESTEP_INTO		0x55
#define START_RUNNING		0x56
#define CONTINUE_RUNNING	0x57


#define SYNC_INSTR		1
#define CWAI_INSTR		2

#ifdef USE_GCCASM
   #ifdef BITFIELDS_LSB_FIRST
      #define CC_BIT_C		0x01
      #define CC_BIT_V		0x02
      #define CC_BIT_Z		0x04
      #define CC_BIT_N		0x08
      #define CC_BIT_H		0x20
   #else
      #define CC_BIT_C		0x80
      #define CC_BIT_V		0x40
      #define CC_BIT_Z		0x20
      #define CC_BIT_N		0x10
      #define CC_BIT_H		0x04
   #endif
   #define CC_BITS_HNZVC (CC_BIT_H | CC_BIT_N | CC_BIT_Z | CC_BIT_V | CC_BIT_C)
   #define CC_BITS_NZVC  (CC_BIT_N | CC_BIT_Z | CC_BIT_V | CC_BIT_C)
   #define CC_BITS_NZV   (CC_BIT_N | CC_BIT_Z | CC_BIT_V)
   #define CC_BITS_NZC   (CC_BIT_N | CC_BIT_Z | CC_BIT_C)
#endif

/* Bugfix for a error in GNU compiler in V3.3.0
   in which functions with specific paramaters can not be inlined */
#ifdef __GNUC__
  #if (__GNUC__ == 3) && (__GNUC_MINOR__ == 3) && (__GNUC_PATCHLEVEL__ == 0)
    #define INLINE1
  #else
    #define INLINE1 inline
  #endif
#else
 #define INLINE1 inline
#endif

// uncomment this for debug output of cpu
//#define	DEBUG_FILE	"cpu.txt"


struct s_cpu_logfile {
	unsigned int minAddr;
	unsigned int maxAddr;
	unsigned int startAddr;
	unsigned int stopAddr;
	char logFileName[PATH_MAX];
};

class BTime;
class Da6809;


class Mc6809 : public ScheduledCpu
{

// Processor registers
protected:

	Byte			indexed_cycles[256];	// add. cycles for
							// indexed addr.
	Byte			psh_pul_cycles[256];	// add. cycles for psh
							// and pull-instr. 
#ifdef USE_GCCASM
	Byte			cc_hnzvc_from_x86flags[1 << 16];
	Byte			cc_nzvc_from_x86flags[1 << 16];
	Byte			cc_nzv_from_x86flags[1 << 16];
	Byte			cc_nzc_from_x86flags[1 << 16];
#endif
	Byte			nmi_armed;		// for handling
							// interrupts
	Word			events;			// event status flags
	tInterruptStatus	interrupt_status;
#ifdef FASTFLEX
	Word			ipcreg, iureg, isreg, ixreg, iyreg;
	Byte			iareg, ibreg, iccreg, idpreg;
	Word			eaddr;
	Byte			ireg;
	Byte			iflag;
	Byte			tb;
	Word			tw;
	Byte			k;
	Byte			*pMem;		// needed for memory access
#else
	Word			pc;
	Word			u, s;		// Stack pointers
	Word			x, y;		// Index registers
	union {
		Word			d;	// Combined accumulator
		struct {
#ifdef WORDS_BIGENDIAN
			Byte		a;	// Accumulator a
			Byte		b;	// Accumulator b
#else
			Byte		b;	// Accumulator b
			Byte		a;	// Accumulator a
#endif
		} byte;
	} acc;
	union {
		Word			dp16;
		struct {
#ifdef WORDS_BIGENDIAN
			Byte		h;
			Byte		l;
#else
			Byte		l;
			Byte		h;
#endif
		} byte;
	} dpreg;
	Byte&			a;
	Byte&			b;
	Word&			d;
	Byte&			dp;
	union {
		Byte			all;	// Condition code register
		struct {
#ifdef BITFIELDS_LSB_FIRST
			bool		c : 1;	// Carry
			bool		v : 1;	// Overflow
			bool		z : 1;	// Zero
			bool		n : 1;	// Negative
			bool		i : 1;	// IRQ disable
			bool		h : 1;	// Half carry
			bool		f : 1;	// FIRQ disable
			bool		e : 1;	// Entire
#else
			bool		e : 1;	// Entire
			bool		f : 1;	// FIRQ disable
			bool		h : 1;	// Half carry
			bool		i : 1;	// IRQ disable
			bool		n : 1;	// Negative
			bool		z : 1;	// Zero
			bool		v : 1;	// Overflow
			bool		c : 1;	// Carry
#endif
		} bit;
	} cc;
#ifdef USE_GCCASM
	union {
		DWord			all;
		struct {
#ifdef WORDS_BIGENDIAN
			Word		h;
			Word		l;
#else
			Word		l;
			Word		h;
#endif
		} word;
		struct {
#ifdef BITFIELDS_LSB_FIRST
			bool		cf    : 1; // carry flag
			bool		one   : 1; // always one
			bool		pf    : 1; // parity flag
			bool		zero1 : 1; // always zero
			bool		af    : 1; // auxilliary carry flag
			bool		zero2 : 1; // always zero
			bool		zf    : 1; // zero flag
			bool		sf    : 1; // sign flag
			bool		tf    : 1; // trap flag
			bool		_if   : 1; // interrupt enable flag
			bool		df    : 1; // direction flag
			bool		of    : 1; // overflow flag 
			bool		iopl0 : 1; // i/o previlidge level
			bool		iopl1 : 1; // i/o previlidge level
			bool		nt    : 1; // nested task 
			bool		zero3 : 1; // always zero
#else
			bool		zero3 : 1; // always zero
			bool		nt    : 1; // nested task 
			bool		iopl1 : 1; // i/o previlidge level
			bool		iopl0 : 1; // i/o previlidge level
			bool		of    : 1; // overflow flag 
			bool		df    : 1; // direction flag
			bool		_if   : 1; // interrupt enable flag
			bool		tf    : 1; // trap flag
			bool		sf    : 1; // sign flag
			bool		zf    : 1; // zero flag
			bool		zero2 : 1; // always zero
			bool		af    : 1; // auxilliary carry flag
			bool		zero1 : 1; // always zero
			bool		pf    : 1; // parity flag
			bool		one   : 1; // always one
			bool		cf    : 1; // carry flag
#endif
		} bit;
	} x86flags;
#endif
#endif // FASTFLEX

protected:

	Da6809*		disassembler;

// funcitons for instruction execution:

private:

	void			init(void);
	void			init_indexed_cycles(void);
	void			init_psh_pul_cycles(void);
#ifdef USE_GCCASM
	void			init_cc_hnzvc_from_x86flags(Byte a[1 << 16],
					Byte type);
#endif
	void			illegal();
#ifndef FASTFLEX
	inline Word		fetch_ea_ext(void);
	inline Word		fetch_ea_dir(void);
	inline Word		fetch_ea_idx(t_cycles *c);
	inline Byte		fetch_imm_08(void);
	inline Byte		fetch_ext_08(void);
	inline Byte		fetch_dir_08(void);
	inline Byte		fetch_idx_08(t_cycles *c);
	inline Word		fetch_imm_16(void);
	inline Word		fetch_ext_16(void);
	inline Word		fetch_dir_16(void);
	inline Word		fetch_idx_16(t_cycles *c);

	Word		do_effective_address(Byte);

	inline void		abx();
	inline void		and_(Byte &reg, Byte operand);
	inline void		andcc(Byte operand);
	INLINE1 void		asr(Byte &reg);
	inline void		asr(Word addr);
	inline void		bcc();
	inline t_cycles		lbcc();
	inline void		bcs();
	inline t_cycles		lbcs();
	inline void		beq();
	inline t_cycles		lbeq();
	inline void		bge();
	inline t_cycles		lbge();
	inline void		bgt();
	inline t_cycles		lbgt();
	inline void		bhi();
	inline t_cycles		lbhi();
	inline void		ble();
	inline t_cycles		lble();
	inline void		bls();
	inline t_cycles		lbls();
	inline void		blt();
	inline t_cycles		lblt();
	inline void		bmi();
	inline t_cycles		lbmi();
	inline void		bne();
	inline t_cycles		lbne();
	inline void		bpl();
	inline t_cycles		lbpl();
	inline void		bra();
	inline t_cycles		lbra();
	inline void		brn();
	inline t_cycles		lbrn();
	inline void		bsr();
	inline t_cycles		lbsr();
	inline void		bvc();
	inline t_cycles		lbvc();
	inline void		bvs();
	inline t_cycles		lbvs();
	inline void		bit(Byte reg, Byte operand);
	INLINE1 void		clr(Byte &reg);
	inline void		clr(Word addr);
	inline void		cmp(Byte reg, Byte operand);
	inline void		cmp(Word reg, Word operand);
	INLINE1 void		com(Byte &reg);
	inline void		com(Word addr);
	inline void		daa();
	INLINE1 void		dec(Byte &reg);
	inline void		dec(Word addr);
	inline void		eor(Byte &reg, Byte operand);
	inline void		exg();
	inline void		inc(Word addr);
	inline void		inc(Byte &reg);
	inline void		jsr(Word addr);
	inline void		ld(Byte &reg, Byte operand);
	inline void		ld(Word &reg, Word operand);
	inline void		lea(Word &reg, Word addr); 
	inline void		lea_nocc(Word &reg, Word addr); 
	INLINE1 void		lsl(Byte &reg);
	INLINE1 void		lsr(Byte &reg);
	inline void		lsr(Word addr);
	inline void		lsl(Word addr);
	inline void		mul();
	inline void		jmp(Word addr);
	inline void		neg(Byte &reg);
	inline void		neg(Word addr);
	INLINE1 void		tst(Byte &reg);
	inline void		tst(Word addr);
	inline void		nop();
	inline void		or_(Byte &reg, Byte operand);
	inline void		orcc(Byte operand);
	inline t_cycles		psh(Byte what, Word &s, Word &u);
	inline t_cycles		pul(Byte what, Word &s, Word &u);
	INLINE1 void		rol(Byte &reg);
	inline void		rol(Word addr);
	INLINE1 void		ror(Byte &reg);
	inline void		ror(Word addr);
	inline t_cycles		rti();
	inline void		rts();
	inline void		sex();
	inline void		st(Byte &reg, Word addr);
	inline void		st(Word &reg, Word addr);
	inline void		adc(Byte &reg, Byte operand);
	inline void 		add(Byte &reg, Byte operand);
	inline void 		add(Word &reg, Word operand);
	inline void		sbc(Byte &reg, Byte operand);
	inline void 		sub(Byte &reg, Byte operand);
	inline void 		sub(Word &reg, Word operand);
	inline void		swi(), swi2(), swi3();
	inline void		cwai(), sync();
	inline void		tfr();

	inline void		do_br(int);
	inline t_cycles		do_lbr(int);

	// additional undocumented instructions
	void			rst();
	void			negcom(Word addr);
	void			clr1(Byte &reg);
#endif
	inline void		nmi(bool save_state);
	inline void		firq(bool save_state);
	inline void		irq(bool save_state);
	void			invalid(const char *pmessage);
	bool			use_undocumented;
public:
	void			set_use_undocumented(bool b);
	bool			is_use_undocumented() { return use_undocumented; };

// Scheduler Interface implemenation
public:
	void			do_reset(void);
	Byte			run(Word mode);
	void			exit_run(void);
	QWord			get_cycles(bool reset = false);
	CpuStatus		*get_status();
	void			set_required_cyclecount(t_cycles x_cycles);
protected:
	Byte			runloop();

// interrupt handling:
public:
	void			reset(void);		// CPU reset
	t_cycles		exec_irqs(bool save_state = true);
	void			set_nmi(void);
	void			set_firq(void);
	void			set_irq(void);

protected:
	QWord			total_cycles; // total cycle count with 64 Bit resolution
	t_cycles		cycles;		// cycle cnt for one timer tick
	t_cycles		required_cyclecount;//cycle count for freq ctrl

// breakpoint support
protected:
		unsigned int	bp[3];
public:
		void		set_bp(int which, Word address);
		unsigned int	get_bp(int which);
		int		is_bp_set(int which);
		void		reset_bp(int which);

// interface to other classes
public:
		void		get_interrupt_status(tInterruptStatus &s);
		void		set_disassembler(Da6809 *x_da);
		void		set_serpar(Byte b);
		bool		set_logfile(const struct s_cpu_logfile *lf);
protected:
		int		Disassemble(Word address, DWord *pFlags,
					char **pb1, char **pb2);
#ifdef USE_GCCASM
		inline void	update_cc();
		inline void	update_x86flags();
#endif
		FILE		*log_fp;
		bool		do_logging;
		struct s_cpu_logfile lfs;

		Memory		*memory;

// Public constructor and destructor
public:
		Mc6809(Memory *x_memory);
	virtual	~Mc6809();
};
#endif // __mc6809_h__

