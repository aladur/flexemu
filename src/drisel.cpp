/*
    drisel.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2018-2022  W. Schwotzer

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
#include "drisel.h"
#include "e2floppy.h"


DriveSelect::DriveSelect(E2floppy &x_fdc) : fdc(x_fdc)
{
}

DriveSelect::~DriveSelect()
{
}

Byte DriveSelect::requestReadValue()
{
    Byte status = 0xff;  // Unused bits have logical high
        
    if (!fdc.getSide())
    {   
        status &= ~READ_SIDE_MASK;
    }
            
    if (!fdc.isIrq())
    {   
        status &= ~READ_IRQ_MASK;
    }
               
    if (!fdc.isDrq())
    {   
        status &= ~READ_DRQ_MASK;
    }
                    
    return status;
}

void DriveSelect::requestWriteValue(Byte value)
{
    Byte selected = 4;

    fdc.setSide((value & WRITE_SIDE_MASK) ? 1 : 0);

    switch (value & 0x0f)
    {
        case SELECT_DRIVE0:
            selected = 0;
            break;

        case SELECT_DRIVE1:
            selected = 1;
            break;

        case SELECT_DRIVE2:
            selected = 2;
            break;

        case SELECT_DRIVE3:
            selected = 3;
            break;
    }

    fdc.select_drive(selected);
}

void DriveSelect::resetIo()
{
    requestWriteValue(0);
}

