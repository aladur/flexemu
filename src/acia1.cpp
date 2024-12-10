/*
    acia1.cc


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


#include "typedefs.h"
#include "acia1.h"
#include "terminal.h"
#include "inout.h"
#include "mc6809.h"

Acia1::Acia1(TerminalIO &p_terminalIO, Inout &p_inout) :
             terminalIO(p_terminalIO)
             , inout(p_inout)
{
}

void Acia1::resetIo()
{
    Mc6850::resetIo();
    terminalIO.reset_serial();
}

void Acia1::requestInput()
{
    if (terminalIO.has_char_serial())
    {
        activeTransition();
    }
}

Byte Acia1::readInput()
{
    Byte temp;

    temp = 0;

    if (terminalIO.has_char_serial())
    {
        temp = terminalIO.read_char_serial();
    }

    return temp;
}

void Acia1::writeOutput(Byte val)
{
    if (inout.read_serpar() == 0x00)
    {
        // Redirect serial output to gui.
        inout.write_char_serial(val);
    }
    else
    {
        terminalIO.write_char_serial(val);
    }
}


void Acia1::set_irq()
{
    Mc6850::set_irq();
    Notify(NotifyId::SetIrq);
}

