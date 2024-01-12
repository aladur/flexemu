/*
    mc6809st.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2023  W. Schwotzer

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


#include "misc1.h"
#include "mc6809st.h"

Mc6809CpuStatus::Mc6809CpuStatus() :
    total_cycles(0),
    a(0), b(0), cc(0), dp(0),
    pc(0), s(0), u(0), x(0), y(0)
{
    memset(instruction, 0, 4);
    memset(mnemonic,    0, 28);
    memset(memory,      0, 48);
}

Mc6809CpuStatus &Mc6809CpuStatus::operator=(const Mc6809CpuStatus &lhs)
{
    if (this == &lhs)
    {
        return *this;
    }

    freq         = lhs.freq,
    total_cycles = lhs.total_cycles;
    a            = lhs.a;
    b            = lhs.b;
    cc           = lhs.cc;
    dp           = lhs.dp;
    pc           = lhs.pc;
    s            = lhs.s;
    u            = lhs.u;
    x            = lhs.x;
    y            = lhs.y;
    state        = lhs.state;
    memcpy(instruction, lhs.instruction, 4);
    memcpy(mnemonic,    lhs.mnemonic,    28);
    memcpy(memory,      lhs.memory,      48);
    return *this;
}

