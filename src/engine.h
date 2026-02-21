/* engine.h

   flexemu, an MC6809 emulator running FLEX
   Copyright (C) 1997-2026  W. Schwotzer

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
                        char must be 8 bits.
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

#ifndef ENGINE_INCLUDED
#define ENGINE_INCLUDED

#define engine


#define GETBYTE(a) memory.read_byte(a)
#define GETBYTE_PI(a) memory.read_byte((a)++) // get byte with post increment
#define GETWORD(a) memory.read_word(a)
#define SETBYTE(a,n) memory.write_byte(a, static_cast<Byte>(n));
#define SETWORD(a,n) memory.write_word(a, n);

/* Two bytes of a word are fetched separately because of
   the possible wrap-around at address $ffff and alignment
*/

#define IMMBYTE(b) b=memory.read_byte(ipcreg++)
#define IMMWORD(w) {(w)=GETWORD(ipcreg);ipcreg+=2;}

#define PUSHBYTE(b) {--isreg;SETBYTE(isreg,b);}
#define PUSHWORD(w) {isreg-=2;SETWORD(isreg,w);}
#define PULLWORD(w) {(w)=GETWORD(isreg);isreg+=2;}
#define PSHUBYTE(b) {--iureg;SETBYTE(iureg,b);}
#define PSHUWORD(w) {iureg-=2;SETWORD(iureg,w);}
#define PULUWORD(w) {(w)=GETWORD(iureg);iureg+=2;}
#define PULLBYTE(b) b=memory.read_byte(isreg++)
#define PULUBYTE(b) b=memory.read_byte(iureg++)

#define SIGNED(b) (static_cast<Word>((b)&0x80?(b)|0xff00:(b)))

#define GETDREG ((iareg<<8)|ibreg)
#define SETDREG(n) {iareg=static_cast<Byte>((n)>>8);ibreg=static_cast<Byte>(n);}

/* Macros for addressing modes (postbytes have their own code) */
#define DIRECT {IMMBYTE(eaddr); eaddr|=(idpreg<<8);}
#define IMM8 {eaddr=ipcreg++;}
#define IMM16 {eaddr=ipcreg;ipcreg+=2;}
#define EXTENDED {IMMWORD(eaddr);}

/* macros to set status flags */
#define SEC iccreg|=0x01
#define CLC iccreg&=0xfe
#define SEZ iccreg|=0x04
#define CLZ iccreg&=0xfb
#define SEN iccreg|=0x08
#define CLN iccreg&=0xf7
#define SEV iccreg|=0x02
#define CLV iccreg&=0xfd
#define SEH iccreg|=0x20
#define CLH iccreg&=0xdf

/* handling illegal instructions */
#define INVALID_INSTR invalid("instruction")
#define INVALID_POST ipcreg--; invalid("indirect addressing postbyte")
#define INVALID_EXGTFR ipcreg--; invalid("exchange/transfer register")

/* set N and Z flags depending on 8 or 16 bit result */
#define SETNZ8(b) {if(b)CLZ; else SEZ; if((b)&0x80)SEN; else CLN;}
#define SETNZ16(b) {if(b)CLZ; else SEZ; if((b)&0x8000)SEN; else CLN;}

#define SETSTATUS(a,b,res) if(((a)^(b)^(res))&0x10) SEH; else CLH; \
            if(((a)^(b)^(res)^((res)>>1))&0x80)SEV; else CLV; \
                    if((res)&0x100)SEC; else CLC; SETNZ8(static_cast<Byte>(res))

#define SETSTATUSD(a,b,res) {if((res)&0x10000) SEC; else CLC; \
                if((((res)>>1)^(a)^(b)^(res))&0x8000) SEV; else CLV; \
                        SETNZ16(static_cast<Word>(res));}

/* Macros for branch instructions */
#define BRANCH(f) if(!iflag){IMMBYTE(tb); if(f)ipcreg+=SIGNED(tb);}\
    else{IMMWORD(tw); if(f)ipcreg+=tw;}
#define NXORV  ((iccreg&0x08)^((iccreg&0x02)<<2))

/* MAcros for setting/getting registers in TFR/EXG instructions */
#define GETREG(val,reg) switch(reg) {\
    case 0: (val)=GETDREG;break;\
    case 1: (val)=ixreg;break;\
    case 2: (val)=iyreg;break;\
    case 3: (val)=iureg;break;\
    case 4: (val)=isreg;break;\
    case 5: (val)=ipcreg;break;\
    case 8: (val)=static_cast<Byte>(iareg);break;\
    case 9: (val)=static_cast<Byte>(ibreg);break;\
    case 10: (val)=static_cast<Byte>(iccreg);break;\
    case 11: (val)=static_cast<Byte>(idpreg);break;\
    default: (val)=0; INVALID_EXGTFR; break;}

#define SETREG(val,reg) switch(reg) {\
    case 0: SETDREG(val); break;\
    case 1: ixreg=val;break;\
    case 2: iyreg=val;break;\
    case 3: iureg=val;break;\
    case 4: isreg=val;break;\
    case 5: ipcreg=val;break;\
    case 8: iareg=static_cast<Byte>(val);break;\
    case 9: ibreg=static_cast<Byte>(val);break;\
    case 10: iccreg=static_cast<Byte>(val);break;\
    case 11: idpreg=static_cast<Byte>(val);break;\
    default: INVALID_EXGTFR; break;}

/* Macros for load and store of accumulators. Can be modified to check
   for port addresses */

#define LOADAC(reg) reg=GETBYTE(eaddr)
#define STOREAC(reg) SETBYTE(eaddr,reg)

#define LOADREGS
#define SAVEREGS

#define PUSH_ENTIRE PUSHWORD(ipcreg);\
    PUSHWORD(iureg);\
    PUSHWORD(iyreg);\
    PUSHWORD(ixreg);\
    PUSHBYTE(idpreg);\
    PUSHBYTE(ibreg);\
    PUSHBYTE(iareg);\
    PUSHBYTE(iccreg)

#endif

