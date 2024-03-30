/*
    mc6809st.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2024  W. Schwotzer

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

class Mc6809CpuStatus : public CpuStatus
{
public:
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
    Byte instruction[4];
    char mnemonic[28];
    Byte memory[48];
    Mc6809CpuStatus();
    ~Mc6809CpuStatus() override { };
    Mc6809CpuStatus &operator=(const Mc6809CpuStatus &lhs);
};

#endif // MC6809ST_INCLUDED

