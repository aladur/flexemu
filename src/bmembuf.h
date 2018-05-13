/*
    bmembuf.h


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

#ifndef __bmembuf_h__
#define __bmembuf_h__

#include "misc1.h"


class BMemoryBuffer
{
public:
    BMemoryBuffer(DWord aSize = 65536, DWord aBase = 0);
    BMemoryBuffer(const BMemoryBuffer &src);
    ~BMemoryBuffer();

    void FillWith(const Byte pattern = 0);
    inline DWord GetSize(void) const
    {
        return size;
    };
    inline DWord GetBaseAddress(void) const
    {
        return baseAddress;
    };
    bool CopyFrom(const Byte *from, DWord aSize, DWord address);
    const Byte *GetBuffer(DWord address);
    Byte operator[](DWord address);

private:
    DWord   baseAddress;
    DWord   size;
    Byte    *pBuffer;
};

#endif // #ifdef __bmembuf_h__
