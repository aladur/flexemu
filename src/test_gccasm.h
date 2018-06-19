#ifndef TEST_GCCASM_INCLUDED
#define TEST_GCCASM_INCLUDED

#include "misc1.h"


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

