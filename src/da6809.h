//
//  da6809.h
//

#ifndef DA6809_INCLUDED
#define DA6809_INCLUDED

#include "misc1.h"
#include "absdisas.h"


class Da6809 : public AbstractDisassembler
{

protected:

    char        code_buf[28]; // buffer for machinecode
    char        mnem_buf[28]; // buffer for mnemonic
    bool use_undocumented{false};

public:
    Da6809();
    ~Da6809() override = default;

    int Disassemble(
            const Byte *pMemory,
            DWord pc,
            InstFlg *pFlags,
            DWord *pAddr,
            char **pCode,
            char **pMnemonic) override;
    void set_use_undocumented(bool value) override;

private:

    inline Byte D_Page10(InstFlg *pFlags, Word pc, const Byte *pMemory,
            DWord *pAddr);
    inline Byte D_Page11(InstFlg *pFlags, Word pc, const Byte *pMemory,
            DWord *pAddr);
    inline Byte D_Illegal(const char *mnemo, Word pc, Byte bytes,
            const Byte *pMemory);
    inline Byte D_Direct(const char *mnemo, Word pc, Byte bytes,
            const Byte *pMemory);
    inline Byte D_Immediat(const char *mnemo, Word pc, Byte bytes,
            const Byte *pMemory);
    inline Byte D_ImmediatL(const char *mnemo, Word pc, Byte bytes,
            const Byte *pMemory, DWord *pAddr);
    inline Byte D_Inherent(const char *mnemo, Word pc, Byte bytes,
            const Byte *pMemory);
    Byte D_Indexed(const char *mnemo, Word pc, Byte bytes,
            const Byte *pMemory);
    inline Byte D_Extended(const char *mnemo, Word pc, Byte bytes,
            const Byte *pMemory, DWord *pAddr);
    inline Byte D_Relative(const char *mnemo, Word pc, Byte bytes,
            const Byte *pMemory, DWord *pAddr);
    inline Byte D_RelativeL(const char *mnemo, Word pc, Byte bytes,
            const Byte * pMemory, DWord *pAddr);
    inline Byte D_Register0(const char *mnemo, Word pc, Byte bytes,
            const Byte * pMemory);
    inline Byte D_Register1(const char *mnemo, Word pc, Byte bytes,
            const Byte * pMemory);
    inline Byte D_Register2(const char *mnemo, Word pc, Byte bytes,
            const Byte * pMemory);

    static const char *IndexedRegister(Byte which);
    static const char *InterRegister(Byte which);
    static const char *StackRegister(Byte which, const char *not_stack);
    static const char *FlexLabel(Word addr);
}; // class Da6809

#endif // DA6809_INCLUDED
