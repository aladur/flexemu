/*
    bmembuf.cpp


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

#include <misc1.h>
#include <string.h>	// needed for NULL
#include "bmembuf.h"


BMemoryBuffer::BMemoryBuffer(DWord aSize /* = 65536 */, DWord aBase /* = 0 */) :
	baseAddress(aBase), size(aSize)
{
	pBuffer = new Byte[size];
}

BMemoryBuffer::~BMemoryBuffer()
{
	delete [] pBuffer;
}

void BMemoryBuffer::FillWith(const Byte pattern /* = 0 */)
{
	for (DWord i = 0; i < size; i++)
		pBuffer[i] = pattern;
}

Byte BMemoryBuffer::operator[] (DWord address)
{
	// if out of range always return 0!
	if (address < baseAddress || address > baseAddress + size - 1)
		return 0;
	return *(pBuffer + address - baseAddress);
}

bool BMemoryBuffer::CopyFrom(const Byte *from, DWord aSize, DWord address)
{
	DWord secureSize;

	if (address >= baseAddress + size)
		return false;
	secureSize = (address + aSize > baseAddress + size) ? size - (address - baseAddress) : aSize;
	memcpy(pBuffer + address - baseAddress, from, secureSize);
	return true;
}

const Byte *BMemoryBuffer::GetBuffer(DWord address)
{
	if (address < baseAddress || address >= baseAddress + size)
		return NULL;
	return pBuffer + address - baseAddress;
}

