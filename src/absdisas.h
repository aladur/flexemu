/*
    absdisas.h - Abstract disassembler interface


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2001-2025  W. Schwotzer

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


#ifndef ABSDISAS_INCLUDED
#define ABSDISAS_INCLUDED

#include "typedefs.h"
#include <type_traits>
#include <string>


static constexpr Byte PAGE2{0x10};
static constexpr Byte PAGE3{0x11};

// Instruction flags as scoped enum.
enum class InstFlg : uint8_t
{
    NONE = 0U,
    Jump = (1U << 0U),         // next instruction will not be processed
    Sub = (1U << 1U),          // jump into a subroutine
    ComputedGoto = (1U << 2U), // an instruction containing a computed goto
    Illegal = (1U << 3U),      // illegal instruction
    Noop = (1U << 4U),         // no operation
    JumpAddr = (1U << 5U),     // return a jump target address
    LabelAddr = (1U << 6U),    // return a label address
};

// Polymorphic interface, virtual dtor is required.
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class AbstractDisassembler
{

public:
    AbstractDisassembler() = default;
    virtual ~AbstractDisassembler() = default;

    virtual InstFlg Disassemble(
                  const Byte *p_memory,
                  DWord p_pc,
                  DWord &p_jumpaddr,
                  std::string &p_code,
                  std::string &p_mnemonic,
                  std::string &p_operands) = 0;
    virtual void set_use_undocumented(bool value) = 0;
    virtual unsigned getByteSize(const Byte *p_memory) = 0;
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

