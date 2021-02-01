/*
    pia1.h

    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2021  W. Schwotzer

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



#ifndef PIA1_INCLUDED
#define PIA1_INCLUDED

#include "misc1.h"
#include "mc6821.h"
#include "bobservd.h"


class KeyboardIO;
class Scheduler;
class BObserver;

class Pia1 : public Mc6821, public BObserved
{

    // Terminal connected to port A of pia1

protected:

    Scheduler &scheduler;
    KeyboardIO &keyboardIO;
    bool a_set_msb;
    bool request_a_updated;

protected:

    void requestInputA() override;
    Byte readInputA() override;
    void set_irq_A() override;
    void set_irq_B() override;

public:
    Pia1(Scheduler &x_scheduler, KeyboardIO &x_keyboardIO,
         bool x_a_set_msb = false);
    void resetIo() override;
    const char *getName() override
    {
        return "pia1";
    }
};

#endif // PIA1_INCLUDED

