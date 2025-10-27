/*
    testmain.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2024-2025  W. Schwotzer

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


#include "test_gccasm.h"


#if defined(__GNUC__) && defined(__x86_64__)
int main(int argc, char *argv[])
#else
int main()
#endif
{
    bool failure = false;

#if defined(__GNUC__) && defined(__x86_64__)
    init_test_gccasm(argc, argv);

    failure |= !test_gccasm_add8();
    failure |= !test_gccasm_add16();
    failure |= !test_gccasm_adc();
    failure |= !test_gccasm_sub8();
    failure |= !test_gccasm_sub16();
    failure |= !test_gccasm_sbc();
    failure |= !test_gccasm_inc();
    failure |= !test_gccasm_dec();
    failure |= !test_gccasm_neg();
    failure |= !test_gccasm_tst();
    failure |= !test_gccasm_cmp8();
    failure |= !test_gccasm_cmp16();
    failure |= !test_gccasm_lsl();
    failure |= !test_gccasm_lsr();
    failure |= !test_gccasm_asr();
    failure |= !test_gccasm_ror();
    failure |= !test_gccasm_rol();
#endif

    return failure ? 1 : 0;
}

