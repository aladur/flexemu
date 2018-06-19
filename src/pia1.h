/*
    pia1.h

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



#ifndef __pia1__h
#define __pia1__h

#include "misc1.h"
#include "mc6821.h"

class Mc6809;
class KeyboardIO;
class Scheduler;

class Pia1 : public Mc6821
{

    // Terminal connected to port A of pia1

protected:

    Mc6809 &cpu;
    Scheduler &scheduler;
    KeyboardIO &keyboardIO;

protected:

    virtual void            requestInputA();
    virtual Byte            readInputA();
    virtual void            set_irq_A();
    virtual void            set_irq_B();

public:
    Pia1(Mc6809 &x_cpu, Scheduler &x_scheduler, KeyboardIO &x_keyboardIO);
    void resetIo() override;
    const char *getName() override
    {
        return "pia1";
    };
};

#endif // __pia1__h

