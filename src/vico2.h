/*
    vico2.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2018-2020  W. Schwotzer

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


#ifndef VICO2_INCLUDED
#define VICO2_INCLUDED

#include "misc1.h"
#include "bytereg.h"
#include "bobservd.h"

// VideoControl2 emulates the Eurocom II VICO2 register,
// a single byte write-only register (SN74LS377, Octal
// D Flip-Flop). It defines which video raster line is displayed
// first on the video display. All 8 bits are used for 256 
// raster lines.

class VideoControl2 : public ByteRegister, public BObserved
{
private:

    Byte value;
    bool isFirstWrite;

    void requestWriteValue(Byte value) override;

public:

    VideoControl2();
    virtual ~VideoControl2();

    const char *getName() override
    {
        return "vico2";
    }

    Byte get_value() const
    {
        return value;
    }
};

#endif

