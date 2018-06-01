/*
    pia1.cpp


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
#include "pia1.h"

Pia1::Pia1(Inout *x_io, Mc6809 *x_cpu)
{
    cpu = x_cpu;
    io  = x_io;
}


void Pia1::resetIo()
{
    Mc6821::resetIo();
    io->reset_parallel();
}

void Pia1::requestInputA()
{
    //  if (io->poll()) {
    //      activeTransition(CA1);
    //  }
}

Byte Pia1::readInputA()
{
    if (io->poll())
    {
        ora = io->read_ch();
    }

    return ora;
}

void Pia1::set_irq_A()
{
    cpu->set_irq();
}


void Pia1::set_irq_B()
{
    cpu->set_irq();
}

