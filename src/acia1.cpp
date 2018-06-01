/*
    acia1.cc


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2018  W. Schwotzer

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

#include "acia1.h"
#include "inout.h"

Acia1::Acia1(Inout *x_io, Mc6809 *x_cpu)
{
    cpu = x_cpu;
    io  = x_io;
}

Acia1::~Acia1()
{
}

void Acia1::resetIo()
{
    Mc6850::resetIo();
    io->reset_serial();
}

void Acia1::requestInput()
{
    if (io->poll_serial())
    {
        activeTransition();
    }
}

Byte Acia1::readInput()
{
    Byte temp;

    temp = 0;

    if (io->poll_serial())
    {
        temp = io->read_ch_serial();
    }

    return temp;
}

void Acia1::writeOutput(Byte val)
{
    io->write_ch_serial(val);
}


void Acia1::set_irq()
{
    Mc6850::set_irq();
    cpu->set_irq();
}

