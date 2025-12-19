//
//  da6809.h
//

#ifndef DA6809_INCLUDED
#define DA6809_INCLUDED

#include "absdisas.h"
#include <map>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;


class Da6809 : public AbstractDisassembler
{

protected:

    Word pc{};
    const Byte *memory{}; // non-owning
    bool use_undocumented{false};

public:
    Da6809() = default;

    InstFlg Disassemble(
            const Byte *p_memory,
            DWord p_pc,
            DWord &p_jumpaddr,
            std::string &p_code,
            std::string &p_mnemonic,
            std::string &p_operands) override;
    void set_use_undocumented(bool value) override;
    unsigned getByteSize(const Byte *p_memory) override;
    void SetFlexLabelFile(const fs::path &path);

private:

    inline InstFlg D_Page2(InstFlg p_flags, DWord &p_jumpaddr,
            std::string &p_code, std::string &p_mnemonic,
            std::string &p_operands);
    inline InstFlg D_Page3(InstFlg p_flags, std::string &p_code,
            std::string &p_mnemonic, std::string &p_operands);
    inline InstFlg D_Illegal(const char *mnemo, Byte bytes, std::string &p_code,
            std::string &p_mnemonic, std::string &p_operands);
    inline void D_Direct(const char *mnemo, Byte bytes, std::string &p_code,
            std::string &p_mnemonic, std::string &p_operands);
    inline void D_Immediate8(const char *mnemo, Byte bytes, std::string &p_code,
            std::string &p_mnemonic, std::string &p_operands);
    inline void D_Immediate16(const char *mnemo, Byte bytes,
            std::string &p_code, std::string &p_mnemonic,
            std::string &p_operands);
    inline void D_Inherent(const char *mnemo, Byte bytes, std::string &p_code,
            std::string &p_mnemonic);
    void D_Indexed(const char *mnemo, Byte bytes, std::string &p_code,
            std::string &p_mnemonic, std::string &p_operands);
    inline void D_Extended(const char *mnemo, Byte bytes, std::string &p_code,
            std::string &p_mnemonic, std::string &p_operands);
    inline void D_Relative8(const char *mnemo, Byte bytes, DWord &p_jumpaddr,
            std::string &p_code, std::string &p_mnemonic,
            std::string &p_operands);
    inline void D_Relative16(const char *mnemo, Byte bytes, DWord &p_jumpaddr,
            std::string &p_code, std::string &p_mnemonic,
            std::string &p_operands);
    inline void D_RegisterRegister(const char *mnemo, Byte bytes,
            std::string &p_code, std::string &p_mnemonic,
            std::string &p_operands);
    inline void D_RegisterList(const char *mnemo, const char *ns_reg,
            Byte bytes, std::string &p_code, std::string &p_mnemonic,
            std::string &p_operands);
    std::string PrintCode(int bytes);
    const char *FlexLabel(Word addr);

    fs::path flexLabelFile;
    std::map<unsigned, std::string> label_for_address;
    static const char *IndexRegister(Byte which);
    static const char *InterRegister(Byte which);
    static const char *StackRegister(Byte which, const char *not_stack);
};

#endif // DA6809_INCLUDED
