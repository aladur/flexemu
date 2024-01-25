/*
    acia1.h


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



#ifndef ACIA1_INCLUDED
#define ACIA1_INCLUDED

#include <stdio.h>
#include "mc6850.h"
#include "bobservd.h"

class TerminalIO;
class Inout;

class Acia1 : public Mc6850, public BObserved
{

protected:

    TerminalIO &terminalIO;
    Inout &inout;

public:
    // read data from serial line
    Byte readInput() override;

    // write data to serial line
    void writeOutput(Byte val) override;

    // set an interrupt
    void set_irq() override;

    // request for an input ready to be read
    void requestInput() override;

    void resetIo() override;

    const char *getName() override
    {
        return "acia1";
    };

    // Public constructor and destructor
public:

    Acia1() = delete;
    Acia1(TerminalIO &x_terminalIO, Inout &x_inout);
    virtual             ~Acia1();

};

#endif // ACIA1_INCLUDED
