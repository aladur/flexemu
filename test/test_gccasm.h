/*
    test_gccasm.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2024-2026  W. Schwotzer

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
*/


#ifndef TEST_GCCASM_INCLUDED
#define TEST_GCCASM_INCLUDED


/* Parameter comes from main(). */
/* NOLINTNEXTLINE(modernize-avoid-c-arrays) */
extern void init_test_gccasm(int argc, char *argv[]);

extern bool test_gccasm_add8();
extern bool test_gccasm_add16();
extern bool test_gccasm_adc();
extern bool test_gccasm_sub8();
extern bool test_gccasm_sub16();
extern bool test_gccasm_sbc();
extern bool test_gccasm_inc();
extern bool test_gccasm_dec();
extern bool test_gccasm_neg();
extern bool test_gccasm_tst();
extern bool test_gccasm_cmp8();
extern bool test_gccasm_cmp16();
extern bool test_gccasm_lsl();
extern bool test_gccasm_lsr();
extern bool test_gccasm_asr();
extern bool test_gccasm_ror();
extern bool test_gccasm_rol();

#endif

