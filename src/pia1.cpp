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
#include "mc6809.h"
#include "schedule.h"
#include "keyboard.h"
#include "cacttrns.h"


Pia1::Pia1(Mc6809 &x_cpu, Scheduler &x_scheduler, KeyboardIO &x_keyboardIO,
           bool x_a_set_msb) :
    cpu(x_cpu), scheduler(x_scheduler), keyboardIO(x_keyboardIO),
    a_set_msb(x_a_set_msb)
{
}

void Pia1::resetIo()
{
    Mc6821::resetIo();
    keyboardIO.reset_parallel();
}

void Pia1::requestInputA()
{
    //  if (keyboardIO.has_key_parallel()) {
    //      activeTransition(CA1);
    //  }
}

Byte Pia1::readInputA()
{
    if (keyboardIO.has_key_parallel())
    {
        bool do_notify = false;
        ora = keyboardIO.read_char_parallel(do_notify);
        if (do_notify)
        {
            auto command = BCommandPtr(
                    new CActiveTransition(*this, Mc6821::ControlLine::CA1));

            scheduler.sync_exec(std::move(command));
        }
    }

    // The Eurocom V5 needs the msb to be set to 1.
    // This is needed for monitor V2.4 to use RAM Bank 1 ($4000 - $7FFF)
    // for video display.
    // If it is 0 RAM Bank 2 ($8000 - $BFFF) will be used for video display
    // which also contains the stack and the direct page registers.
    return ora | (a_set_msb ? 0x80 : 0);
}

void Pia1::set_irq_A()
{
    cpu.set_irq();
}


void Pia1::set_irq_B()
{
    cpu.set_irq();
}

