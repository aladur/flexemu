/*
    pia1.h

    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 1997-2025  W. Schwotzer

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
#include "soptions.h"


class KeyboardIO;
class Scheduler;
class BObserver;

class Pia1 : public Mc6821, public BObserved
{

    // Terminal connected to port A of pia1

protected:

    Scheduler &scheduler;
    KeyboardIO &keyboardIO;
    const struct sOptions &options;
    bool request_a_updated{false};

protected:

    void requestInputA() override;
    Byte readInputA() override;
    void set_irq_A() override;
    void set_irq_B() override;

public:
    Pia1() = delete;
    ~Pia1() override = default;
    Pia1(Scheduler &p_scheduler, KeyboardIO &p_keyboardIO,
         const struct sOptions &p_options);
    Pia1(const Pia1 &src) = delete;
    Pia1(Pia1 &&src) = delete;
    Pia1 &operator=(const Pia1 &src) = delete;
    Pia1 &operator=(Pia1 &&src) = delete;

    void resetIo() override;
    const char *getName() override
    {
        return "pia1";
    }
};

#endif // PIA1_INCLUDED

