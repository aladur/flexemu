/*
    mc6850.cpp


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



#include "misc1.h"
#include "iodevice.h"
#include "mc6850.h"


void Mc6850::resetIo()
{
    cr = 0; // control register
    sr = 0; // status register
    tdr = 0; // transmit data register
    rdr = 0; // receive data register
}

Byte Mc6850::readIo(Word offset)
{
    switch (offset & 0x01U)
    {
        case 0:
            sr &= 0x80U; // only receive data register full
            BSET<Byte>(sr, 1U);
            requestInput(); // and interrupt request is set
            // the other status bits are always 0
            return sr; // return status register

        case 1:
            rdr = readInput(); // read character
            BCLR<Byte>(cr, 7U); // reset interrupt flag
            return rdr; // return receive data register
    }

    return 0; // should never happen
}

// read data from serial line (should be overwritten by subclass)

Byte Mc6850::readInput()
{
    return rdr;
}

// check if character is ready to read from serial line
// (should be overwritten by subclass)

void Mc6850::requestInput()
{
}

// that a character from serial line is ready to read


void Mc6850::writeIo(Word offset, Byte val)
{
    switch (offset & 0x01U)
    {
        case 0:
            cr = val;
            break;

        case 1:
            tdr = val;
            BCLR<Byte>(cr, 7U); // reset interrupt flag
            writeOutput(tdr); // write output to serial line

            if ((cr & 0x60U) == 0x20U)// if enabled
            {
                set_irq(); // set transmitting interrupt
            }

            break;
    }
}

// write output to port-Pins (should be overwritten by subclass)

void Mc6850::writeOutput(Byte /*value*/)
{
}

// set an receive/transmit interrupt (should be overwritten by subclass)

void Mc6850::set_irq()
{
    BSET<Byte>(sr, 7U);
}

// actions when a character is ready to be received

void Mc6850::activeTransition()
{
    BSET<Byte>(sr, 0U);

    if (BTST<Byte>(cr, 7U))
    {
        set_irq();
    }
}

