//
//  absdisas.h
//

#ifndef ABSDISAS_INCLUDED
#define ABSDISAS_INCLUDED

#include "typedefs.h"
#include <type_traits>
#include <string>


// Instruction flags as scoped enum.
enum class InstFlg : uint8_t
{
    NONE = 0,
    Jump = (1 << 0),         // next instruction will not be processed
    Sub = (1 << 1),          // jump into a subroutine
    ComputedGoto = (1 << 2), // an instruction containing a computed goto
    Illegal = (1 << 3),      // illegal instruction
    Noop = (1 << 4),         // no operation
    JumpAddr = (1 << 5),     // return a jump target address
    LabelAddr = (1 << 6),    // return a label address
};

class AbstractDisassembler
{

public:
    virtual ~AbstractDisassembler() = default;
    virtual int Disassemble(
                  const Byte *p_memory,
                  DWord p_pc,
                  InstFlg &p_flags,
                  DWord &p_jumpaddr,
                  std::string &p_code,
                  std::string &p_mnemonic) = 0;
    virtual void set_use_undocumented(bool value) = 0;
}; // class AbstractDisassembler

inline InstFlg operator| (InstFlg lhs, InstFlg rhs)
{
    using TYP = std::underlying_type_t<InstFlg>;

    return static_cast<InstFlg>(static_cast<TYP>(lhs) | static_cast<TYP>(rhs));
}

inline InstFlg operator& (InstFlg lhs, InstFlg rhs)
{
    using TYP = std::underlying_type_t<InstFlg>;

    return static_cast<InstFlg>(static_cast<TYP>(lhs) & static_cast<TYP>(rhs));
}


inline InstFlg operator|= (InstFlg &lhs, InstFlg rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

inline InstFlg operator&= (InstFlg &lhs, InstFlg rhs)
{
    lhs = lhs & rhs;
    return lhs;
}

inline InstFlg operator~ (InstFlg rhs)
{
    using TYP = std::underlying_type_t<InstFlg>;

    return static_cast<InstFlg>(~static_cast<TYP>(rhs));
}

inline bool operator! (InstFlg rhs)
{
    using TYP = std::underlying_type_t<InstFlg>;

    return static_cast<TYP>(rhs) == 0;
}

#endif // ABSDISAS_INCLUDED

