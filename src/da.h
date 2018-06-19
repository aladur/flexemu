/*
    da.h


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

#ifndef DA_INCLUDED
#define DA_INCLUDED

#include "misc1.h"
#include "absdisas.h"

class DisassemblerConfig;
class BMemoryBuffer;

class Disassembler
{

protected:

    AbstractDisassembler *da;

public:
    Disassembler();
    virtual ~Disassembler();

    void SetLbLDisassembler(AbstractDisassembler *d);
    int DisassembleOneLine(
            const Byte *pMemory,
            DWord pc,
            DWord *pFlags,
            DWord *pJumpAddr,
            char **pCode,
            char **pMnemonic);
    void DisassembleWithConfig(DisassemblerConfig &aConfig,
                               BMemoryBuffer *pMemBuf);

protected:
    int DisassembleUptoEnd(DisassemblerConfig &aConfig, const Byte *pMemory,
                           DWord pc);
};

inline void Disassembler::SetLbLDisassembler(AbstractDisassembler *d)
{
    da = d;
}

#endif // DA_INCLUDED

