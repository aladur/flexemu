/*
    da.cpp


    FLEXplorer, An explorer for any FLEX file or disk container
    Copyright (C) 2003-2004  W. Schwotzer

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
#include "disconf.h"
#include "bident.h"
#include "bmembuf.h"

Disassembler::Disassembler() : da(NULL)
{
}

Disassembler::~Disassembler()
{
	delete da;
}

int Disassembler::DisassembleOneLine(const Byte *pMemory, DWord pc, DWord *pFlags, DWord *pAddr, char **pb1, char **pb2)
{
	if (!da)
		return 0;
	return da->Disassemble(pMemory, pc, pFlags, pAddr, pb1, pb2);
}

void Disassembler::DisassembleWithConfig(DisassemblerConfig &aConfig, BMemoryBuffer *pMemBuf)
{
	DWord startAddress, length;
	const Byte *pMemory;

	while (aConfig.GetFirstEntryPoint(&startAddress)) {
		pMemory = pMemBuf->GetBuffer(startAddress);
		if (pMemory) {
			length = DisassembleUptoEnd(aConfig, pMemory, startAddress);
			if (length)
				aConfig.Add(BInterval(startAddress, startAddress+length-1),
				DisassemblerConfig::DISCONF_CODE);
		}
		aConfig.RemoveEntryPoint(startAddress);
		if (aConfig.GetFirstEntryPoint(&startAddress)) {
			do {
				if (aConfig.Contains(startAddress, DisassemblerConfig::DISCONF_CODE))
					aConfig.RemoveEntryPoint(startAddress);
			} while (aConfig.GetNextEntryPoint(&startAddress));
		}
	}
}

int	Disassembler::DisassembleUptoEnd(DisassemblerConfig &aConfig, const Byte *pMemory, DWord pc)
{
	DWord flags;
	DWord length = 0;
	DWord addr;
	BString label;
	BIdentifier identifier;
	char *p1, *p2;

	if (!da || !pMemory)
		return length;
	do {
		length += da->Disassemble(pMemory+length, pc+length, &flags, &addr, &p1, &p2);
		if (flags | DA_LABEL_ADDR) {
			label.printf("L%0.4X", addr);
			identifier.SetTo(addr, (const char *)label);
			aConfig.AddLabel(identifier);
		}
		if (flags | DA_JUMP_ADDR)
			aConfig.AddEntryPoint(addr);
	} while (!((flags | DA_JUMP) || (flags | DA_ILLEGAL)));
	return length;
}
