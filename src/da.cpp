/*
    da.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 2003-2018  W. Schwotzer

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


#include "da.h"

Disassembler::Disassembler() : da(nullptr)
{
}

Disassembler::~Disassembler()
{
    delete da;
}

int Disassembler::DisassembleOneLine(
        const Byte * const pMemory,
        DWord pc,
        DWord *pFlags,
        DWord *pJumpAddr,
        char **pCode,
        char **pMnemonic)
{
    if (!da)
    {
        return 0;
    }

    return da->Disassemble(pMemory, pc, pFlags, pJumpAddr, pCode, pMnemonic);
}

