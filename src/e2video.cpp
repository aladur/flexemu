/*
    e2video.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2004  W. Schwotzer

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

#include "e2.h"
#include "e2video.h"
#include "memory.h"


E2video::E2video(Inout *x_io, Memory *x_mem)
{
    memory = x_mem;
    io     = x_io;
}

E2video::~E2video()
{
    memory = NULL;
    io     = NULL;
}


void E2video::resetIo()
{
    vico1            = 0;
    vico2            = 0;
    divided_block    = -1;
}

Byte E2video::readIo(Word)
{
    return 0xff;    // there is nothing to be read !
}


// if bit 1 of vico1 is set, no video ram is selected and all one's is
// displayed on the window like on the real Eurocom II

void E2video::writeIo(Word offset, Byte val)
{
    if (offset == 0)
    {
        vico1 = val & 0x03;
        memory->init_blocks_to_update();
    }
    else
    {
        vico2 = val;
        memory->init_blocks_to_update();

        if (YBLOCKS != 1 && vico2 % BLOCKHEIGHT)
        {
            divided_block = (Word)(vico2 / BLOCKHEIGHT);
        }
        else
        {
            divided_block = -1;    // means: no such block
        }
    } // else
}

