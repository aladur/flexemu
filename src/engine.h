/* engine.h

   flexemu, an MC6809 emulator running FLEX
   Copyright (C) 1997-2018  W. Schwotzer

   Copyright 1994,1995 L.C. Benschop, Eidnhoven The Netherlands.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   This program simulates a 6809 processor.

   System dependencies: short must be 16 bits.
                        char  must be 8 bits.
                        long must be more than 16 bits.
                        arrays up to 65536 bytes must be supported.
                        machine must be twos complement.
   Most Unix machines will work. For MSODS you need long pointers
   and you may have to malloc() the mem array of 65536 bytes.

   Special instructions:
   SWI2 writes char to stdout from register B.
   SWI3 reads char from stdout to register B, sets carry at EOF.
               (or when no key available when using term control).
   SWI retains its normal function.
   CWAI and SYNC stop simulator.
   Note: special instructions are gone for now.

   ACIA emulation at port $E000

   Note: BIG_ENDIAN option is no longer needed.
*/

#ifndef __ENGINE_H__
#define __ENGINE_H__

#define engine

#ifndef FASTFLEX
    #include "v09.h"
#else
    #include "intmem.h"
#endif

Byte aca, acb;
Byte *breg = &aca, *areg = &acb;
#ifndef FASTFLEX
    static int tracetrick = 0;
#endif

#ifdef FASTFLEX
    #define GETBYTE(a)  READ(a)
    #define GETBYTE_PI(a)   READ_PI(a) // get byte with post increment
    #define GETWORD(a)  READ_WORD(a)
    #define SETBYTE(a,n)    WRITE(a, (Byte)n);
    #define SETWORD(a,n)    WRITE_WORD(a, n);
#else
    #define GETBYTE(a) (mem[a])
    #define GETBYTE_PI(a) (mem[a])
    #define GETWORD(a) (mem[a]<<8|mem[(a)+1])
    #define SETBYTE(a,n) {if(!(a&0x8000))mem[a]=n;}
    #define SETWORD(a,n) if(!(a&0x8000)){mem[a]=(n)>>8;mem[(a)+1]=n;}
#endif
/* Two bytes of a word are fetched separately because of
   the possible wrap-around at address $ffff and alignment
*/

#ifdef FASTFLEX
    #define IMMBYTE(b) b=READ_PI(ipcreg);
#else
    #define IMMBYTE(b) b=mem[ipcreg++];
#endif
#define IMMWORD(w) {w=GETWORD(ipcreg);ipcreg+=2;}

#define PUSHBYTE(b) {--isreg;SETBYTE(isreg,b)}
#define PUSHWORD(w) {isreg-=2;SETWORD(isreg,w)}
#define PULLWORD(w) {w=GETWORD(isreg);isreg+=2;}
#define PSHUBYTE(b) {--iureg;SETBYTE(iureg,b)}
#define PSHUWORD(w) {iureg-=2;SETWORD(iureg,w)}
#define PULUWORD(w) {w=GETWORD(iureg);iureg+=2;}
#ifdef FASTFLEX
    #define PULLBYTE(b) b=READ_PI(isreg);
    #define PULUBYTE(b) b=READ_PI(iureg);
#else
    #define PULLBYTE(b) b=mem[isreg++];
    #define PULUBYTE(b) b=mem[iureg++];
#endif

#define SIGNED(b) ((Word)(b&0x80?b|0xff00:b))

#define GETDREG ((iareg<<8)|ibreg)
#define SETDREG(n) {iareg=(Byte)((n)>>8);ibreg=(Byte)(n);}

/* Macros for addressing modes (postbytes have their own code) */
#define DIRECT {IMMBYTE(eaddr) eaddr|=(idpreg<<8);}
#define IMM8 {eaddr=ipcreg++;}
#define IMM16 {eaddr=ipcreg;ipcreg+=2;}
#define EXTENDED {IMMWORD(eaddr)}

/* macros to set status flags */
#define SEC iccreg|=0x01;
#define CLC iccreg&=0xfe;
#define SEZ iccreg|=0x04;
#define CLZ iccreg&=0xfb;
#define SEN iccreg|=0x08;
#define CLN iccreg&=0xf7;
#define SEV iccreg|=0x02;
#define CLV iccreg&=0xfd;
#define SEH iccreg|=0x20;
#define CLH iccreg&=0xdf;

/* handling illegal instructions */
#ifdef FASTFLEX
    #define INVALID_INSTR invalid("instruction");
    #define INVALID_POST ipcreg--; invalid("indirect addressing postbyte");
    #define INVALID_EXGTFR ipcreg--; invalid("exchange/transfer register");
#else
    #define INVALID_INSTR
    #define INVALID_POST
    #define INVALID_EXGTFR
#endif

/* set N and Z flags depending on 8 or 16 bit result */
#define SETNZ8(b) {if(b)CLZ else SEZ if(b&0x80)SEN else CLN}
#define SETNZ16(b) {if(b)CLZ else SEZ if(b&0x8000)SEN else CLN}

#define SETSTATUS(a,b,res) if((a^b^res)&0x10) SEH else CLH \
            if((a^b^res^(res>>1))&0x80)SEV else CLV \
                    if(res&0x100)SEC else CLC SETNZ8((Byte)res)

#define SETSTATUSD(a,b,res) {if(res&0x10000) SEC else CLC \
                if(((res>>1)^a^b^res)&0x8000) SEV else CLV \
                        SETNZ16((Word)res)}

/* Macros for branch instructions */
#define BRANCH(f) if(!iflag){IMMBYTE(tb) if(f)ipcreg+=SIGNED(tb);}\
    else{IMMWORD(tw) if(f)ipcreg+=tw;}
#define NXORV  ((iccreg&0x08)^((iccreg&0x02)<<2))

/* MAcros for setting/getting registers in TFR/EXG instructions */
#define GETREG(val,reg) switch(reg) {\
    case 0: val=GETDREG;break;\
    case 1: val=ixreg;break;\
    case 2: val=iyreg;break;\
    case 3: val=iureg;break;\
    case 4: val=isreg;break;\
    case 5: val=ipcreg;break;\
    case 8: val=(Byte)iareg;break;\
    case 9: val=(Byte)ibreg;break;\
    case 10: val=(Byte)iccreg;break;\
    case 11: val=(Byte)idpreg;break;\
    default: val=0; INVALID_EXGTFR break;};

#define SETREG(val,reg) switch(reg) {\
    case 0: SETDREG(val) break;\
    case 1: ixreg=val;break;\
    case 2: iyreg=val;break;\
    case 3: iureg=val;break;\
    case 4: isreg=val;break;\
    case 5: ipcreg=val;break;\
    case 8: iareg=(Byte)val;break;\
    case 9: ibreg=(Byte)val;break;\
    case 10: iccreg=(Byte)val;break;\
    case 11: idpreg=(Byte)val;break;\
    default: INVALID_EXGTFR break;};

/* Macros for load and store of accumulators. Can be modified to check
   for port addresses */

#ifdef FASTFLEX
#define LOADAC(reg) reg=GETBYTE(eaddr);
#define STOREAC(reg) SETBYTE(eaddr,reg);
#else
#define LOADAC(reg) if((eaddr&0xff00)!=IOPAGE)reg=mem[eaddr];else\
        reg=do_input(eaddr&0xff);
#define STOREAC(reg) if((eaddr&0xff00)!=IOPAGE)SETBYTE(eaddr,reg)else\
            do_output(eaddr&0xff,reg);
#endif

#ifdef FASTFLEX
#define LOADREGS
#define SAVEREGS
#else
#define LOADREGS ixreg=xreg;iyreg=yreg;\
    iureg=ureg;isreg=sreg;\
    ipcreg=pcreg;\
    iareg=*areg;ibreg=*breg;\
    idpreg=dpreg;iccreg=ccreg;

#define SAVEREGS xreg=ixreg;yreg=iyreg;\
    ureg=iureg;sreg=isreg;\
    pcreg=ipcreg;\
    *areg=iareg;*breg=ibreg;\
    dpreg=idpreg;ccreg=iccreg;
#endif

#define PUSH_ENTIRE  PUSHWORD(ipcreg)\
    PUSHWORD(iureg)\
    PUSHWORD(iyreg)\
    PUSHWORD(ixreg)\
    PUSHBYTE(idpreg)\
    PUSHBYTE(ibreg)\
    PUSHBYTE(iareg)\
    PUSHBYTE(iccreg)

#ifdef FASTFLEX
#define EXEC_IRQ(save_state)    if (save_state) {\
        PUSH_ENTIRE\
    }\
    iccreg|=0x90;\
    ipcreg=GETWORD(0xfff8);
#else
#define EXEC_IRQ    PUSH_ENTIRE\
    iccreg|=0x90;\
    ipcreg=GETWORD(0xfff8);
#endif

#ifdef FASTFLEX
#define EXEC_NMI(save_state)    if (save_state) {\
        PUSH_ENTIRE\
    }\
    iccreg|=0xD0;\
    ipcreg=GETWORD(0xfffc);
#else
#define EXEC_NMI    PUSH_ENTIRE\
    iccreg|=0xD0;\
    ipcreg=GETWORD(0xfffc);
#endif

#ifdef FASTFLEX
#define EXEC_FIRQ(save_state)   if (save_state) {\
        PUSHWORD(ipcreg)\
        PUSHBYTE(iccreg)\
        iccreg&=0x7f;\
    }\
    iccreg|=0x50;\
    ipcreg=GETWORD(0xfff6);
#else
#define EXEC_FIRQ    PUSHWORD(ipcreg)\
    PUSHBYTE(iccreg)\
    iccreg&=0x7f;\
    iccreg|=0x50;\
    ipcreg=GETWORD(0xfff6);
#endif

unsigned char haspostbyte[] =
{
    /*0*/      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*1*/      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*2*/      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*3*/      1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*4*/      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*5*/      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*6*/      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /*7*/      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*8*/      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*9*/      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*A*/      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /*B*/      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*C*/      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*D*/      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*E*/      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /*F*/      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

#endif

