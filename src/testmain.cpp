#include "misc1.h"
#include "test_gccasm.h"


int main(int argc, char *argv[])
{
    bool failure = false;

#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
    init_test_gccasm(argc, argv);

    failure |= test_gccasm_add8();
    failure |= test_gccasm_add16();
    failure |= test_gccasm_adc();
    failure |= test_gccasm_sub8();
    failure |= test_gccasm_sub16();
    failure |= test_gccasm_sbc();
    failure |= test_gccasm_inc();
    failure |= test_gccasm_dec();
    failure |= test_gccasm_neg();
    failure |= test_gccasm_tst();
    failure |= test_gccasm_cmp8();
    failure |= test_gccasm_cmp16();
    failure |= test_gccasm_lsl();
    failure |= test_gccasm_lsr();
    failure |= test_gccasm_asr();
    failure |= test_gccasm_ror();
    failure |= test_gccasm_rol();
#endif

    return failure ? 1 : 0;
}

