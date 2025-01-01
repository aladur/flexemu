/*
    mc6809st.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2025  W. Schwotzer

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



#ifndef MC6809ST_INCLUDED
#define MC6809ST_INCLUDED

#include "cpustate.h"


enum : uint8_t
{
    CPU_STACK_LINES = 6,
    CPU_STACK_BYTES = 8,
    CPU_LINES = 14,
    CPU_LINE_WIDTH = 39,
};

/* The following struct represents the current Mc6809 CPU status */
/* For performance reasons it uses plain C arrays. */
/* NOLINTBEGIN(modernize-avoid-c-arrays) */
struct Mc6809CpuStatus : public CpuStatus
{
    QWord total_cycles{0};
    Byte a{0};
    Byte b{0};
    Byte cc{0};
    Byte dp{0};
    Word pc{0};
    Word s{0};
    Word u{0};
    Word x{0};
    Word y{0};
    Word insn_size{0};
    Byte instruction[8]{};
    char mnemonic[8]{};
    char operands[20]{};
    Byte memory[CPU_STACK_LINES * CPU_STACK_BYTES]{};
    Mc6809CpuStatus() = default;
    ~Mc6809CpuStatus() override = default;
    Mc6809CpuStatus(const Mc6809CpuStatus &src) = default;
    Mc6809CpuStatus &operator=(const Mc6809CpuStatus &lhs);
};
/* NOLINTEND(modernize-avoid-c-arrays) */

#endif

