//
//  da6809.h
//

#ifndef DA6809_INCLUDED
#define DA6809_INCLUDED

#include "absdisas.h"


class Da6809 : public AbstractDisassembler
{

protected:

    Word pc{};
    const Byte *memory{};
    bool use_undocumented{false};

public:
    Da6809() = default;
    ~Da6809() override = default;

    int Disassemble(
            const Byte *p_memory,
            DWord p_pc,
            InstFlg &p_flags,
            DWord &p_jumpaddr,
            std::string &p_code,
            std::string &p_mnemonic) override;
    void set_use_undocumented(bool value) override;

private:

    inline Byte D_Page10(InstFlg &p_flags, DWord &p_jumpaddr,
            std::string &p_code, std::string &p_mnemonic);
    inline Byte D_Page11(InstFlg &p_flags, std::string &p_code,
            std::string &p_mnemonic);
    inline Byte D_Illegal(const char *mnemo, Byte bytes, std::string &p_code,
            std::string &p_mnemonic);
    inline Byte D_Direct(const char *mnemo, Byte bytes, std::string &p_code,
            std::string &p_mnemonic);
    inline Byte D_Immediat(const char *mnemo, Byte bytes, std::string &p_code,
            std::string &p_mnemonic);
    inline Byte D_ImmediatL(const char *mnemo, Byte bytes, std::string &p_code,
            std::string &p_mnemonic);
    inline Byte D_Inherent(const char *mnemo, Byte bytes, std::string &p_code,
            std::string &p_mnemonic);
    Byte D_Indexed(const char *mnemo, Byte bytes, std::string &p_code,
            std::string &p_mnemonic);
    inline Byte D_Extended(const char *mnemo, Byte bytes, std::string &p_code,
            std::string &p_mnemonic);
    inline Byte D_Relative(const char *mnemo, Byte bytes, DWord &p_jumpaddr,
            std::string &p_code, std::string &p_mnemonic);
    inline Byte D_RelativeL(const char *mnemo, Byte bytes, DWord &p_jumpaddr,
            std::string &p_code, std::string &p_mnemonic);
    inline Byte D_Register0(const char *mnemo, Byte bytes, std::string &p_code,
            std::string &p_mnemonic);
    inline Byte D_Register1(const char *mnemo, const char *ns_reg, Byte bytes,
            std::string &p_code, std::string &p_mnemonic);

    std::string PrintCode(int bytes);
    static const char *IndexedRegister(Byte which);
    static const char *InterRegister(Byte which);
    static const char *StackRegister(Byte which, const char *not_stack);
    static const char *FlexLabel(Word addr);
}; // class Da6809

#endif // DA6809_INCLUDED
