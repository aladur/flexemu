/*
    vico1.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2018  W. Schwotzer

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


#ifndef VICO1_INCLUDED
#define VICO1_INCLUDED

#include "misc1.h"
#include "bytereg.h"

class Memory;

// VideoControl1 emulates the Eurocom II VICO1 register,
// a single byte write-only register (SN74LS377, Octal
// D Flip-Flop). It defines which RAM bank is used for
// video display. Only Bit 0 and 1 are used.
//
// 1. Without RAM extension:
//   Bit 0 | Bit 1 | RAM bank used for Video display
//   ------+-------+------------------------------------
//     0   |   0   | $0000 - $3FFF
//     0   |   1   | $4000 - $7FFF
//     1   |   0   | $8000 - $BFFF
//     1   |   1   | none (white screen)
//
// 2. With RAM extension:
//   Bit 0 | Bit 1 | RAM bank used for Video display
//   ------+-------+------------------------------------
//     0   |   0   | RAM Bank 0,
//         |       | Pages 0x0C,0x0D,0x0E,0x04,0x05,0x06
//     0   |   1   | RAM Bank 1,
//         |       | Pages 0x08,0x09,0x0A,0x00,0x01,0x02
//     1   |   0   | none (white screen)
//     1   |   1   | none (white screen)

class VideoControl1 : public ByteRegister
{
private:

    Memory &memory;
    Byte value;

    void requestWriteValue(Byte value) override;

public:

    VideoControl1() = delete;
    VideoControl1(Memory &memory);
    virtual ~VideoControl1();

    const char *getName() override
    {
        return "vico1";
    }

    Byte get_value() const
    {
        return value;
    }
};

#endif

